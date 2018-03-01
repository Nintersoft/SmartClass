#include "frmimportexportdb.h"
#include "ui_frmimportexportdb.h"

frmImportExportDB::frmImportExportDB(QWidget *parent, ImportMode mode, const QStringList &dbData) :
    QMainWindow(parent),
    ui(new Ui::frmImportExportDB),
    RESIZE_LIMIT(2),
    CURRENT_MODE(mode)
{
    ui->setupUi(this);
    setWindowFlags(Qt::Widget | Qt::FramelessWindowHint);
    centralWidget()->installEventFilter(this);
    ui->titleBar->installEventFilter(this);
    ui->statusBar->installEventFilter(this);

    centralWidget()->setMouseTracking(true);
    ui->titleBar->setMouseTracking(true);
    ui->statusBar->setMouseTracking(true);

    setWindowTitle("Import/Export utility | SmartClass");
    locked = LockMoveType::None;

    ui->titleBar->setMaximizeButtonEnabled(false);

    /*
     *  End of GUI implementation
     */

    myDB = new DBManager(dbData, DBManager::getUniqueConnectionName("importExport"),
                         dbData.length() == 2 ? "SQLITE" : "MYSQL");

    if (CURRENT_MODE == frmImportExportDB::Export){
        ui->grpImportOptions->setVisible(false);
        ui->btImport->setVisible(false);
        this->setMaximumHeight(552);
        this->setMinimumHeight(552);
        this->setGeometry(QRect(this->geometry().topLeft(), QSize(this->width(), 552)));


        connect(ui->cbExportImages, SIGNAL(toggled(bool)), ui->cbStudentImages, SLOT(setEnabled(bool)));
        connect(ui->cbExportImages, SIGNAL(toggled(bool)), ui->cbParentImages, SLOT(setEnabled(bool)));

        connect(ui->cbExportImages, SIGNAL(toggled(bool)), ui->grpExportImages, SLOT(setEnabled(bool)));
        connect(ui->btSearchDirectory, SIGNAL(clicked(bool)), this, SLOT(getExportDir()));

        connect(ui->btExport, SIGNAL(clicked(bool)), this, SLOT(exportDB()));
    }
    else{
        ui->grpExportOptions->setVisible(false);
        ui->btExport->setVisible(false);
        this->setMaximumHeight(409);
        this->setMinimumHeight(409);
        this->setGeometry(QRect(this->geometry().topLeft(), QSize(this->width(), 409)));

        connect(ui->btSearchStudentFile, SIGNAL(clicked(bool)), this, SLOT(getImportFilePath()));
        connect(ui->btSearchParentFile, SIGNAL(clicked(bool)), this, SLOT(getImportFilePath()));
        connect(ui->btSearchCoursesFile, SIGNAL(clicked(bool)), this, SLOT(getImportFilePath()));
        connect(ui->btSearchPaymentFile, SIGNAL(clicked(bool)), this, SLOT(getImportFilePath()));
        connect(ui->btSearchUsersFile, SIGNAL(clicked(bool)), this, SLOT(getImportFilePath()));

        connect(ui->btSearchStudentImagesDir, SIGNAL(clicked(bool)), this, SLOT(getImportDir()));
        connect(ui->btSearchParentImagesDir, SIGNAL(clicked(bool)), this, SLOT(getImportDir()));

        connect(ui->btClearStudentsPath, SIGNAL(clicked(bool)), ui->edtStudentFilePath, SLOT(clear()));
        connect(ui->btClearParentsPath, SIGNAL(clicked(bool)), ui->edtParentFilePath, SLOT(clear()));
        connect(ui->btClearCoursesPath, SIGNAL(clicked(bool)), ui->edtCoursesFilePath, SLOT(clear()));
        connect(ui->btClearPricingPath, SIGNAL(clicked(bool)), ui->edtPaymentFilePath, SLOT(clear()));
        connect(ui->btClearUsersPath, SIGNAL(clicked(bool)), ui->edtUsersFilePath, SLOT(clear()));
        connect(ui->btClearPImagesPath, SIGNAL(clicked(bool)), ui->edtParentImagesDirPath, SLOT(clear()));
        connect(ui->btClearSImagesPath, SIGNAL(clicked(bool)), ui->edtStudentImagesDirPath, SLOT(clear()));

        connect(ui->btImport, SIGNAL(clicked(bool)), this, SLOT(importDB()));
    }

    this->setGeometry(QStyle::alignedRect(Qt::LeftToRight, Qt::AlignCenter,
            this->size(), qApp->desktop()->availableGeometry()));

    myDB->openDB();

    tableColumns = new QStringList[7];
    tableColumns[0] << "student" << "birthday" << "studentID" << "school" << "observations"
                    << "experimentalCourse" << "experimentalCourseDate" << "experimentalCourseObservations"
                    << "courses" << "address";
    tableColumns[1] << "parent" << "students" << "phone" << "mobileOperator" << "mobile" << "email"
                    << "parentID" << "parentCPG" << "meeting";
    tableColumns[2] << "course" << "teacher" << "shortDescription" << "longDescription" << "class"
                    << "dayNTime" << "beginningDate" << "endDate" << "price" << "students";
    tableColumns[3] << "student" << "course" << "discount" << "beginningDate" << "installments";
    tableColumns[4] << "username" << "name" << "salt" << "hash" << "question"
                    << "answerSalt" << "hashAnswer" << "role";
    tableColumns[5] << "student" << "studentImage" << "studentID";
    tableColumns[6] << "parent" << "parentID" << "parentCPG" << "addressComprobation";

    currentImportDir = QDir::home();
}

