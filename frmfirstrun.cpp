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

    profileID = -1;

    currentLangIndex = 0;
    ui->grpMySQLSettings->setEnabled(false);
    ui->btPrevStep->setVisible(false);
    ui->btPrevStep->setEnabled(false);
    ui->tabManager->tabBar()->hide();

    //NMainWindow::setTitleBarFixedWidth(this->width());

    connect(ui->btNextStep, SIGNAL(clicked(bool)), this, SLOT(nextStep()));
    connect(ui->btPrevStep, SIGNAL(clicked(bool)), this, SLOT(previousStep()));

    connect(ui->btSearchCompanyLogo, SIGNAL(clicked(bool)), this, SLOT(selectCompanyLogo()));
    connect(ui->btRemoveLogo, SIGNAL(clicked(bool)), this, SLOT(removeCompanyLogo()));

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
    if (db_manager)
        db_manager->removeInstance();
    delete ui;
}

void frmFirstRun::closeEvent(QCloseEvent *event){
    if (allowed){
        event->accept();
        QMainWindow::closeEvent(event);
        return;
    }

    QMessageBox confirmation;
    confirmation.setWindowTitle(tr("Critical warning! | SmartClass"));
    confirmation.setText(tr("You are going to quit without saving the database settings. This may corrupt the setup data.\nDo you want to proceed?"));
    confirmation.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
    confirmation.setIcon(QMessageBox::Critical);

    if (confirmation.exec() == QMessageBox::Yes) qApp->quit();
    else event->ignore();
}

