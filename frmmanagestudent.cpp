#include "frmmanagestudent.h"
#include "ui_frmmanagestudent.h"

frmManageStudent::frmManageStudent(QWidget *parent, Role role, const qint64 &studentID,
                                   const DBManager::DBData &dbData) :
    NMainWindow(parent),
    ui(new Ui::frmManageStudent),
    CURRENT_ROLE(role),
    STUDENT_ID(studentID)
{
    ui->setupUi(this);

    // Sets the custom Widgets on the parent Class
    // Otherwise, the window resizing feature will not work
    NMainWindow::setCustomWidgets(ui->centralWidget, ui->statusBar);
    this->setMaximizeButtonEnabled(false);

    ui->lblDescriptionOfCourse->setVisible(false);
    ui->lblPaymentDetails->setVisible(false);

    /*
     * End of GUI operations
     */

    myDB = new DBManager(dbData, SmartClassGlobal::tablePrefix(),
                            SmartClassGlobal::databaseType(), DBManager::getUniqueConnectionName("manageStudent"));

    blockPaymentUpdate = false;

    imageViewerSender = "";
    currentImg = frmManageStudent::NoManagement;
    frmImgViewer = NULL;
    pics = new QPixmap*[5];
    for (int i = 0; i < 5; ++i) pics[i] = new QPixmap();

    connect(ui->btCancel, SIGNAL(clicked(bool)), this, SLOT(close()));
    connect(ui->btReset, SIGNAL(clicked(bool)), this, SLOT(resetData()));
    connect(ui->btSave, SIGNAL(clicked(bool)), this, SLOT(saveData()));

    connect(ui->btStudentPicture, SIGNAL(clicked(bool)), this, SLOT(openImageViewer()));
    connect(ui->btStudentIDPicture, SIGNAL(clicked(bool)), this, SLOT(openImageViewer()));
    connect(ui->btParentIDPicture, SIGNAL(clicked(bool)), this, SLOT(openImageViewer()));
    connect(ui->btParentCPGPicture, SIGNAL(clicked(bool)), this, SLOT(openImageViewer()));
    connect(ui->btStudentAddressConfirmationPicture, SIGNAL(clicked(bool)), this, SLOT(openImageViewer()));

    connect(ui->btAddCourse, SIGNAL(clicked(bool)), this, SLOT(addCourse()));
    connect(ui->btRemoveCourse, SIGNAL(clicked(bool)), this, SLOT(removeCourse()));

    connect(ui->edtPaymentFirstInstallment, SIGNAL(dateChanged(QDate)), this, SLOT(paymentValuesChanged()));
    connect(ui->sbDiscount, SIGNAL(valueChanged(int)), this, SLOT(paymentValuesChanged()));
    connect(ui->sbInstallments, SIGNAL(valueChanged(int)), this, SLOT(paymentValuesChanged()));

    connect(ui->sbDiscount, SIGNAL(valueChanged(double)), this, SLOT(changeDiscountPercentage(double)));
    connect(ui->sbDiscountV, SIGNAL(valueChanged(double)), this, SLOT(changeDiscountValue(double)));

    connect(ui->listPaymentCourses, SIGNAL(currentRowChanged(int)), this, SLOT(changePaymentDetails()));

    connect(ui->listCourses, SIGNAL(currentRowChanged(int)), this, SLOT(enableRegistrationDetails(int)));

    myDB->openDB();

    this->retrieveData();

    if (role == frmManageStudent::Create) ui->edtParentName->setCurrentText("");
    if (CURRENT_ROLE != frmManageStudent::View)
        connect(ui->edtParentName, SIGNAL(currentIndexChanged(QString)), this, SLOT(updateParentInfo(QString)));
}

frmManageStudent::~frmManageStudent()
{
    for (int i = 0; i < 5; ++i) delete pics[i];
    delete[] pics;

    if (myDB->isOpen()) myDB->closeDB();
    delete myDB;
    delete ui;
}

void frmManageStudent::openImageViewer(){
    imageViewerSender = sender()->objectName();
    if (imageViewerSender == "btStudentPicture") currentImg = frmManageStudent::Student;
    else if (imageViewerSender == "btStudentIDPicture") currentImg = frmManageStudent::StudentID;
    else if (imageViewerSender == "btParentIDPicture") currentImg = frmManageStudent::ParentID;
    else if (imageViewerSender == "btParentCPGPicture") currentImg = frmManageStudent::ParentCPG;
    else if (imageViewerSender == "btStudentAddressConfirmationPicture") currentImg = frmManageStudent::StudentAddress;

    if (frmImgViewer){
        delete frmImgViewer;
        frmImgViewer = NULL;
    }

    frmImgViewer = new frmImageViewer(NULL, *pics[currentImg]);
    connect(frmImgViewer, SIGNAL(exec(QPixmap)), this, SLOT(receiveImage(QPixmap)));
    frmImgViewer->show();
}

void frmManageStudent::receiveImage(const QPixmap &image){
    delete pics[currentImg];
    pics[currentImg] = new QPixmap(image);
    if (currentImg == frmManageStudent::Student){
        if (image.isNull()) ui->lblStudentImage->setPixmap(QPixmap(QString::fromUtf8(":/images/buttons/Parent.PNG")));
        else ui->lblStudentImage->setPixmap(image);
    }
    currentImg = frmManageStudent::NoManagement;
}