frmImportExportDB::~frmImportExportDB()
{
    if (myDB->isOpen()) myDB->closeDB();
    delete myDB;
    delete ui;
}

void frmImportExportDB::importDB(){
    QStringList paths, tables;
    paths << ui->edtStudentFilePath->text()
          << ui->edtParentFilePath->text()
          << ui->edtCoursesFilePath->text()
          << ui->edtPaymentFilePath->text()
          << ui->edtUsersFilePath->text()
          << ui->edtStudentImagesDirPath->text()
          << ui->edtParentImagesDirPath->text();

    tables << "myclass_students"
           << "myclass_parents"
           << "myclass_courses"
           << "myclass_pricing"
           << "myclass_users"
           << "myclass_simages"
           << "myclass_pimages";

    bool importErrorsB = false;
    bool checkForErrors = ui->cbCheckForErrors->isChecked();

    for (int i = 0; i < 5; ++i){
        QString importErrors = "";
        if (!paths.at(i).isEmpty()){
            if (!QFile::exists(paths.at(i))){
                QString argument = QString(tables.at(i)).split("_").at(1);
                QMessageBox::critical(this, tr("Error | SmartClass"),
                                                      tr("The path to the %1 file does not exists. This file has been skipped during the process of importation.").arg(argument),
                                                      QMessageBox::Ok, QMessageBox::NoButton);
                importErrors += tr("The path to the %1 file does not exists. This file has been skipped during the process of importation.").arg(argument);
                importErrorsB = true;
            }
            else {
                QFile file(paths.at(i));
                file.open(QIODevice::ReadOnly);
                QString content = QString::fromLocal8Bit(file.readAll());
                file.close();

                QStringList lines = content.split(QRegExp("(\\n|\\r\\n)"));
                int linesCount = lines.length() - 1;
                if (linesCount < 5){
                    QMessageBox::warning(this, tr("File error | SmartClass"),
                                         tr("The quantity of lines does not match with the minimal requirements to import into %1 table. This file has been skipped.").arg(tables.at(i)),
                                         QMessageBox::Ok, QMessageBox::NoButton);
                    importErrors += tr("The quantity of lines does not match with the minimal requirements to import into %1 table. This file has been skipped.").arg(tables.at(i));
                    importErrorsB = true;
                }
                else {
                    for (int k = 4; k < linesCount; ++k){
                        QStringList lineContent;
                        lineContent = QString(lines.at(k)).split(";");
                        if (checkForErrors){
                            if (myDB->lineExists(tables.at(i), tableColumns[i], lineContent)){
                                importErrors += tr("[SKIPPED] The following line already exists: %1 . Line skipped.\n").arg(lines.at(k));
                                importErrorsB = true;
                            }
                            else if (i != 3 && myDB->lineExists(tables.at(i), QString(tableColumns[i].at(0)), QString(lineContent.at(0)))){
                                QString property = QString(QString(tables.at(0)).split("_").at(1));
                                property.remove(property.length() - 1, 1);
                                importErrors += tr("[UPDATED / WARNING] The following %1 already exists with different values: %2 . It has been updated to the content of the CSV file.\n").arg(property).arg(lines.at(k));
                                importErrorsB = true;
                                if (!myDB->updateLine(tables.at(i), tableColumns[i], lineContent, QString(tableColumns[i].at(0)), QString(lineContent.at(0)))){
                                    importErrors += tr("[ERROR] The following line could not be added to the database: %1 . Please, ignore the previous update warning.\n").arg(lines.at(k));
                                    importErrorsB = true;
                                }
                            }
                            else {
                                if (!myDB->addLine(tables.at(i), tableColumns[i], lineContent)){
                                    importErrors += tr("[ERROR] The following line could not be added to the database: %1 .\n").arg(lines.at(k));
                                    importErrorsB = true;
                                }
                            }
                        }
                        else myDB->addLine(tables.at(i), tableColumns[i], lineContent);
                    }
                }
            }
        }

        if (importErrorsB && checkForErrors){
            QFileInfo fInfo(paths.at(i));
            QString parentPath = fInfo.filePath();
            parentPath += "log.txt";

            QFile logFile(parentPath);
            logFile.open(QIODevice::WriteOnly | QIODevice::Text);
            logFile.write(importErrors.toUtf8());
            logFile.close();
        }

    }

    if (!paths.at(5).isEmpty()){
        QString importErrors = "";

        QDir imagesPath(paths.at(5));
        if (imagesPath.exists()){
            QStringList directories = imagesPath.entryList(QDir::Dirs, QDir::Name);
            directories.removeOne(".");
            directories.removeOne("..");
            for (int i = 0; i < directories.length(); ++i){
                if (myDB->lineExists("myclass_simages", "student", directories.at(i)) && checkForErrors){
                    importErrors += tr("[OVERWRITTEN] The the following student \"%1\" already exists in the database. Every image in the register have been overwritten by the existent images in the import directory.\n").arg(directories.at(0));
                    importErrorsB = true;
                }
                QString parentPath = paths.at(5) + QDir::separator() + directories.at(i) + QDir::separator();
                if (QDir(parentPath).exists()){
                    QString imagePath = parentPath + "studentImage.png";
                    QFile currentImage(imagePath);
                    if (currentImage.exists()){
                        currentImage.open(QIODevice::ReadOnly);
                        if (myDB->lineExists("myclass_simages", "student", directories.at(i))){
                            if (!myDB->updateImage("myclass_simages", "studentImage", QPixmap(imagePath, "PNG"), "student", directories.at(i))
                                    && checkForErrors){
                                importErrors += tr("[BROKEN] The the following file \"%1\" could not be added to the database.\n").arg(imagePath);
                                importErrorsB = true;
                            }
                        }
                        else if (!myDB->addImage("myclass_simages", "studentImage", "student", directories.at(i), QPixmap(imagePath, "PNG"))
                                 && checkForErrors){
                            importErrors += tr("[BROKEN] The the following file \"%1\" could not be added to the database.\n").arg(imagePath);
                            importErrorsB = true;
                        }
                        currentImage.close();
                    }
                    else if (checkForErrors){
                        importErrors += tr("[SKIPPED] The the following file \"%1\" does not exists.\n").arg(imagePath);
                        importErrorsB = true;
                    }

                    imagePath = parentPath + "studentID.png";
                    currentImage.setFileName(imagePath);
                    if (currentImage.exists()){
                        currentImage.open(QIODevice::ReadOnly);
                        if (!myDB->updateImage("myclass_simages", "studentID", QPixmap(imagePath, "PNG"), "student", directories.at(i))
                                && checkForErrors){
                            importErrors += tr("[BROKEN] The the following file \"%1\" could not be added to the database.\n").arg(imagePath);
                            importErrorsB = true;
                        }
                        currentImage.close();
                    }
                    else if (checkForErrors){
                        importErrors += tr("[SKIPPED] The the following file \"%1\" does not exists.\n").arg(imagePath);
                        importErrorsB = true;
                    }

                }
                else if (checkForErrors){
                    importErrors += tr("[SKIPPED] The the following directory \"%1\" does not exists.\n").arg(parentPath);
                    importErrorsB = true;
                }
            }
        }
        else if (checkForErrors){
            importErrors += tr("[UNAVAILABLE] The selected directory does not exists.\n");
            importErrorsB = true;
        }

        if (importErrorsB && checkForErrors){
            QString parentPath = (paths.at(5) + QString(QDir::separator()) + "log.txt");

            QFile logFile(parentPath);
            logFile.open(QIODevice::WriteOnly | QIODevice::Text);
            logFile.write(importErrors.toLocal8Bit());
            logFile.close();
        }
    }

    if (!paths.at(6).isEmpty()){
        QString importErrors;
        bool importErrorsB;

        QDir imagesPath(paths.at(6));
        if (imagesPath.exists()){
            QStringList directories = imagesPath.entryList(QDir::Dirs, QDir::Name);
            directories.removeOne(".");
            directories.removeOne("..");
            for (int i = 0; i < directories.length(); ++i){
                if (myDB->lineExists("myclass_pimages", "parent", directories.at(i)) && checkForErrors){
                    importErrors += tr("[OVERWRITTEN] The the following parent \"%1\" already exists in the database. Every image in the register have been overwritten by the existent images in the import directory.\n").arg(directories.at(0));
                    importErrorsB = true;
                }
                QString parentPath = paths.at(6) + QDir::separator() + directories.at(i) + QDir::separator();
                if (QDir(parentPath).exists()){
                    QString imagePath = parentPath + "parentID.png";
                    QFile currentImage(imagePath);
                    if (currentImage.exists()){
                        currentImage.open(QIODevice::ReadOnly);
                        if (myDB->lineExists("myclass_pimages", "parent", directories.at(i))){
                            if (!myDB->updateImage("myclass_pimages", "parentID", QPixmap(imagePath, "PNG"), "parent", directories.at(i))
                                    && checkForErrors){
                                importErrors += tr("[BROKEN] The the following file \"%1\" could not be added to the database.\n").arg(imagePath);
                                importErrorsB = true;
                            }
                        }
                        else if (!myDB->addImage("myclass_pimages", "parentID", "parent", directories.at(i), QPixmap(imagePath, "PNG"))
                                 && checkForErrors){
                            importErrors += tr("[BROKEN] The the following file \"%1\" could not be added to the database.\n").arg(imagePath);
                            importErrorsB = true;
                        }
                        currentImage.close();
                    }
                    else if (checkForErrors){
                        importErrors += tr("[SKIPPED] The the following file \"%1\" does not exists.\n").arg(imagePath);
                        importErrorsB = true;
                    }

                    imagePath = parentPath + "parentCPG.png";
                    currentImage.setFileName(imagePath);
                    if (currentImage.exists()){
                        currentImage.open(QIODevice::ReadOnly);
                        if (!myDB->updateImage("myclass_pimages", "parentCPG", QPixmap(imagePath, "PNG"), "parent", directories.at(i))
                                && checkForErrors){
                            importErrors += tr("[BROKEN] The the following file \"%1\" could not be added to the database.\n").arg(imagePath);
                            importErrorsB = true;
                        }
                        currentImage.close();
                    }
                    else if (checkForErrors){
                        importErrors += tr("[SKIPPED] The the following file \"%1\" does not exists.\n").arg(imagePath);
                        importErrorsB = true;
                    }

                    imagePath = parentPath + "addressComprobation.png";
                    currentImage.setFileName(imagePath);
                    if (currentImage.exists()){
                        currentImage.open(QIODevice::ReadOnly);
                        if (!myDB->updateImage("myclass_pimages", "addressComprobation", QPixmap(imagePath, "PNG"), "parent", directories.at(i))
                                && checkForErrors){
                            importErrors += tr("[BROKEN] The the following file \"%1\" could not be added to the database.\n").arg(imagePath);
                            importErrorsB = true;
                        }
                        currentImage.close();
                    }
                    else if (checkForErrors){
                        importErrors += tr("[SKIPPED] The the following file \"%1\" does not exists.\n").arg(imagePath);
                        importErrorsB = true;
                    }
                }
                else if (checkForErrors){
                    importErrors += tr("[SKIPPED] The the following directory \"%1\" does not exists.\n").arg(parentPath);
                    importErrorsB = true;
                }
            }
        }
        else if (checkForErrors){
            importErrors += tr("[UNAVAILABLE] The selected directory does not exists.\n");
            importErrorsB = true;
        }

        if (importErrorsB && checkForErrors){
            QString parentPath = (paths.at(6) + QString(QDir::separator()) + "log.txt");

            QFile logFile(parentPath);
            logFile.open(QIODevice::WriteOnly | QIODevice::Text);
            logFile.write(importErrors.toLocal8Bit());
            logFile.close();
        }
    }

    QString finalMessage;
    if (importErrorsB) finalMessage = tr("Seems that an error has occurred or some wanings have been generated while importing the data into the database."
                                         "\nHave a look at the log (on the directory of the file) for details.");
    else finalMessage = tr("The importantion process runned suecessfully. Click \"OK\" in order to dismiss this message."
                           "\nHave a look at the log (on the directory of the file) for details.");
    QMessageBox::warning(this, tr("Errors | SmartClass"), finalMessage,
                             QMessageBox::Ok, QMessageBox::NoButton);
    this->close();
}

