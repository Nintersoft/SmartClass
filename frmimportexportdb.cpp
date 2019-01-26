#include "frmimportexportdb.h"
#include "ui_frmimportexportdb.h"

frmImportExportDB::frmImportExportDB(QWidget *parent, ImportMode mode) :
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

    db_manager = DBManager::getInstance();

    if (CURRENT_MODE == frmImportExportDB::Export){
        ui->grpImportOptions->setVisible(false);
        ui->btImport->setVisible(false);
        this->setMaximumHeight(640);
        this->setMinimumHeight(640);
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
        connect(ui->btSearchSettings, SIGNAL(clicked(bool)), this, SLOT(getImportFilePath()));
        connect(ui->btSearchActiveConnectionsFile, SIGNAL(clicked(bool)), this, SLOT(getImportFilePath()));
        connect(ui->btSearchCompanyLogo, SIGNAL(clicked(bool)), this, SLOT(getImportFilePath()));

        connect(ui->btSearchStudentImagesDir, SIGNAL(clicked(bool)), this, SLOT(getImportDir()));
        connect(ui->btSearchParentImagesDir, SIGNAL(clicked(bool)), this, SLOT(getImportDir()));

        connect(ui->btClearStudentsPath, SIGNAL(clicked(bool)), ui->edtStudentFilePath, SLOT(clear()));
        connect(ui->btClearParentsPath, SIGNAL(clicked(bool)), ui->edtParentFilePath, SLOT(clear()));
        connect(ui->btClearCoursesPath, SIGNAL(clicked(bool)), ui->edtCoursesFilePath, SLOT(clear()));
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

    currentImportDir = QDir::home();
}

frmImportExportDB::~frmImportExportDB()
{
    delete ui;
}

QVariantList frmImportExportDB::stringToVariant(const QStringList &sList, SmartClassGlobal::TablesSpec tableSpec){
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
        case SmartClassGlobal::SETTINGS:
            converted << sList[0]
                      << (sList[1].isEmpty() ? QVariant() : sList[1].replace("$<new_line>$", "\n"))
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
                      << sList[4].toInt();
            break;
        default:
            break;
    }

    return converted;
}

void frmImportExportDB::importDB(){
    if (QMessageBox::question(NULL, tr("Confirmation | SmartClass"),
                              tr("You are about to import the data contained on the specified files to the database.\n"
                                 "This action cannot be undone. So, DO NOT PROCEED WITHOUT AN UPDATED BACKUP!\n"
                                 "It is also important to state that you should not abort this operation, under any circumstances (DO NOT close the application or shut down the computer).\n"
                                 "Continue?"),
                              QMessageBox::Yes, QMessageBox::No) == QMessageBox::No){
        return;
    }

    if (ui->cbCheckForErrors->isChecked()) importCheckingErrors();
    else if (QMessageBox::warning(NULL, tr("Warning | SmartClass"),
                                  tr("You have choosen to import the data without cheking for possible errors."
                                     "\nAlthough it becomes the operation a bit faster, it also makes harder to discover where there is an error, if any."),
                                  QMessageBox::Yes, QMessageBox::No) == QMessageBox::Yes){
        importWithoutCheckErrors();
    }
}