void frmManageStudent::enableRegistrationDetails(int index){
    bool enable = index >= 0;
    ui->grpPaymentData->setEnabled(enable);
    ui->lblDescriptionOfCourse->setEnabled(enable);
    ui->listCourses->setEnabled(enable);
}

void frmManageStudent::saveData(){
    QString errorMessage = tr("Well, unfortunately we cannot proceed. Some data are either inconsistent or inexistent."
                              " Please, fix the folowing issues before trying to save again:\n");
    bool error = false;

    if (ui->edtStudentName->text().isEmpty()){
        errorMessage += tr("\n->The name of the student cannot be empty;");
        error = true;
    }
    else if (myDB->rowExists(SmartClassGlobal::getTableName(SmartClassGlobal::STUDENT),
                             SmartClassGlobal::getTableStructure(SmartClassGlobal::STUDENT).at(1),
                             ui->edtStudentName->text())
             && CURRENT_ROLE == frmManageStudent::Create){
        errorMessage += tr("\n->A student with the same same name already exists in your database. If you want to update him/her, please, use the update button available in the main screen;");
        error = true;
    }
    if (!ui->edtStudentBirthday->date().isValid()){
        errorMessage += tr("\n->The birthday of the student is invalid;");
        error = true;
    }
    if (ui->edtParentName->currentText().isEmpty()){
        errorMessage += tr("\n->The name of the parent cannot be empty;");
        error = true;
    }
    if  (ui->edtParentPhone->text().isEmpty() && ui->edtParentEmail->text().isEmpty() && ui->edtParentMobile->text().isEmpty()){
        errorMessage += tr("\n->It must have either the parent's email or telephone. Currently, both of them are empty;");
        error = true;
    }

    if (error){
        QMessageBox::information(this, tr("Warning | SmartClass"), errorMessage, QMessageBox::Ok, QMessageBox::Ok);
        return;
    }

    QStringList courses;
    for (int i = 0; i < ui->listCourses->count(); ++i)
        courses << QString::number(ui->listCourses->item(i)->data(Qt::UserRole).toLongLong());

    QList<QVariant> studentInfo, responsibleInfo, courseInfo;
    studentInfo << ui->edtStudentName->text()
                << ui->edtStudentBirthday->date()
                << ui->edtStudentID->text()
                << ui->edtStudentSchool->text()
                << ui->edtStudentObservations->toPlainText()
                << ui->cbExperimentalClassCourse->currentText()
                << ui->edtExperimentalClassDateTime->dateTime()
                << ui->edtExperimentalClassObservations->toPlainText();

    responsibleInfo << ui->edtParentName->currentText()
               << ui->edtParentPhone->text()
               << ui->cbParentMobileOperator->currentIndex()
               << ui->edtParentMobile->text()
               << ui->edtParentEmail->text()
               << ui->edtParentID->text()
               << ui->edtParentCPG->text()
               << ui->cbParentIndication->currentText()
               << ui->edtRegistrationAddress->text();

    if (CURRENT_ROLE == frmManageStudent::Create){
        qlonglong sID, rID;

        if (ui->edtParentName->currentIndex() < 0 ||
                ui->edtParentName->currentText() != ui->edtParentName->itemText(ui->edtParentName->currentIndex())){
            myDB->insertRow(SmartClassGlobal::getTableName(SmartClassGlobal::RESPONSIBLE),
                            SmartClassGlobal::getTableStructure(SmartClassGlobal::RESPONSIBLE).mid(1),
                            responsibleInfo);
            rID = myDB->retrieveRow(SmartClassGlobal::getTableName(SmartClassGlobal::RESPONSIBLE),
                                    SmartClassGlobal::getTableStructure(SmartClassGlobal::RESPONSIBLE).at(1),
                                    responsibleInfo.at(0)).at(0).toLongLong();
        }
        else {
            rID = ui->edtParentName->itemData(ui->edtParentName->currentIndex(), Qt::UserRole).toLongLong();
            myDB->updateRow(SmartClassGlobal::getTableName(SmartClassGlobal::RESPONSIBLE),
                            SmartClassGlobal::getTableStructure(SmartClassGlobal::RESPONSIBLE).at(0),
                            rID,
                            SmartClassGlobal::getTableStructure(SmartClassGlobal::RESPONSIBLE).mid(1),
                            responsibleInfo);
        }

        studentInfo.insert(2, rID);
        myDB->insertRow(SmartClassGlobal::getTableName(SmartClassGlobal::STUDENT),
                        SmartClassGlobal::getTableStructure(SmartClassGlobal::STUDENT).mid(1),
                        studentInfo);

        sID = myDB->retrieveRow(SmartClassGlobal::getTableName(SmartClassGlobal::STUDENT),
                                SmartClassGlobal::getTableStructure(SmartClassGlobal::STUDENT).at(1),
                                studentInfo.at(0)).at(0).toLongLong();

        for (int i = 0; i < ui->listCourses->count(); ++i){
            QList<QVariant> csRow;
            csRow << ui->listCourses->item(i)->data(Qt::UserRole) << sID;
            myDB->insertRow(SmartClassGlobal::getTableName(SmartClassGlobal::COURSEENROLLMENTS),
                            SmartClassGlobal::getTableStructure(SmartClassGlobal::COURSEENROLLMENTS),
                            csRow);
        }

        for (int i = 0; i < ui->listPaymentCourses->count(); ++i){
            QListWidgetPaymentItem* item =  static_cast<QListWidgetPaymentItem*>(ui->listPaymentCourses->item(i));

            QList<QVariant> paymentInfo;
            paymentInfo << sID
                        << item->data(Qt::UserRole)
                        << item->discount()
                        << item->date()
                        << item->installments();

            myDB->insertRow(SmartClassGlobal::getTableName(SmartClassGlobal::PAYMENTDETAILS),
                            SmartClassGlobal::getTableStructure(SmartClassGlobal::PAYMENTDETAILS),
                            paymentInfo);
        }

        QList<QVariant> sImages, rImages;
        sImages << sID
                << myDB->pixmapToVariant(*pics[0])
                << myDB->pixmapToVariant(*pics[1]);

        rImages << rID
                << myDB->pixmapToVariant(*pics[2])
                << myDB->pixmapToVariant(*pics[3])
                << myDB->pixmapToVariant(*pics[4]);

        myDB->insertRow(SmartClassGlobal::getTableName(SmartClassGlobal::STUDENTIMAGES),
                        SmartClassGlobal::getTableStructure(SmartClassGlobal::STUDENTIMAGES),
                        sImages);

        myDB->insertRow(SmartClassGlobal::getTableName(SmartClassGlobal::RESPONSIBLEIMAGES),
                        SmartClassGlobal::getTableStructure(SmartClassGlobal::RESPONSIBLEIMAGES),
                        rImages);

        emit newData(QList<QVariant>() << sID
                                       << studentInfo.at(0)
                                       << rID
                                       << responsibleInfo.at(0)
                                       << studentInfo.at(4)
                                       << studentInfo.at(5));
        this->close();
        return;
    }

    qlonglong rID;

    QList< QList<QVariant> > siblingsData = myDB->retrieveAllCond(SmartClassGlobal::getTableName(SmartClassGlobal::STUDENT),
                                                                  SmartClassGlobal::getTableStructure(SmartClassGlobal::STUDENT).at(2),
                                                                  responsibleData.at(0));
    int siblings = siblingsData.length();

    if (responsibleInfo.at(0) != responsibleData.at(1) && ui->edtParentName->currentIndex() >= 0 &&
            ui->edtParentName->itemText(ui->edtParentName->currentIndex()) == responsibleInfo.at(0)
            && siblings == 1){
        rID = ui->edtParentName->itemData(ui->edtParentName->currentIndex(), Qt::UserRole).toLongLong();
        myDB->removeRow(SmartClassGlobal::getTableName(SmartClassGlobal::RESPONSIBLE),
                        SmartClassGlobal::getTableStructure(SmartClassGlobal::RESPONSIBLE).at(0),
                        responsibleData.at(0));

        myDB->updateRow(SmartClassGlobal::getTableName(SmartClassGlobal::RESPONSIBLE),
                        SmartClassGlobal::getTableStructure(SmartClassGlobal::RESPONSIBLE).at(0),
                        rID,
                        SmartClassGlobal::getTableStructure(SmartClassGlobal::RESPONSIBLE).mid(1),
                        responsibleInfo);
    }
    else if (responsibleInfo.at(0) == responsibleData.at(1) || siblings == 1){
        rID = responsibleData.at(0).toLongLong();
        myDB->updateRow(SmartClassGlobal::getTableName(SmartClassGlobal::RESPONSIBLE),
                        SmartClassGlobal::getTableStructure(SmartClassGlobal::RESPONSIBLE).at(0),
                        responsibleData.at(0),
                        SmartClassGlobal::getTableStructure(SmartClassGlobal::RESPONSIBLE).mid(1),
                        responsibleInfo);
    }
    else {
        QMessageBox parentWarning;
        parentWarning.setIcon(QMessageBox::Information);
        parentWarning.setWindowTitle(tr("Parent data changed | SmartClass"));
        parentWarning.setText(tr("Looks like the name of the parent has changed. You have three options from now on.\n\n"
                                 "-> Update : If you choose this option, the responsible data will be changed for every student attached to him/her.\n"
                                 "-> Ignore : If you choose this option, the responsible name will remain the same.\n"
                                 "-> Change : If you choose this option, the responsible data will be updated just for this student. Other students related to this parent will remain intact."));
        parentWarning.setStandardButtons(QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel);
        parentWarning.setButtonText(QMessageBox::Yes,tr("Update"));
        parentWarning.setButtonText(QMessageBox::Ignore,tr("Ignore"));
        parentWarning.setButtonText(QMessageBox::Cancel,tr("Change"));
        int choice = parentWarning.exec();

        if (choice == QMessageBox::Yes){
            if (ui->edtParentName->currentIndex() >= 0 &&
                    ui->edtParentName->itemText(ui->edtParentName->currentIndex()) == responsibleInfo.at(0)){
                rID = ui->edtParentName->itemData(ui->edtParentName->currentIndex(), Qt::UserRole).toLongLong();
                myDB->removeRow(SmartClassGlobal::getTableName(SmartClassGlobal::RESPONSIBLE),
                                SmartClassGlobal::getTableStructure(SmartClassGlobal::RESPONSIBLE).at(0),
                                responsibleData.at(0));

                myDB->updateRow(SmartClassGlobal::getTableName(SmartClassGlobal::RESPONSIBLE),
                                SmartClassGlobal::getTableStructure(SmartClassGlobal::RESPONSIBLE).at(0),
                                rID,
                                SmartClassGlobal::getTableStructure(SmartClassGlobal::RESPONSIBLE).mid(1),
                                responsibleInfo);
                for (int i = 0; i < siblings; ++i)
                    if (siblingsData[i][0].toLongLong() != STUDENT_ID)
                        myDB->updateRow(SmartClassGlobal::getTableName(SmartClassGlobal::STUDENT),
                                        SmartClassGlobal::getTableStructure(SmartClassGlobal::STUDENT).at(0),
                                        siblingsData[i][0].toLongLong(),
                                        QStringList() << SmartClassGlobal::getTableStructure(SmartClassGlobal::STUDENT).at(2),
                                        QList<QVariant>() << rID);
            }
            else {
                rID = responsibleData.at(0).toLongLong();
                myDB->updateRow(SmartClassGlobal::getTableName(SmartClassGlobal::RESPONSIBLE),
                                SmartClassGlobal::getTableStructure(SmartClassGlobal::RESPONSIBLE).at(0),
                                responsibleData.at(0),
                                SmartClassGlobal::getTableStructure(SmartClassGlobal::RESPONSIBLE).mid(1),
                                responsibleInfo);
            }
        }
        else if (choice == QMessageBox::Ignore){
            rID = responsibleData.at(0).toLongLong();
            responsibleInfo.replace(0, responsibleData.at(1));
            myDB->updateRow(SmartClassGlobal::getTableName(SmartClassGlobal::RESPONSIBLE),
                            SmartClassGlobal::getTableStructure(SmartClassGlobal::RESPONSIBLE).at(0),
                            responsibleData.at(0),
                            SmartClassGlobal::getTableStructure(SmartClassGlobal::RESPONSIBLE).mid(1),
                            responsibleInfo);
        }
        else {
            myDB->insertRow(SmartClassGlobal::getTableName(SmartClassGlobal::RESPONSIBLE),
                            SmartClassGlobal::getTableStructure(SmartClassGlobal::RESPONSIBLE).mid(1),
                            responsibleInfo);
            rID = myDB->retrieveRow(SmartClassGlobal::getTableName(SmartClassGlobal::RESPONSIBLE),
                                    SmartClassGlobal::getTableStructure(SmartClassGlobal::RESPONSIBLE).at(1),
                                    responsibleInfo.at(0)).at(0).toLongLong();
        }
    }

    while (myDB->rowExists(SmartClassGlobal::getTableName(SmartClassGlobal::COURSEENROLLMENTS),
                           SmartClassGlobal::getTableStructure(SmartClassGlobal::COURSEENROLLMENTS).at(1),
                           STUDENT_ID))
        myDB->removeRow(SmartClassGlobal::getTableName(SmartClassGlobal::COURSEENROLLMENTS),
                        SmartClassGlobal::getTableStructure(SmartClassGlobal::COURSEENROLLMENTS).at(1),
                        STUDENT_ID);

    for (int i = 0; i < ui->listCourses->count(); ++i)
        myDB->insertRow(SmartClassGlobal::getTableName(SmartClassGlobal::COURSEENROLLMENTS),
                        SmartClassGlobal::getTableStructure(SmartClassGlobal::COURSEENROLLMENTS),
                        QList<QVariant>() << ui->listCourses->item(i)->data(Qt::UserRole) << STUDENT_ID);

    while(myDB->rowExists(SmartClassGlobal::getTableName(SmartClassGlobal::PAYMENTDETAILS),
                          SmartClassGlobal::getTableStructure(SmartClassGlobal::PAYMENTDETAILS).at(0),
                          STUDENT_ID))
        myDB->removeRow(SmartClassGlobal::getTableName(SmartClassGlobal::PAYMENTDETAILS),
                        SmartClassGlobal::getTableStructure(SmartClassGlobal::PAYMENTDETAILS).at(0),
                        STUDENT_ID);

    for (int i = 0; i < ui->listPaymentCourses->count(); ++i){
        QListWidgetPaymentItem* item =  static_cast<QListWidgetPaymentItem*>(ui->listPaymentCourses->item(i));

        QList<QVariant> paymentInfo;
        paymentInfo << STUDENT_ID
                    << item->data(Qt::UserRole)
                    << item->discount()
                    << item->date()
                    << item->installments();

        myDB->insertRow(SmartClassGlobal::getTableName(SmartClassGlobal::PAYMENTDETAILS),
                        SmartClassGlobal::getTableStructure(SmartClassGlobal::PAYMENTDETAILS),
                        paymentInfo);
    }

    studentInfo.insert(2, rID);
    myDB->updateRow(SmartClassGlobal::getTableName(SmartClassGlobal::STUDENT),
                    SmartClassGlobal::getTableStructure(SmartClassGlobal::STUDENT).at(0),
                    STUDENT_ID,
                    SmartClassGlobal::getTableStructure(SmartClassGlobal::STUDENT).mid(1),
                    studentInfo);

    QList<QVariant> sImages, rImages;
    sImages << STUDENT_ID
            << myDB->pixmapToVariant(*pics[0])
            << myDB->pixmapToVariant(*pics[1]);

    rImages << rID
            << myDB->pixmapToVariant(*pics[2])
            << myDB->pixmapToVariant(*pics[3])
            << myDB->pixmapToVariant(*pics[4]);

    if (myDB->rowExists(SmartClassGlobal::getTableName(SmartClassGlobal::STUDENTIMAGES),
                        SmartClassGlobal::getTableStructure(SmartClassGlobal::STUDENTIMAGES).at(0),
                        STUDENT_ID))
        myDB->updateRow(SmartClassGlobal::getTableName(SmartClassGlobal::STUDENTIMAGES),
                        SmartClassGlobal::getTableStructure(SmartClassGlobal::STUDENTIMAGES).at(0),
                        STUDENT_ID,
                        SmartClassGlobal::getTableStructure(SmartClassGlobal::STUDENTIMAGES),
                        sImages);
    else myDB->insertRow(SmartClassGlobal::getTableName(SmartClassGlobal::STUDENTIMAGES),
                         SmartClassGlobal::getTableStructure(SmartClassGlobal::STUDENTIMAGES),
                         sImages);

    if (myDB->rowExists(SmartClassGlobal::getTableName(SmartClassGlobal::RESPONSIBLEIMAGES),
                        SmartClassGlobal::getTableStructure(SmartClassGlobal::RESPONSIBLEIMAGES).at(0),
                        rID))
        myDB->updateRow(SmartClassGlobal::getTableName(SmartClassGlobal::RESPONSIBLEIMAGES),
                        SmartClassGlobal::getTableStructure(SmartClassGlobal::RESPONSIBLEIMAGES).at(0),
                        rID,
                        SmartClassGlobal::getTableStructure(SmartClassGlobal::RESPONSIBLEIMAGES),
                        rImages);
    else myDB->insertRow(SmartClassGlobal::getTableName(SmartClassGlobal::RESPONSIBLEIMAGES),
                         SmartClassGlobal::getTableStructure(SmartClassGlobal::RESPONSIBLEIMAGES),
                         rImages);

    emit updatedData(QList<QVariant>() << studentInfo.at(0)
                                       << rID
                                       << responsibleInfo.at(0)
                                       << studentInfo.at(4)
                                       << studentInfo.at(5),
                     STUDENT_ID);
    this->close();
}

