#include "frmprintcontract.h"
#include "ui_frmprintcontract.h"

frmPrintContract::frmPrintContract(QWidget *parent, QStringList studentData,
                                   QStringList parentData, QStringList *paymentData,
                                   int paymentDataSize, QStringList* coursesData,
                                   int coursesDataSize, const QString &companyName) :
    QMainWindow(parent),
    ui(new Ui::frmPrintContract),
    RESIZE_LIMIT(2),
    PAYMENT_DATA_SIZE(paymentDataSize),
    COURSES_DATA_SIZE(coursesDataSize)
{
    ui->setupUi(this);
    setWindowFlags(Qt::Widget | Qt::FramelessWindowHint);
    centralWidget()->installEventFilter(this);
    ui->titleBar->installEventFilter(this);
    ui->statusBar->installEventFilter(this);

    centralWidget()->setMouseTracking(true);
    ui->titleBar->setMouseTracking(true);
    ui->statusBar->setMouseTracking(true);

    setWindowTitle(tr("Contract generator | SmartClass"));
    locked = LockMoveType::None;

    /*
     *  End of GUI implementation
     */

    QSettings mySettings("Nintersoft", "SmartClass");
    if (mySettings.childGroups().contains("external tools")){
        mySettings.beginGroup("external tools");

        programPath = mySettings.value("program path", "").toString();
        ui->btOpenExternal->setEnabled(mySettings.value("use external tool", false).toBool() &&
                                       !programPath.isEmpty());
        if (ui->btOpenExternal->isEnabled()){
            externalCommand = mySettings.value("command",
                                                tr("$prog_path $s_name $s_birthday $s_id $s_school $s_experimental_course  $s_experimental_date $s_address $s_parent $p_telephone $p_mobile $p_email $p_id $p_cpg $s_course $p_cost $p_discount $p_installments")).toString();
        }

        mySettings.endGroup();
    }

    ui->titleBar->setMaximizeButtonEnabled(false);

    this->setGeometry(QStyle::alignedRect(Qt::LeftToRight, Qt::AlignCenter,
            this->size(), qApp->desktop()->availableGeometry()));

    this->studentData = studentData;
    this->parentalData = parentData;
    this->paymentData = paymentData;
    this->coursesData = coursesData;

    ui->lblStudentName->setText(studentData.at(0));
    ui->lblParentName->setText(parentData.at(0));
    ui->lblParentID->setText(parentData.at(6));
    ui->lblParentCPG->setText(parentData.at(7));
    QStringList studentsCourse = QString(studentData.at(8)).split("|", QString::SkipEmptyParts);
    for (int i = 0; i < studentsCourse.length(); ++i)
        ui->cbStudentCourse->addItem(studentsCourse.at(i));
    ui->lblCurrentDate->setText(QDate::currentDate().toString("dd/MM/yyyy"));
    ui->edtCompanyName->setText(companyName);

    connect(ui->btOpenExternal, SIGNAL(clicked(bool)), this, SLOT(openExternalTool()));
    connect(ui->btGenerateForm, SIGNAL(clicked(bool)), this, SLOT(generateContractForm()));
}

frmPrintContract::~frmPrintContract()
{
    delete ui;
    if (coursesData) delete[] coursesData;
    if (paymentData) delete[] paymentData;
}

void frmPrintContract::openExternalTool(){
    if (ui->cbStudentCourse->currentIndex() == 0){
        QMessageBox::warning(this, tr("Warning | SmartClass"),
                             tr("You have to select a course in order to continue the operation. Otherwise it will not be possible to continue."),
                             QMessageBox::Ok);
        return;
    }
    QProcess externalTool;
    externalTool.start(parseArguments());
    QMessageBox::information(this, tr("Test | SmartClass"), parseArguments(), QMessageBox::Ok);
}

