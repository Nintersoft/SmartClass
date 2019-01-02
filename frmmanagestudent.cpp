#include "frmmanagestudent.h"
#include "ui_frmmanagestudent.h"

frmManageStudent::frmManageStudent(QWidget *parent, Role role, const qint64 &studentID) :
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

    db_manager = DBManager::getInstance();

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

    this->retrieveData();

    if (role == frmManageStudent::Create) ui->edtParentName->setCurrentText("");
    if (CURRENT_ROLE != frmManageStudent::View)
        connect(ui->edtParentName, SIGNAL(currentIndexChanged(QString)), this, SLOT(updateParentInfo(QString)));
}

frmManageStudent::~frmManageStudent()
{
    for (int i = 0; i < 5; ++i) delete pics[i];
    delete[] pics;

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
    QString baseMessage = tr("Well, unfortunately we cannot proceed. Some data are either inconsistent or inexistent."
                             " Please, fix the folowing issues before trying to save again:\n"),
            errorMessage = "";

    if (ui->edtStudentName->text().isEmpty())
        errorMessage += tr("\n->The name of the student cannot be empty;");
    else if (CURRENT_ROLE == frmManageStudent::Create)
        if (db_manager->rowExists(SmartClassGlobal::getTableName(SmartClassGlobal::STUDENT),
                                  SmartClassGlobal::getTableAliases(SmartClassGlobal::STUDENT).at(1),
                                  ui->edtStudentName->text()))
            errorMessage += tr("\n->A student with the same same name already exists in your database. If you want to update him/her, please, use the update button available in the main screen;");

    if (!ui->edtStudentBirthday->date().isValid())
        errorMessage += tr("\n->The birthday of the student is invalid;");

    if (ui->edtParentName->currentText().isEmpty())
        errorMessage += tr("\n->The name of the parent cannot be empty;");

    if  (ui->edtParentPhone->text().isEmpty() && ui->edtParentEmail->text().isEmpty() && ui->edtParentMobile->text().isEmpty())
        errorMessage += tr("\n->It must have either the parent's email or telephone. Currently, both of them are empty;");

    if (!errorMessage.isEmpty()){
        baseMessage += errorMessage;
        QMessageBox::information(this, tr("Warning | SmartClass"), baseMessage, QMessageBox::Ok, QMessageBox::Ok);
        return;
    }

    QStringList courses;
    for (int i = 0; i < ui->listCourses->count(); ++i)
        courses << QString::number(ui->listCourses->item(i)->data(Qt::UserRole).toLongLong());

    QVariantList studentInfo, responsibleInfo;
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
            db_manager->insertRow(SmartClassGlobal::getTableName(SmartClassGlobal::RESPONSIBLE),
                                  SmartClassGlobal::getTableAliases(SmartClassGlobal::RESPONSIBLE).mid(1),
                                  responsibleInfo);
            rID = db_manager->retrieveRow(SmartClassGlobal::getTableName(SmartClassGlobal::RESPONSIBLE),
                                          SmartClassGlobal::getTableAliases(SmartClassGlobal::RESPONSIBLE).at(1),
                                          responsibleInfo.at(0)).at(0).toLongLong();
        }
        else {
            rID = ui->edtParentName->itemData(ui->edtParentName->currentIndex(), Qt::UserRole).toLongLong();
            db_manager->updateRow(SmartClassGlobal::getTableName(SmartClassGlobal::RESPONSIBLE),
                                  SmartClassGlobal::getTableAliases(SmartClassGlobal::RESPONSIBLE).at(0),
                                  rID,
                                  SmartClassGlobal::getTableAliases(SmartClassGlobal::RESPONSIBLE).mid(1),
                                  responsibleInfo);
        }

        studentInfo.insert(1, rID);
        db_manager->insertRow(SmartClassGlobal::getTableName(SmartClassGlobal::STUDENT),
                              SmartClassGlobal::getTableAliases(SmartClassGlobal::STUDENT).mid(1),
                              studentInfo);

        sID = db_manager->retrieveRow(SmartClassGlobal::getTableName(SmartClassGlobal::STUDENT),
                                      SmartClassGlobal::getTableAliases(SmartClassGlobal::STUDENT).at(1),
                                      studentInfo.at(0)).at(0).toLongLong();

        for (int i = 0; i < ui->listCourses->count(); ++i){
            QVariantList csRow;
            csRow << ui->listCourses->item(i)->data(Qt::UserRole) << sID;
            db_manager->insertRow(SmartClassGlobal::getTableName(SmartClassGlobal::COURSEENROLLMENTS),
                                  SmartClassGlobal::getTableAliases(SmartClassGlobal::COURSEENROLLMENTS),
                                  csRow);
        }

        for (int i = 0; i < ui->listPaymentCourses->count(); ++i){
            QListWidgetPaymentItem* item =  static_cast<QListWidgetPaymentItem*>(ui->listPaymentCourses->item(i));

            QVariantList paymentInfo;
            paymentInfo << sID
                        << item->data(Qt::UserRole)
                        << item->discount()
                        << item->date()
                        << item->installments();

            db_manager->insertRow(SmartClassGlobal::getTableName(SmartClassGlobal::PAYMENTDETAILS),
                                  SmartClassGlobal::getTableAliases(SmartClassGlobal::PAYMENTDETAILS),
                                  paymentInfo);
        }

        QVariantList sImages, rImages;
        sImages << sID
                << db_manager->pixmapToVariant(*pics[0])
                << db_manager->pixmapToVariant(*pics[1]);

        rImages << rID
                << db_manager->pixmapToVariant(*pics[2])
                << db_manager->pixmapToVariant(*pics[3])
                << db_manager->pixmapToVariant(*pics[4]);

        db_manager->insertRow(SmartClassGlobal::getTableName(SmartClassGlobal::STUDENTIMAGES),
                              SmartClassGlobal::getTableAliases(SmartClassGlobal::STUDENTIMAGES),
                              sImages);

        db_manager->insertRow(SmartClassGlobal::getTableName(SmartClassGlobal::RESPONSIBLEIMAGES),
                              SmartClassGlobal::getTableAliases(SmartClassGlobal::RESPONSIBLEIMAGES),
                              rImages);

        emit newData(QVariantList() << sID
                                    << studentInfo.at(0)
                                    << rID
                                    << responsibleInfo.at(0)
                                    << studentInfo.at(4)
                                    << studentInfo.at(5));
        this->close();
        return;
    }

    qlonglong rID;

    QList< QVariantList > siblingsData = db_manager->retrieveAllCond(SmartClassGlobal::getTableName(SmartClassGlobal::STUDENT),
                                                                     SmartClassGlobal::getTableAliases(SmartClassGlobal::STUDENT).at(2),
                                                                     responsibleData.at(0));
    int siblings = siblingsData.length();

    if (responsibleInfo.at(0) != responsibleData.at(1) && ui->edtParentName->currentIndex() >= 0 &&
            ui->edtParentName->itemText(ui->edtParentName->currentIndex()) == responsibleInfo.at(0)
            && siblings == 1){
        rID = ui->edtParentName->itemData(ui->edtParentName->currentIndex(), Qt::UserRole).toLongLong();

        db_manager->updateRow(SmartClassGlobal::getTableName(SmartClassGlobal::RESPONSIBLE),
                              SmartClassGlobal::getTableAliases(SmartClassGlobal::RESPONSIBLE).at(0),
                              rID,
                              SmartClassGlobal::getTableAliases(SmartClassGlobal::RESPONSIBLE).mid(1),
                              responsibleInfo);

        if (db_manager->rowExists(SmartClassGlobal::getTableName(SmartClassGlobal::RESPONSIBLEIMAGES),
                                  SmartClassGlobal::getTableAliases(SmartClassGlobal::RESPONSIBLEIMAGES).at(0),
                                  responsibleData.at(0)))
            db_manager->removeRow(SmartClassGlobal::getTableName(SmartClassGlobal::RESPONSIBLEIMAGES),
                                  SmartClassGlobal::getTableAliases(SmartClassGlobal::RESPONSIBLEIMAGES).at(0),
                                  responsibleData.at(0));

        db_manager->removeRow(SmartClassGlobal::getTableName(SmartClassGlobal::RESPONSIBLE),
                              SmartClassGlobal::getTableAliases(SmartClassGlobal::RESPONSIBLE).at(0),
                              responsibleData.at(0));
    }
    else if (responsibleInfo.at(0) == responsibleData.at(1) || siblings == 1){
        rID = responsibleData.at(0).toLongLong();
        db_manager->updateRow(SmartClassGlobal::getTableName(SmartClassGlobal::RESPONSIBLE),
                        SmartClassGlobal::getTableAliases(SmartClassGlobal::RESPONSIBLE).at(0),
                        responsibleData.at(0),
                        SmartClassGlobal::getTableAliases(SmartClassGlobal::RESPONSIBLE).mid(1),
                        responsibleInfo);
    }
    else {
        QMessageBox parentWarning;
        parentWarning.setIcon(QMessageBox::Information);
        parentWarning.setWindowTitle(tr("Parent data changed | SmartClass"));
        parentWarning.setText(tr("Looks like the name of the responsible has changed. You have three options from now on.\n\n"
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
                db_manager->updateRow(SmartClassGlobal::getTableName(SmartClassGlobal::RESPONSIBLE),
                                      SmartClassGlobal::getTableAliases(SmartClassGlobal::RESPONSIBLE).at(0),
                                      rID,
                                      SmartClassGlobal::getTableAliases(SmartClassGlobal::RESPONSIBLE).mid(1),
                                      responsibleInfo);

                if (db_manager->rowExists(SmartClassGlobal::getTableName(SmartClassGlobal::RESPONSIBLEIMAGES),
                                          SmartClassGlobal::getTableAliases(SmartClassGlobal::RESPONSIBLEIMAGES).at(0),
                                          responsibleData.at(0)))
                    db_manager->removeRow(SmartClassGlobal::getTableName(SmartClassGlobal::RESPONSIBLEIMAGES),
                                          SmartClassGlobal::getTableAliases(SmartClassGlobal::RESPONSIBLEIMAGES).at(0),
                                          responsibleData.at(0));

                db_manager->removeRow(SmartClassGlobal::getTableName(SmartClassGlobal::RESPONSIBLE),
                                      SmartClassGlobal::getTableAliases(SmartClassGlobal::RESPONSIBLE).at(0),
                                      responsibleData.at(0));

                for (int i = 0; i < siblings; ++i)
                    if (siblingsData[i][0].toLongLong() != STUDENT_ID)
                        db_manager->updateRow(SmartClassGlobal::getTableName(SmartClassGlobal::STUDENT),
                                              SmartClassGlobal::getTableAliases(SmartClassGlobal::STUDENT).at(0),
                                              siblingsData[i][0].toLongLong(),
                                              QStringList() << SmartClassGlobal::getTableAliases(SmartClassGlobal::STUDENT).at(2),
                                              QVariantList() << rID);
            }
            else {
                rID = responsibleData.at(0).toLongLong();
                db_manager->updateRow(SmartClassGlobal::getTableName(SmartClassGlobal::RESPONSIBLE),
                                      SmartClassGlobal::getTableAliases(SmartClassGlobal::RESPONSIBLE).at(0),
                                      responsibleData.at(0),
                                      SmartClassGlobal::getTableAliases(SmartClassGlobal::RESPONSIBLE).mid(1),
                                      responsibleInfo);
            }
        }
        else if (choice == QMessageBox::Ignore){
            rID = responsibleData.at(0).toLongLong();
            responsibleInfo.replace(0, responsibleData.at(1));
            db_manager->updateRow(SmartClassGlobal::getTableName(SmartClassGlobal::RESPONSIBLE),
                                  SmartClassGlobal::getTableAliases(SmartClassGlobal::RESPONSIBLE).at(0),
                                  responsibleData.at(0),
                                  SmartClassGlobal::getTableAliases(SmartClassGlobal::RESPONSIBLE).mid(1),
                                  responsibleInfo);
        }
        else {
            db_manager->insertRow(SmartClassGlobal::getTableName(SmartClassGlobal::RESPONSIBLE),
                                  SmartClassGlobal::getTableAliases(SmartClassGlobal::RESPONSIBLE).mid(1),
                                  responsibleInfo);
            rID = db_manager->retrieveRow(SmartClassGlobal::getTableName(SmartClassGlobal::RESPONSIBLE),
                                          SmartClassGlobal::getTableAliases(SmartClassGlobal::RESPONSIBLE).at(1),
                                          responsibleInfo.at(0)).at(0).toLongLong();
        }
    }

    while (db_manager->rowExists(SmartClassGlobal::getTableName(SmartClassGlobal::COURSEENROLLMENTS),
                                 SmartClassGlobal::getTableAliases(SmartClassGlobal::COURSEENROLLMENTS).at(1),
                                 STUDENT_ID))
        db_manager->removeRow(SmartClassGlobal::getTableName(SmartClassGlobal::COURSEENROLLMENTS),
                              SmartClassGlobal::getTableAliases(SmartClassGlobal::COURSEENROLLMENTS).at(1),
                              STUDENT_ID);

    for (int i = 0; i < ui->listCourses->count(); ++i)
        db_manager->insertRow(SmartClassGlobal::getTableName(SmartClassGlobal::COURSEENROLLMENTS),
                              SmartClassGlobal::getTableAliases(SmartClassGlobal::COURSEENROLLMENTS),
                              QVariantList() << ui->listCourses->item(i)->data(Qt::UserRole) << STUDENT_ID);

    while(db_manager->rowExists(SmartClassGlobal::getTableName(SmartClassGlobal::PAYMENTDETAILS),
                                SmartClassGlobal::getTableAliases(SmartClassGlobal::PAYMENTDETAILS).at(0),
                                STUDENT_ID))
        db_manager->removeRow(SmartClassGlobal::getTableName(SmartClassGlobal::PAYMENTDETAILS),
                              SmartClassGlobal::getTableAliases(SmartClassGlobal::PAYMENTDETAILS).at(0),
                              STUDENT_ID);

    for (int i = 0; i < ui->listPaymentCourses->count(); ++i){
        QListWidgetPaymentItem* item =  static_cast<QListWidgetPaymentItem*>(ui->listPaymentCourses->item(i));

        QVariantList paymentInfo;
        paymentInfo << STUDENT_ID
                    << item->data(Qt::UserRole)
                    << item->discount()
                    << item->date()
                    << item->installments();

        db_manager->insertRow(SmartClassGlobal::getTableName(SmartClassGlobal::PAYMENTDETAILS),
                              SmartClassGlobal::getTableAliases(SmartClassGlobal::PAYMENTDETAILS),
                              paymentInfo);
    }

    studentInfo.insert(2, rID);
    db_manager->updateRow(SmartClassGlobal::getTableName(SmartClassGlobal::STUDENT),
                          SmartClassGlobal::getTableAliases(SmartClassGlobal::STUDENT).at(0),
                          STUDENT_ID,
                          SmartClassGlobal::getTableAliases(SmartClassGlobal::STUDENT).mid(1),
                          studentInfo);

    QVariantList sImages, rImages;
    sImages << STUDENT_ID
            << DBManager::pixmapToVariant(*pics[0])
            << DBManager::pixmapToVariant(*pics[1]);

    rImages << rID
            << DBManager::pixmapToVariant(*pics[2])
            << DBManager::pixmapToVariant(*pics[3])
            << DBManager::pixmapToVariant(*pics[4]);

    if (db_manager->rowExists(SmartClassGlobal::getTableName(SmartClassGlobal::STUDENTIMAGES),
                              SmartClassGlobal::getTableAliases(SmartClassGlobal::STUDENTIMAGES).at(0),
                              STUDENT_ID))
        db_manager->updateRow(SmartClassGlobal::getTableName(SmartClassGlobal::STUDENTIMAGES),
                              SmartClassGlobal::getTableAliases(SmartClassGlobal::STUDENTIMAGES).at(0),
                              STUDENT_ID,
                              SmartClassGlobal::getTableAliases(SmartClassGlobal::STUDENTIMAGES),
                              sImages);
    else db_manager->insertRow(SmartClassGlobal::getTableName(SmartClassGlobal::STUDENTIMAGES),
                               SmartClassGlobal::getTableAliases(SmartClassGlobal::STUDENTIMAGES),
                               sImages);

    if (db_manager->rowExists(SmartClassGlobal::getTableName(SmartClassGlobal::RESPONSIBLEIMAGES),
                              SmartClassGlobal::getTableAliases(SmartClassGlobal::RESPONSIBLEIMAGES).at(0),
                              rID))
        db_manager->updateRow(SmartClassGlobal::getTableName(SmartClassGlobal::RESPONSIBLEIMAGES),
                              SmartClassGlobal::getTableAliases(SmartClassGlobal::RESPONSIBLEIMAGES).at(0),
                              rID,
                              SmartClassGlobal::getTableAliases(SmartClassGlobal::RESPONSIBLEIMAGES),
                              rImages);
    else db_manager->insertRow(SmartClassGlobal::getTableName(SmartClassGlobal::RESPONSIBLEIMAGES),
                               SmartClassGlobal::getTableAliases(SmartClassGlobal::RESPONSIBLEIMAGES),
                               rImages);

    emit updatedData(QVariantList() << studentInfo.at(0)
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
    courseData = db_manager->retrieveAll(SmartClassGlobal::getTableName(SmartClassGlobal::COURSEDETAILS),
                                         SmartClassGlobal::getTableAliases(SmartClassGlobal::COURSEDETAILS));
    courseDataCount = courseData.length();

    for (int i = 0; i < courseDataCount; ++i){
        if (courseData[i].at(8).toDate() >= QDate::currentDate()){
            QString courseSyntesis = courseData[i].at(1).toString() + tr(" ( class #") + courseData[i].at(5).toString() + tr(" ) - ") + courseData[i].at(6).toString()
                            + tr(" * starts on: ") + courseData[i].at(7).toString();
            ui->cbRegistrationCourse->addItem(courseSyntesis, courseData[i].at(0));
            ui->cbExperimentalClassCourse->addItem(courseSyntesis, courseData.at(0));
        }
    }

    QList< QVariantList > allResponsibleData = db_manager->retrieveAll(SmartClassGlobal::getTableName(SmartClassGlobal::RESPONSIBLE),
                                                                       SmartClassGlobal::getTableAliases(SmartClassGlobal::RESPONSIBLE).mid(0, 2),
                                                                       QStringList(),
                                                                       QStringList() << (SmartClassGlobal::getTableAliases(SmartClassGlobal::RESPONSIBLE).at(1) + " ASC"));
    int aPDataSize = allResponsibleData.length();
    int responsibleIndex = -1;

    studentData = db_manager->retrieveRow(SmartClassGlobal::getTableName(SmartClassGlobal::STUDENT),
                                          SmartClassGlobal::getTableAliases(SmartClassGlobal::STUDENT).at(0),
                                          QVariant(STUDENT_ID));

    if (!allResponsibleData.isEmpty())
        for (int i = 0; i < aPDataSize; ++i){
            ui->edtParentName->addItem(allResponsibleData[i].at(1).toString(),
                                       allResponsibleData[i].at(0));
            if (allResponsibleData[i].at(0) == studentData.at(2)) responsibleIndex = i;
        }

    if (CURRENT_ROLE == frmManageStudent::Create) return;
    if (CURRENT_ROLE == frmManageStudent::View) ui->screenManagerLayout->setEnabled(false);

    for (int i = 0; i < 5; ++i){
        delete pics[i];
        pics[i] = NULL;
    }

    responsibleData = allResponsibleData.at(responsibleIndex);

    QVariantList rImages = db_manager->retrieveRow(SmartClassGlobal::getTableName(SmartClassGlobal::RESPONSIBLEIMAGES),
                                                   SmartClassGlobal::getTableAliases(SmartClassGlobal::RESPONSIBLEIMAGES).mid(0, 1),
                                                   QVariantList() << responsibleData.at(0),
                                                   SmartClassGlobal::getTableAliases(SmartClassGlobal::RESPONSIBLEIMAGES).mid(1));
    QVariantList sImages = db_manager->retrieveRow(SmartClassGlobal::getTableName(SmartClassGlobal::STUDENTIMAGES),
                                                   SmartClassGlobal::getTableAliases(SmartClassGlobal::STUDENTIMAGES).mid(0, 1),
                                                   QVariantList() << studentData.at(0),
                                                   SmartClassGlobal::getTableAliases(SmartClassGlobal::STUDENTIMAGES).mid(1));

    pics[0] = new QPixmap(DBManager::variantToPixmap(sImages.at(0)));
    pics[1] = new QPixmap(DBManager::variantToPixmap(sImages.at(1)));
    pics[2] = new QPixmap(DBManager::variantToPixmap(rImages.at(0)));
    pics[3] = new QPixmap(DBManager::variantToPixmap(rImages.at(1)));
    pics[4] = new QPixmap(DBManager::variantToPixmap(rImages.at(2)));

    if (!pics[0]->isNull()) ui->lblStudentImage->setPixmap(*pics[0]);    

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

    QString crName = SmartClassGlobal::getTableName(SmartClassGlobal::COURSEDETAILS), cerName = SmartClassGlobal::getTableName(SmartClassGlobal::COURSEENROLLMENTS);
    QStringList studentCoursesRelationAliases, cAliases = SmartClassGlobal::getTableAliases(SmartClassGlobal::COURSEDETAILS),
            cerAliases = SmartClassGlobal::getTableAliases(SmartClassGlobal::COURSEENROLLMENTS);

    QString studentCoursesRelation = cerName + " INNER JOIN " + crName + " ON " + cerName + "." + cerAliases.at(0)
                                        + " = " + crName + "." + cAliases.at(0);

    studentCoursesRelationAliases << crName + "." + cAliases.at(0)
                                  << crName + "." + cAliases.at(1)
                                  << crName + "." + cAliases.at(5)
                                  << crName + "." + cAliases.at(6)
                                  << crName + "." + cAliases.at(7);

    QList< QVariantList > scRelation = db_manager->retrieveAllCond(studentCoursesRelation,
                                                                   studentCoursesRelationAliases,
                                                                   cerName + "." + cerAliases.at(1),
                                                                   STUDENT_ID);

    for (int i = 0; i < scRelation.size(); ++i){
        QString courseSyntesis = scRelation[i][1].toString() + tr(" ( class #")
                                    + QString::number(scRelation[i][2].toInt())
                                    + tr(" ) - ") + scRelation[i][3].toString()+ tr(" * starts on: ")
                                    + scRelation[i][4].toDate().toString(tr("dd/MM/yyyy"));
        QListWidgetItem *newItem = new QListWidgetItem(courseSyntesis);
        newItem->setData(Qt::UserRole, scRelation[i].at(0));
        ui->listCourses->addItem(newItem);
    }
    ui->listCourses->setCurrentRow(ui->listCourses->count() - 1);

    QString perName = SmartClassGlobal::getTableName(SmartClassGlobal::PAYMENTDETAILS);
    QStringList perAliases = SmartClassGlobal::getTableAliases(SmartClassGlobal::PAYMENTDETAILS), paymentDataAliases;
    QString paymentCoursesRelation = perName + " INNER JOIN " + crName + " ON " + perName + "." + perAliases.at(1)
                                        + " = " + crName + "." + cAliases.at(0);

    paymentDataAliases << crName + "." + cAliases.at(0)
                       << crName + "." + cAliases.at(1)
                       << crName + "." + cAliases.at(5)
                       << crName + "." + cAliases.at(6)
                       << crName + "." + cAliases.at(7)
                       << crName + "." + cAliases.at(9)
                       << perName + "." + perAliases.at(2)
                       << perName + "." + perAliases.at(3)
                       << perName + "." + perAliases.at(4);

    QList< QVariantList > paymentData = db_manager->retrieveAllCond(paymentCoursesRelation,
                                                                    paymentDataAliases,
                                                                    SmartClassGlobal::getTableAliases(SmartClassGlobal::PAYMENTDETAILS).at(0),
                                                                    STUDENT_ID);
    int paymentDataCount = paymentData.length();

    for (int i = 0; i < paymentDataCount; ++i){
        QString courseSyntesis = paymentData[i][1].toString() + tr(" ( class #")
                                    + QString::number(paymentData[i][2].toInt())
                                    + tr(" ) - ") + paymentData[i][3].toString()+ tr(" * starts on: ")
                                    + paymentData[i][4].toDate().toString(tr("dd/MM/yyyy"));

        QListWidgetPaymentItem *item = new QListWidgetPaymentItem(courseSyntesis, paymentData[i][5].toDouble());
        item->setData(Qt::UserRole, paymentData[i][0]);
        item->setDiscount(paymentData[i][6].toDouble());
        item->setDate(paymentData[i][7].toDate());
        item->setInstallments(paymentData[i][8].toInt());

        ui->listPaymentCourses->addItem(static_cast<QListWidgetItem*>(item));
    }
    ui->listPaymentCourses->setCurrentRow(ui->listPaymentCourses->count() - 1);

    this->courseQuantityChanged();
}