void frmManageStudent::resetData(){
    if (QMessageBox::question(this, tr("Reset confirmation | SmartClass"),
                              tr("You are going to erase all the data that you have already inserted/modified. This action cannot be undone. Do you still want to proceed?"),
                              QMessageBox::Yes, QMessageBox::No) != QMessageBox::Yes) return;

    ui->edtStudentName->setText("");
    ui->edtStudentBirthday->setDate(QDate(2000, 01, 01));
    ui->edtStudentID->setText("");
    ui->edtStudentSchool->setText("");
    ui->edtStudentObservations->setPlainText("");

    ui->cbExperimentalClassCourse->setCurrentIndex(0);
    ui->edtExperimentalClassDateTime->setDateTime(QDateTime(QDate(2000, 01, 91), QTime(00, 00)));
    ui->edtExperimentalClassObservations->setPlainText("");

    ui->edtParentName->setCurrentText("");
    ui->edtParentPhone->setText("");
    ui->cbParentMobileOperator->setCurrentIndex(0);
    ui->edtParentMobile->setText("");
    ui->edtParentEmail->setText("");
    ui->edtParentID->setText("");
    ui->edtParentCPG->setText("");
    ui->cbParentIndication->setCurrentIndex(0);

    ui->cbRegistrationCourse->setCurrentIndex(0);
    ui->btRemoveCourse->setEnabled(false);
    ui->listCourses->clear();
    ui->edtRegistrationAddress->setText("");
    ui->btStudentAddressConfirmationPicture->setEnabled(false);

    ui->listPaymentCourses->clear();
    ui->sbPaymentCost->setValue(0.00);
    ui->sbDiscount->setValue(0.00);
    ui->sbDiscountV->setValue(0.00);
    ui->edtPaymentFirstInstallment->setDate(QDate(2000, 01, 01));
    ui->sbInstallments->setValue(1);
    ui->lblPaymentDetails->setVisible(false);

    if (CURRENT_ROLE != frmManageStudent::Create) retrieveData();
}

