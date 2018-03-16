#include "frmmanagestudent.h"
#include "ui_frmmanagestudent.h"

frmManageStudent::frmManageStudent(QWidget *parent, Role role, const QString &studentName, const QStringList &dbData) :
    QMainWindow(parent),
    ui(new Ui::frmManageStudent),
    RESIZE_LIMIT(2), CURRENT_ROLE(role),
    STUDENT_NAME(studentName)
{
    ui->setupUi(this);

    setWindowFlags(Qt::Widget | Qt::FramelessWindowHint);
    centralWidget()->installEventFilter(this);
    ui->titleBar->installEventFilter(this);
    ui->statusBar->installEventFilter(this);

    centralWidget()->setMouseTracking(true);
    ui->titleBar->setMouseTracking(true);
    ui->statusBar->setMouseTracking(true);

    setWindowTitle(tr("Manage Student | SmartClass"));
    locked = LockMoveType::None;

    ui->titleBar->setMaximizeButtonEnabled(false);

    this->setGeometry(QStyle::alignedRect(Qt::LeftToRight, Qt::AlignCenter,
            this->size(), qApp->desktop()->availableGeometry()));

    ui->lblDescriptionOfCourse->setVisible(false);
    ui->lblPaymentDetails->setVisible(false);

    /*
     * End of GUI operations
     */

    myDB = new DBManager(dbData, DBManager::getUniqueConnectionName("importExport"),
                         dbData.length() == 2 ? "SQLITE" : "MYSQL");

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

    connect(ui->listPaymentCourses, SIGNAL(currentRowChanged(int)), this, SLOT(changePaymentDetails()));

    connect(ui->listCourses, SIGNAL(currentRowChanged(int)), this, SLOT(enableRegistrationDetails(int)));

    myDB->openDB();

    studentsTable << "student" << "birthday" << "studentID" << "school" << "observations"
                    << "experimentalCourse" << "experimentalCourseDate" << "experimentalCourseObservations"
                    << "courses" << "address";
    coursesTable << "course" << "teacher" << "shortDescription" << "longDescription" << "class"
                 << "dayNTime" << "beginningDate" << "endDate" << "price" << "students";
    parentsTable << "parent" << "students" << "phone" << "mobileOperator" << "mobile" << "email"
                    << "parentID" << "parentCPG" << "meeting";
    studentImagesTable << "student" << "studentImage" << "studentID";
    parentImagesTable << "parent" << "parentID" << "parentCPG" << "addressComprobation";
    pricingTable << "student" << "course" << "discount" << "beginningDate" << "installments";

    courseData = NULL;
    paymentData = NULL;
    this->retrieveData();

    if (role == frmManageStudent::Create) ui->edtParentName->setCurrentText("");
    if (CURRENT_ROLE != frmManageStudent::View)
        connect(ui->edtParentName, SIGNAL(currentIndexChanged(QString)), this, SLOT(updateParentInfo(QString)));
}

