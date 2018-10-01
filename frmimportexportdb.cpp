#include "frmimportexportdb.h"
#include "ui_frmimportexportdb.h"

frmImportExportDB::frmImportExportDB(QWidget *parent, ImportMode mode, const DBManager::DBData &dbData) :
    NMainWindow(parent),
    ui(new Ui::frmImportExportDB),
    CURRENT_MODE(mode)
{
    ui->setupUi(this);

    // Sets the custom Widgets on the parent Class
    // Otherwise, the window resizing feature will not work
    NMainWindow::setCustomWidgets(ui->centralWidget, ui->statusBar);
    this->setMaximizeButtonEnabled(false);

    /*
     *  End of GUI implementation
     */

    myDB = new DBManager(dbData, SmartClassGlobal::tablePrefix(),
                            SmartClassGlobal::databaseType(), DBManager::getUniqueConnectionName("importExport"));

    if (CURRENT_MODE == frmImportExportDB::Export){
        ui->grpImportOptions->setVisible(false);
        ui->btImport->setVisible(false);
        this->setMaximumHeight(650);
        this->setMinimumHeight(650);
        this->setGeometry(QRect(this->geometry().topLeft(), QSize(this->width(), 650)));


        connect(ui->cbExportImages, SIGNAL(toggled(bool)), ui->cbStudentImages, SLOT(setEnabled(bool)));
        connect(ui->cbExportImages, SIGNAL(toggled(bool)), ui->cbParentImages, SLOT(setEnabled(bool)));

        connect(ui->cbExportImages, SIGNAL(toggled(bool)), ui->grpExportImages, SLOT(setEnabled(bool)));
        connect(ui->btSearchDirectory, SIGNAL(clicked(bool)), this, SLOT(getExportDir()));

        connect(ui->btExport, SIGNAL(clicked(bool)), this, SLOT(exportDB()));
    }
    else{
        ui->grpExportOptions->setVisible(false);
        ui->btExport->setVisible(false);
        this->setMaximumHeight(500);
        this->setMinimumHeight(500);
        this->setGeometry(QRect(this->geometry().topLeft(), QSize(this->width(), 500)));

        connect(ui->btSearchStudentFile, SIGNAL(clicked(bool)), this, SLOT(getImportFilePath()));
        connect(ui->btSearchParentFile, SIGNAL(clicked(bool)), this, SLOT(getImportFilePath()));
        connect(ui->btSearchCoursesFile, SIGNAL(clicked(bool)), this, SLOT(getImportFilePath()));
        connect(ui->btSearchPaymentFile, SIGNAL(clicked(bool)), this, SLOT(getImportFilePath()));
        connect(ui->btSearchUsersFile, SIGNAL(clicked(bool)), this, SLOT(getImportFilePath()));
        connect(ui->btSearchCourseEnrollmentsFile, SIGNAL(clicked(bool)), this, SLOT(getImportFilePath()));
        connect(ui->btSearchSettings, SIGNAL(clicked(bool)), this, SLOT(getImportFilePath()));
        connect(ui->btSearchActiveConnectionsFile, SIGNAL(clicked(bool)), this, SLOT(getImportFilePath()));
        connect(ui->btSearchCompanyLogo, SIGNAL(clicked(bool)), this, SLOT(getImportFilePath()));

        connect(ui->btSearchStudentImagesDir, SIGNAL(clicked(bool)), this, SLOT(getImportDir()));
        connect(ui->btSearchParentImagesDir, SIGNAL(clicked(bool)), this, SLOT(getImportDir()));

        connect(ui->btClearStudentsPath, SIGNAL(clicked(bool)), ui->edtStudentFilePath, SLOT(clear()));
        connect(ui->btClearParentsPath, SIGNAL(clicked(bool)), ui->edtParentFilePath, SLOT(clear()));
        connect(ui->btClearCoursesPath, SIGNAL(clicked(bool)), ui->edtCoursesFilePath, SLOT(clear()));
        connect(ui->btClearCourseEnrollmentsPath, SIGNAL(clicked(bool)), ui->edtCourseEnrollmentsFilePath, SLOT(clear()));
        connect(ui->btClearPricingPath, SIGNAL(clicked(bool)), ui->edtPaymentFilePath, SLOT(clear()));
        connect(ui->btClearUsersPath, SIGNAL(clicked(bool)), ui->edtUsersFilePath, SLOT(clear()));
        connect(ui->btClearSettingsPath, SIGNAL(clicked(bool)), ui->edtSettingsFilePath, SLOT(clear()));
        connect(ui->btClearActiveConnectionsPath, SIGNAL(clicked(bool)), ui->edtActiveConnectionsFilePath, SLOT(clear()));
        connect(ui->btClearPImagesPath, SIGNAL(clicked(bool)), ui->edtParentImagesDirPath, SLOT(clear()));
        connect(ui->btClearSImagesPath, SIGNAL(clicked(bool)), ui->edtStudentImagesDirPath, SLOT(clear()));
        connect(ui->btClearCompanyLogoPath, SIGNAL(clicked(bool)), ui->edtCompanyLogoPath, SLOT(clear()));

        connect(ui->btImport, SIGNAL(clicked(bool)), this, SLOT(importDB()));
    }

    this->setGeometry(QStyle::alignedRect(Qt::LeftToRight, Qt::AlignCenter,
            this->size(), qApp->desktop()->availableGeometry()));

    myDB->openDB();

    currentImportDir = QDir::home();
}