void frmManageStudent::retrieveData(){
    if (!courseData.isEmpty()){
        courseData.clear();

        while (ui->cbExperimentalClassCourse->count() != 1)
            ui->cbExperimentalClassCourse->removeItem(1);

        while (ui->cbRegistrationCourse->count() != 1)
            ui->cbRegistrationCourse->removeItem(1);
    }
    courseData = myDB->retrieveAll(SmartClassGlobal::getTableName(SmartClassGlobal::COURSEDETAILS),
                                   SmartClassGlobal::getTableStructure(SmartClassGlobal::COURSEDETAILS));
    courseDataCount = courseData.length();

    for (int i = 0; i < courseDataCount; ++i){
        if (courseData[i].at(8).toDate() >= QDate::currentDate()){
            QString courseSyntesis = courseData[i].at(1).toString() + tr(" ( class #") + courseData[i].at(5).toString() + tr(" ) - ") + courseData[i].at(6).toString()
                            + tr(" * starts on: ") + courseData[i].at(7).toString();
            ui->cbRegistrationCourse->addItem(courseSyntesis, courseData[i].at(0));
            ui->cbExperimentalClassCourse->addItem(courseSyntesis, courseData.at(0));
        }
    }

    QList< QList<QVariant>> allResponsibleData = myDB->retrieveAll(SmartClassGlobal::getTableName(SmartClassGlobal::RESPONSIBLE),
                                                                   QStringList() << "id" << "parent" << "students");
    int aPDataSize = allResponsibleData.length();
    QStringList responsibleNames;

    if (!allResponsibleData.isEmpty()){
        for (int i = 0; i < aPDataSize; ++i)
            responsibleNames << allResponsibleData[i].at(1).toString();

        responsibleNames.sort(Qt::CaseInsensitive);
        ui->edtParentName->addItems(responsibleNames);

        for (int i = 0; i < aPDataSize; ++i)
            for (int j = 0; j < aPDataSize; ++j)
                if (ui->edtParentName->itemText(j) == allResponsibleData[i][1].toString()){
                    ui->edtParentName->setItemData(j, allResponsibleData[i][0], Qt::UserRole);
                    break;
                }
    }

    if (CURRENT_ROLE == frmManageStudent::Create) return;
    if (CURRENT_ROLE == frmManageStudent::View) ui->screenManagerLayout->setEnabled(false);

    for (int i = 0; i < 5; ++i){
        delete pics[i];
        pics[i] = NULL;
    }

    studentData = myDB->retrieveRow(SmartClassGlobal::getTableName(SmartClassGlobal::STUDENT),
                                    SmartClassGlobal::getTableStructure(SmartClassGlobal::STUDENT).at(0),
                                    QVariant(STUDENT_ID));

    qlonglong responsibleIndex = -1;
    for (int i = 0; i < aPDataSize; ++i)
        if (allResponsibleData[i].at(0).toLongLong() == studentData.at(2).toLongLong()){
            responsibleIndex = i;
            break;
        }

    responsibleData = myDB->retrieveRow(SmartClassGlobal::getTableName(SmartClassGlobal::RESPONSIBLE),
                                        SmartClassGlobal::getTableStructure(SmartClassGlobal::RESPONSIBLE).at(0),
                                        QVariant(allResponsibleData[responsibleIndex].at(0).toLongLong()));

    QList<QVariant> rImages = myDB->retrieveRow(SmartClassGlobal::getTableName(SmartClassGlobal::RESPONSIBLEIMAGES),
                                                QStringList() << SmartClassGlobal::getTableStructure(SmartClassGlobal::RESPONSIBLEIMAGES).at(0),
                                                QList<QVariant>() << allResponsibleData[responsibleIndex].at(0),
                                                SmartClassGlobal::getTableStructure(SmartClassGlobal::RESPONSIBLEIMAGES).mid(1));
    QList<QVariant> sImages = myDB->retrieveRow(SmartClassGlobal::getTableName(SmartClassGlobal::STUDENTIMAGES),
                                                QStringList() << SmartClassGlobal::getTableStructure(SmartClassGlobal::STUDENTIMAGES).at(0),
                                                QList<QVariant>() << studentData.at(0),
                                                SmartClassGlobal::getTableStructure(SmartClassGlobal::STUDENTIMAGES).mid(1));

    pics[0] = new QPixmap(myDB->variantToPixmap(sImages.at(0)));
    pics[1] = new QPixmap(myDB->variantToPixmap(sImages.at(1)));
    pics[2] = new QPixmap(myDB->variantToPixmap(rImages.at(0)));
    pics[3] = new QPixmap(myDB->variantToPixmap(rImages.at(1)));
    pics[4] = new QPixmap(myDB->variantToPixmap(rImages.at(2)));

    if (!pics[0]->isNull()) ui->lblStudentImage->setPixmap(*pics[0]);    

    paymentData = myDB->retrieveAllCond(SmartClassGlobal::getTableName(SmartClassGlobal::PAYMENTDETAILS),
                                        SmartClassGlobal::getTableStructure(SmartClassGlobal::PAYMENTDETAILS).at(0),
                                        STUDENT_ID);
    paymentDataCount = paymentData.length();

    ui->edtStudentName->setText(studentData.at(1).toString());
    ui->edtStudentBirthday->setDate(studentData.at(3).toDate());
    ui->edtStudentID->setText(studentData.at(4).toString());
    ui->edtStudentSchool->setText(studentData.at(5).toString());
    ui->edtStudentObservations->setPlainText(studentData.at(6).toString());
    ui->cbExperimentalClassCourse->setCurrentText(studentData.at(7).toString());
    ui->edtExperimentalClassDateTime->setDateTime(studentData.at(8).toDateTime());
    ui->edtExperimentalClassObservations->setPlainText(studentData.at(9).toString());

    ui->edtParentName->setCurrentText(responsibleData.at(1).toString());
    ui->edtParentPhone->setText(responsibleData.at(2).toString());
    ui->cbParentMobileOperator->setCurrentIndex(responsibleData.at(3).toInt());
    ui->edtParentMobile->setText(responsibleData.at(4).toString());
    ui->edtParentEmail->setText(responsibleData.at(5).toString());
    ui->edtParentID->setText(responsibleData.at(6).toString());
    ui->edtParentCPG->setText(responsibleData.at(7).toString());
    ui->cbParentIndication->setCurrentText(responsibleData.at(8).toString());
    ui->edtRegistrationAddress->setText(responsibleData.at(9).toString());

    QList< QList<QVariant> > studentCoursesID = myDB->retrieveAllCond(SmartClassGlobal::getTableName(SmartClassGlobal::COURSEENROLLMENTS),
                                                                      SmartClassGlobal::getTableStructure(SmartClassGlobal::COURSEENROLLMENTS).at(1),
                                                                      STUDENT_ID);


    for (int i = 0; i < studentCoursesID.length(); ++i){
        for (int k = 0; k < courseDataCount; ++k){
            if (studentCoursesID[i][0].toLongLong() == courseData[k].at(0).toLongLong()){
                QString courseSyntesis = courseData[k][1].toString() + tr(" ( class #") + courseData[k][5].toString() + tr(" ) - ") + courseData[k][6].toString()
                                            + tr(" * starts on: ") + courseData[k][7].toString();
                QListWidgetItem *newItem = new QListWidgetItem(courseSyntesis);
                newItem->setData(Qt::UserRole, courseData[k][0]);
                ui->listCourses->addItem(newItem);
                ui->listCourses->setCurrentItem(newItem);
            }
        }
    }

    for (int i = 0; i < paymentDataCount; ++i){
        for (int k = 0; k < courseDataCount; k++){
            if (courseData[k][0].toLongLong() == paymentData[i][1].toLongLong()){
                QString courseSyntesis = courseData[k][1].toString() + tr(" ( class #") + courseData[k][5].toString() + tr(" ) - ") + courseData[k][6].toString()
                                            + tr(" * starts on: ") + courseData[k][7].toString();

                QListWidgetPaymentItem *item;
                item = new QListWidgetPaymentItem(courseSyntesis,courseData[k][9].toDouble());
                item->setData(Qt::UserRole, courseData[k][0]);
                item->setDiscount(paymentData[i][2].toDouble());
                item->setDate(paymentData[i][3].toDate());
                item->setInstallments(paymentData[i][4].toInt());

                ui->listPaymentCourses->addItem(static_cast<QListWidgetItem*>(item));
                ui->listPaymentCourses->setCurrentRow(ui->listPaymentCourses->count() - 1);
            }
        }
    }

    this->courseQuantityChanged();
}

