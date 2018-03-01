#include "frmaddclass.h"
#include "ui_frmaddclass.h"

frmAddClass::frmAddClass(QWidget *parent, Role role, QString name, const QStringList &dbData) :
    QMainWindow(parent), ui(new Ui::frmAddClass),
    RESIZE_LIMIT(2), COURSE_NAME(name),
    CURRENT_ROLE(role)
{
  ui->setupUi(this);

  this->setWindowFlags(Qt::Widget | Qt::FramelessWindowHint);
  this->centralWidget()->installEventFilter(this);
  ui->titleBar->installEventFilter(this);
  ui->statusBar->installEventFilter(this);

  this->centralWidget()->setMouseTracking(true);
  ui->titleBar->setMouseTracking(true);
  ui->statusBar->setMouseTracking(true);

  this->setWindowTitle(tr("Manage Class | SmartClass"));
  locked = LockMoveType::None;

  ui->titleBar->setMaximizeButtonEnabled(false);

  this->setGeometry(QStyle::alignedRect(Qt::LeftToRight, Qt::AlignCenter,
          this->size(), qApp->desktop()->availableGeometry()));

  /*
   * End of GUI operations
   */
  myDB = new DBManager(dbData, DBManager::getUniqueConnectionName("importExport"),
                       dbData.length() == 1 ? "SQLITE" : "MYSQL");

  studentsData = NULL;
  paymentData = NULL;

  coursesTable << "course" << "teacher" << "shortDescription"
               << "longDescription" << "class" << "dayNTime"
               << "beginningDate" << "endDate" << "price" << "students";

  switch (CURRENT_ROLE) {
      case frmAddClass::View:
          ui->screenManagerLayout->setEnabled(false);
          ui->btReset->setEnabled(false);
          ui->btSave->setEnabled(false);
      case frmAddClass::Edit:
          courseData = myDB->retrieveLine("myclass_courses",
                                         QStringList() << coursesTable.at(0) << coursesTable.at(4) << coursesTable.at(5) << coursesTable.at(6),
                                         COURSE_NAME.split("|"));
          studentsData = myDB->retrieveAll("myclass_students");
          paymentData = myDB->retrieveAll("myclass_pricing");
          applyCourseData();
          break;
      default:
          ui->edtBeginningDate->setMinimumDate(QDate::currentDate());
          ui->edtEndDate->setMinimumDate(QDate::currentDate());
          break;
  }

  connect(ui->btCancel, SIGNAL(clicked(bool)), this, SLOT(close()));
  connect(ui->btReset, SIGNAL(clicked(bool)), this, SLOT(resetNewClass()));
  connect(ui->btSave, SIGNAL(clicked(bool)), this, SLOT(saveNewClass()));

  connect(ui->btAddDay, SIGNAL(clicked(bool)), this, SLOT(addDayNTime()));
  connect(ui->btRemoveDay, SIGNAL(clicked(bool)), this, SLOT(removeDayNTime()));
}

frmAddClass::~frmAddClass()
{
    if (studentsData){
        delete[] studentsData;
        studentsData = NULL;
    }
    if (paymentData){
        delete[] paymentData;
        paymentData = NULL;
    }
    if (myDB->isOpen()) myDB->closeDB();
    delete myDB;
    delete ui;
}

void frmAddClass::resetNewClass(){
    QMessageBox confirmation;
    confirmation.setWindowTitle(tr("Reset confirmation | SmartClass"));
    confirmation.setText(tr("You are going to erase all the data that you have already inserted/modified. This action cannot be undone. Do you steel want to proceed?"));
    confirmation.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
    if (confirmation.exec() == QMessageBox::Yes){
        if (CURRENT_ROLE == Create){
            ui->edtCourseName->setText("");
            ui->edtShortDescription->setText("");
            ui->edtDetailedDescription->setPlainText("");
            ui->edtClassNumber->setValue(0);
            ui->cbDay->setCurrentIndex(0);
            ui->edtBegin->setTime(QTime(00, 00, 00));
            ui->edtEnd->setTime(QTime(00, 00, 00));
            ui->btRemoveDay->setEnabled(false);
            ui->listDaysAndTime->clear();
            ui->edtBeginningDate->setDate(QDate::currentDate());
            ui->edtEndDate->setDate(QDate::currentDate());
            ui->edtPrice->setValue(0.00);
        }
        else applyCourseData();
    }
}