void frmFirstRun::nextStep(){
    int currentTab = ui->tabManager->currentIndex();

    if (currentTab == 1){
        if (db_manager){
            if (db_manager->isOpen()) db_manager->close();
            db_manager->removeInstance();
            db_manager = NULL;
        }

        DBManager::DBData db_data;

        if (ui->rbSQLite->isChecked()){
            SmartClassGlobal::setTablePrefix(ui->edtDatabasePrefix->text());
            SmartClassGlobal::setDatabaseType(DBManager::SQLITE);

            db_data.setDatabaseName(SmartClassGlobal::getDBPath());
            db_data.setTablePrefix(SmartClassGlobal::tablePrefix());
            db_data.setDatabaseConnectionType(SmartClassGlobal::databaseType());
            db_data.setConnectionName(DBManager::getUniqueConnectionName("firstRun"));

            db_manager = DBManager::getInstance(db_data);
        }
        else{
            SmartClassGlobal::setTablePrefix(ui->edtDatabasePrefix->text());
            SmartClassGlobal::setDatabaseType(DBManager::MYSQL);

            db_data.setHostName(ui->edtHost->text());
            db_data.setDatabaseName(ui->edtDatabase->text());
            db_data.setPort(ui->edtPortNumber->value());
            db_data.setUserName(ui->edtUsername->text());
            db_data.setPassword(ui->edtPassword->text());
            db_data.setTablePrefix(SmartClassGlobal::tablePrefix());
            db_data.setDatabaseConnectionType(SmartClassGlobal::databaseType());
            db_data.setConnectionName(DBManager::getUniqueConnectionName("firstRun"));

            db_manager = DBManager::getInstance(db_data);
        }

        if (!db_manager->open()){
            QMessageBox::critical(NULL, tr("Error | SmartClass"),
                                  tr("An error has occurred while we tried to connect to the database. Please, check the input and try again.\nDetails: %1.").arg(db_manager->lastError().text()),
                                  QMessageBox::Ok, QMessageBox::NoButton);
            db_manager->removeInstance();
            db_manager = NULL;
            return;
        }
        db_export_data = db_data;

        QStringList existingTables = db_manager->tables();
        if (existingTables.contains(SmartClassGlobal::getTableName(SmartClassGlobal::SETTINGS, true))){
            QList< QVariantList > settingsb = db_manager->retrieveAll(SmartClassGlobal::getTableName(SmartClassGlobal::SETTINGS),
                                                                      SmartClassGlobal::getTableAliases(SmartClassGlobal::SETTINGS));

            settingsExists = (bool)settingsb.size();
            if (settingsExists){
                QVariantList settings(settingsb.at(0));
                ui->edtCompanyName->setText(settings.at(0).toString());
                cLogo = DBManager::variantToPixmap(settings.at(2));
                ui->lblCompanyLogoImg->setPixmap(cLogo.scaled(ui->lblCompanyLogoImg->size(), Qt::KeepAspectRatio));

                ui->grpCompanySettings->setEnabled(false);
            }
        }
        else settingsExists = false;

        ui->grpCompanySettings->setDisabled(settingsExists);

        ui->edtDeviceName->setText(QHostInfo::localHostName());
        ui->edtDeviceOS->setText(QSysInfo::productType());
        ui->edtDeviceOSMV->setText(QSysInfo::productVersion());
        ui->edtDeviceLastAccess->setText(QDateTime::currentDateTime().toString("dd/MM/yyyy - HH:mm:ss"));

        if (!existingTables.contains(SmartClassGlobal::getTableName(SmartClassGlobal::SETTINGS, true))){
            db_manager->createTable(SmartClassGlobal::getTableName(SmartClassGlobal::USERS),
                                    SmartClassGlobal::getTableStructure(SmartClassGlobal::USERS));
        }

        if (existingTables.contains(SmartClassGlobal::getTableName(SmartClassGlobal::ACTIVECONNECTIONS, true))){
            QVariantList connection = db_manager->retrieveRow(SmartClassGlobal::getTableName(SmartClassGlobal::ACTIVECONNECTIONS),
                                                              SmartClassGlobal::getTableAliases(SmartClassGlobal::ACTIVECONNECTIONS).mid(1, 3),
                                                              QVariantList() << ui->edtDeviceName->text() << ui->edtDeviceOS->text() << ui->edtDeviceOSMV->text(),
                                                              SmartClassGlobal::getTableAliases(SmartClassGlobal::ACTIVECONNECTIONS));
            if (connection.size())
                profileID = connection.at(0).toLongLong();
        }
        else if(!db_manager->createTable(SmartClassGlobal::getTableName(SmartClassGlobal::ACTIVECONNECTIONS),
                                         QStringList() << SmartClassGlobal::getTableStructure(SmartClassGlobal::ACTIVECONNECTIONS) <<
                                         SmartClassGlobal::getTableConstraints(SmartClassGlobal::ACTIVECONNECTIONS))){
            QMessageBox::critical(NULL, tr("Error | SmartClass"),
                                  tr("It was not possible to create the table of device access control. Please, try again later. Details: %1 .").arg(db_manager->lastError().text()),
                                  QMessageBox::Ok, QMessageBox::NoButton);
            return;
        }
    }
    else if (currentTab == 2){
        bool complete[4], check, tryAgain;
        for (int i = 0; i < 4; ++i) complete[i] = false;

        do {
            if (!complete[0]){
                if (!settingsExists){
                    QStringList columns = SmartClassGlobal::getTableAliases(SmartClassGlobal::SETTINGS);
                    columns.removeAt(1);

                    QVariantList settingsData;
                    settingsData << ui->edtCompanyName->text()
                                 << db_manager->pixmapToVariant(cLogo);

                    complete[0] = db_manager->createTable(SmartClassGlobal::getTableName(SmartClassGlobal::SETTINGS),
                                                          SmartClassGlobal::getTableStructure(SmartClassGlobal::SETTINGS));
                    if (complete[0])
                        complete[0] &= db_manager->insertRow(SmartClassGlobal::getTableName(SmartClassGlobal::SETTINGS),
                                                             columns,
                                                             settingsData);
                }
                else complete[0] = true;
            }

            if (!complete[1] || !complete[2] || !complete[3]){
                QStringList columns = SmartClassGlobal::getTableAliases(SmartClassGlobal::ACTIVECONNECTIONS);

                if (profileID != -1){
                    complete[1] = db_manager->updateRow(SmartClassGlobal::getTableName(SmartClassGlobal::ACTIVECONNECTIONS),
                                                        columns.at(0),
                                                        profileID,
                                                        QStringList() << columns.at(4),
                                                        QVariantList() <<  ui->edtDeviceLastAccess->text());
                    complete[2] = true;
                    complete[3] = true;
                }
                else {
                    QVariantList data;
                    data << ui->edtDeviceName->text()
                         << ui->edtDeviceOS->text()
                         << ui->edtDeviceOSMV->text()
                         << ui->edtDeviceLastAccess->text();

                    if (!complete[2]) complete[2] = db_manager->insertRow(SmartClassGlobal::getTableName(SmartClassGlobal::ACTIVECONNECTIONS),
                                                                          columns.mid(1),
                                                                          data);

                    if (complete[2]){
                        QVariantList profID = db_manager->retrieveRow(SmartClassGlobal::getTableName(SmartClassGlobal::ACTIVECONNECTIONS),
                                                                      columns.mid(1),
                                                                      data,
                                                                      QStringList() << columns.at(0));
                        complete[3] = profID.size();
                        if (complete[3]) profileID = profID.at(0).toLongLong();
                        complete[1] = true;
                    }
                }
            }

            check = complete[0];
            for (int i = 1; i < 4; ++i) check &= complete[i];

            if (!check){
                QString errors = "";
                if (!complete[0]) errors += tr("- It was not possible to create or write in the settings table.\n");
                if (!complete[1]) errors += tr("- It was not possible to update an existing ID with your device data.\n");
                if (!complete[2]) errors += tr("- It was not possible to create a new ID to your device.\n");
                if (!complete[3]) errors += tr("- It was not possible to retrieve the new ID of your device.\n");
                tryAgain  = QMessageBox::critical(NULL, tr("Error | SmartClass"),
                                                  tr("Some errors have occurred while we tried to store your settings into the database.\n"
                                                     "Here are the details:\n"
                                                     "\n"
                                                     "%1"
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

        settings.setValue("table prefix", SmartClassGlobal::tablePrefix());

        if (SmartClassGlobal::databaseType() == DBManager::MYSQL){
            settings.setValue("database type", QString("MYSQL"));
            settings.setValue("database", db_export_data.databaseName());
            settings.setValue("host", db_export_data.hostName());
            settings.setValue("port", db_export_data.port());
            settings.setValue("username", db_export_data.username());
            settings.setValue("password", db_export_data.password());
        }
        else settings.setValue("database type", QString("SQLITE"));

        settings.endGroup();

        settings.beginGroup("language options");
        settings.setValue("current language index", currentLangIndex);
        settings.endGroup();

        settings.beginGroup("device settings");
        settings.setValue("device ID", profileID);
        settings.endGroup();

        db_manager->close();
        db_manager->removeInstance();
        db_manager = NULL;
    }

    bool appSettingsS = currentTab != 2;
    ui->btPrevStep->setVisible(appSettingsS);
    ui->btPrevStep->setEnabled(appSettingsS);
    ui->btNextStep->setVisible(appSettingsS);
    ui->btNextStep->setEnabled(appSettingsS);

    ui->tabManager->setCurrentIndex(++currentTab);
}

void frmFirstRun::previousStep(){
    int currentTab = ui->tabManager->currentIndex();
    ui->tabManager->setCurrentIndex(--currentTab);

    ui->btPrevStep->setVisible(currentTab);
    ui->btPrevStep->setEnabled(currentTab);

    if (currentTab == 2) profileID = -1;
}

void frmFirstRun::saveDBSettings(){
    allowed = true;
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

void frmFirstRun::removeCompanyLogo(){
    ui->edtCompanyLogoPath->clear();
    ui->lblCompanyLogoImg->setPixmap(QPixmap(QString::fromUtf8(":/images/logos/ns-watermark.png")));
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