void frmImportExportDB::getImportDir(){
    QFileDialog selectDir;
    selectDir.setWindowTitle(tr("Select directory | SmartClass"));
    selectDir.setAcceptMode(QFileDialog::AcceptOpen);
    selectDir.setFileMode(QFileDialog::Directory);
    selectDir.setOption(QFileDialog::ShowDirsOnly);
    selectDir.setDirectory(QDir::homePath());
    if (selectDir.exec()){
        QString senderName = sender()->objectName();
        if (senderName == "btSearchParentImagesDir")
            ui->edtParentImagesDirPath->setText(selectDir.selectedFiles().at(0));
        else ui->edtStudentImagesDirPath->setText(selectDir.selectedFiles().at(0));
    }
}

void frmImportExportDB::getImportFilePath(){
    QFileDialog selectFile;
    selectFile.setWindowTitle(tr("Select file | SmartClass"));
    selectFile.setAcceptMode(QFileDialog::AcceptOpen);
    selectFile.setFileMode(QFileDialog::ExistingFile);
    selectFile.setNameFilters(QStringList() << tr("Comma separated values files (*.csv)")
                                            << tr("All files (*.*)"));
    selectFile.setDirectory(currentImportDir);
    if (selectFile.exec()){
        QString senderName = sender()->objectName();
        if (senderName == "btSearchStudentFile")
            ui->edtStudentFilePath->setText(selectFile.selectedFiles().at(0));
        else if (senderName == "btSearchParentFile")
            ui->edtParentFilePath->setText(selectFile.selectedFiles().at(0));
        else if (senderName == "btSearchCoursesFile")
            ui->edtCoursesFilePath->setText(selectFile.selectedFiles().at(0));
        else if (senderName == "btSearchPaymentFile")
            ui->edtPaymentFilePath->setText(selectFile.selectedFiles().at(0));
        else ui->edtUsersFilePath->setText(selectFile.selectedFiles().at(0));
        currentImportDir = selectFile.directory();
    }
}