void frmAddClass::saveNewClass(){
    QString errorMsg = tr("Well, unfortunately we cannot proceed. Some data are either inconsistent or inexistent."
                          " Please, fix the folowing issues before trying to save again:\n");
    bool error = false;

    int bd = ui->edtBeginningDate->date().day(),
        bm = ui->edtBeginningDate->date().month(),
        by = ui->edtBeginningDate->date().year(),
        ed = ui->edtEndDate->date().day(),
        em = ui->edtEndDate->date().month(),
        ey = ui->edtEndDate->date().year(),
        cd = QDate::currentDate().day(),
        cm = QDate::currentDate().month(),
        cy = QDate::currentDate().year();

    if (ui->edtCourseName->text().isEmpty()){
        errorMsg += tr("\n->The name of the course cannot be empty;");
        error = true;
    }
    if (ui->edtCourseName->text().contains("|")){
        errorMsg += tr("\n->Unfortunately you cannot use the character \"|\" in the \"Course\" field;");
        error = true;
    }
    if (ui->edtTeacher->text().isEmpty()){
        errorMsg += tr("\n->The name of theacher/instructor cannot be empty;");
        error = true;
    }
    if (ui->edtShortDescription->text().isEmpty()){
        errorMsg += tr("\n->The short description of the course cannot be empty;");
        error = true;
    }
    if (ui->edtDetailedDescription->toPlainText().isEmpty()){
        errorMsg += tr("\n->The detailed description of the course cannot be empty;");
        error = true;
    }
    if (ui->listDaysAndTime->count() == 0){
        errorMsg += tr("\n->The days and time of the class cannot be empty;");
        error = true;
    }
    if (ui->edtBeginningDate->date().toString("dd/MM/yyyy") == ui->edtEndDate->date().toString("dd/MM/yyyy")){
        errorMsg += tr("\n->The initial and end date of the course cannot be the same;");
        error = true;
    }
    if (by > ey || (by == ey && bm > em) || (by == ey && bm == em && bd > ed)){
        errorMsg += tr("\n->The initial date cannot be major than the end date;");
        error = true;
    }
    if (by < cy || (by == cy && bm < cm) || (by == cy && bm < cm) || (by == cy && bm == cm && bd < cd))
        errorMsg += tr("\n->WARNING: The initial date should not be minor than the current date. Use this if only you want to have references to old courses;");
    if (ui->edtPrice->value() == 0.00){
        errorMsg += tr("\n->WARNING: The price is set to $0.00;");
    }

    if (error){
        QMessageBox::information(this, tr("Warning | SmartClass"), errorMsg, QMessageBox::Ok, QMessageBox::Ok);
        return;
    }

    QStringList dayNTime;
    for (int i = 0; i < ui->listDaysAndTime->count(); ++i) dayNTime << ui->listDaysAndTime->item(i)->text();
    QString dayNTimeS = dayNTime.join(" , ");

    QStringList newCData;
    newCData << ui->edtCourseName->text()
            << ui->edtTeacher->text()
            << ui->edtShortDescription->text()
            << ui->edtDetailedDescription->toPlainText()
            << QString::number(ui->edtClassNumber->value())
            << dayNTimeS
            << ui->edtBeginningDate->date().toString("dd/MM/yyyy")
            << ui->edtEndDate->date().toString("dd/MM/yyyy")
            << QString::number(ui->edtPrice->value());

    QStringList columsN = (QStringList() << coursesTable.at(0) << coursesTable.at(3) << coursesTable.at(4) << coursesTable.at(5));
    QStringList dataN = (QStringList() << newCData.at(0) << newCData.at(3) << newCData.at(4) << newCData.at(5));
    if (CURRENT_ROLE != frmAddClass::Edit)
        if (myDB->lineExists("myclass_courses", columsN, dataN)){
            QMessageBox::information(this, tr("Warning | SmartClass"), tr("There is another course with the same name, class, days, time and beginning date registered. Please, check this before continue."), QMessageBox::Ok, QMessageBox::Ok);
            return;
        }

    if (CURRENT_ROLE != frmAddClass::Create){
        myDB->removeLine("myclass_courses", coursesTable, courseData);
        newCData << courseData.at(9);

        QString courseSytesis = courseData.at(0) + " [ " + courseData.at(4) + " ] - " + courseData.at(5)
                + tr(" * starts on: ") + courseData.at(6);
        QString newCourseSytesis = newCData.at(0) + " [ " + newCData.at(4) + " ] - " + newCData.at(5)
                + tr(" * starts on: ") + newCData.at(6);

        if (paymentData){
            int pSize = myDB->rowsCount("myclass_pricing");
            for (int i = 0; i < pSize; ++i){
                if (paymentData[i].at(2) == courseSytesis)
                    myDB->updateLine("myclass_pricing", QStringList() << "course",
                                    QStringList() << newCourseSytesis, "course", courseSytesis);
            }
        }

        if (studentsData){
            int sSize = myDB->rowsCount("myclass_students");
            for (int i = 0; i < sSize; ++i){
                if (QString(studentsData[i].at(9)).contains(courseSytesis))
                    myDB->updateLine("myclass_students", QStringList() << "courses",
                                    QStringList() << QString(studentsData[i].at(9)).replace(courseSytesis, newCourseSytesis),
                                    "courses", studentsData[i].at(9));
            }
        }

        emit updatedData(QStringList() << newCData.at(0) << newCData.at(4) << newCData.at(1) << newCData.at(6) << newCData.at(5),
                                                                                       courseData.at(0));
    }
    else emit newData(QStringList() << newCData.at(0) << newCData.at(4) << newCData.at(1) << newCData.at(6) << newCData.at(5));
    coursesTable.removeOne("students");
    myDB->addLine("myclass_courses", coursesTable, newCData);
    this->close();
}

