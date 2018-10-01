#include "frmaddclass.h"
#include "ui_frmaddclass.h"

frmAddClass::frmAddClass(QWidget *parent, Role role, qint64 courseID, const DBManager::DBData &dbData) :
    NMainWindow(parent), ui(new Ui::frmAddClass),
    COURSE_ID(courseID),
    CURRENT_ROLE(role)
{
    ui->setupUi(this);

    // Sets the custom Widgets on the parent Class
    // Otherwise, the window resizing feature will not work
    NMainWindow::setCustomWidgets(ui->centralWidget, ui->statusBar);
    this->setMaximizeButtonEnabled(false);

    /*
    * End of GUI operations
    */
    myDB = new DBManager(dbData, SmartClassGlobal::tablePrefix(),
                            SmartClassGlobal::databaseType(), DBManager::getUniqueConnectionName("importExport"));

    switch (CURRENT_ROLE) {
          case frmAddClass::View:
              ui->screenManagerLayout->setEnabled(false);
              ui->btReset->setEnabled(false);
              ui->btSave->setEnabled(false);
          case frmAddClass::Edit:
              courseData = myDB->retrieveRow(SmartClassGlobal::getTableName(SmartClassGlobal::COURSEDETAILS),
                                             SmartClassGlobal::getTableStructure(SmartClassGlobal::COURSEDETAILS).at(0),
                                             QVariant(COURSE_ID));
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
    QString errorMsg = tr("Well, unfortunately we cannot proceed. Some data is either inconsistent or inexistent."
                          " Please, fix the folowing issues before trying to save again:\n");
    bool error = false, warning = false;

    if (ui->edtCourseName->text().isEmpty()){
        errorMsg += tr("\n->The name of the course cannot be empty;");
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
    if (ui->edtBeginningDate->date() == ui->edtEndDate->date()){
        errorMsg += tr("\n->The initial and end date of the course cannot be the same;");
        error = true;
    }
    if (ui->edtBeginningDate->date() > ui->edtEndDate->date()){
        errorMsg += tr("\n->The initial date cannot be major than the end date;");
        error = true;
    }

    if (error){
        QMessageBox::critical(this, tr("Warning | SmartClass"), errorMsg, QMessageBox::Ok, QMessageBox::Ok);
        return;
    }

    if (ui->edtBeginningDate->date().toJulianDay() < QDate::currentDate().toJulianDay()){
        errorMsg += tr("\n->WARNING: The initial date should not be minor than the current date. Use this if only you want to have references to old courses;");
        warning = true;
    }
    if (ui->edtPrice->value() == 0.00){
        errorMsg += tr("\n->WARNING: The price is set to $0.00;");
        warning = true;
    }

    if (warning) QMessageBox::warning(this, tr("Warning | SmartClass"), errorMsg, QMessageBox::Ok, QMessageBox::NoButton);

    QStringList dayNTime;
    for (int i = 0; i < ui->listDaysAndTime->count(); ++i) dayNTime << ui->listDaysAndTime->item(i)->text();
    QString dayNTimeS = dayNTime.join(" , ");

    QList<QVariant> newCData;
    newCData << ui->edtCourseName->text()
             << ui->edtTeacher->text()
             << ui->edtShortDescription->text()
             << ui->edtDetailedDescription->toPlainText()
             << ui->edtClassNumber->value()
             << dayNTimeS
             << ui->edtBeginningDate->date()
             << ui->edtEndDate->date()
             << ui->edtPrice->value();

    QStringList coursesTable = SmartClassGlobal::getTableStructure(SmartClassGlobal::COURSEDETAILS);

    QStringList columsN = (QStringList() << coursesTable.at(1) << coursesTable.at(5) << coursesTable.at(6) << coursesTable.at(7));
    QList<QVariant> dataN = (QList<QVariant>() << newCData.at(0) << newCData.at(4) << newCData.at(5) << newCData.at(6));

    if (CURRENT_ROLE != frmAddClass::Edit)
        if (myDB->rowExists("myclass_courses", columsN, dataN)){
            QMessageBox::information(this, tr("Warning | SmartClass"), tr("There is another course with the same name, class, days, time and beginning date registered. Please, check it before continuing."), QMessageBox::Ok, QMessageBox::Ok);
            return;
        }

    if (CURRENT_ROLE != frmAddClass::Create){
        if (!myDB->updateRow(SmartClassGlobal::getTableName(SmartClassGlobal::COURSEDETAILS),
                             SmartClassGlobal::getTableStructure(SmartClassGlobal::COURSEDETAILS).at(0),
                             COURSE_ID,
                             SmartClassGlobal::getTableStructure(SmartClassGlobal::COURSEDETAILS).mid(1),
                             newCData))
            QMessageBox::critical(this, tr("Critical | SmartClass"), tr("Unfortunately an error has occured while we tried to update the course details for you."
                                                                        "\nHere are the technical details of what happened: %1."
                                                                        "\nWe suggest you to try again after the problem has been resolved (You will not lose any data until you close the form)!").arg(myDB->lastError().text()),
                                    QMessageBox::Ok, QMessageBox::NoButton);

        emit updatedData(QList<QVariant>() << newCData.at(0) << newCData.at(4) << newCData.at(1)
                                            << newCData.at(6) << newCData.at(5),
                            COURSE_ID);
    }
    else {
        if (!myDB->insertRow("myclass_courses", coursesTable.mid(1), newCData)){
            QMessageBox::critical(this, tr("Critical | SmartClass"), tr("Unfortunately an error has occured while we tried to register the course details for you."
                                                                        "\nHere are the technical details of what happened: %1."
                                                                        "\nWe suggest you to try again after the problem has been resolved (You will not lose any data until you close the form)!").arg(myDB->lastError().text()),
                                    QMessageBox::Ok, QMessageBox::NoButton);
            return;
        }

        qlonglong newIndex = -1;
        QSqlQuery getIndex = myDB->runCustomQuery();
        getIndex.prepare("SELECT MAX(id) FROM " + SmartClassGlobal::getTableName(SmartClassGlobal::COURSEDETAILS));
        if (getIndex.exec())
            if (getIndex.next()) newIndex = getIndex.value(0).toLongLong();

        if (newIndex == -1) QMessageBox::warning(this, tr("Critical | SmartClass"), tr("A new course has been created successfuly. However, we could not retrieve its new ID."
                                                                                       "\nAlthough the application continues working, we strongly recommend you to restart it, since the new index may not correspond to the recently created course."
                                                                                       "\nHere are the technical details of what happened: %1.").arg(myDB->lastError().text()),
                                     QMessageBox::Ok, QMessageBox::NoButton);

        emit newData(QList<QVariant>() << newCData.at(0) << newCData.at(4) << newCData.at(1)
                                        << newCData.at(6) << newCData.at(5) << newIndex);
    }
    this->close();
}

void frmAddClass::applyCourseData(){
    ui->listDaysAndTime->clear();

    ui->edtCourseName->setText(courseData.at(1).toString());
    ui->edtTeacher->setText(courseData.at(2).toString());
    ui->edtShortDescription->setText(courseData.at(3).toString());
    ui->edtDetailedDescription->setPlainText(courseData.at(4).toString());
    ui->edtClassNumber->setValue(courseData.at(5).toInt());
    QStringList daysNTime = courseData.at(6).toString().split(" , ");
    for (int i = 0; i < daysNTime.count(); ++i) ui->listDaysAndTime->addItem(daysNTime.at(i));
    ui->edtBeginningDate->setDate(courseData.at(7).toDate());
    ui->edtEndDate->setDate(courseData.at(8).toDate());
    ui->edtPrice->setValue(courseData.at(9).toDouble());
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