void frmImportExportDB::importCheckingErrors(){
    QStringList paths, tables;
    paths << ui->edtParentFilePath->text()
          << ui->edtStudentFilePath->text()
          << ui->edtCoursesFilePath->text()
          << ui->edtPaymentFilePath->text()
          << ui->edtUsersFilePath->text()
          << ui->edtActiveConnectionsFilePath->text()
          << ui->edtSettingsFilePath->text()
          << ui->edtStudentImagesDirPath->text()
          << ui->edtParentImagesDirPath->text()
          << ui->edtCompanyLogoPath->text();

    tables << SmartClassGlobal::getTableName(SmartClassGlobal::RESPONSIBLE)
           << SmartClassGlobal::getTableName(SmartClassGlobal::STUDENT)
           << SmartClassGlobal::getTableName(SmartClassGlobal::COURSEDETAILS)
           << SmartClassGlobal::getTableName(SmartClassGlobal::PAYMENTDETAILS)
           << SmartClassGlobal::getTableName(SmartClassGlobal::USERS)
           << SmartClassGlobal::getTableName(SmartClassGlobal::ACTIVECONNECTIONS)
           << SmartClassGlobal::getTableName(SmartClassGlobal::SETTINGS)
           << SmartClassGlobal::getTableName(SmartClassGlobal::STUDENTIMAGES)
           << SmartClassGlobal::getTableName(SmartClassGlobal::RESPONSIBLEIMAGES);

    QFile files[6];
    for (int i = 0; i < tables.size() - 3; ++i){
        QString importErrors = "";
        bool abort = false;

        if (!paths.at(i).isEmpty()){
            if (!QFile::exists(paths.at(i))){
                QString argument = QString(tables.at(i)).split("_").at(1);
                abort = QMessageBox::critical(NULL, tr("Error | SmartClass"),
                                              tr("The path to the %1 file does not exists. This file will be skipped during the process of importation."
                                              "\nContinue?").arg(argument),
                                              QMessageBox::Yes, QMessageBox::Abort) == QMessageBox::Abort;
                importErrors += tr("The path to the %1 file does not exists. This file has been skipped during the process of importation.").arg(argument);
            }
            else {
                files[i].setFileName(paths.at(i));
                if (!files[i].open(QIODevice::ReadOnly)){
                    QString argument = QString(tables.at(i)).split("_").at(1);
                    abort = QMessageBox::critical(NULL, tr("Error | SmartClass"),
                                                  tr("The file \"%1\" could not be opened."
                                                  "\nContinue?").arg(argument),
                                                  QMessageBox::Yes, QMessageBox::Abort) == QMessageBox::Abort;
                    importErrors += tr("The file %1 could not be opened. This file has been skipped during the process of importation.").arg(argument);
                }
            }
        }

        if (!importErrors.isEmpty()){
            QFileInfo fInfo(paths.at(i));
            QString parentPath = fInfo.path() + QDir::separator() + fInfo.baseName();
            parentPath += "_log.txt";

            QFile logFile(parentPath);
            logFile.open(QIODevice::WriteOnly | QIODevice::Text);
            logFile.write(importErrors.toLocal8Bit());
            logFile.close();
        }

        if (abort){
            QMessageBox::information(NULL, tr("Info | SmartClass"),
                                     tr("The operation has been aborted successfully!"),
                                     QMessageBox::Ok);
            return;
        }
    }

    for (int i = 0; i < tables.size() - 3; ++i){
        SmartClassGlobal::TablesSpec currentTable;
        switch (i) {
            case 0:
                currentTable = SmartClassGlobal::RESPONSIBLE;
                break;
            case 1:
                currentTable = SmartClassGlobal::STUDENT;
                break;
            case 2:
                currentTable = SmartClassGlobal::COURSEDETAILS;
                break;
            case 3:
                currentTable = SmartClassGlobal::PAYMENTDETAILS;
                break;
            case 4:
                currentTable = SmartClassGlobal::USERS;
                break;
            default:
                currentTable = SmartClassGlobal::ACTIVECONNECTIONS;
                break;
        }

        if (files[i].isOpen()){
            QString importErrors = "";

            QString content = QString::fromLocal8Bit(files[i].readAll());
            files[i].close();

            QStringList lines = content.split(QRegExp("(\\n|\\r\\n)"));
            int linesCount = lines.length() - 1;
            for (int k = 4; k < linesCount; ++k){
                QStringList lineContent;
                lineContent = QString(lines.at(k)).split(";");

                QVariantList newData = stringToVariant(lineContent, currentTable);
                QStringList tableSchema = SmartClassGlobal::getTableAliases(currentTable);

                ImportOperation operation = Insert;

                if (db_manager->rowExists(tables.at(i), tableSchema, newData)){
                    importErrors += tr("[SKIPPED] The following line already exists or conflicts with an existing one: %1 . Line skipped.\n").arg(lines.at(k));
                    operation = NoOperation;
                }
                else if (i == 3){
                    operation = Insert;
                }
                else if (db_manager->rowExists(tables.at(i), tableSchema.at(0), newData.at(0))){
                    importErrors += tr("[UPDATED / WARNING] The following property (%1) already exists with different values (%2) in %3. It has been updated to the content of the CSV file.\n").arg(tableSchema.at(0))
                                                                                                                                                                                                .arg(lines.at(k))
                                                                                                                                                                                                .arg(tables.at(i));
                    operation = Update;
                }
                else if (i <= 4){
                    if (db_manager->rowExists(tables.at(i), tableSchema.at(1), newData.at(1))){
                        importErrors += tr("[UPDATED / WARNING] The following property (%1) already exists with different values (%2) in %3. It has been updated to the content of the CSV file.\n").arg(tableSchema.at(0))
                                                                                                                                                                                                    .arg(lines.at(k))
                                                                                                                                                                                                    .arg(tables.at(i));
                        operation = Update;
                    }
                }

                if (operation == Update){
                    if (!db_manager->updateRow(tables.at(i), tableSchema.at(0), newData.at(0), tableSchema, newData))
                         importErrors += tr("[ERROR] The following line could not be added to the database: %1 . Details: %2 . Please, ignore the previous update warning.\n").arg(lines.at(k)).arg(db_manager->lastError().text());
                }
                else if (operation == Insert){
                    if (!db_manager->insertRow(tables.at(i), tableSchema, newData))
                        importErrors += tr("[ERROR] The following line could not be added to the database: %1 . Details: %2 .\n").arg(lines.at(k)).arg(db_manager->lastError().text());
                }
            }
            if (linesCount < 5) importErrors += tr("The quantity of lines does not match with the minimal requirements to import into %1 table. This file has been skipped.").arg(tables.at(i));

            if (!importErrors.isEmpty()){
                QFileInfo fInfo(paths.at(i));
                QString parentPath = fInfo.path() + QDir::separator() + fInfo.baseName();
                parentPath += "_log.txt";

                QFile logFile(parentPath);
                logFile.open(QIODevice::WriteOnly | QIODevice::Text);
                logFile.write(importErrors.toLocal8Bit());
                logFile.close();
            }
        }
    }

    if (!paths.at(6).isEmpty()){
        QString importErrors = "";

        if (!QFile::exists(paths.at(6))){
            QString argument = QString(tables.at(6)).split("_").at(1);
            QMessageBox::critical(NULL, tr("Error | SmartClass"),
                                  tr("The path to the file %1 does not exists. This file has been skipped during the process of importation.").arg(argument),
                                  QMessageBox::Ok, QMessageBox::NoButton);
            importErrors += tr("The path to the file %1 does not exists. This file has been skipped during the process of importation.").arg(argument);
        }
        else {
            QFile file(paths.at(6));
            if (file.open(QIODevice::ReadOnly)){
                QString content = QString::fromLocal8Bit(file.readAll());
                file.close();

                QStringList lines = content.split(QRegExp("(\\n|\\r\\n)"));
                int linesCount = lines.length() - 1;

                for (int k = 4; k < linesCount; ++k){
                    QStringList lineContent;
                    lineContent = QString(lines.at(k)).split(";");

                    QVariantList newData = stringToVariant(lineContent, SmartClassGlobal::SETTINGS);
                    QStringList tableSchema = SmartClassGlobal::getTableAliases(SmartClassGlobal::SETTINGS);
                    if (db_manager->rowExists(tables.at(6), tableSchema, newData))
                        importErrors += tr("[SKIPPED] The following line already exists or conflicts with an existing one: %1 . Line skipped.\n").arg(lines.at(k));
                    else {
                        importErrors += tr("[CLEAN-STEP] Settings table content removed. Every global setting (such as branding and contract model) has been reset to default. This is a standard step, not an error.");
                        db_manager->clearTable(tables.at(6));
                        if (!paths.at(9).isEmpty()){
                            if (!QFile::exists(paths.at(9))){
                                importErrors += tr("The path to the logo file does not exists. This file has been skipped during the process of importation.");

                                if (!db_manager->insertRow(tables.at(6), tableSchema, newData))
                                    importErrors += tr("[ERROR] The following line could not be added to the database: %1 . Details: %2 .\n").arg(lines.at(k)).arg(db_manager->lastError().text());
                            }
                            else {
                                newData << DBManager::pixmapToVariant(QPixmap(paths.at(9)));
                                if (!db_manager->insertRow(tables.at(6), tableSchema, newData))
                                    importErrors += tr("[ERROR] The following line could not be added to the database: %1 . Details: %2 .\n").arg(lines.at(k)).arg(db_manager->lastError().text());
                            }
                        }
                        else if (!db_manager->insertRow(tables.at(6), tableSchema, newData))
                                importErrors += tr("[ERROR] The following line could not be added to the database: %1 . Details: %2 .\n").arg(lines.at(k)).arg(db_manager->lastError().text());
                    }
                }
                if (linesCount < 5) importErrors += tr("The quantity of lines does not match with the minimal requirements to import into %1 table. This file has been skipped.").arg(tables.at(6));
            }
            else {
                QString argument = QString(tables.at(6)).split("_").at(1);
                QMessageBox::information(NULL, tr("Error | SmartClass"),
                                         tr("The file \"%1\" could not be opened."
                                            " This file has been skipped during the process of importation.").arg(argument),
                                         QMessageBox::Ok, QMessageBox::NoButton);
                importErrors += tr("The file %1 could not be opened. This file has been skipped during the process of importation.").arg(argument);
            }
        }

        if (!importErrors.isEmpty()){
            QFileInfo fInfo(paths.at(6));
            QString parentPath = fInfo.path() + QDir::separator() + fInfo.baseName();
            parentPath += "_log.txt";

            QFile logFile(parentPath);
            logFile.open(QIODevice::WriteOnly | QIODevice::Text);
            logFile.write(importErrors.toLocal8Bit());
            logFile.close();
        }
    }

    if (!paths.at(7).isEmpty()){
        QString importErrors = "";

        QDir imagesPath(paths.at(7));
        if (imagesPath.exists()){
            QStringList directories = imagesPath.entryList(QDir::Dirs, QDir::Name),
                        tableSchema = SmartClassGlobal::getTableAliases(SmartClassGlobal::STUDENTIMAGES);
            directories.removeOne(".");
            directories.removeOne("..");

            for (int i = 0; i < directories.length(); ++i){
                qlonglong sID = directories.at(i).toLongLong();
                bool sIExists = db_manager->rowExists(SmartClassGlobal::getTableName(SmartClassGlobal::STUDENTIMAGES),
                                                      tableSchema.at(0), sID);
                if (sIExists)
                    importErrors += tr("[OVERWRITTEN] The student with ID \"%1\" already exists in the database. Every image in the register have been overwritten by the existent images in the import directory.\n").arg(directories.at(i));

                QString parentPath = paths.at(7) + QDir::separator() + directories.at(i) + QDir::separator();
                QVariantList newData;
                newData << sID;

                if (QDir(parentPath).exists()){
                    for (int k = 1; k < tableSchema.length(); ++k){
                        QString imagePath = parentPath + tableSchema.at(k) + ".png";
                        newData << (QFile::exists(imagePath) ? DBManager::pixmapToVariant(QPixmap(imagePath)) : QVariant());
                    }

                    if (sIExists){
                        if (!db_manager->updateRow(SmartClassGlobal::getTableName(SmartClassGlobal::STUDENTIMAGES),
                                                   tableSchema.at(0), sID,
                                                   tableSchema.mid(1, 2), newData.mid(1, 2)))
                            importErrors += tr("[BROKEN] The following ID \"%1\" could not be added to the database. Details: %2 .\n").arg(directories.at(i)).arg(db_manager->lastError().text());
                    }
                    else if (!db_manager->insertRow(SmartClassGlobal::getTableName(SmartClassGlobal::STUDENTIMAGES),
                                              tableSchema, newData))
                        importErrors += tr("[BROKEN] The following ID \"%1\" could not be added to the database. Details: %2 .\n").arg(directories.at(i)).arg(db_manager->lastError().text());
                }
                else importErrors += tr("[SKIPPED] The following directory \"%1\" does not exists.\n").arg(parentPath);
            }
        }
        else importErrors += tr("[UNAVAILABLE] The selected directory does not exists.\n");

        if (!importErrors.isEmpty()){
            QString parentPath = (paths.at(7) + QString(QDir::separator()) + "log.txt");

            QFile logFile(parentPath);
            logFile.open(QIODevice::WriteOnly | QIODevice::Text);
            logFile.write(importErrors.toLocal8Bit());
            logFile.close();
        }
    }

    if (!paths.at(8).isEmpty()){
        QString importErrors = "";

        QDir imagesPath(paths.at(8));
        if (imagesPath.exists()){
            QStringList directories = imagesPath.entryList(QDir::Dirs, QDir::Name),
                        tableSchema = SmartClassGlobal::getTableAliases(SmartClassGlobal::RESPONSIBLEIMAGES);
            directories.removeOne(".");
            directories.removeOne("..");

            for (int i = 0; i < directories.length(); ++i){
                qlonglong rID = directories.at(i).toLongLong();
                bool sIExists = db_manager->rowExists(SmartClassGlobal::getTableName(SmartClassGlobal::RESPONSIBLEIMAGES),
                                                tableSchema.at(0), rID);

                if (sIExists)
                    importErrors += tr("[OVERWRITTEN] The responsible with ID \"%1\" already exists in the database. Every image in the register have been overwritten by the existent images in the import directory.\n").arg(directories.at(i));

                QString parentPath = paths.at(8) + QDir::separator() + directories.at(i) + QDir::separator();
                QVariantList newData;
                newData << rID;

                if (QDir(parentPath).exists()){
                    for (int k = 1; k < tableSchema.length(); ++k){
                        QString imagePath = parentPath + tableSchema.at(k) + ".png";
                        newData << (QFile::exists(imagePath) ? DBManager::pixmapToVariant(QPixmap(imagePath)) : QVariant());
                    }

                    if (sIExists){
                        if (!db_manager->updateRow(SmartClassGlobal::getTableName(SmartClassGlobal::RESPONSIBLEIMAGES),
                                             tableSchema.at(0), rID,
                                             tableSchema.mid(1, 3), newData.mid(1, 3)))
                            importErrors += tr("[BROKEN] The following ID \"%1\" could not be added to the database. Details: %2 .\n").arg(directories.at(i)).arg(db_manager->lastError().text());
                    }
                    else if (!db_manager->insertRow(SmartClassGlobal::getTableName(SmartClassGlobal::RESPONSIBLEIMAGES),
                                              tableSchema, newData))
                        importErrors += tr("[BROKEN] The following ID \"%1\" could not be added to the database. Details: %2 .\n").arg(directories.at(i).arg(db_manager->lastError().text()));
                }
                else importErrors += tr("[SKIPPED] The following directory \"%1\" does not exists.\n").arg(parentPath);
            }
        }
        else importErrors += tr("[UNAVAILABLE] The selected directory does not exists.\n");

        if (!importErrors.isEmpty()){
            QString parentPath = (paths.at(8) + QString(QDir::separator()) + "log.txt");

            QFile logFile(parentPath);
            logFile.open(QIODevice::WriteOnly | QIODevice::Text);
            logFile.write(importErrors.toLocal8Bit());
            logFile.close();
        }
    }

    QMessageBox::information(NULL, tr("Errors | SmartClass"),
                             tr("The process of importation has finished successfully. Please, refer to the log files in order to check if there is any important occurrence.\n"
                                "This application is about to restart!"),
                             QMessageBox::Ok, QMessageBox::NoButton);
    QDesktopServices::openUrl(QUrl(QCoreApplication::applicationFilePath()));
    qApp->quit();
}

