#include "frmfirstrun.h"
#include "ui_frmfirstrun.h"

frmFirstRun::frmFirstRun(QWidget *parent) :
    NMainWindow(parent),
    ui(new Ui::frmFirstRun)
{
    ui->setupUi(this);

    // Sets the custom Widgets on the parent Class
    // Otherwise, the window resizing feature will not work
    NMainWindow::setCustomWidgets(ui->centralWidget, ui->statusBar);
    this->setMaximizeButtonEnabled(false);

    /*
     *  End of GUI implementation
     */

    allowed = false;
    canChangeLang = true;
    settingsExists = false;

    profileID = 0;

    currentLangIndex = 0;
    ui->grpMySQLSettings->setEnabled(false);
    ui->btPrevStep->setVisible(false);
    ui->btPrevStep->setEnabled(false);
    ui->tabManager->tabBar()->hide();

    connect(ui->btNextStep, SIGNAL(clicked(bool)), this, SLOT(nextStep()));
    connect(ui->btPrevStep, SIGNAL(clicked(bool)), this, SLOT(previousStep()));

    connect(ui->btSearchCompanyLogo, SIGNAL(clicked(bool)), this, SLOT(selectCompanyLogo()));
    connect(ui->btRemoveLogo, SIGNAL(clicked(bool)), ui->edtCompanyLogoPath, SLOT(clear()));

    connect(ui->btSaveData, SIGNAL(clicked(bool)), this, SLOT(saveDBSettings()));
    connect(ui->rbMySQL, SIGNAL(toggled(bool)), ui->grpMySQLSettings, SLOT(setEnabled(bool)));

    connect(ui->cbLanguage, SIGNAL(currentIndexChanged(int)), this, SLOT(changeLanguage(int)));

    langPath = QApplication::applicationDirPath().append("/lang/");
    if (QLocale::system().name().split("_").at(0) == "pt")
        ui->cbLanguage->setCurrentIndex(1);

    db_manager = NULL;
}

frmFirstRun::~frmFirstRun()
{
    if (db_manager){
        if (db_manager->isOpen()) db_manager->closeDB();
        delete db_manager;
        db_manager = NULL;
    }
    delete ui;
}

void frmFirstRun::closeEvent(QCloseEvent *event){
    if (allowed){
        event->accept();
        QMainWindow::closeEvent(event);
        return;
    }

    QMessageBox confirmation;
    confirmation.setWindowTitle(tr("SmartClass | Critical warning!"));
    confirmation.setText(tr("You are going to quit without save the database settings. This may corrupt the setup data.\nDo you want to proceed?"));
    confirmation.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
    confirmation.setIcon(QMessageBox::Critical);

    if (confirmation.exec() == QMessageBox::Yes) qApp->quit();
    else event->ignore();
}