void frmImportExportDB::exportDB(){
    QString exportDir = ui->edtExportDirectory->text();
    QString errorMsg = tr("Well, unfortunately we cannot proceed. Some data are either inconsistent or inexistent."
                          " Please, fix the folowing issues before trying to save again:\n");
    bool error = false;
    if (exportDir.isEmpty()){
        errorMsg += tr("\n->The path to the directory cannot be empty.");
        error = true;
    }
    if (QDir().exists(exportDir)){
        errorMsg += tr("\n->The path to the directory already exists, please select another name to your backup folder.");
        error = true;
    }

    if (error){
        QMessageBox::information(this, tr("Warning | SmartClass"), errorMsg, QMessageBox::Ok, QMessageBox::NoButton);
        return;
    }

    QDir saveDir(exportDir);
    if (!saveDir.mkpath(exportDir)){
        QMessageBox::critical(this, tr("Error | SmartClass"), tr("It was not possible to create the backup directory. Does this path requires special permissions?"), QMessageBox::Ok, QMessageBox::Ok);
        return;
    }

    if (ui->cbExportStudentsTable->isChecked()){
        QString studentsTableOutput = tr("Autogenarated file | SmartClass ~ Students Table\n\n");

        studentsTableOutput += tr("Student;Birthday;Student ID;School;Observations;"
                                  "Experimental course;Experimental course date;Experimental course observations;Courses; Address\n\n");

        QStringList *studentsTable = myDB->retrieveAll("myclass_students");
        int stSize = myDB->rowsCount("myclass_students");

        for (int i = 0; i < stSize; ++i){
            for (int k = 1; k < 11; ++k){
                studentsTableOutput += (studentsTable[i].at(k) + (k == 10 ? "\n" : ";"));
            }
        }

        QFile outFile(QString(saveDir.path() + QDir::separator() + tr("students.csv")));
        outFile.open(QIODevice::WriteOnly | QIODevice::Text);
        outFile.write(studentsTableOutput.toLocal8Bit());
        outFile.close();

        delete[] studentsTable;
    }

    if (ui->cbExportParentsTable->isChecked()){
        QString parentsTableOutput = tr("Autogenarated file | SmartClass ~ Parents Table\n\n");

        parentsTableOutput += tr("Parent;Students;Phone number;Mobile phone operator;Mobile phone number;"
                                    "Email;Parent ID;Parent CPG;How have he met us?\n\n");

        QStringList *parentsTable = myDB->retrieveAll("myclass_parents");
        int pSize = myDB->rowsCount("myclass_parents");

        for (int i = 0; i < pSize; ++i)
            for (int k = 1; k < 10; ++k)
                parentsTableOutput += (parentsTable[i].at(k) + (k == 9 ? "\n" : ";"));

        QFile outFile(QString(saveDir.path() + QDir::separator() + tr("parents.csv")));
        outFile.open(QIODevice::WriteOnly | QIODevice::Text);
        outFile.write(parentsTableOutput.toLocal8Bit());
        outFile.close();

        delete[] parentsTable;
    }

    if (ui->cbExportCoursesTable->isChecked()){
        QString coursesTableOutput = tr("Autogenarated file | SmartClass ~ Courses Table\n\n");

        coursesTableOutput += tr("Course;Teacher;Short description;Long description;Classroom;"
                                 "Days and time;Beginning date;End date;Price; Students\n\n");

        QStringList *coursesTable = myDB->retrieveAll("myclass_courses");
        int cSize = myDB->rowsCount("myclass_courses");

        for (int i = 0; i < cSize; ++i)
            for (int k = 1; k < 11; ++k)
                coursesTableOutput += (coursesTable[i].at(k) + (k == 10 ? "\n" : ";"));

        QFile outFile(QString(saveDir.path() + QDir::separator() + tr("courses.csv")));
        outFile.open(QIODevice::WriteOnly | QIODevice::Text);
        outFile.write(coursesTableOutput.toLocal8Bit());
        outFile.close();

        delete[] coursesTable;
    }

    if (ui->cbExportPricingTable->isChecked()){
        QString paymentTableOutput = tr("Autogenarated file | SmartClass ~ Parents Table\n\n");

        paymentTableOutput += tr("Student;Course;Discount (percentage);First installment;Installments\n\n");

        QStringList *paymentTable = myDB->retrieveAll("myclass_pricing");
        int pSize = myDB->rowsCount("myclass_pricing");

        for (int i = 0; i < pSize; ++i)
            for (int k = 1; k < 6; ++k)
                paymentTableOutput += (paymentTable[i].at(k) + (k == 5 ? "\n" : ";"));

        QFile outFile(QString(saveDir.path() + QDir::separator() + tr("payment.csv")));
        outFile.open(QIODevice::WriteOnly | QIODevice::Text);
        outFile.write(paymentTableOutput.toLocal8Bit());
        outFile.close();

        delete[] paymentTable;
    }

    if (ui->cbExportUsersTable->isChecked()){
        QString usersTableOutput = tr("Autogenarated file | SmartClass ~ Parents Table\n\n");

        usersTableOutput += tr("Username;Name;Password salt;Password hash;Security question;Answer salt;Answer hash;Role\n\n");

        QStringList *usersTable = myDB->retrieveAll("myclass_users");
        int uSize = myDB->rowsCount("myclass_users");

        for (int i = 0; i < uSize; ++i)
            for (int k = 1; k < 9; ++k)
                usersTableOutput += (usersTable[i].at(k) + (k == 8 ? "\n" : ";"));

        QFile outFile(QString(saveDir.path() + QDir::separator() + tr("users.csv")));
        outFile.open(QIODevice::WriteOnly | QIODevice::Text);
        outFile.write(usersTableOutput.toLocal8Bit());
        outFile.close();

        delete[] usersTable;
    }

    if (ui->cbExportImages->isChecked()){
        if (ui->cbParentImages->isChecked()){
            if (saveDir.mkdir(tr("Parental images"))){
                QString pImagesDirPath = exportDir + QDir::separator() + tr("Parental images");
                QDir pImagesDir(pImagesDirPath);
                QStringList* parentsTable = myDB->retrieveAll("myclass_parents", QStringList() << "parent");
                int pCount = myDB->rowsCount("myclass_parents");
                for (int i = 0; i < pCount; ++i){
                    pImagesDir.mkdir(parentsTable[i].at(0));
                    QString newPath = pImagesDirPath + QDir::separator() + parentsTable[i].at(0);

                    QPixmap currentImg = myDB->retrieveImage("myclass_pimages", "parentID", "parent", parentsTable[i].at(0));
                    if (!currentImg.isNull()){
                        QFile imageOut(newPath + QDir::separator() + tr("parentID.png"));
                        imageOut.open(QIODevice::WriteOnly);
                        currentImg.save(&imageOut, "PNG");
                    }

                    currentImg = myDB->retrieveImage("myclass_pimages", "parentCPG", "parent", parentsTable[i].at(0));
                    if (!currentImg.isNull()){
                        QFile imageOut(newPath + QDir::separator() + tr("parentCPG.png"));
                        imageOut.open(QIODevice::WriteOnly);
                        currentImg.save(&imageOut, "PNG");
                    }

                    currentImg = myDB->retrieveImage("myclass_pimages", "addressComprobation", "parent", parentsTable[i].at(0));
                    if (!currentImg.isNull()){
                        QFile imageOut(newPath + QDir::separator() + tr("addressComprobation.png"));
                        imageOut.open(QIODevice::WriteOnly);
                        currentImg.save(&imageOut, "PNG");
                    }
                }
            }
            else QMessageBox::warning(this, tr("Error | SmartClass"),
                                      tr("It was not possible to create the folder which will contain the images related to the parents"),
                                      QMessageBox::Ok, QMessageBox::NoButton);
        }
        if (ui->cbStudentImages->isChecked()){
            if (saveDir.mkdir(tr("Students images"))){
                QString sImagesDirPath = exportDir + QDir::separator() + tr("Students images");
                QDir sImagesDir(sImagesDirPath);
                QStringList* studentsTable = myDB->retrieveAll("myclass_students", QStringList() << "student");
                int sCount = myDB->rowsCount("myclass_students");
                for (int i = 0; i < sCount; ++i){
                    sImagesDir.mkdir(studentsTable[i].at(0));
                    QString newPath = sImagesDirPath + QDir::separator() + studentsTable[i].at(0);

                    QPixmap currentImg = myDB->retrieveImage("myclass_simages", "studentImage", "student", studentsTable[i].at(0));
                    if (!currentImg.isNull()){
                        QFile imageOut(newPath + QDir::separator() + tr("studentImage.png"));
                        imageOut.open(QIODevice::WriteOnly);
                        currentImg.save(&imageOut, "PNG");
                    }

                    currentImg = myDB->retrieveImage("myclass_simages", "studentID", "student", studentsTable[i].at(0));
                    if (!currentImg.isNull()){
                        QFile imageOut(newPath + QDir::separator() + tr("studentID.png"));
                        imageOut.open(QIODevice::WriteOnly);
                        currentImg.save(&imageOut, "PNG");
                    }
                }
            }
            else QMessageBox::warning(this, tr("Error | SmartClass"),
                                      tr("It was not possible to create the folder which will contain the images related to the students"),
                                      QMessageBox::Ok, QMessageBox::NoButton);
        }
    }

    this->close();
}