void frmManageStudent::addCourse(){
    if (!ui->cbRegistrationCourse->currentIndex()) return;

    for (int i = 0; i < ui->listCourses->count(); ++i){
        if (ui->listCourses->item(i)->data(Qt::UserRole).toLongLong() == ui->cbRegistrationCourse->currentData(Qt::UserRole).toLongLong()){
            QMessageBox::information(this, tr("Warning | SmartClass"), tr("This student is already registered on this course!"), QMessageBox::Ok, QMessageBox::Ok);
            return;
        }
    }

    ui->listCourses->addItem(ui->cbRegistrationCourse->currentText());
    QListWidgetPaymentItem *item = new QListWidgetPaymentItem(ui->cbRegistrationCourse->currentText(),
                                                              courseData[ui->cbRegistrationCourse->currentIndex() - 1].at(9).toDouble());
    item->setData(Qt::UserRole, ui->cbRegistrationCourse->currentData(Qt::UserRole));
    ui->listPaymentCourses->addItem(static_cast<QListWidgetItem*>(item));

    ui->listCourses->setCurrentRow(ui->listCourses->count() - 1);
    ui->listPaymentCourses->setCurrentRow(ui->listPaymentCourses->count() - 1);
    this->courseQuantityChanged();
}

void frmManageStudent::removeCourse(){
    if (ui->listCourses->currentRow() < 0) return;

    QListWidgetItem* item = ui->listCourses->item(ui->listCourses->currentRow());
    QString itemText = item->text();
    qlonglong itemID = item->data(Qt::UserRole).toLongLong();

    if (QMessageBox::question(this, tr("SmartClass | Confirmation"), tr("You are going to remove the student from the %1 course. Are you sure?").arg(itemText), QMessageBox::Yes, QMessageBox::No)
            == QMessageBox::No) return;

    for (int i = 0; i < ui->listPaymentCourses->count(); ++i){
        if (itemID == ui->listPaymentCourses->item(i)->data(Qt::UserRole).toLongLong()){
            delete ui->listPaymentCourses->item(i);
            break;
        }
    }
    delete item;

    ui->listCourses->setCurrentRow(ui->listCourses->count() - 1);
    ui->listPaymentCourses->setCurrentRow(ui->listPaymentCourses->count() - 1);
    this->courseQuantityChanged();
}