void frmImportExportDB::importWithoutCheckErrors(){
    QStringList paths, tables;
    paths << ui->edtParentFilePath->text()
          << ui->edtStudentFilePath->text()
          << ui->edtCoursesFilePath->text()
          << ui->edtPaymentFilePath->text()
          << ui->edtUsersFilePath->text()
          << ui->edtActiveConnectionsFilePath->text()
          << ui->edtSettingsFilePath->text()
          << ui->edtStudentImagesDirPath->text()
          << ui->edtParentImagesDirPath->text()
          << ui->edtCompanyLogoPath->text();

    tables << SmartClassGlobal::getTableName(SmartClassGlobal::RESPONSIBLE)
           << SmartClassGlobal::getTableName(SmartClassGlobal::STUDENT)
           << SmartClassGlobal::getTableName(SmartClassGlobal::COURSEDETAILS)
           << SmartClassGlobal::getTableName(SmartClassGlobal::PAYMENTDETAILS)
           << SmartClassGlobal::getTableName(SmartClassGlobal::USERS)
           << SmartClassGlobal::getTableName(SmartClassGlobal::ACTIVECONNECTIONS)
           << SmartClassGlobal::getTableName(SmartClassGlobal::SETTINGS)
           << SmartClassGlobal::getTableName(SmartClassGlobal::STUDENTIMAGES)
           << SmartClassGlobal::getTableName(SmartClassGlobal::RESPONSIBLEIMAGES);

    QFile files[6];
    for (int i = 0; i < tables.size() - 3; ++i){
        if (!paths.at(i).isEmpty()){
            if (QFile::exists(paths.at(i))){
                files[i].setFileName(paths.at(i));
                files[i].open(QIODevice::ReadOnly);
            }
        }
    }

    for (int i = 0; i < tables.size() - 3; ++i){
        SmartClassGlobal::TablesSpec currentTable;
        switch (i) {
            case 0:
                currentTable = SmartClassGlobal::RESPONSIBLE;
                break;
            case 1:
                currentTable = SmartClassGlobal::STUDENT;
                break;
            case 2:
                currentTable = SmartClassGlobal::COURSEDETAILS;
                break;
            case 3:
                currentTable = SmartClassGlobal::PAYMENTDETAILS;
                break;
            case 4:
                currentTable = SmartClassGlobal::USERS;
                break;
            default:
                currentTable = SmartClassGlobal::ACTIVECONNECTIONS;
                break;
        }

        if (files[i].isOpen()){
            QString content = QString::fromLocal8Bit(files[i].readAll());
            files[i].close();

            QStringList lines = content.split(QRegExp("(\\n|\\r\\n)"));
            int linesCount = lines.length() - 1;
            for (int k = 4; k < linesCount; ++k){
                QStringList lineContent;
                lineContent = QString(lines.at(k)).split(";");

                QVariantList newData = stringToVariant(lineContent, currentTable);
                QStringList tableSchema = SmartClassGlobal::getTableAliases(currentTable);

                ImportOperation operation = Insert;

                if (db_manager->rowExists(tables.at(i), tableSchema, newData)) operation = NoOperation;
                else if (i == 3) operation = Insert;
                else if (db_manager->rowExists(tables.at(i), tableSchema.at(0), newData.at(0))) operation = Update;
                else if (i <= 4)
                    if (db_manager->rowExists(tables.at(i), tableSchema.at(1), newData.at(1))) operation = Update;

                if (operation == Update) db_manager->updateRow(tables.at(i), tableSchema.at(0), newData.at(0), tableSchema, newData);
                else if (operation == Insert) db_manager->insertRow(tables.at(i), tableSchema, newData);
            }
        }
    }

    if (!paths.at(6).isEmpty()){
        QFile file(paths.at(6));
        if (file.open(QIODevice::ReadOnly)){
            QString content = QString::fromLocal8Bit(file.readAll());
            file.close();

            QStringList lines = content.split(QRegExp("(\\n|\\r\\n)"));
            int linesCount = lines.length() - 1;

            for (int k = 4; k < linesCount; ++k){
                QStringList lineContent;
                lineContent = QString(lines.at(k)).split(";");

                QVariantList newData = stringToVariant(lineContent, SmartClassGlobal::SETTINGS);
                QStringList tableSchema = SmartClassGlobal::getTableAliases(SmartClassGlobal::SETTINGS);
                if (!db_manager->rowExists(tables.at(6), tableSchema, newData)){
                    db_manager->clearTable(tables.at(6));
                    if (!paths.at(9).isEmpty()){
                        if (!QFile::exists(paths.at(9)))
                            db_manager->insertRow(tables.at(6), tableSchema, newData);
                        else {
                            newData << DBManager::pixmapToVariant(QPixmap(paths.at(9)));
                            db_manager->insertRow(tables.at(6), tableSchema, newData);
                        }
                    }
                    else db_manager->insertRow(tables.at(6), tableSchema, newData);
                }
            }
        }
    }

    if (!paths.at(7).isEmpty()){
        QDir imagesPath(paths.at(7));
        if (imagesPath.exists()){
            QStringList directories = imagesPath.entryList(QDir::Dirs, QDir::Name),
                        tableSchema = SmartClassGlobal::getTableAliases(SmartClassGlobal::STUDENTIMAGES);
            directories.removeOne(".");
            directories.removeOne("..");

            for (int i = 0; i < directories.length(); ++i){
                qlonglong sID = directories.at(i).toLongLong();

                QString parentPath = paths.at(7) + QDir::separator() + directories.at(i) + QDir::separator();
                QVariantList newData;
                newData << sID;

                if (QDir(parentPath).exists()){
                    for (int k = 1; k < tableSchema.length(); ++k){
                        QString imagePath = parentPath + tableSchema.at(k) + ".png";
                        newData << (QFile::exists(imagePath) ? DBManager::pixmapToVariant(QPixmap(imagePath)) : QVariant());
                    }

                    if (db_manager->rowExists(SmartClassGlobal::getTableName(SmartClassGlobal::STUDENTIMAGES),
                                              tableSchema.at(0), sID)){
                        db_manager->updateRow(SmartClassGlobal::getTableName(SmartClassGlobal::STUDENTIMAGES),
                                              tableSchema.at(0), sID,
                                              tableSchema.mid(1, 2), newData.mid(1, 2));
                    }
                    else db_manager->insertRow(SmartClassGlobal::getTableName(SmartClassGlobal::STUDENTIMAGES),
                                              tableSchema, newData);
                }
            }
        }
    }

    if (!paths.at(8).isEmpty()){
        QDir imagesPath(paths.at(8));
        if (imagesPath.exists()){
            QStringList directories = imagesPath.entryList(QDir::Dirs, QDir::Name),
                        tableSchema = SmartClassGlobal::getTableAliases(SmartClassGlobal::RESPONSIBLEIMAGES);
            directories.removeOne(".");
            directories.removeOne("..");

            for (int i = 0; i < directories.length(); ++i){
                qlonglong rID = directories.at(i).toLongLong();

                QString parentPath = paths.at(8) + QDir::separator() + directories.at(i) + QDir::separator();
                QVariantList newData;
                newData << rID;

                if (QDir(parentPath).exists()){
                    for (int k = 1; k < tableSchema.length(); ++k){
                        QString imagePath = parentPath + tableSchema.at(k) + ".png";
                        newData << (QFile::exists(imagePath) ? DBManager::pixmapToVariant(QPixmap(imagePath)) : QVariant());
                    }

                    if (db_manager->rowExists(SmartClassGlobal::getTableName(SmartClassGlobal::RESPONSIBLEIMAGES),
                                              tableSchema.at(0), rID)){
                        db_manager->updateRow(SmartClassGlobal::getTableName(SmartClassGlobal::RESPONSIBLEIMAGES),
                                              tableSchema.at(0), rID,
                                              tableSchema.mid(1, 3), newData.mid(1, 3));
                    }
                    else db_manager->insertRow(SmartClassGlobal::getTableName(SmartClassGlobal::RESPONSIBLEIMAGES),
                                               tableSchema, newData);
                }
            }
        }
    }
    QMessageBox::information(NULL, tr("Errors | SmartClass"),
                             tr("The process of importation has finished successfully.\n"
                                "This application is about to restart!"),
                             QMessageBox::Ok, QMessageBox::NoButton);
    QDesktopServices::openUrl(QUrl(QCoreApplication::applicationFilePath()));
    qApp->quit();
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
    QString senderName = sender()->objectName();

    QFileDialog selectFile;
    selectFile.setWindowTitle(tr("Select file | SmartClass"));
    selectFile.setAcceptMode(QFileDialog::AcceptOpen);
    selectFile.setFileMode(QFileDialog::ExistingFile);
    if (senderName == "btSearchCompanyLogo")
        selectFile.setNameFilters(QStringList() << tr("PNG image file (*.png)")
                                                << tr("All files (*.*)"));
    else
        selectFile.setNameFilters(QStringList() << tr("Comma separated values files (*.csv)")
                                                << tr("All files (*.*)"));
    selectFile.setDirectory(currentImportDir);
    if (selectFile.exec()){
        if (senderName == "btSearchStudentFile")
            ui->edtStudentFilePath->setText(selectFile.selectedFiles().at(0));
        else if (senderName == "btSearchParentFile")
            ui->edtParentFilePath->setText(selectFile.selectedFiles().at(0));
        else if (senderName == "btSearchCoursesFile")
            ui->edtCoursesFilePath->setText(selectFile.selectedFiles().at(0));
        else if (senderName == "btSearchPaymentFile")
            ui->edtPaymentFilePath->setText(selectFile.selectedFiles().at(0));
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
        QMessageBox::information(NULL, tr("Warning | SmartClass"), errorMsg, QMessageBox::Ok, QMessageBox::NoButton);
        return;
    }

    QDir saveDir(exportDir);
    if (!saveDir.mkpath(exportDir)){
        QMessageBox::critical(NULL, tr("Error | SmartClass"), tr("It was not possible to create the backup directory. Does this path requires special permissions?"), QMessageBox::Ok, QMessageBox::Ok);
        return;
    }

    if (ui->cbExportStudentsTable->isChecked()){
        QFile outFile(QString(saveDir.path() + QDir::separator() + tr("students.csv")));
        outFile.open(QIODevice::WriteOnly | QIODevice::Text);

        QString studentsTableOutput = tr("Autogenarated file | SmartClass ~ Students Table\n\n");

        studentsTableOutput += tr("ID;Name;Responsible ID;Birthday;ID (document);School;Observations;"
                                  "Experimental course;Experimental course date;Experimental course observations\n\n");

        QList< QVariantList > studentsTable = db_manager->retrieveAll(SmartClassGlobal::getTableName(SmartClassGlobal::STUDENT),
                                                                      SmartClassGlobal::getTableAliases(SmartClassGlobal::STUDENT));

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

            outFile.write(studentsTableOutput.toLocal8Bit());
            studentsTableOutput = "";
        }

        outFile.close();
    }

    if (ui->cbExportParentsTable->isChecked()){
        QFile outFile(QString(saveDir.path() + QDir::separator() + tr("responsibles.csv")));
        outFile.open(QIODevice::WriteOnly | QIODevice::Text);

        QString parentsTableOutput = tr("Autogenarated file | SmartClass ~ Responsibles Table\n\n");

        parentsTableOutput += tr("ID;Name;Phone number;Mobile phone operator;Mobile phone number;"
                                    "Email;ID (document);CPG;How have he met us?;Address\n\n");

        QList< QVariantList > parentsTable = db_manager->retrieveAll(SmartClassGlobal::getTableName(SmartClassGlobal::RESPONSIBLE),
                                                                     SmartClassGlobal::getTableAliases(SmartClassGlobal::RESPONSIBLE));

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

            outFile.write(parentsTableOutput.toLocal8Bit());
            parentsTableOutput = "";
        }

        outFile.close();
    }

    if (ui->cbExportCoursesTable->isChecked()){
        QFile outFile(QString(saveDir.path() + QDir::separator() + tr("courses.csv")));
        outFile.open(QIODevice::WriteOnly | QIODevice::Text);

        QString coursesTableOutput = tr("Autogenarated file | SmartClass ~ Courses Table\n\n");

        coursesTableOutput += tr("ID;Course name;Teacher;Short description;Long description;Classroom number;"
                                 "Days and time;Beginning date;End date;Price\n\n");

        QList< QVariantList > coursesTable = db_manager->retrieveAll(SmartClassGlobal::getTableName(SmartClassGlobal::COURSEDETAILS),
                                                                     SmartClassGlobal::getTableAliases(SmartClassGlobal::COURSEDETAILS));

        for (int i = 0; i < coursesTable.size(); ++i){
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

            outFile.write(coursesTableOutput.toLocal8Bit());
            coursesTableOutput = "";
        }

        outFile.close();
    }

    if (ui->cbExportPricingTable->isChecked()){
        QFile outFile(QString(saveDir.path() + QDir::separator() + tr("payment.csv")));
        outFile.open(QIODevice::WriteOnly | QIODevice::Text);

        QString paymentTableOutput = tr("Autogenarated file | SmartClass ~ Payment Details Table\n\n");

        paymentTableOutput += tr("Student ID;Course ID;Discount (percentage);First installment;Installments\n\n");

        QList< QVariantList > paymentTable = db_manager->retrieveAll(SmartClassGlobal::getTableName(SmartClassGlobal::PAYMENTDETAILS),
                                                                     SmartClassGlobal::getTableAliases(SmartClassGlobal::PAYMENTDETAILS));

        for (int i = 0; i < paymentTable.size(); ++i){
            paymentTableOutput += QString::number(paymentTable[i].at(0).toLongLong()) + ";";
            paymentTableOutput += QString::number(paymentTable[i].at(1).toLongLong()) + ";";
            paymentTableOutput += QString::number(paymentTable[i].at(2).toDouble(), 'f', 2) + ";";
            paymentTableOutput += paymentTable[i].at(3).toDate().toString("dd/MM/yyyy") + ";";
            paymentTableOutput += QString::number(paymentTable[i].at(4).toInt()) + "\n";

            outFile.write(paymentTableOutput.toLocal8Bit());
            paymentTableOutput = "";
        }

        outFile.close();
    }

    if (ui->cbExportUsersTable->isChecked()){
        QFile outFile(QString(saveDir.path() + QDir::separator() + tr("users.csv")));
        outFile.open(QIODevice::WriteOnly | QIODevice::Text);

        QString usersTableOutput = tr("Autogenarated file | SmartClass ~ Users Table\n\n");

        usersTableOutput += tr("ID;Username;Name;Password salt;Password hash;Security question;Answer salt;Answer hash;Role\n\n");

        QList< QVariantList > usersTable = db_manager->retrieveAll(SmartClassGlobal::getTableName(SmartClassGlobal::USERS),
                                                                   SmartClassGlobal::getTableAliases(SmartClassGlobal::USERS));

        for (int i = 0; i < usersTable.size(); ++i){
            usersTableOutput += QString::number(usersTable[i].at(0).toLongLong()) + ";";
            usersTableOutput += usersTable[i].at(1).toString() + ";";
            usersTableOutput += usersTable[i].at(2).toString() + ";";
            usersTableOutput += usersTable[i].at(3).toString() + ";";
            usersTableOutput += usersTable[i].at(4).toString() + ";";
            usersTableOutput += usersTable[i].at(5).toString() + ";";
            usersTableOutput += usersTable[i].at(6).toString() + ";";
            usersTableOutput += usersTable[i].at(7).toString() + ";";
            usersTableOutput += QString::number(usersTable[i].at(8).toInt()) + "\n";

            outFile.write(usersTableOutput.toLocal8Bit());
            usersTableOutput = "";
        }

        outFile.close();
    }

    if (ui->cbExportActiveConnectionsTable->isChecked()){
        QFile outFile(QString(saveDir.path() + QDir::separator() + tr("active_connections.csv")));
        outFile.open(QIODevice::WriteOnly | QIODevice::Text);

        QString connectionsTableOutput = tr("Autogenarated file | SmartClass ~ Active Connections Table\n\n");

        connectionsTableOutput += tr("ID;Device name;Operating System;OS Version;Last Access\n\n");

        QList< QList<QVariant> > connectionsTable = db_manager->retrieveAll(SmartClassGlobal::getTableName(SmartClassGlobal::ACTIVECONNECTIONS),
                                                                            SmartClassGlobal::getTableAliases(SmartClassGlobal::ACTIVECONNECTIONS));

        for (int i = 0; i < connectionsTable.size(); ++i){
            connectionsTableOutput += QString::number(connectionsTable[i].at(0).toLongLong()) + ";";
            connectionsTableOutput += connectionsTable[i].at(1).toString() + ";";
            connectionsTableOutput += connectionsTable[i].at(2).toString() + ";";
            connectionsTableOutput += connectionsTable[i].at(3).toString() + ";";
            connectionsTableOutput += connectionsTable[i].at(4).toDate().toString("dd/MM/yyyy") + "\n";

            outFile.write(connectionsTableOutput.toLocal8Bit());
            connectionsTableOutput = "";
        }

        outFile.close();
    }

    if (ui->cbExportSettings->isChecked()){
        QFile outFile(QString(saveDir.path() + QDir::separator() + tr("settings.csv")));
        outFile.open(QIODevice::WriteOnly | QIODevice::Text);

        QString settingsTableOutput = tr("Autogenarated file | SmartClass ~ Settings Table\n\n");

        settingsTableOutput += tr("Company name;Contract\n\n");

        QList< QVariantList > settingsTable = db_manager->retrieveAll(SmartClassGlobal::getTableName(SmartClassGlobal::SETTINGS),
                                                                      SmartClassGlobal::getTableAliases(SmartClassGlobal::SETTINGS));

        for (int i = 0; i < settingsTable.size(); ++i){
            settingsTableOutput += settingsTable[i].at(0).toString() + ";";
            settingsTableOutput += settingsTable[i].at(1).toString().replace(QRegExp("(\\n|\\r\\n)"), "$<new_line>$") + "\n";

            outFile.write(settingsTableOutput.toLocal8Bit());
        }

        outFile.close();
    }

    if (ui->cbExportImages->isChecked()){
        if (ui->cbParentImages->isChecked()){
            if (saveDir.mkdir(tr("Responsible images"))){
                QString pImagesDirPath = exportDir + QDir::separator() + tr("Responsible images");
                QDir pImagesDir(pImagesDirPath);

                QList< QVariantList > parentsTable = db_manager->retrieveAll(SmartClassGlobal::getTableName(SmartClassGlobal::RESPONSIBLE),
                                                                             SmartClassGlobal::getTableAliases(SmartClassGlobal::RESPONSIBLE).mid(0, 1));
                QStringList pImagesTableAliases = SmartClassGlobal::getTableAliases(SmartClassGlobal::RESPONSIBLEIMAGES);

                for (int i = 0; i < parentsTable.size(); ++i){
                    QString pIDs = QString::number(parentsTable[i].at(0).toLongLong());
                    pImagesDir.mkdir(pIDs);
                    QString newPath = pImagesDirPath + QDir::separator() + pIDs;

                    QVariantList pImagesTable = db_manager->retrieveRow(SmartClassGlobal::getTableName(SmartClassGlobal::RESPONSIBLEIMAGES),
                                                                        pImagesTableAliases.at(0),
                                                                        parentsTable[i].at(0));

                    for (int j = 1; j < pImagesTable.length(); ++j){
                        if (!pImagesTable.at(j).isNull()){
                            QFile imageOut(newPath + QDir::separator() + pImagesTableAliases.at(j) + ".png");
                            imageOut.open(QIODevice::WriteOnly);
                            DBManager::variantToPixmap(pImagesTable.at(j)).save(&imageOut, "PNG");
                            imageOut.close();
                        }
                    }
                }
            }
            else QMessageBox::warning(NULL, tr("Error | SmartClass"),
                                      tr("It was not possible to create the folder which will contain the images related to the responsibles"),
                                      QMessageBox::Ok, QMessageBox::NoButton);
        }

        if (ui->cbStudentImages->isChecked()){
            if (saveDir.mkdir(tr("Student images"))){
                QString sImagesDirPath = exportDir + QDir::separator() + tr("Student images");
                QDir sImagesDir(sImagesDirPath);

                QList< QVariantList > studentsTable = db_manager->retrieveAll(SmartClassGlobal::getTableName(SmartClassGlobal::STUDENT),
                                                                              SmartClassGlobal::getTableAliases(SmartClassGlobal::STUDENT).mid(0, 1));
                QStringList sImagesTableAliases = SmartClassGlobal::getTableAliases(SmartClassGlobal::STUDENTIMAGES);

                for (int i = 0; i < studentsTable.size(); ++i){
                    QString sIDs = QString::number(studentsTable[i].at(0).toLongLong());
                    sImagesDir.mkdir(sIDs);
                    QString newPath = sImagesDirPath + QDir::separator() + sIDs;

                    QList<QVariant> sImagesTable = db_manager->retrieveRow(SmartClassGlobal::getTableName(SmartClassGlobal::STUDENTIMAGES),
                                                                           sImagesTableAliases.at(0),
                                                                           studentsTable[i].at(0));

                    for (int j = 1; j < sImagesTable.length(); ++j){
                        if (!sImagesTable.at(j).isNull()){
                            QFile imageOut(newPath + QDir::separator() + sImagesTableAliases.at(j) + ".png");
                            imageOut.open(QIODevice::WriteOnly);
                            DBManager::variantToPixmap(sImagesTable.at(j)).save(&imageOut, "PNG");
                            imageOut.close();
                        }
                    }
                }
            }
            else QMessageBox::warning(NULL, tr("Error | SmartClass"),
                                      tr("It was not possible to create the folder which will contain the images related to the students"),
                                      QMessageBox::Ok, QMessageBox::NoButton);
        }

        if (ui->cbCompanyLogo->isChecked()){
            if (saveDir.mkdir(tr("Global settings"))){
                QString sImagesDirPath = exportDir + QDir::separator() + tr("Global settings");

                QList< QVariantList > settingsTable = db_manager->retrieveAll(SmartClassGlobal::getTableName(SmartClassGlobal::SETTINGS));

                if (settingsTable.size()){
                    QPixmap logo(DBManager::variantToPixmap(settingsTable[0].at(2)));
                    if (!logo.isNull()){
                        QFile imageOut(sImagesDirPath + QDir::separator() + tr("company_logo.png"));
                        imageOut.open(QIODevice::WriteOnly);
                        logo.save(&imageOut, "PNG");
                        imageOut.close();
                    }
                }
            }
            else QMessageBox::warning(NULL, tr("Error | SmartClass"),
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