void frmImportExportDB::getExportDir(){
    QFileDialog saveDir;
    saveDir.setWindowTitle("Export to directory | SmartClass");
    saveDir.setAcceptMode(QFileDialog::AcceptSave);
    saveDir.setFileMode(QFileDialog::Directory);
    saveDir.setOption(QFileDialog::ShowDirsOnly);
    saveDir.setDirectory(QDir::homePath());
    if (saveDir.exec())
        ui->edtExportDirectory->setText(saveDir.selectedFiles().at(0));
}

/*
 * GUI Functions (don't change, unless necessary)
 */

void frmImportExportDB::mousePressEvent(QMouseEvent *event)
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

void frmImportExportDB::undefMouseMoveEvent(QObject* object, QMouseEvent* event){
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

void frmImportExportDB::mouseReleaseEvent(QMouseEvent *event){
    locked = LockMoveType::None;
    event->accept();
}

bool frmImportExportDB::eventFilter(QObject* object, QEvent* event)
{
    if(event->type() == QEvent::MouseMove)
        undefMouseMoveEvent(object, static_cast<QMouseEvent*>(event));
    else if (event->type() == QEvent::MouseButtonPress && object->objectName() == "titleBar"){
        mousePressEvent(static_cast<QMouseEvent*>(event));
    }
    return false;
}