void frmManageStudent::courseQuantityChanged(){
    bool enable = ui->listCourses->count() > 0;
    ui->btRemoveCourse->setEnabled(enable);
    ui->grpPaymentData->setEnabled(enable);
}

//Continue from here
void frmManageStudent::changePaymentDetails(){
    blockPaymentUpdate = true;
    if (ui->listPaymentCourses->currentRow() < 0){
        ui->edtPaymentFirstInstallment->setDate(QDate(2000, 01, 01));
        ui->sbDiscount->setValue(0.00);
        ui->sbDiscountV->setValue(0.00);
        ui->sbInstallments->setValue(1);
        ui->sbPaymentCost->setValue(0.00);
        ui->lblPaymentDetails->setVisible(false);
        blockPaymentUpdate = false;
        return;
    }

    QListWidgetPaymentItem* item = static_cast<QListWidgetPaymentItem*>(ui->listPaymentCourses->item(ui->listPaymentCourses->currentRow()));
    ui->edtPaymentFirstInstallment->setDate(item->date());
    ui->sbDiscount->setValue(item->discount());
    ui->sbDiscountV->setValue(item->discount() * 0.01 * item->value());
    ui->sbInstallments->setValue(item->installments());
    ui->sbPaymentCost->setValue(item->value());
    ui->lblPaymentDetails->setText(tr("You have choosen %1 installments of $ %2 . After the discount, each installment is going to cost $ %3 .")
                                    .arg(item->installments()).arg((double)(item->value()/item->installments()), 0, 'f', 2).arg((double)((item->value()/item->installments()) * (1 - (item->discount()/100.0f))), 0, 'f', 2));
    ui->lblPaymentDetails->setVisible(true);
    blockPaymentUpdate = false;
}