void frmAddClass::applyCourseData(){
    /*
     * course|class|dayNTime|beginningDate
     */

    ui->listDaysAndTime->clear();

    ui->edtCourseName->setText(courseData.at(0));
    ui->edtTeacher->setText(courseData.at(1));
    ui->edtShortDescription->setText(courseData.at(2));
    ui->edtDetailedDescription->setPlainText(courseData.at(3));
    ui->edtClassNumber->setValue(QString(courseData.at(4)).toInt());
    QStringList daysNTime = QString(courseData.at(5)).split(" , ");
    for (int i = 0; i < daysNTime.count(); ++i) ui->listDaysAndTime->addItem(daysNTime.at(i));
    ui->edtBeginningDate->setDate(QDate::fromString(courseData.at(6), "dd/MM/yyyy"));
    ui->edtEndDate->setDate(QDate::fromString(courseData.at(7), "dd/MM/yyyy"));
    ui->edtPrice->setValue(QString(courseData.at(8)).toDouble());
}

void frmAddClass::addDayNTime(){
    bool error = false;
    QString errorMsg = tr("Well, unfortunately we cannot proceed. Some data are either inconsistent or inexistent."
                          " Please, fix the folowing issues before trying to add the day and the time:\n");

    if (ui->edtBegin->time().toString("HH:mm:ss") == ui->edtEnd->time().toString("HH:mm:ss")){
        errorMsg += tr("\n->The beginning and end time cannot be the same;");
        error = true;
    }
    if (ui->edtBegin->time().msec() > ui->edtEnd->time().msec()){
        errorMsg += tr("\n->You cannot finish an activity before starting it :) [The end time is earlier than the beginning one];");
        error = true;
    }

    QString newDate = ui->cbDay->currentText() + " ( " + ui->edtBegin->time().toString("HH:mm:ss")
            + " ~ " + ui->edtEnd->time().toString("HH:mm:ss") + " )";
    for (int i = 0; i < ui->listDaysAndTime->count(); ++i)
            if (ui->listDaysAndTime->item(i)->text() == newDate){
                errorMsg += tr("\n->The date and time that you want to add to the list already exists;");
                error = true;
            }

    if (error){
        QMessageBox::information(this, tr("Warning | SmartClass"), errorMsg, QMessageBox::Ok, QMessageBox::Ok);
        return;
    }

    ui->listDaysAndTime->addItem(newDate);
    if (!ui->btRemoveDay->isEnabled()) ui->btRemoveDay->setEnabled(true);
}

void frmAddClass::removeDayNTime(){
    if (ui->listDaysAndTime->currentRow() < 0) return;
    delete ui->listDaysAndTime->item(ui->listDaysAndTime->currentRow());
    if (ui->listDaysAndTime->count() < 1) ui->btRemoveDay->setEnabled(false);
}

/*
 * GUI Functions (don't change, unless necessary)
 */