frmImportExportDB::~frmImportExportDB()
{
    if (myDB->isOpen()) myDB->closeDB();
    delete myDB;
    delete ui;
}

QList<QVariant> frmImportExportDB::stringToVariant(const QStringList &sList, SmartClassGlobal::TablesSpec tableSpec){
    QList<QVariant> converted;

    switch (tableSpec) {
        case SmartClassGlobal::USERS:
            converted << sList[0].toLongLong()
                      << sList[1]
                      << sList[2]
                      << sList[3]
                      << sList[4]
                      << sList[5]
                      << sList[6]
                      << sList[7]
                      << sList[8].toInt();
            break;
        case SmartClassGlobal::STUDENT:
            converted << sList[0].toLongLong()
                      << sList[1]
                      << sList[2].toLongLong()
                      << QDate::fromString(sList[3], "dd/MM/yyyy")
                      << sList[4]
                      << sList[5]
                      << sList[6]
                      << sList[7]
                      << QDateTime::fromString(sList[8], "dd/MM/yyyy - HH:mm")
                      << sList[9];
            break;
        case SmartClassGlobal::RESPONSIBLE:
            converted << sList[0].toLongLong()
                      << sList[1]
                      << sList[2]
                      << sList[3].toInt()
                      << sList[4]
                      << sList[5]
                      << sList[6]
                      << sList[7]
                      << sList[8]
                      << sList[9];
            break;
        case SmartClassGlobal::COURSEDETAILS:
            converted << sList[0].toLongLong()
                      << sList[1]
                      << sList[2]
                      << sList[3]
                      << sList[4]
                      << sList[5].toInt()
                      << sList[6]
                      << QDate::fromString(sList[7], "dd/MM/yyyy")
                      << QDate::fromString(sList[8], "dd/MM/yyyy")
                      << sList[9].toDouble();
            break;
        case SmartClassGlobal::COURSEENROLLMENTS:
            converted << sList[0].toLongLong()
                      << sList[1].toLongLong();
            break;
        case SmartClassGlobal::SETTINGS:
            converted << sList[0]
                      << sList[1]
                      << QVariant();
            break;
        case SmartClassGlobal::ACTIVECONNECTIONS:
            converted << sList[0].toLongLong()
                      << sList[1]
                      << sList[2]
                      << sList[3]
                      << QDate::fromString(sList[4], "dd/MM/yyyy");
            break;
        case SmartClassGlobal::PAYMENTDETAILS:
            converted << sList[0].toLongLong()
                      << sList[1].toLongLong()
                      << sList[2].toDouble()
                      << QDate::fromString(sList[3], "dd/MM/yyyy")
                      << sList[8].toInt();
            break;
        default:
            break;
    }

    return converted;
}