QString frmPrintContract::parseArguments(){
    QString commandLine = externalCommand;
    int currentLine = 0;
    for (int i = 0; i < PAYMENT_DATA_SIZE; ++i)
        if (paymentData[i].at(0) == ui->cbStudentCourse->currentText()){
            currentLine = i;
            break;
        }

    QString cost = "0.00";
    for (int i = 0; i < COURSES_DATA_SIZE; ++i){
        QString courseSyntesis = coursesData[i].at(0) + " [ " + coursesData[i].at(4) + " ] - " + coursesData[i].at(5)
                        + tr(" * starts on: ") + coursesData[i].at(6);
        if (courseSyntesis == ui->cbStudentCourse->currentText()){
            cost = coursesData[i].at(8);
            break;
        }
    }

    return commandLine.replace(tr("$prog_path"), programPath)
            .replace(tr("$s_name"), "\"" + studentData.at(0) + "\"")
            .replace(tr("$s_birthday"), "\"" + studentData.at(1) + "\"")
            .replace(tr("$s_id"), "\"" + studentData.at(2) + "\"")
            .replace(tr("$s_school"), "\"" + studentData.at(3) + "\"")
            .replace(tr("$s_experimental_course"), "\"" + studentData.at(4) + "\"")
            .replace(tr("$s_experimental_date"), "\"" + studentData.at(5) + "\"")
            .replace(tr("$s_address"), "\"" + studentData.at(9) + "\"")
            // Parental details
            .replace(tr("$s_parent"), "\"" + parentalData.at(0) + "\"")
            .replace(tr("$p_telephone"), "\"" + parentalData.at(2) + "\"")
            .replace(tr("$p_mobile"), "\"" + parentalData.at(4) + "\"")
            .replace(tr("$p_email"), "\"" + parentalData.at(5) + "\"")
            .replace(tr("$p_id"), "\"" + parentalData.at(6) + "\"")
            .replace(tr("$p_cpg"), "\"" + parentalData.at(7) + "\"")
            // Course and payment details
            .replace(tr("$s_course"), "\"" + ui->cbStudentCourse->currentText() + "\"")
            // Payment details
            .replace(tr("$p_cost"), "\"" + cost + "\"")
            .replace(tr("$p_discount"), (PAYMENT_DATA_SIZE > 0) ? ("\"" + paymentData[currentLine].at(1) + "\"") : "\"\"")
            .replace(tr("$p_installments"), (PAYMENT_DATA_SIZE > 0) ? ("\"" + paymentData[currentLine].at(3) + "\"") : "\"\"");
}

void frmPrintContract::generateContractForm(){
    QString errorMsg = tr("Well, unfortunately we cannot proceed. Some data are either inconsistent or inexistent."
                          " Please, fix the folowing issues before trying to generate the form again:\n");
    bool error = false;

    if (ui->edtCompanyName->text().isEmpty()){
        errorMsg += tr("\n->The name of the companny cannot be empty;");
        error = true;
    }
    if (ui->cbStudentCourse->currentIndex() == 0){
        errorMsg += tr("\n->You must select a course in order to generate the form;");
        error = true;
    }

    if (error){
        QMessageBox::warning(this, tr("Warning | SmartClass"), errorMsg, QMessageBox::Ok, QMessageBox::Ok);
        return;
    }

    QString defaultTemplate = tr("I, %1, owner of the ID %2 and CPG %3,"
                                 " hereby confirm that I am enrolling my son/daugther %4 on the following course:"
                                 "\n"
                                 "\n - %5."
                                 "\n"
                                 "\nI am aware that this course is going to be ministred at %6.");

    QSettings settings("Nintersoft", "SmartClass");
    if (settings.childGroups().contains("contract")){
        settings.beginGroup("contract");
        defaultTemplate = settings.value("text", defaultTemplate).toString();
        settings.endGroup();
    }

    defaultTemplate = defaultTemplate.arg(ui->lblParentName->text())
                                     .arg(ui->lblParentID->text())
                                     .arg(ui->lblParentCPG->text())
                                     .arg(ui->lblStudentName->text())
                                     .arg(ui->cbStudentCourse->currentText())
                                     .arg(ui->edtCompanyName->text());

    if (ui->cbIncludeCompanyLogo->isChecked()){
        QPixmap cLogo;
        QString logoPath  = QDir::homePath() + ((QSysInfo::windowsVersion() != QSysInfo::WV_None) ?
                    "/AppData/Roaming/Nintersoft/SmartClass/images/Logo.png" :
                    "/.Nintersoft/SmartClass/images/Logo.png");
        cLogo.load(logoPath);
        frmPrintPrev = new PrintPreviewForm(NULL, QStringList() << defaultTemplate
                                                                << ui->lblParentName->text()
                                                                << ui->edtCompanyName->text(), cLogo);
    }
    else frmPrintPrev = new PrintPreviewForm(NULL, QStringList() << defaultTemplate
                                                    << ui->lblParentName->text()
                                                    << ui->edtCompanyName->text());
    frmPrintPrev->showMaximized();
}

/*
 * GUI Functions (don't change, unless necessary)
 */

void frmPrintContract::mousePressEvent(QMouseEvent *event)
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

void frmPrintContract::undefMouseMoveEvent(QObject* object, QMouseEvent* event){
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

void frmPrintContract::mouseReleaseEvent(QMouseEvent *event){
    locked = LockMoveType::None;
    event->accept();
}

bool frmPrintContract::eventFilter(QObject* object, QEvent* event)
{
    if(event->type() == QEvent::MouseMove)
        undefMouseMoveEvent(object, static_cast<QMouseEvent*>(event));
    else if (event->type() == QEvent::MouseButtonPress && object->objectName() == "titleBar"){
        mousePressEvent(static_cast<QMouseEvent*>(event));
    }
    return false;
}