void frmFirstRun::nextStep(){
    int currentTab = ui->tabManager->currentIndex();

    if (currentTab == 1){
        if (db_manager){
            delete db_manager;
            db_manager = NULL;
        }

        DBManager::DBData db_data;

        if (ui->rbSQLite->isChecked()){
            db_data.setDatabaseName(SmartClassGlobal::getDBPath());
            SmartClassGlobal::setTablePrefix(ui->edtDatabasePrefix->text());
            SmartClassGlobal::setDatabaseType(DBManager::SQLITE);
            db_manager = new DBManager(db_data, SmartClassGlobal::tablePrefix(),
                                        SmartClassGlobal::databaseType(), DBManager::getUniqueConnectionName("firstRun"));
        }
        else{
            db_data.setHostName(ui->edtHost->text());
            db_data.setDatabaseName(ui->edtDatabase->text());
            db_data.setPort(ui->edtPortNumber->value());
            db_data.setUserName(ui->edtUsername->text());
            db_data.setPassword(ui->edtPassword->text());
            SmartClassGlobal::setTablePrefix(ui->edtDatabasePrefix->text());
            SmartClassGlobal::setDatabaseType(DBManager::MYSQL);
            db_manager = new DBManager(db_data, ui->edtDatabasePrefix->text(),
                                        SmartClassGlobal::databaseType(), DBManager::getUniqueConnectionName("firstRun"));
        }

        if (!db_manager->openDB()){
            QMessageBox::critical(this, tr("Error | SmartClass"),
                                    tr("An error has occurred while we tried to connect to the database. Please, check the input and try again.\nDetails: %1.").arg(db_manager->lastError().text()),
                                    QMessageBox::Ok, QMessageBox::NoButton);
            delete db_manager;
            db_manager = NULL;
            return;
        }

        db_export_data = db_data;

        QSqlQuery checkSettings = db_manager->runCustomQuery();
        checkSettings.prepare("SELECT 1 FROM " + SmartClassGlobal::getTableName(SmartClassGlobal::SETTINGS) + "LIMIT 1");
        if (checkSettings.exec()){
            ui->grpCompanySettings->setEnabled(false);
            settingsExists = true;

            if (db_manager->retrieveAll(SmartClassGlobal::getTableName(SmartClassGlobal::SETTINGS)).size()){
                QList<QVariant> settings = db_manager->retrieveAll(SmartClassGlobal::getTableName(SmartClassGlobal::SETTINGS)).at(0);
                ui->edtCompanyName->setText(settings.at(0).toString());
                cLogo = db_manager->variantToPixmap(settings.at(2));
                ui->lblCompanyLogoImg->setPixmap(cLogo.scaled(ui->lblCompanyLogoImg->size(), Qt::KeepAspectRatio));
            }
        }

        ui->edtDeviceName->setText(QHostInfo::localHostName());
        ui->edtDeviceOS->setText(QSysInfo::productType());
        ui->edtDeviceOSMV->setText(QSysInfo::productVersion());
        ui->edtDeviceLastAccess->setText(QDateTime::currentDateTime().toString("dd/MM/yyyy - HH:mm:ss"));

        QSqlQuery checkActiveC = db_manager->runCustomQuery();
        checkActiveC.prepare("SELECT 1 FROM " + SmartClassGlobal::getTableName(SmartClassGlobal::ACTIVECONNECTIONS) + "LIMIT 1");
        if (checkActiveC.exec()){
            activeConnections = db_manager->retrieveAll(SmartClassGlobal::getTableName(SmartClassGlobal::ACTIVECONNECTIONS));
            for (int i = 0; i < activeConnections.size(); ++i){
                if (activeConnections[i].at(1).toString() == ui->edtDeviceName->text()
                        && activeConnections[i].at(2).toString() == ui->edtDeviceOS->text()
                        && activeConnections[i].at(3).toString() == ui->edtDeviceOSMV->text()){
                    profileID = activeConnections[i].at(0).toLongLong();
                    break;
                }
            }
        }
        else if(!db_manager->createTable(SmartClassGlobal::getTableName(SmartClassGlobal::ACTIVECONNECTIONS),
                                         SmartClassGlobal::getTableStructure(SmartClassGlobal::ACTIVECONNECTIONS))){
            QMessageBox::critical(this, tr("Error | SmartClass"),
                                  tr("It was not possible to create the table of device access control. Please, try again."), QMessageBox::Ok, QMessageBox::NoButton);
            return;
        }
    }
    else if (currentTab == 2){
        bool complete[4], check, tryAgain;
        for (int i = 0; i < 4; ++i) complete[i] = false;

        do {
            if (!complete[0]){
                if (!settingsExists){
                    QStringList columns = SmartClassGlobal::getTableStructure(SmartClassGlobal::SETTINGS);
                    columns.removeAt(1);

                    QList<QVariant> settingsData;
                    settingsData << ui->edtCompanyName->text()
                                 << db_manager->pixmapToVariant(cLogo);

                    complete[0] = db_manager->createTable(SmartClassGlobal::getTableName(SmartClassGlobal::SETTINGS),
                                                          SmartClassGlobal::getTableStructure(SmartClassGlobal::SETTINGS))
                                    && db_manager->insertRow(SmartClassGlobal::getTableName(SmartClassGlobal::SETTINGS),
                                                             columns,
                                                             settingsData);
                }
                else complete[0] = true;
            }

            if (!complete[1] || !complete[2] || !complete[3]){
                if (profileID != -1){
                    QStringList columns = SmartClassGlobal::getTableStructure(SmartClassGlobal::ACTIVECONNECTIONS);

                    complete[1] = db_manager->updateRow(SmartClassGlobal::getTableName(SmartClassGlobal::ACTIVECONNECTIONS),
                                                        columns.at(0),
                                                        profileID,
                                                        QStringList() << columns.at(4),
                                                        QList<QVariant>() <<  ui->edtDeviceLastAccess->text());
                    complete[2] = true;
                    complete[3] = true;
                }
                else {
                    QStringList columns = SmartClassGlobal::getTableStructure(SmartClassGlobal::ACTIVECONNECTIONS);
                    QList<QVariant> data;
                    data << ui->edtDeviceName->text()
                         << ui->edtDeviceOS->text()
                         << ui->edtDeviceOSMV->text()
                         << ui->edtDeviceLastAccess->text();

                    if (complete[2]) complete[2] = db_manager->insertRow(SmartClassGlobal::getTableName(SmartClassGlobal::ACTIVECONNECTIONS),
                                                                            columns.mid(1),
                                                                            data);

                    QList<QVariant> profID = db_manager->retrieveRow(SmartClassGlobal::getTableName(SmartClassGlobal::ACTIVECONNECTIONS),
                                                                     columns.mid(1),
                                                                     data,
                                                                     QStringList() << columns.at(0));
                    complete[3] = profID.size();
                    if (complete[3]) profileID = profID.at(0).toLongLong();
                    complete[1] = true;
                }
            }

            check = complete[0];
            for (int i = 1; i < 4; ++i) check = check && complete[i];

            if (!check){
                QString errors = "";
                if (!complete[0]) errors += "- It was not possible to create or write in the settings table.\n";
                if (!complete[1]) errors += "- It was not possible to update an existing ID with your device data.\n";
                if (!complete[2]) errors += "- It was not possible to create a new ID to your device.\n";
                if (!complete[3]) errors += "- It was not possible to retrieve the new ID of your device.\n";
                tryAgain  = QMessageBox::critical(this, tr("Error | SmartClass"),
                                                  tr("Some errors have occurred while we tried to store your settings into the database.\n"
                                                     "Here are the details:\n"
                                                     "\n"
                                                     "%1\n"
                                                     "\n"
                                                     "Would you like to try to save it again?").arg(errors),
                                                  QMessageBox::Retry, QMessageBox::Abort)
                                ==  QMessageBox::Retry;
            }
            else tryAgain = false;
        } while (!check && tryAgain);
        if (!check) return;

        QSettings settings("Nintersoft", "SmartClass");

        settings.beginGroup("dbinfo");
        if (SmartClassGlobal::databaseType() == DBManager::SQLITE){
            settings.setValue("database type", QString("SQLITE"));
            settings.setValue("table prefix", SmartClassGlobal::tablePrefix());
        }
        else {
            settings.setValue("database type", QString("MYSQL"));
            settings.setValue("host", db_export_data.hostName());
            settings.setValue("database", db_export_data.databaseName());
            settings.setValue("port", db_export_data.port());
            settings.setValue("username", db_export_data.username());
            settings.setValue("password", db_export_data.password());
            settings.setValue("table prefix", SmartClassGlobal::tablePrefix());
        }
        settings.endGroup();

        settings.beginGroup("language options");
        settings.setValue("current language index", currentLangIndex);
        settings.endGroup();

        settings.beginGroup("device settings");
        settings.setValue("device ID", profileID);
        settings.endGroup();

        db_manager->closeDB();
        delete db_manager;
        db_manager = NULL;

        allowed = true;

        ui->btNextStep->setVisible(false);
        ui->btNextStep->setVisible(false);
        ui->btPrevStep->setVisible(false);
        ui->btPrevStep->setEnabled(false);
    }

    if (currentTab != 2) {
        ui->btPrevStep->setVisible(true);
        ui->btPrevStep->setEnabled(true);
    }

    ui->tabManager->setCurrentIndex(currentTab + 1);
}