void frmImportExportDB::importDB(){
    QStringList paths, tables;
    paths << ui->edtStudentFilePath->text()
          << ui->edtParentFilePath->text()
          << ui->edtCoursesFilePath->text()
          << ui->edtCourseEnrollmentsFilePath->text()
          << ui->edtPaymentFilePath->text()
          << ui->edtUsersFilePath->text()
          << ui->edtActiveConnectionsFilePath->text()
          << ui->edtSettingsFilePath->text()
          << ui->edtStudentImagesDirPath->text()
          << ui->edtParentImagesDirPath->text()
          << ui->edtCompanyLogoPath->text();

    tables << SmartClassGlobal::getTableName(SmartClassGlobal::STUDENT)
           << SmartClassGlobal::getTableName(SmartClassGlobal::RESPONSIBLE)
           << SmartClassGlobal::getTableName(SmartClassGlobal::COURSEDETAILS)
           << SmartClassGlobal::getTableName(SmartClassGlobal::COURSEENROLLMENTS)
           << SmartClassGlobal::getTableName(SmartClassGlobal::PAYMENTDETAILS)
           << SmartClassGlobal::getTableName(SmartClassGlobal::USERS)
           << SmartClassGlobal::getTableName(SmartClassGlobal::ACTIVECONNECTIONS)
           << SmartClassGlobal::getTableName(SmartClassGlobal::SETTINGS)
           << SmartClassGlobal::getTableName(SmartClassGlobal::STUDENTIMAGES)
           << SmartClassGlobal::getTableName(SmartClassGlobal::RESPONSIBLEIMAGES);

    bool importErrorsB = false;
    bool checkForErrors = ui->cbCheckForErrors->isChecked();

    for (int i = 0; i < 7; ++i){
        SmartClassGlobal::TablesSpec currentTable;
        if (i == 0) currentTable = SmartClassGlobal::STUDENT;
        else if (i == 1) currentTable = SmartClassGlobal::RESPONSIBLE;
        else if (i == 2) currentTable = SmartClassGlobal::COURSEDETAILS;
        else if (i == 3) currentTable = SmartClassGlobal::COURSEENROLLMENTS;
        else if (i == 4) currentTable = SmartClassGlobal::PAYMENTDETAILS;
        else if (i == 5) currentTable = SmartClassGlobal::USERS;
        else currentTable = SmartClassGlobal::ACTIVECONNECTIONS;

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
                            QList<QVariant> newData = stringToVariant(lineContent, currentTable);
                            QStringList tableSchema = SmartClassGlobal::getTableStructure(currentTable);
                            if (myDB->rowExists(tables.at(i), tableSchema, newData)){
                                importErrors += tr("[SKIPPED] The following line already exists or conflicts with an existing one: %1 . Line skipped.\n").arg(lines.at(k));
                                importErrorsB = true;
                            }
                            else if (myDB->rowExists(tables.at(i), tableSchema.at(0), newData.at(0))){
                                importErrors += tr("[UPDATED / WARNING] The following property (%1) already exists with different values (%2) in %3. It has been updated to the content of the CSV file.\n").arg(tableSchema.at(0))
                                                                                                                                                                                                            .arg(lines.at(k))
                                                                                                                                                                                                            .arg(tables.at(i));
                                importErrorsB = true;
                                if (!myDB->updateRow(tables.at(i), tableSchema.at(0), newData.at(0), tableSchema, newData)){
                                    importErrors += tr("[ERROR] The following line could not be added to the database: %1 . Please, ignore the previous update warning.\n").arg(lines.at(k));
                                    importErrorsB = true;
                                }
                            }
                            else if (i != 6 && myDB->rowExists(tables.at(i), tableSchema.at(1), newData.at(1))){
                                importErrors += tr("[UPDATED / WARNING] The following property (%1) already exists with different values (%2) in %3. It has been updated to the content of the CSV file.\n").arg(tableSchema.at(0))
                                                                                                                                                                                                            .arg(lines.at(k))
                                                                                                                                                                                                            .arg(tables.at(i));
                                importErrorsB = true;
                                if (!myDB->updateRow(tables.at(i), tableSchema.at(0), newData.at(0), tableSchema, newData)){
                                     importErrors += tr("[ERROR] The following line could not be added to the database: %1 . Please, ignore the previous update warning.\n").arg(lines.at(k));
                                     importErrorsB = true;
                                }
                            }
                            else {
                                if (!myDB->insertRow(tables.at(i), tableSchema, newData)){
                                    importErrors += tr("[ERROR] The following line could not be added to the database: %1 .\n").arg(lines.at(k));
                                    importErrorsB = true;
                                }
                            }
                        }
                        else myDB->insertRow(tables.at(i),
                                             SmartClassGlobal::getTableStructure(currentTable),
                                             stringToVariant(lineContent, currentTable));
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
            logFile.write(importErrors.toLocal8Bit());
            logFile.close();
        }

    }

    if (!paths.at(7).isEmpty()){
        QString importErrors = "";

        if (!QFile::exists(paths.at(7))){
            QString argument = QString(tables.at(7)).split("_").at(1);
            QMessageBox::critical(this, tr("Error | SmartClass"),
                                                  tr("The path to the %1 file does not exists. This file has been skipped during the process of importation.").arg(argument),
                                                  QMessageBox::Ok, QMessageBox::NoButton);
            importErrors += tr("The path to the %1 file does not exists. This file has been skipped during the process of importation.").arg(argument);
            importErrorsB = true;
        }
        else {
            QFile file(paths.at(7));
            file.open(QIODevice::ReadOnly);
            QString content = QString::fromLocal8Bit(file.readAll());
            file.close();

            QStringList lines = content.split(QRegExp("(\\n|\\r\\n)"));
            int linesCount = lines.length() - 1;
            if (linesCount < 5){
                QMessageBox::warning(this, tr("File error | SmartClass"),
                                     tr("The quantity of lines does not match with the minimal requirements to import into %1 table. This file has been skipped.").arg(tables.at(7)),
                                     QMessageBox::Ok, QMessageBox::NoButton);
                importErrors += tr("The quantity of lines does not match with the minimal requirements to import into %1 table. This file has been skipped.").arg(tables.at(7));
                importErrorsB = true;
            }
            else {
                for (int k = 4; k < linesCount; ++k){
                    QStringList lineContent;
                    lineContent = QString(lines.at(k)).split(";");
                    if (checkForErrors){
                        QList<QVariant> newData = stringToVariant(lineContent, SmartClassGlobal::SETTINGS);
                        QStringList tableSchema = SmartClassGlobal::getTableStructure(SmartClassGlobal::SETTINGS);
                        if (myDB->rowExists(tables.at(7), tableSchema, newData)){
                            importErrors += tr("[SKIPPED] The following line already exists or conflicts with an existing one: %1 . Line skipped.\n").arg(lines.at(k));
                            importErrorsB = true;
                        }
                        else {
                            myDB->clearTable(tables.at(7));
                            if (!paths.at(10).isEmpty()){
                                if (!QFile::exists(paths.at(10))){
                                    QMessageBox::critical(this, tr("Error | SmartClass"),
                                                                          tr("The path to the logo file does not exists. This file has been skipped during the process of importation."),
                                                                          QMessageBox::Ok, QMessageBox::NoButton);
                                    importErrors += tr("The path to the logo file does not exists. This file has been skipped during the process of importation.");
                                    importErrorsB = true;

                                    if (!myDB->insertRow(tables.at(7), tableSchema.mid(0, 2), newData)){
                                        importErrors += tr("[ERROR] The following line could not be added to the database: %1 .\n").arg(lines.at(k));
                                        importErrorsB = true;
                                    }
                                }
                                else {
                                    newData << myDB->pixmapToVariant(QPixmap(paths.at(10)));
                                    if (!myDB->insertRow(tables.at(7), tableSchema, newData)){
                                        importErrors += tr("[ERROR] The following line could not be added to the database: %1 .\n").arg(lines.at(k));
                                        importErrorsB = true;
                                    }
                                }
                            }
                            else {
                                if (!myDB->insertRow(tables.at(7), tableSchema.mid(0, 2), newData)){
                                    importErrors += tr("[ERROR] The following line could not be added to the database: %1 .\n").arg(lines.at(k));
                                    importErrorsB = true;
                                }
                            }
                        }
                    }
                    else {
                        myDB->insertRow(tables.at(7),
                                         SmartClassGlobal::getTableStructure(SmartClassGlobal::SETTINGS),
                                         stringToVariant(lineContent, SmartClassGlobal::SETTINGS));
                    }
                }
            }
        }

        if (importErrorsB && checkForErrors){
            QFileInfo fInfo(paths.at(7));
            QString parentPath = fInfo.filePath();
            parentPath += "log.txt";

            QFile logFile(parentPath);
            logFile.open(QIODevice::WriteOnly | QIODevice::Text);
            logFile.write(importErrors.toLocal8Bit());
            logFile.close();
        }
    }
    else if (!paths.at(10).isEmpty()){
        QMessageBox::information(this, tr("Info | SmartClass"),
                                       tr("In order to submit a new logo into the database you must either change it through the application settings or import the settings file as well.\nThis file has been skipped."),
                                       QMessageBox::Ok, QMessageBox::NoButton);
    }

    if (!paths.at(8).isEmpty()){
        QString importErrors = "";

        QDir imagesPath(paths.at(8));
        if (imagesPath.exists()){
            QStringList directories = imagesPath.entryList(QDir::Dirs, QDir::Name),
                        tableSchema = SmartClassGlobal::getTableStructure(SmartClassGlobal::STUDENTIMAGES);
            directories.removeOne(".");
            directories.removeOne("..");
            for (int i = 0; i < directories.length(); ++i){
                qlonglong sID = directories.at(i).toLongLong();
                bool sIExists = myDB->rowExists(SmartClassGlobal::getTableName(SmartClassGlobal::STUDENTIMAGES),
                                                tableSchema.at(0), sID);
                if (sIExists && checkForErrors){
                    importErrors += tr("[OVERWRITTEN] The the following student \"%1\" already exists in the database. Every image in the register have been overwritten by the existent images in the import directory.\n").arg(directories.at(0));
                    importErrorsB = true;
                }
                QString parentPath = paths.at(8) + QDir::separator() + directories.at(i) + QDir::separator();
                QList <QVariant> newData;
                newData << sID;
                if (QDir(parentPath).exists()){
                    for (int k = 1; k < tableSchema.length(); ++k){
                        QString imagePath = parentPath + tableSchema.at(k) + ".png";
                        newData << (QFile::exists(imagePath) ? QPixmap(imagePath) : QVariant());
                    }

                    if (sIExists){
                        if (!myDB->updateRow(SmartClassGlobal::getTableName(SmartClassGlobal::STUDENTIMAGES),
                                             tableSchema.at(0), sID,
                                             tableSchema.mid(1, 2), newData.mid(1, 2))){
                            importErrors += tr("[BROKEN] The the following ID \"%1\" could not be added to the database.\n").arg(directories.at(i));
                            importErrorsB = true;
                        }
                    }
                    else if (!myDB->insertRow(SmartClassGlobal::getTableName(SmartClassGlobal::STUDENTIMAGES),
                                              tableSchema, newData) && checkForErrors){
                        importErrors += tr("[BROKEN] The the following ID \"%1\" could not be added to the database.\n").arg(directories.at(i));
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
            QString parentPath = (paths.at(8) + QString(QDir::separator()) + "log.txt");

            QFile logFile(parentPath);
            logFile.open(QIODevice::WriteOnly | QIODevice::Text);
            logFile.write(importErrors.toLocal8Bit());
            logFile.close();
        }
    }

    if (!paths.at(9).isEmpty()){
        QString importErrors = "";

        QDir imagesPath(paths.at(9));
        if (imagesPath.exists()){
            QStringList directories = imagesPath.entryList(QDir::Dirs, QDir::Name),
                        tableSchema = SmartClassGlobal::getTableStructure(SmartClassGlobal::RESPONSIBLEIMAGES);
            directories.removeOne(".");
            directories.removeOne("..");
            for (int i = 0; i < directories.length(); ++i){
                qlonglong rID = directories.at(i).toLongLong();
                bool sIExists = myDB->rowExists(SmartClassGlobal::getTableName(SmartClassGlobal::RESPONSIBLEIMAGES),
                                                tableSchema.at(0), rID);
                if (sIExists && checkForErrors){
                    importErrors += tr("[OVERWRITTEN] The the following responsible \"%1\" already exists in the database. Every image in the register have been overwritten by the existent images in the import directory.\n").arg(directories.at(0));
                    importErrorsB = true;
                }
                QString parentPath = paths.at(9) + QDir::separator() + directories.at(i) + QDir::separator();
                QList <QVariant> newData;
                newData << rID;
                if (QDir(parentPath).exists()){
                    for (int k = 1; k < tableSchema.length(); ++k){
                        QString imagePath = parentPath + tableSchema.at(k) + ".png";
                        newData << (QFile::exists(imagePath) ? QPixmap(imagePath) : QVariant());
                    }

                    if (sIExists){
                        if (!myDB->updateRow(SmartClassGlobal::getTableName(SmartClassGlobal::RESPONSIBLEIMAGES),
                                             tableSchema.at(0), rID,
                                             tableSchema.mid(1, 3), newData.mid(1, 3))){
                            importErrors += tr("[BROKEN] The the following ID \"%1\" could not be added to the database.\n").arg(directories.at(i));
                            importErrorsB = true;
                        }
                    }
                    else if (!myDB->insertRow(SmartClassGlobal::getTableName(SmartClassGlobal::RESPONSIBLEIMAGES),
                                              tableSchema, newData) && checkForErrors){
                        importErrors += tr("[BROKEN] The the following ID \"%1\" could not be added to the database.\n").arg(directories.at(i));
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
            QString parentPath = (paths.at(9) + QString(QDir::separator()) + "log.txt");

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
        else if (senderName == "btSearchCourseEnrollmentsFile")
            ui->edtCourseEnrollmentsFilePath->setText(selectFile.selectedFiles().at(0));
        else if (senderName == "btSearchSettings")
            ui->edtSettingsFilePath->setText(selectFile.selectedFiles().at(0));
        else if (senderName == "btSearchActiveConnectionsFile")
            ui->edtActiveConnectionsFilePath->setText(selectFile.selectedFiles().at(0));
        else if (senderName == "btSearchCompanyLogo")
            ui->edtCompanyLogoPath->setText(selectFile.selectedFiles().at(0));
        else ui->edtUsersFilePath->setText(selectFile.selectedFiles().at(0));
        currentImportDir = selectFile.directory();
    }
}

void frmImportExportDB::exportDB(){
    QString exportDir = ui->edtExportDirectory->text();
    QString errorMsg = tr("Well, unfortunately we cannot proceed. There are either inconsistent or inexistent critical data."
                          " Please, fix the folowing issues before trying to export again:\n");
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

        studentsTableOutput += tr("ID;Name;Responsible ID;Birthday;ID (document);School;Observations;"
                                  "Experimental course;Experimental course date;Experimental course observations\n\n");

        QList< QList<QVariant> > studentsTable = myDB->retrieveAll(SmartClassGlobal::getTableName(SmartClassGlobal::STUDENT));

        for (int i = 0; i < studentsTable.size(); ++i){
            studentsTableOutput += QString::number(studentsTable[i].at(0).toLongLong()) + ";";
            studentsTableOutput += studentsTable[i].at(1).toString() + ";";
            studentsTableOutput += QString::number(studentsTable[i].at(2).toLongLong()) + ";";
            studentsTableOutput += studentsTable[i].at(3).toDate().toString("dd/MM/yyyy") + ";";
            studentsTableOutput += studentsTable[i].at(4).toString() + ";";
            studentsTableOutput += studentsTable[i].at(5).toString() + ";";
            studentsTableOutput += studentsTable[i].at(6).toString() + ";";
            studentsTableOutput += studentsTable[i].at(7).toString() + ";";
            studentsTableOutput += studentsTable[i].at(3).toDateTime().toString("dd/MM/yyyy - HH:mm") + ";";
            studentsTableOutput += studentsTable[i].at(9).toString() + "\n";
        }

        QFile outFile(QString(saveDir.path() + QDir::separator() + tr("students.csv")));
        outFile.open(QIODevice::WriteOnly | QIODevice::Text);
        outFile.write(studentsTableOutput.toLocal8Bit());
        outFile.close();
    }

    if (ui->cbExportParentsTable->isChecked()){
        QString parentsTableOutput = tr("Autogenarated file | SmartClass ~ Parents Table\n\n");

        parentsTableOutput += tr("ID;Name;Phone number;Mobile phone operator;Mobile phone number;"
                                    "Email;ID (document);CPG;How have he met us?;Address\n\n");

        QList< QList<QVariant> > parentsTable = myDB->retrieveAll(SmartClassGlobal::getTableName(SmartClassGlobal::RESPONSIBLE));

        for (int i = 0; i < parentsTable.size(); ++i){
            parentsTableOutput += QString::number(parentsTable[i].at(0).toLongLong()) + ";";
            parentsTableOutput += parentsTable[i].at(1).toString() + ";";
            parentsTableOutput += parentsTable[i].at(2).toString() + ";";
            parentsTableOutput += QString::number(parentsTable[i].at(3).toInt()) + ";";
            parentsTableOutput += parentsTable[i].at(4).toString() + ";";
            parentsTableOutput += parentsTable[i].at(5).toString() + ";";
            parentsTableOutput += parentsTable[i].at(6).toString() + ";";
            parentsTableOutput += parentsTable[i].at(7).toString() + ";";
            parentsTableOutput += parentsTable[i].at(8).toString() + ";";
            parentsTableOutput += parentsTable[i].at(9).toString() + "\n";
        }

        QFile outFile(QString(saveDir.path() + QDir::separator() + tr("parents.csv")));
        outFile.open(QIODevice::WriteOnly | QIODevice::Text);
        outFile.write(parentsTableOutput.toLocal8Bit());
        outFile.close();
    }

    if (ui->cbExportCoursesTable->isChecked()){
        QString coursesTableOutput = tr("Autogenarated file | SmartClass ~ Courses Table\n\n");

        coursesTableOutput += tr("ID;Course name;Teacher;Short description;Long description;Classroom number;"
                                 "Days and time;Beginning date;End date;Price\n\n");

        QList< QList<QVariant> > coursesTable = myDB->retrieveAll(SmartClassGlobal::getTableName(SmartClassGlobal::COURSEDETAILS));

        for (int i = 0; i < coursesTableOutput.size(); ++i){
            coursesTableOutput += QString::number(coursesTable[i].at(0).toLongLong()) + ";";
            coursesTableOutput += coursesTable[i].at(1).toString() + ";";
            coursesTableOutput += coursesTable[i].at(2).toString() + ";";
            coursesTableOutput += coursesTable[i].at(3).toString() + ";";
            coursesTableOutput += coursesTable[i].at(4).toString() + ";";
            coursesTableOutput += QString::number(coursesTable[i].at(5).toInt()) + ";";
            coursesTableOutput += coursesTable[i].at(6).toString() + ";";
            coursesTableOutput += coursesTable[i].at(7).toDate().toString("dd/MM/yyyy") + ";";
            coursesTableOutput += coursesTable[i].at(8).toDate().toString("dd/MM/yyyy") + ";";
            coursesTableOutput += QString::number(coursesTable[i].at(9).toDouble(), 'f', 2) + "\n";
        }

        QFile outFile(QString(saveDir.path() + QDir::separator() + tr("courses.csv")));
        outFile.open(QIODevice::WriteOnly | QIODevice::Text);
        outFile.write(coursesTableOutput.toLocal8Bit());
        outFile.close();
    }

    if (ui->cbExportPricingTable->isChecked()){
        QString paymentTableOutput = tr("Autogenarated file | SmartClass ~ Parents Table\n\n");

        paymentTableOutput += tr("Student ID;Course ID;Discount (percentage);First installment;Installments\n\n");

        QList< QList<QVariant> > paymentTable = myDB->retrieveAll(SmartClassGlobal::getTableName(SmartClassGlobal::PAYMENTDETAILS));

        for (int i = 0; i < paymentTable.size(); ++i){
            paymentTableOutput += QString::number(paymentTable[i].at(0).toLongLong()) + ";";
            paymentTableOutput += QString::number(paymentTable[i].at(1).toLongLong()) + ";";
            paymentTableOutput += QString::number(paymentTable[i].at(2).toDouble(), 'f', 2) + ";";
            paymentTableOutput += paymentTable[i].at(3).toDate().toString("dd/MM/yyyy") + ";";
            paymentTableOutput += QString::number(paymentTable[i].at(4).toInt()) + "\n";
        }

        QFile outFile(QString(saveDir.path() + QDir::separator() + tr("payment.csv")));
        outFile.open(QIODevice::WriteOnly | QIODevice::Text);
        outFile.write(paymentTableOutput.toLocal8Bit());
        outFile.close();
    }

    if (ui->cbExportUsersTable->isChecked()){
        QString usersTableOutput = tr("Autogenarated file | SmartClass ~ Parents Table\n\n");

        usersTableOutput += tr("ID;Username;Name;Password salt;Password hash;Security question;Answer salt;Answer hash;Role\n\n");

        QList< QList<QVariant> > usersTable = myDB->retrieveAll("myclass_users");

        for (int i = 0; i < usersTable.size(); ++i){
            usersTableOutput += QString::number(usersTable[i].at(0).toLongLong()) + ";";
            usersTableOutput += usersTable[i].at(1).toString() + ";";
            usersTableOutput += usersTable[i].at(2).toString() + ";";
            usersTableOutput += usersTable[i].at(3).toString() + ";";
            usersTableOutput += usersTable[i].at(4).toString() + ";";
            usersTableOutput += usersTable[i].at(5).toString() + ";";
            usersTableOutput += usersTable[i].at(6).toString() + ";";
            usersTableOutput += usersTable[i].at(7).toString() + ";";
            usersTableOutput += QString::number(usersTable[i].at(0).toInt()) + "\n";
        }

        QFile outFile(QString(saveDir.path() + QDir::separator() + tr("users.csv")));
        outFile.open(QIODevice::WriteOnly | QIODevice::Text);
        outFile.write(usersTableOutput.toLocal8Bit());
        outFile.close();
    }

    if (ui->cbExportCourseEntrollmentsTable->isChecked()){
        QString courseEnrollmentsTableOutput = tr("Autogenarated file | SmartClass ~ Course Enrollments Table\n\n");

        courseEnrollmentsTableOutput += tr("Course ID;Student ID\n\n");

        QList< QList<QVariant> > courseEnrollmentsTable = myDB->retrieveAll(SmartClassGlobal::getTableName(SmartClassGlobal::COURSEENROLLMENTS));

        for (int i = 0; i < courseEnrollmentsTable.size(); ++i){
            courseEnrollmentsTableOutput += QString::number(courseEnrollmentsTable[i].at(0).toLongLong()) + ";";
            courseEnrollmentsTableOutput += QString::number(courseEnrollmentsTable[i].at(1).toLongLong()) + "\n";
        }

        QFile outFile(QString(saveDir.path() + QDir::separator() + tr("course_enrollments.csv")));
        outFile.open(QIODevice::WriteOnly | QIODevice::Text);
        outFile.write(courseEnrollmentsTableOutput.toLocal8Bit());
        outFile.close();
    }

    if (ui->cbExportActiveConnectionsTable->isChecked()){
        QString connectionsTableOutput = tr("Autogenarated file | SmartClass ~ Active Connections Table\n\n");

        connectionsTableOutput += tr("ID;Device name;Operating System;OS Version;Last Access\n\n");

        QList< QList<QVariant> > connectionsTable = myDB->retrieveAll(SmartClassGlobal::getTableName(SmartClassGlobal::ACTIVECONNECTIONS));

        for (int i = 0; i < connectionsTable.size(); ++i){
            connectionsTableOutput += QString::number(connectionsTable[i].at(0).toLongLong()) + ";";
            connectionsTableOutput += connectionsTable[i].at(1).toString() + ";";
            connectionsTableOutput += connectionsTable[i].at(2).toString() + ";";
            connectionsTableOutput += connectionsTable[i].at(3).toString() + ";";
            connectionsTableOutput += connectionsTable[i].at(4).toDate().toString("dd/MM/yyyy") + "\n";
        }

        QFile outFile(QString(saveDir.path() + QDir::separator() + tr("active_connections.csv")));
        outFile.open(QIODevice::WriteOnly | QIODevice::Text);
        outFile.write(connectionsTableOutput.toLocal8Bit());
        outFile.close();
    }

    if (ui->cbExportSettings->isChecked()){
        QString settingsTableOutput = tr("Autogenarated file | SmartClass ~ Settings Table\n\n");

        settingsTableOutput += tr("Company name;Contract\n\n");

        QList< QList<QVariant> > settingsTable = myDB->retrieveAll(SmartClassGlobal::getTableName(SmartClassGlobal::SETTINGS));

        for (int i = 0; i < settingsTable.size(); ++i){
            settingsTableOutput += settingsTable[i].at(0).toString() + ";";
            settingsTableOutput += settingsTable[i].at(1).toString() + "\n";
        }

        QFile outFile(QString(saveDir.path() + QDir::separator() + tr("settings.csv")));
        outFile.open(QIODevice::WriteOnly | QIODevice::Text);
        outFile.write(settingsTableOutput.toLocal8Bit());
        outFile.close();
    }

    if (ui->cbExportImages->isChecked()){
        if (ui->cbParentImages->isChecked()){
            if (saveDir.mkdir(tr("Responsible images"))){
                QString pImagesDirPath = exportDir + QDir::separator() + tr("Responsible images");
                QDir pImagesDir(pImagesDirPath);

                QList< QList<QVariant> > parentsTable = myDB->retrieveAll(SmartClassGlobal::getTableName(SmartClassGlobal::RESPONSIBLE),
                                                                          QStringList() << SmartClassGlobal::getTableStructure(SmartClassGlobal::RESPONSIBLE).at(0));
                QStringList pImagesTableStructure = SmartClassGlobal::getTableStructure(SmartClassGlobal::RESPONSIBLEIMAGES);

                for (int i = 0; i < parentsTable.size(); ++i){
                    QString pIDs = QString::number(parentsTable[i].at(0).toLongLong());
                    pImagesDir.mkdir(pIDs);
                    QString newPath = pImagesDirPath + QDir::separator() + pIDs;

                    QList<QVariant> pImagesTable = myDB->retrieveRow(SmartClassGlobal::getTableName(SmartClassGlobal::RESPONSIBLEIMAGES),
                                                                     pImagesTableStructure.at(0),
                                                                     parentsTable[i].at(0));
                    for (int j = 1; j < pImagesTable.length(); ++j){
                        if (!pImagesTable.at(j).isNull()){
                            QFile imageOut(newPath + QDir::separator() + pImagesTableStructure.at(j));
                            imageOut.open(QIODevice::WriteOnly);
                            myDB->variantToPixmap(pImagesTable.at(j)).save(&imageOut, "PNG");
                        }
                    }
                }
            }
            else QMessageBox::warning(this, tr("Error | SmartClass"),
                                      tr("It was not possible to create the folder which will contain the images related to the parents"),
                                      QMessageBox::Ok, QMessageBox::NoButton);
        }

        if (ui->cbStudentImages->isChecked()){
            if (saveDir.mkdir(tr("Student images"))){
                QString sImagesDirPath = exportDir + QDir::separator() + tr("Student images");
                QDir sImagesDir(sImagesDirPath);

                QList< QList<QVariant> > studentsTable = myDB->retrieveAll(SmartClassGlobal::getTableName(SmartClassGlobal::STUDENT),
                                                                          QStringList() << SmartClassGlobal::getTableStructure(SmartClassGlobal::STUDENT).at(0));
                QStringList sImagesTableStructure = SmartClassGlobal::getTableStructure(SmartClassGlobal::STUDENTIMAGES);

                for (int i = 0; i < studentsTable.size(); ++i){
                    QString sIDs = QString::number(studentsTable[i].at(0).toLongLong());
                    sImagesDir.mkdir(sIDs);
                    QString newPath = sImagesDirPath + QDir::separator() + sIDs;

                    QList<QVariant> sImagesTable = myDB->retrieveRow(SmartClassGlobal::getTableName(SmartClassGlobal::STUDENTIMAGES),
                                                                     sImagesTableStructure.at(0),
                                                                     studentsTable[i].at(0));

                    for (int j = 1; j < sImagesTable.length(); ++j){
                        if (!sImagesTable.at(j).isNull()){
                            QFile imageOut(newPath + QDir::separator() + sImagesTableStructure.at(j));
                            imageOut.open(QIODevice::WriteOnly);
                            myDB->variantToPixmap(sImagesTable.at(j)).save(&imageOut, "PNG");
                        }
                    }
                }
            }
            else QMessageBox::warning(this, tr("Error | SmartClass"),
                                      tr("It was not possible to create the folder which will contain the images related to the students"),
                                      QMessageBox::Ok, QMessageBox::NoButton);
        }

        if (ui->cbCompanyLogo->isChecked()){
            if (saveDir.mkdir(tr("Global settings"))){
                QString sImagesDirPath = exportDir + QDir::separator() + tr("Global settings");

                QList< QList<QVariant> > settingsTable = myDB->retrieveAll(SmartClassGlobal::getTableName(SmartClassGlobal::SETTINGS));

                for (int i = 0; i < settingsTable.size(); ++i){
                    QPixmap logo(myDB->variantToPixmap(settingsTable[i].at(2)));
                    if (!logo.isNull()){
                        QFile imageOut(sImagesDirPath + QDir::separator() + tr("company_logo.png"));
                        imageOut.open(QIODevice::WriteOnly);
                        logo.save(&imageOut, "PNG");
                    }
                }
            }
            else QMessageBox::warning(this, tr("Error | SmartClass"),
                                      tr("It was not possible to create the folder which will contain the company logo"),
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