void frmManageStudent::paymentValuesChanged(){
    if (blockPaymentUpdate) return;
    QListWidgetPaymentItem* item = static_cast<QListWidgetPaymentItem*>(ui->listPaymentCourses->item(ui->listPaymentCourses->currentRow()));
    item->setDate(ui->edtPaymentFirstInstallment->date());
    item->setDiscount(ui->sbDiscount->value());
    item->setInstallments(ui->sbInstallments->value());
    ui->lblPaymentDetails->setText(tr("You have choosen %1 installments of $ %2 . After the discount, each installment is going to cost $ %3 .")
                                    .arg(item->installments()).arg((double)(item->value()/item->installments()), 0, 'f', 2).arg((double)((item->value()/item->installments()) * (1 - (item->discount()/100.0f))), 0, 'f', 2));
    ui->lblPaymentDetails->setVisible(true);
}

void frmManageStudent::changeDiscountValue(double value){
    disconnect(ui->sbDiscount, SIGNAL(valueChanged(double)), this, SLOT(changeDiscountPercentage(double)));
    ui->sbDiscount->setValue((value * 100) / ui->sbPaymentCost->value());
    connect(ui->sbDiscount, SIGNAL(valueChanged(double)), this, SLOT(changeDiscountPercentage(double)));
}