void frmFirstRun::previousStep(){
    ui->tabManager->setCurrentIndex(ui->tabManager->currentIndex() - 1);

    int currentTab = ui->tabManager->currentIndex();
    if (!currentTab){
        ui->btPrevStep->setVisible(false);
        ui->btPrevStep->setEnabled(false);
    }
    else if (currentTab == 1) profileID = -1;
}

void frmFirstRun::saveDBSettings(){
    sendData(db_export_data, !currentLangIndex ? "en" : "pt");
}

void frmFirstRun::selectCompanyLogo(){
    QFileDialog selectLogoDialog;
    selectLogoDialog.setWindowTitle(tr("Select company logo | SmartClass"));
    selectLogoDialog.setAcceptMode(QFileDialog::AcceptOpen);
    selectLogoDialog.setNameFilters(QStringList() << tr("Image files (*.png *.jpeg *.jpg)") << tr("All files (*.*)"));
    selectLogoDialog.setDirectory(QDir::homePath());
    if (selectLogoDialog.exec()){
        ui->edtCompanyLogoPath->setText(selectLogoDialog.selectedFiles().at(0));
        cLogo = QPixmap(ui->edtCompanyLogoPath->text());
        ui->lblCompanyLogoImg->setPixmap(cLogo.scaled(ui->lblCompanyLogoImg->size(), Qt::KeepAspectRatio));
    }
}

void frmFirstRun::changeLanguage(int index){
    if (index == currentLangIndex) return;
    if (!canChangeLang){
        ui->cbLanguage->setCurrentIndex(currentLangIndex);
        return;
    }

    canChangeLang = false;
    currentLangIndex = index;
    if (!index) readLanguage("en");
    else readLanguage("pt");
}

void frmFirstRun::readLanguage(const QString &langSlug){
    changeTranslator(translator, QString(langPath + "SmartClass_%1.qm").arg(langSlug));
    changeTranslator(qtTranslator, QString(langPath + "qt_%1.qm").arg(langSlug));
}

void frmFirstRun::changeTranslator(QTranslator &transl, const QString &filePath){
    QApplication::removeTranslator(&transl);
    if (transl.load(filePath)) QApplication::installTranslator(&transl);
}

void frmFirstRun::changeEvent(QEvent *event){
    if (event != NULL && event->type() == QEvent::LanguageChange){
        ui->retranslateUi(this);
        canChangeLang = true;
    }
    NMainWindow::changeEvent(event);
}