frmManageStudent::~frmManageStudent()
{
    if (courseData) delete[] courseData;
    if (paymentData) delete[] paymentData;
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

    if (frmImgViewer != NULL){
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

    ui->edtRegistrationAddress->setEnabled(enable);
    ui->btStudentAddressConfirmationPicture->setEnabled(enable);
    ui->lblDescriptionOfCourse->setEnabled(enable);
    ui->listCourses->setEnabled(enable);
    ui->lblRegistrationAddress->setEnabled(enable);
}

void frmManageStudent::saveData(){
    QString errorMessage = tr("Well, unfortunately we cannot proceed. Some data are either inconsistent or inexistent."
                              " Please, fix the folowing issues before trying to save again:\n");
    bool error = false;

    if (ui->edtStudentName->text().isEmpty()){
        errorMessage += tr("\n->The name of the student cannot be empty;");
        error = true;
    }
    else if (myDB->lineExists("myclass_students", "student", ui->edtStudentName->text()) && CURRENT_ROLE == frmManageStudent::Create){
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
        courses << ui->listCourses->item(i)->text();

    QStringList studentInfo, parentInfo, courseInfo;
    studentInfo << ui->edtStudentName->text()
                << ui->edtStudentBirthday->date().toString("dd/MM/yyyy")
                << ui->edtStudentID->text()
                << ui->edtStudentSchool->text()
                << ui->edtStudentObservations->toPlainText()
                << ui->cbExperimentalClassCourse->currentText()
                << ui->edtExperimentalClassDateTime->dateTime().toString("dd/MM/yyyy HH:mm")
                << ui->edtExperimentalClassObservations->toPlainText()
                << courses.join("|")
                << ui->edtRegistrationAddress->text();

    parentInfo << ui->edtParentName->currentText()
               << ui->edtParentPhone->text()
               << QString::number(ui->cbParentMobileOperator->currentIndex())
               << ui->edtParentMobile->text()
               << ui->edtParentEmail->text()
               << ui->edtParentID->text()
               << ui->edtParentCPG->text()
               << ui->cbParentIndication->currentText();

    if (CURRENT_ROLE == frmManageStudent::Create){
        myDB->addLine("myclass_students", studentsTable, studentInfo);
        parentalData = myDB->retrieveLine("myclass_parents", "parent", parentInfo.at(0));

        if (!myDB->lineExists("myclass_parents", "parent", parentInfo.at(0))){
            parentInfo.insert(1, studentInfo.at(0));
            myDB->addLine("myclass_parents", parentsTable, parentInfo);
        }
        else {
            QStringList students = QString(parentalData.at(1)).split("|");
            students << studentInfo.at(0);
            parentInfo.insert(1, students.join("|"));
            myDB->updateLine("myclass_parents", parentsTable, parentInfo, "parent", parentalData.at(0));
        }

        for (int i = 0; i < courseDataCount; ++i){
            QString courseSyntesis = courseData[i].at(0) + " [ " + courseData[i].at(4) + " ] - " + courseData[i].at(5)
                            + tr(" * starts on: ") + courseData[i].at(6);
            for (int j = 0; j < ui->listCourses->count(); ++j){
                if (courseSyntesis == ui->listCourses->item(j)->text()){
                    courseInfo = courseData[i];
                    courseInfo.replace(9, courseInfo.at(9).isEmpty() ? studentInfo.at(0) : (QString(courseInfo.at(9)).split("|") << studentInfo.at(0)).join("|"));
                    myDB->updateLine("myclass_courses", coursesTable, courseInfo, "course", courseData[i].at(0));
                }
            }
        }

        while (myDB->lineExists("myclass_pricing", "student", studentInfo.at(0)))
            myDB->removeLine("myclass_pricing", "student", studentInfo.at(0));

        for (int i = 0; i < ui->listPaymentCourses->count(); ++i){
            QListWidgetPaymentItem* item =  static_cast<QListWidgetPaymentItem*>(ui->listPaymentCourses->item(i));
            QStringList pricingInfo = QStringList() << studentInfo.at(0)
                                                    << item->text()
                                                    << QString::number(ui->sbDiscount->value())
                                                    << ui->edtPaymentFirstInstallment->date().toString("dd/MM/yyyy")
                                                    << QString::number(ui->sbInstallments->value());
            myDB->addLine("myclass_pricing", pricingTable, pricingInfo);
        }

        myDB->addImage("myclass_simages", studentImagesTable.at(1), "student", studentInfo.at(0), *pics[0]);
        myDB->updateImage("myclass_simages", studentImagesTable.at(2), *pics[1], "student", studentInfo.at(0));

        if (myDB->lineExists("myclass_pimages", "parent", parentInfo.at(0)))
            myDB->updateImage("myclass_pimages", parentImagesTable.at(1), *pics[2], "parent",parentInfo.at(0));
        else myDB->addImage("myclass_pimages", parentImagesTable.at(1), "parent", parentInfo.at(0), *pics[2]);
        myDB->updateImage("myclass_pimages", parentImagesTable.at(2), *pics[3], "parent", studentInfo.at(0));
        myDB->updateImage("myclass_pimages", parentImagesTable.at(3), *pics[4], "parent", studentInfo.at(0));

        emit newData(QStringList() << studentInfo.at(0) << parentInfo.at(0) << studentInfo.at(3) << studentInfo.at(4));
        this->close();
        return;
    }

    if (parentInfo.at(0) == parentalData.at(0) || QString(parentalData.at(1)).split("|", QString::SkipEmptyParts).count() == 1){
        parentInfo.insert(1, QString(parentalData.at(1)).replace(studentData.at(0), studentInfo.at(0)));
        myDB->updateLine("myclass_parents", parentsTable, parentInfo, "parent", parentalData.at(0));
    }
    else if (QMessageBox::information(this, tr("Parent data changed | SmartClass"),
                                      tr("Looks like the name of the parent has changed. Would you like to mantain this changes (note that all the kids who have this person as parent are going to change their parents)?"),
                                      QMessageBox::Yes, QMessageBox::No) == QMessageBox::Yes){
        parentInfo.insert(1, QString(parentalData.at(1)).replace(studentData.at(0), studentInfo.at(0)));
        myDB->updateLine("myclass_parents", parentsTable, parentInfo, "parent", parentalData.at(0));
    }
    else {
        parentInfo.replace(0, parentalData.at(0));
        parentInfo.insert(1, QString(parentalData.at(1)).replace(studentData.at(0), studentInfo.at(0)));
        myDB->updateLine("myclass_parents", parentsTable, parentInfo, "parent", parentalData.at(0));
    }

    for (int i = 0; i < courseDataCount; ++i){
        bool update = false;
        QString courseSyntesis = courseData[i].at(0) + " [ " + courseData[i].at(4) + " ] - " + courseData[i].at(5)
                        + tr(" * starts on: ") + courseData[i].at(6);
        if (QString(courseData[i].at(9)).contains(STUDENT_NAME)){
            courseInfo = courseData[i];
            QStringList studentsInCourse = QString(courseInfo.at(9)).split("|");
            studentsInCourse.removeOne(STUDENT_NAME);
            courseInfo.replace(9, studentsInCourse.join("|"));
            update = true;
        }
        for (int j = 0; j < ui->listCourses->count(); ++j){
            if (courseSyntesis == ui->listCourses->item(j)->text()){
                if (!update) courseInfo = courseData[i];
                courseInfo.replace(9, QString(courseInfo.at(9)).isEmpty() ? studentInfo.at(0) : (QString(courseInfo.at(9)).split("|") << studentInfo.at(0)).join("|"));
                update = true;
            }
        }
        if (update) myDB->updateLine("myclass_courses", coursesTable, courseInfo, "course", courseData[i].at(0));
        update = false;
    }

    while (myDB->lineExists("myclass_pricing", "student", STUDENT_NAME))
        myDB->removeLine("myclass_pricing", "student", STUDENT_NAME);

    for (int i = 0; i < ui->listPaymentCourses->count(); ++i){
        QListWidgetPaymentItem* item =  static_cast<QListWidgetPaymentItem*>(ui->listPaymentCourses->item(i));
        QStringList pricingInfo = QStringList() << studentInfo.at(0)
                                                << item->text()
                                                << QString::number(item->discount())
                                                << item->date().toString("dd/MM/yyyy")
                                                << QString::number(item->installments());
        myDB->addLine("myclass_pricing", pricingTable, pricingInfo);
    }

    myDB->updateLine("myclass_students", studentsTable, studentInfo, "student", STUDENT_NAME);

    if (myDB->lineExists("myclass_simages", "student", STUDENT_NAME))
        myDB->updateLine("myclass_simages", QStringList() << studentImagesTable.at(0),
                        QStringList() << studentInfo.at(0), "student", STUDENT_NAME);
    if (myDB->lineExists("myclass_simages", "student", studentInfo.at(0)))
        myDB->updateImage("myclass_simages", studentImagesTable.at(1), *pics[0], "student", studentInfo.at(0));
    else myDB->addImage("myclass_simages", studentImagesTable.at(1), "student", studentInfo.at(0), *pics[0]);
    if (myDB->lineExists("myclass_simages", "student", studentInfo.at(0)))
        myDB->updateImage("myclass_simages", studentImagesTable.at(2), *pics[1], "student", studentInfo.at(0));
    else myDB->addImage("myclass_simages", studentImagesTable.at(2), "student", studentInfo.at(0), *pics[1]);

    if (myDB->lineExists("myclass_pimages", "parent", parentalData.at(0)))
        myDB->updateLine("myclass_pimages", QStringList() << parentImagesTable.at(0),
                        QStringList() << parentInfo.at(0), "parent", parentalData.at(0));
    if (myDB->lineExists("myclass_pimages", "parent", parentInfo.at(0)))
        myDB->updateImage("myclass_pimages", parentImagesTable.at(1), *pics[2], "parent", parentInfo.at(0));
    else myDB->addImage("myclass_pimages", parentImagesTable.at(1), "parent", parentInfo.at(0), *pics[2]);
    if (myDB->lineExists("myclass_pimages", "parent", parentInfo.at(0)))
        myDB->updateImage("myclass_pimages", parentImagesTable.at(2), *pics[3], "parent", parentInfo.at(0));
    else myDB->addImage("myclass_pimages", parentImagesTable.at(2), "parent", parentInfo.at(0), *pics[3]);
    if (myDB->lineExists("myclass_pimages", "parent", parentInfo.at(0)))
        myDB->updateImage("myclass_pimages", parentImagesTable.at(3), *pics[4], "parent", parentInfo.at(0));
    else myDB->addImage("myclass_pimages", parentImagesTable.at(3), "parent", parentInfo.at(0), *pics[4]);

    emit updatedData(QStringList() << studentInfo.at(0) << parentInfo.at(0) << studentInfo.at(3) << studentInfo.at(4),
                     STUDENT_NAME);
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
    ui->sbDiscount->setValue(0);
    ui->edtPaymentFirstInstallment->setDate(QDate(2000, 01, 01));
    ui->sbInstallments->setValue(1);
    ui->lblPaymentDetails->setVisible(false);

    if (CURRENT_ROLE != frmManageStudent::Create) retrieveData();
}

void frmManageStudent::retrieveData(){
    if (courseData != NULL){
        delete[] courseData;
        courseData = NULL;

        while (ui->cbExperimentalClassCourse->count() != 1)
            ui->cbExperimentalClassCourse->removeItem(0);

        while (ui->cbRegistrationCourse->count() != 1)
            ui->cbRegistrationCourse->removeItem(0);
    }
    courseData = myDB->retrieveAll("myclass_courses", coursesTable);
    courseDataCount = myDB->rowsCount("myclass_courses");

    for (int i = 0; i < courseDataCount; ++i){
        if (QDate::fromString(courseData[i].at(7), "dd/MM/yyyy").toJulianDay() >= QDate::currentDate().toJulianDay()){
            QString courseSyntesis = courseData[i].at(0) + " [ " + courseData[i].at(4) + " ] - " + courseData[i].at(5)
                            + tr(" * starts on: ") + courseData[i].at(6);
            ui->cbRegistrationCourse->addItem(courseSyntesis);
            ui->cbExperimentalClassCourse->addItem(courseSyntesis);
        }
    }

    QStringList *allParentData = myDB->retrieveAll("myclass_parents", QStringList() << "parent");
    if (allParentData){
        allParentData[0].sort(Qt::CaseInsensitive);
        ui->edtParentName->addItems(allParentData[0]);
    }

    if (CURRENT_ROLE == frmManageStudent::Create) return;
    if (CURRENT_ROLE == frmManageStudent::View) ui->screenManagerLayout->setEnabled(false);

    for (int i = 0; i < 5; ++i){
        delete pics[i];
        pics[i] = NULL;
    }

    studentData = myDB->retrieveLine("myclass_students", "student", STUDENT_NAME);
    parentalData = myDB->retrieveLine("myclass_parents", "students", "%" + STUDENT_NAME + "%", "LIKE");

    pics[0] = new QPixmap(myDB->retrieveImage("myclass_simages", "studentImage", "student", STUDENT_NAME));
    pics[1] = new QPixmap(myDB->retrieveImage("myclass_simages", "studentID", "student", STUDENT_NAME));
    pics[2] = new QPixmap(myDB->retrieveImage("myclass_pimages", "parentID", "parent", parentalData.at(0)));
    pics[3] = new QPixmap(myDB->retrieveImage("myclass_pimages", "parentCPG", "parent", parentalData.at(0)));
    pics[4] = new QPixmap(myDB->retrieveImage("myclass_pimages", "addressComprobation", "parent", parentalData.at(0)));

    if (!pics[0]->isNull()) ui->lblStudentImage->setPixmap(*pics[0]);    

    if (paymentData != NULL){
        delete[] paymentData;
        paymentData = NULL;
    }

    paymentData = myDB->retrieveAllCondS("myclass_pricing", "student", STUDENT_NAME);
    paymentDataCount = myDB->rowsCountCond("myclass_pricing", "student", STUDENT_NAME);

    ui->edtStudentName->setText(studentData.at(0));
    ui->edtStudentBirthday->setDate(QDate::fromString(studentData.at(1), "dd/MM/yyyy"));
    ui->edtStudentID->setText(studentData.at(2));
    ui->edtStudentSchool->setText(studentData.at(3));
    ui->edtStudentObservations->setPlainText(studentData.at(4));
    ui->cbExperimentalClassCourse->setCurrentText(studentData.at(5));
    ui->edtExperimentalClassDateTime->setDateTime(QDateTime::fromString(studentData.at(6), "dd/MM/yyyy HH:mm"));
    ui->edtExperimentalClassObservations->setPlainText(studentData.at(7));
    ui->edtRegistrationAddress->setText(studentData.at(9));

    ui->edtParentName->setCurrentText(parentalData.at(0));
    ui->edtParentPhone->setText(parentalData.at(2));
    ui->cbParentMobileOperator->setCurrentIndex(QString(parentalData.at(3)).toInt());
    ui->edtParentMobile->setText(parentalData.at(4));
    ui->edtParentEmail->setText(parentalData.at(5));
    ui->edtParentID->setText(parentalData.at(6));
    ui->edtParentCPG->setText(parentalData.at(7));
    ui->cbParentIndication->setCurrentText(parentalData.at(8));

    QStringList studentCourses = QString(studentData.at(8)).split("|", QString::SkipEmptyParts);

    for (int i = 0; i < studentCourses.length(); ++i){
        ui->listCourses->addItem(studentCourses.at(i));
        ui->listCourses->setCurrentRow(ui->listCourses->count() - 1);
    }

    for (int i = 0; i < paymentDataCount; ++i){
        for (int k = 0; k < courseDataCount; k++){
            QString courseSyntesis = courseData[k].at(0) + " [ " + courseData[k].at(4) + " ] - " + courseData[k].at(5)
                            + tr(" * starts on: ") + courseData[k].at(6);
            if (courseSyntesis == paymentData[i].at(1)){
                QListWidgetPaymentItem *item;
                item = new QListWidgetPaymentItem(paymentData[i].at(1),
                                                    QString(courseData[k].at(8)).toDouble());
                item->setDiscount(QString(paymentData[i].at(2)).toInt());
                item->setDate(QDate::fromString(paymentData[i].at(3), "dd/MM/yyyy"));
                item->setInstallments(QString(paymentData[i].at(4)).toInt());

                ui->listPaymentCourses->addItem(static_cast<QListWidgetItem*>(item));
                ui->listPaymentCourses->setCurrentRow(ui->listPaymentCourses->count() - 1);
            }
        }
    }

    this->courseQuantityChanged();
}

void frmManageStudent::addCourse(){
    if (ui->cbRegistrationCourse->currentIndex() == 0) return;

    for (int i = 0; i < ui->listCourses->count(); ++i){
        if (ui->listCourses->item(i)->text() == ui->cbRegistrationCourse->currentText()){
            QMessageBox::information(this, tr("Warning | SmartClass"), tr("This student is already registered on this course!"), QMessageBox::Ok, QMessageBox::Ok);
            return;
        }
    }

    ui->listCourses->addItem(ui->cbRegistrationCourse->currentText());
    QListWidgetPaymentItem *item = new QListWidgetPaymentItem(ui->cbRegistrationCourse->currentText(),
                                                              QString(courseData[ui->cbRegistrationCourse->currentIndex() - 1].at(8)).toDouble());
    ui->listPaymentCourses->addItem(static_cast<QListWidgetItem*>(item));

    ui->listCourses->setCurrentRow(ui->listCourses->count() - 1);
    ui->listPaymentCourses->setCurrentRow(ui->listPaymentCourses->count() - 1);
    this->courseQuantityChanged();
}

void frmManageStudent::removeCourse(){
    if (ui->listCourses->currentRow() < 0) return;

    QListWidgetItem* item = ui->listCourses->item(ui->listCourses->currentRow());
    QString itemText = item->text();

    if (QMessageBox::question(this, tr("SmartClass | Confirmation"), tr("You are going to remove the student from the %1 course. Are you sure?").arg(itemText), QMessageBox::Yes, QMessageBox::No)
            == QMessageBox::No) return;

    for (int i = 0; i < ui->listPaymentCourses->count(); ++i){
        if (itemText == ui->listPaymentCourses->item(i)->text()){
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
    ui->lblRegistrationAddress->setEnabled(enable);
    ui->btRemoveCourse->setEnabled(enable);
    ui->btStudentAddressConfirmationPicture->setEnabled(enable);
    ui->edtRegistrationAddress->setEnabled(enable);
    ui->grpPaymentData->setEnabled(enable);
}

void frmManageStudent::changePaymentDetails(){
    blockPaymentUpdate = true;
    if (ui->listPaymentCourses->currentRow() < 0){
        ui->edtPaymentFirstInstallment->setDate(QDate(2000, 01, 01));
        ui->sbDiscount->setValue(0);
        ui->sbInstallments->setValue(1);
        ui->sbPaymentCost->setValue(0.00);
        ui->lblPaymentDetails->setVisible(false);
        blockPaymentUpdate = false;
        return;
    }

    QListWidgetPaymentItem* item = static_cast<QListWidgetPaymentItem*>(ui->listPaymentCourses->item(ui->listPaymentCourses->currentRow()));
    ui->edtPaymentFirstInstallment->setDate(item->date());
    ui->sbDiscount->setValue(item->discount());
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

void frmManageStudent::courseIndexChanged(){
    if (ui->listPaymentCourses->currentRow() < 0){
        ui->lblDescriptionOfCourse->setVisible(false);
        return;
    }
    QListWidgetItem* item = ui->listCourses->item(ui->listCourses->currentRow());
    for (int i = 1; i < ui->listCourses->count() - 1; ++i){
        if (item->text().contains(courseData[i].at(0)) && item->text().contains(courseData[i].at(4))
                && item->text().contains(courseData[i].at(5)) && item->text().contains(courseData[i].at(6)))
            ui->lblDescriptionOfCourse->setText(courseData[i].at(2));
    }
}

void frmManageStudent::updateParentInfo(const QString &pName){
    if (pName.isEmpty() || pName.isNull()) return;
    QStringList tempParentData;
    bool exists = false;
    if (myDB->lineExists("myclass_parents", "parent", pName)){
        tempParentData = myDB->retrieveLine("myclass_parents", "parent", pName);
        exists = true;
    }

    if (!exists)
        if (QMessageBox::question(this, tr("Inexistent parental data | SmartClass"),
                              tr("We could not find any parent with this name in our database. Would you like to erase all the existent data related to the parent on this form?"),
                              QMessageBox::Yes | QMessageBox::No) == QMessageBox::No)
            return;

    ui->edtParentPhone->setText(exists ? tempParentData.at(2) : "");
    ui->cbParentMobileOperator->setCurrentIndex(QString(exists ? tempParentData.at(3) : "0").toInt());
    ui->edtParentMobile->setText(exists ? tempParentData.at(4) : "");
    ui->edtParentEmail->setText(exists ? tempParentData.at(5) : "");
    ui->edtParentID->setText(exists ? tempParentData.at(6) : "");
    ui->edtParentCPG->setText(exists ? tempParentData.at(7) : "");
    ui->cbParentIndication->setCurrentText(exists ? tempParentData.at(8) : tr("Online advertisements"));

    for (int i = 2; i < 5; ++i){
        delete pics[i];
        pics[i] = NULL;
    }

    if (exists){
        pics[2] = new QPixmap(myDB->retrieveImage("myclass_pimages", "parentID", "parent", tempParentData.at(0)));
        pics[3] = new QPixmap(myDB->retrieveImage("myclass_pimages", "parentCPG", "parent", tempParentData.at(0)));
        pics[4] = new QPixmap(myDB->retrieveImage("myclass_pimages", "addressComprobation", "parent", tempParentData.at(0)));
    }
    else {
        pics[2] = new QPixmap();
        pics[3] = new QPixmap();
        pics[4] = new QPixmap();
    }
}

/*
 * GUI Functions (don't change, unless necessary)
 */

void frmManageStudent::mousePressEvent(QMouseEvent *event)
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

void frmManageStudent::undefMouseMoveEvent(QObject* object, QMouseEvent* event){
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

void frmManageStudent::mouseReleaseEvent(QMouseEvent *event){
    locked = LockMoveType::None;
    event->accept();
}

bool frmManageStudent::eventFilter(QObject* object, QEvent* event)
{
    if(event->type() == QEvent::MouseMove)
        undefMouseMoveEvent(object, static_cast<QMouseEvent*>(event));
    else if (event->type() == QEvent::MouseButtonPress && object->objectName() == "titleBar"){
        mousePressEvent(static_cast<QMouseEvent*>(event));
    }
    return false;
}

const QString frmManageStudent::getDBPath(){
    QString tempDir = QDir::homePath();
    if (QSysInfo::windowsVersion() != QSysInfo::WV_None)
        tempDir += "/AppData/Roaming/Nintersoft/SmartClass/";
    else tempDir += "/.Nintersoft/SmartClass/";

    QString dataDir = tempDir + "data/";
    if (!QDir(dataDir).exists()) QDir(dataDir).mkpath(dataDir);

    return tempDir + "data/classInfo.db";
}