void frmManageStudent::changeDiscountPercentage(double percent){
    disconnect(ui->sbDiscountV, SIGNAL(valueChanged(double)), this, SLOT(changeDiscountValue(double)));
    ui->sbDiscountV->setValue((percent * ui->sbPaymentCost->value()) / 100);
    connect(ui->sbDiscountV, SIGNAL(valueChanged(double)), this, SLOT(changeDiscountValue(double)));
}

void frmManageStudent::courseIndexChanged(){
    if (ui->listPaymentCourses->currentRow() < 0){
        ui->lblDescriptionOfCourse->setVisible(false);
        return;
    }

    QListWidgetItem* item = ui->listCourses->item(ui->listCourses->currentRow());
    for (int i = 1; i < ui->listCourses->count() - 1; ++i){
        if (item->data(Qt::UserRole).toLongLong() == courseData[i].at(0).toLongLong())
            ui->lblDescriptionOfCourse->setText(courseData[i].at(3).toString());
    }
}

void frmManageStudent::updateParentInfo(const QString &pName){
    if (pName.isEmpty() || pName.isNull()) return;
    QList<QVariant> tempParentData;
    bool exists = false;
    if (myDB->rowExists(SmartClassGlobal::getTableName(SmartClassGlobal::RESPONSIBLE),
                        SmartClassGlobal::getTableStructure(SmartClassGlobal::RESPONSIBLE).at(1),
                        pName)){
        tempParentData = myDB->retrieveRow(SmartClassGlobal::getTableName(SmartClassGlobal::RESPONSIBLE),
                                           SmartClassGlobal::getTableStructure(SmartClassGlobal::RESPONSIBLE).at(1),
                                           pName);
        exists = true;
    }

    if (!exists)
        if (QMessageBox::question(this, tr("Inexistent parental data | SmartClass"),
                              tr("We could not find any parent with this name in our database. Would you like to erase all the existent data related to the parent on this form?"),
                              QMessageBox::Yes | QMessageBox::No) == QMessageBox::No)
            return;
    if (exists){
        ui->edtParentPhone->setText(tempParentData.at(2).toString());
        ui->cbParentMobileOperator->setCurrentIndex(tempParentData.at(3).toInt());
        ui->edtParentMobile->setText(tempParentData.at(4).toString());
        ui->edtParentEmail->setText(tempParentData.at(5).toString());
        ui->edtParentID->setText(tempParentData.at(6).toString());
        ui->edtParentCPG->setText(tempParentData.at(7).toString());
        ui->cbParentIndication->setCurrentText(tempParentData.at(8).toString());
    }
    else {
        ui->edtParentPhone->setText("");
        ui->cbParentMobileOperator->setCurrentIndex(0);
        ui->edtParentMobile->setText("");
        ui->edtParentEmail->setText("");
        ui->edtParentID->setText("");
        ui->edtParentCPG->setText("");
        ui->cbParentIndication->setCurrentText(tr("Online advertisements"));
    }

    for (int i = 2; i < 5; ++i){
        delete pics[i];
        pics[i] = NULL;
    }

    if (exists){
        QList<QVariant> rows = myDB->retrieveRow(SmartClassGlobal::getTableName(SmartClassGlobal::RESPONSIBLEIMAGES),
                                                 QStringList() << SmartClassGlobal::getTableStructure(SmartClassGlobal::RESPONSIBLEIMAGES).at(0),
                                                 QList<QVariant>() << QVariant(tempParentData.at(0)),
                                                 SmartClassGlobal::getTableStructure(SmartClassGlobal::RESPONSIBLEIMAGES).mid(1));
        pics[2] = new QPixmap(myDB->variantToPixmap(rows.at(0)));
        pics[3] = new QPixmap(myDB->variantToPixmap(rows.at(1)));
        pics[4] = new QPixmap(myDB->variantToPixmap(rows.at(2)));
    }
    else {
        pics[2] = new QPixmap();
        pics[3] = new QPixmap();
        pics[4] = new QPixmap();
    }
}