void frmManageStudent::addCourse(){
    if (!ui->cbRegistrationCourse->currentIndex()) return;

    for (int i = 0; i < ui->listCourses->count(); ++i){
        if (ui->listCourses->item(i)->data(Qt::UserRole) == ui->cbRegistrationCourse->currentData(Qt::UserRole)){
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
    QVariant itemID = item->data(Qt::UserRole);

    if (QMessageBox::question(this, tr("SmartClass | Confirmation"), tr("You are going to remove the student from the %1 course. Are you sure?").arg(itemText), QMessageBox::Yes, QMessageBox::No)
            == QMessageBox::No) return;

    for (int i = 0; i < ui->listPaymentCourses->count(); ++i){
        if (itemID == ui->listPaymentCourses->item(i)->data(Qt::UserRole)){
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
    QVariantList tempParentData;
    bool exists = db_manager->rowExists(SmartClassGlobal::getTableName(SmartClassGlobal::RESPONSIBLE),
                                        SmartClassGlobal::getTableAliases(SmartClassGlobal::RESPONSIBLE).at(1),
                                        pName);

    if (exists){
        tempParentData = db_manager->retrieveRow(SmartClassGlobal::getTableName(SmartClassGlobal::RESPONSIBLE),
                                                 SmartClassGlobal::getTableAliases(SmartClassGlobal::RESPONSIBLE).at(1),
                                                 pName);
    }
    else if (QMessageBox::question(this, tr("Inexistent parental data | SmartClass"),
                                   tr("We could not find any parent with this name in our database. Would you like to erase all the existent data related to the parent on this form?"),
                                   QMessageBox::Yes | QMessageBox::No) == QMessageBox::No){
                  return;
    }

    for (int i = 2; i < 5; ++i){
        delete pics[i];
        pics[i] = NULL;
    }

    if (exists){
        ui->edtParentPhone->setText(tempParentData.at(2).toString());
        ui->cbParentMobileOperator->setCurrentIndex(tempParentData.at(3).toInt());
        ui->edtParentMobile->setText(tempParentData.at(4).toString());
        ui->edtParentEmail->setText(tempParentData.at(5).toString());
        ui->edtParentID->setText(tempParentData.at(6).toString());
        ui->edtParentCPG->setText(tempParentData.at(7).toString());
        ui->cbParentIndication->setCurrentText(tempParentData.at(8).toString());

        QVariantList rows = db_manager->retrieveRow(SmartClassGlobal::getTableName(SmartClassGlobal::RESPONSIBLEIMAGES),
                                              QStringList() << SmartClassGlobal::getTableAliases(SmartClassGlobal::RESPONSIBLEIMAGES).at(0),
                                              QVariantList() << QVariant(tempParentData.at(0)),
                                              SmartClassGlobal::getTableAliases(SmartClassGlobal::RESPONSIBLEIMAGES).mid(1));

        if (rows.size() > 2){
            pics[2] = new QPixmap(db_manager->variantToPixmap(rows.at(0)));
            pics[3] = new QPixmap(db_manager->variantToPixmap(rows.at(1)));
            pics[4] = new QPixmap(db_manager->variantToPixmap(rows.at(2)));
        }
        else {
            pics[2] = new QPixmap();
            pics[3] = new QPixmap();
            pics[4] = new QPixmap();
        }
    }
    else {
        ui->edtParentPhone->setText("");
        ui->cbParentMobileOperator->setCurrentIndex(0);
        ui->edtParentMobile->setText("");
        ui->edtParentEmail->setText("");
        ui->edtParentID->setText("");
        ui->edtParentCPG->setText("");
        ui->cbParentIndication->setCurrentText(tr("Online advertisements"));

        pics[2] = new QPixmap();
        pics[3] = new QPixmap();
        pics[4] = new QPixmap();
    }
}