void frmAddClass::mousePressEvent(QMouseEvent *event)
{
    if(event->button() == Qt::LeftButton)
    {
        int x = event->x(), y = event->y(), bottom = this->height() - RESIZE_LIMIT, right = this->width() - RESIZE_LIMIT;
        if (x < RESIZE_LIMIT && y < RESIZE_LIMIT){
            posCursor = event->globalPos() - this->geometry().topLeft();
            locked = LockMoveType::TopLeft;
        }
        else if (x < RESIZE_LIMIT && y > bottom){
            posCursor = event->globalPos() - this->geometry().bottomLeft();
            locked = LockMoveType::BottomLeft;
        }
        else if (x > right && y < RESIZE_LIMIT){
            posCursor = event->globalPos() - this->geometry().topRight();
            locked = LockMoveType::TopRight;
        }
        else if (x > right && y > bottom){
            posCursor = event->globalPos() - this->geometry().bottomRight();
            locked = LockMoveType::BottomRight;
        }
        else if (x < RESIZE_LIMIT || y < RESIZE_LIMIT){
            posCursor = event->globalPos() - this->geometry().topLeft();
            locked = x < RESIZE_LIMIT ? LockMoveType::Left : LockMoveType::Top;
        }
        else if (x > right || y > bottom){
            posCursor = event->globalPos() - this->geometry().bottomRight();
            locked = x > right ? LockMoveType::Right : LockMoveType::Bottom;
        }
        event->accept();
    }
}

void frmAddClass::undefMouseMoveEvent(QObject* object, QMouseEvent* event){
    if (locked != LockMoveType::None){
        switch (locked) {
        case LockMoveType::TopLeft:
            this->setGeometry(QRect(QPoint(event->globalPos().x() - posCursor.x(), event->globalPos().y() - posCursor.y()),
                                    this->geometry().bottomRight()));
            break;
        case LockMoveType::TopRight:
            this->setGeometry(QRect(QPoint(this->geometry().left(), event->globalPos().y() - posCursor.y()),
                                    QPoint(event->globalPos().x() - posCursor.x(), this->geometry().bottom())));
            break;
        case LockMoveType::BottomLeft:
            this->setGeometry(QRect(QPoint(event->globalPos().x() - posCursor.x(), this->geometry().top()),
                                    QPoint(this->geometry().right(), event->globalPos().y() - posCursor.y())));
            break;
        case LockMoveType::BottomRight:
            this->setGeometry(QRect(this->geometry().topLeft(),
                                    QPoint(event->globalPos().x() - posCursor.x(), event->globalPos().y() - posCursor.y())));
            break;
        case LockMoveType::Left:
            this->setGeometry(QRect(QPoint(event->globalPos().x() - posCursor.x(), this->geometry().top()),
                                    this->geometry().bottomRight()));
            break;
        case LockMoveType::Right:
            this->setGeometry(QRect(this->geometry().topLeft(),
                                    QPoint(event->globalPos().x() - posCursor.x(), this->geometry().bottom())));
            break;
        case LockMoveType::Top:
            this->setGeometry(QRect(QPoint(this->geometry().left(), event->globalPos().y() - posCursor.y()),
                                    this->geometry().bottomRight()));
            break;
        default:
            this->setGeometry(QRect(this->geometry().topLeft(),
                                    QPoint(this->geometry().right(), event->globalPos().y() - posCursor.y())));
            break;
        }
        return;
    }

    int x = event->x(), y = event->y(), right = this->width() - RESIZE_LIMIT;
    if (object->objectName() == "statusBar"){
        if (x < RESIZE_LIMIT && y > (19 - RESIZE_LIMIT)){
            this->setCursor(QCursor(Qt::SizeBDiagCursor));
            return;
        }
        else if (x > right && y > (19 - RESIZE_LIMIT)){
            this->setCursor(QCursor(Qt::SizeFDiagCursor));
            return;
        }
        else if (y > (19 - RESIZE_LIMIT)){
            this->setCursor(QCursor(Qt::SizeVerCursor));
            return;
        }
    }
    else if (object->objectName() == "titleBar"){
        if (x < RESIZE_LIMIT && y < RESIZE_LIMIT){
            this->setCursor(QCursor(Qt::SizeFDiagCursor));
            return;
        }
        if (x > right && y < RESIZE_LIMIT){
            this->setCursor(QCursor(Qt::SizeBDiagCursor));
            return;
        }
        else if (y < RESIZE_LIMIT){
            this->setCursor(QCursor(Qt::SizeVerCursor));
            return;
        }
    }
    if (x < RESIZE_LIMIT || x > right){
        this->setCursor(QCursor(Qt::SizeHorCursor));
    }
    else {
        this->setCursor(QCursor(Qt::ArrowCursor));
    }
}

void frmAddClass::mouseReleaseEvent(QMouseEvent *event){
    locked = LockMoveType::None;
    event->accept();
}

bool frmAddClass::eventFilter(QObject* object, QEvent* event)
{
    if(event->type() == QEvent::MouseMove)
        undefMouseMoveEvent(object, static_cast<QMouseEvent*>(event));
    else if (event->type() == QEvent::MouseButtonPress && object->objectName() == "titleBar"){
        mousePressEvent(static_cast<QMouseEvent*>(event));
    }
    return false;
}
