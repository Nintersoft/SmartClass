#include "frmmain.h"
#include "ui_frmmain.h"

frmMain::frmMain(QWidget *parent) :
    NMainWindow(parent),
    ui(new Ui::frmMain)
{
    ui->setupUi(this);

    // Sets the custom Widgets on the parent Class
    // Otherwise, the window resizing feature will not work
    NMainWindow::setCustomWidgets(ui->centralWidget, ui->statusBar);

    /*
     *  End of GUI implementation
     */

    returnCode = -1;
    SmartClassGlobal::setGlobalConnectionName("smart_class_c");

    upgradeAvailable = false;

    myDB = NULL;
    firstRunScr = NULL;
    loginScr = NULL;

    currentLanguage = "en";
    langPath = QApplication::applicationDirPath().append("/lang/");
    settings = new QSettings("Nintersoft", "SmartClass");

    ui->tableCourses->setVisible(false);
    ui->cbCourseFilter->setVisible(false);

    connect(ui->btAlternateTable, SIGNAL(clicked(bool)), this, SLOT(alternateTable()));

    connect(ui->btSearchPrevious, SIGNAL(clicked(bool)), this, SLOT(searchPrevious()));
    connect(ui->btSearchNext, SIGNAL(clicked(bool)), this, SLOT(searchNext()));

    bool isMySQL = false,
         dbData = settings->childGroups().contains("dbinfo", Qt::CaseInsensitive);

    if (dbData){
        settings->beginGroup("dbinfo");
        isMySQL = (settings->value("database type", "").toString() == "MYSQL");
        settings->endGroup();
    }

    if (dbData && (QFile::exists(SmartClassGlobal::getDBPath()) || isMySQL)){
        setupDBConnection();
        if (returnCode == -1){
            if (settings->childGroups().contains("language options", Qt::CaseInsensitive)){
                settings->beginGroup("language options");
                if (settings->value("current language index", 1).toInt() != 0) changeLanguage("pt");
                settings->endGroup();
            }

            loginScr = new frmLogin(NULL);
            connect(loginScr, SIGNAL(dataReady(QVariantList)), this, SLOT(setSessionRole(QVariantList)));
            loginScr->show();

            createTables();

            frmAbt = new frmAbout();
            connect(ui->btAbout, SIGNAL(clicked(bool)), frmAbt, SLOT(show()));
            connect(frmAbt, SIGNAL(upgradeAvailable()), this, SLOT(setUpgradeAvailable()));
        }
    }
    else {
        firstRunScr = new frmFirstRun();
        connect(firstRunScr, SIGNAL(sendData(DBManager::DBData,QString)), this, SLOT(getFirstSettings(DBManager::DBData,QString)));
        firstRunScr->show();
    }

    if (returnCode == -1){
        this->hide();

        ui->tableStudents->setContextMenuPolicy(Qt::CustomContextMenu);
        ui->tableCourses->setContextMenuPolicy(Qt::CustomContextMenu);

        connect(ui->btLogOff, SIGNAL(clicked(bool)), this, SLOT(logOut()));
        connect(ui->rbStudentsTable, SIGNAL(toggled(bool)), this, SLOT(changeTable(bool)));

        connect(ui->btAddStudent, SIGNAL(clicked(bool)), this, SLOT(openStudentManager()));
        connect(ui->btUpdateStudent, SIGNAL(clicked(bool)), this, SLOT(openStudentManager()));
        connect(ui->btRemoveStudent, SIGNAL(clicked(bool)), this, SLOT(removeStudent()));
        connect(ui->btAddCourse, SIGNAL(clicked(bool)), this, SLOT(openClassesManager()));
        connect(ui->btUpdateCourse, SIGNAL(clicked(bool)), this, SLOT(openClassesManager()));
        connect(ui->btRemoveCourse, SIGNAL(clicked(bool)), this, SLOT(removeCourse()));

        connect(ui->tableStudents, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(openStudentTableContextMenu(QPoint)));
        connect(ui->tableCourses, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(openClassTableContextMenu(QPoint)));

        connect(ui->btSettings, SIGNAL(clicked(bool)), this, SLOT(openSettingsForm()));
        connect(ui->btManageUsers, SIGNAL(clicked(bool)), this, SLOT(openUserManager()));
        connect(ui->btInfo, SIGNAL(clicked(bool)), this, SLOT(openSettingsForm()));

        connect(ui->btPrintStudentForms, SIGNAL(clicked(bool)), this, SLOT(openContractForm()));
        connect(ui->btGetReceipt, SIGNAL(clicked(bool)), this, SLOT(openReceiptForm()));

        deleteDBStatus = 0;

        connect(ui->btRestoreDB, SIGNAL(clicked(bool)), this, SLOT(restoreDataBase()));
        connect(ui->btBackUpDB, SIGNAL(clicked(bool)), this, SLOT(backupDataBase()));
        connect(ui->btResetDB, SIGNAL(clicked(bool)), this, SLOT(removeDataBase()));
        connect(ui->btImportDB, SIGNAL(clicked(bool)), this, SLOT(openImportExportTool()));
        connect(ui->btExportDB, SIGNAL(clicked(bool)), this, SLOT(openImportExportTool()));
        connect(ui->btUninstall, SIGNAL(clicked(bool)), this, SLOT(removeAppSettings()));

        connect(ui->btOpenNSDocwiki, SIGNAL(clicked(bool)), this, SLOT(openNSDocwiki()));
        connect(ui->btOpenNSWebsite, SIGNAL(clicked(bool)), this, SLOT(openNSWebSite()));
        connect(ui->btSupportEmail, SIGNAL(clicked(bool)), this, SLOT(openSupportEmail()));
        connect(ui->btOpenOnlineSupport, SIGNAL(clicked(bool)), this, SLOT(openOnlineSupport()));

        backupPath = QDir::homePath() + QDir::separator() + ".SmartClassBKP" + QDir::separator();
        if (!QDir(backupPath).exists()) QDir(backupPath).mkpath(backupPath);

        frmImportExport = NULL;
        manageStudent = NULL;
        manageClass = NULL;
        frmMngUsers = NULL;
        frmContract = NULL;
        frmPayment = NULL;
        frmConfig= NULL;
    }
}

frmMain::~frmMain()
{
    if (myDB){
        if (myDB->isOpen()) myDB->close();
        myDB->removeInstance();
    }

    if (frmImportExport) delete frmImportExport;
    if (manageStudent) delete manageStudent;
    if (manageClass) delete manageClass;
    if (firstRunScr) delete firstRunScr;
    if (frmMngUsers) delete frmMngUsers;
    if (frmContract) delete frmContract;
    if (frmPayment) delete frmPayment;
    if (frmConfig) delete frmConfig;
    if (settings) delete settings;

    delete ui;
}

void frmMain::closeEvent(QCloseEvent *event){
    if (upgradeAvailable){
        QMessageBox::information(this, tr("Upgrade | SmartClass"), tr("The upgrade process is going to start as soon as you dismiss this message."), QMessageBox::Ok);
        QDesktopServices::openUrl(QUrl::fromLocalFile(QDir::homePath() + "/AppData/Roaming/Nintersoft/SmartClass/Downloads/SmartClass.exe"));
    }
    qApp->exit();
    event->accept();
}

void frmMain::resizeEvent(QResizeEvent *event){
    int newWidth = ((this->width() - 20) / ui->tableStudents->columnCount()) - ui->tableStudents->columnCount();
    ui->tableStudents->setColumnWidth(0, newWidth);
    ui->tableStudents->setColumnWidth(1, newWidth);
    ui->tableStudents->setColumnWidth(2, newWidth);
    ui->tableStudents->setColumnWidth(3, newWidth);

    newWidth = ((this->width() - 15) / ui->tableCourses->columnCount()) - ui->tableCourses->columnCount();
    ui->tableCourses->setColumnWidth(0, newWidth);
    ui->tableCourses->setColumnWidth(1, newWidth);
    ui->tableCourses->setColumnWidth(2, newWidth);
    ui->tableCourses->setColumnWidth(3, newWidth);
    ui->tableCourses->setColumnWidth(4, newWidth);

    event->accept();
    NMainWindow::resizeEvent(event);
}

void frmMain::createTables(){
    if (!myDB->open()){
        QMessageBox dbWarning;
        dbWarning.setWindowTitle(tr("Connection issue | SmartClass"));
        dbWarning.setIcon(QMessageBox::Critical);
        dbWarning.setStandardButtons(QMessageBox::Discard | QMessageBox::Abort);
        dbWarning.setButtonText(QMessageBox::Discard, tr("Clear settings"));
        dbWarning.setButtonText(QMessageBox::Abort, tr("Quit"));
        dbWarning.setText(tr("It was not possible to establish connection to the database. Unfortunately it will not be possible to continue with the execution of this program. Please, try again later."
                             "\nIf you keep having trouble while trying to connect, please, consider to clear the program settings (the database is not going to be affected)."));
        if (dbWarning.exec() == QMessageBox::Discard){
            QString feedback = "";
            if (QMessageBox::warning(NULL, tr("Confirmation | SmartClass"),
                                     tr("You are about to remove every information about the database connection of this program.\n"
                                        "This step cannot be undone. Do you still want to proceed?"),
                                     QMessageBox::Yes, QMessageBox::No)
                    == QMessageBox::Yes){
                settings->beginGroup("dbinfo");
                settings->remove("");
                settings->endGroup();
                feedback = tr("Settings removed. Please, restart SmartClass.");
            }
            else feedback = tr("Closing SmartClass.");
            if (QMessageBox::information(NULL, tr("Info | SmartClass"), feedback,
                                     QMessageBox::Ok, QMessageBox::NoButton))
                returnCode = 0;
        }
        else returnCode = 1;
    }

    myDB->createTable(SmartClassGlobal::getTableName(SmartClassGlobal::RESPONSIBLE),
                      SmartClassGlobal::getTableStructure(SmartClassGlobal::RESPONSIBLE));
    myDB->createTable(SmartClassGlobal::getTableName(SmartClassGlobal::STUDENT),
                      QStringList() << SmartClassGlobal::getTableStructure(SmartClassGlobal::STUDENT) <<
                      SmartClassGlobal::getTableConstraints(SmartClassGlobal::STUDENT));
    myDB->createTable(SmartClassGlobal::getTableName(SmartClassGlobal::STUDENTIMAGES),
                      QStringList() << SmartClassGlobal::getTableStructure(SmartClassGlobal::STUDENTIMAGES) <<
                      SmartClassGlobal::getTableConstraints(SmartClassGlobal::STUDENTIMAGES));
    myDB->createTable(SmartClassGlobal::getTableName(SmartClassGlobal::RESPONSIBLEIMAGES),
                      QStringList() << SmartClassGlobal::getTableStructure(SmartClassGlobal::RESPONSIBLEIMAGES) <<
                      SmartClassGlobal::getTableConstraints(SmartClassGlobal::RESPONSIBLEIMAGES));
    myDB->createTable(SmartClassGlobal::getTableName(SmartClassGlobal::COURSEDETAILS),
                      SmartClassGlobal::getTableStructure(SmartClassGlobal::COURSEDETAILS));
    myDB->createTable(SmartClassGlobal::getTableName(SmartClassGlobal::COURSEENROLLMENTS),
                      QStringList() << SmartClassGlobal::getTableStructure(SmartClassGlobal::COURSEENROLLMENTS) <<
                      SmartClassGlobal::getTableConstraints(SmartClassGlobal::COURSEENROLLMENTS));
    myDB->createTable(SmartClassGlobal::getTableName(SmartClassGlobal::PAYMENTDETAILS),
                      QStringList() << SmartClassGlobal::getTableStructure(SmartClassGlobal::PAYMENTDETAILS) <<
                      SmartClassGlobal::getTableConstraints(SmartClassGlobal::PAYMENTDETAILS));
}

void frmMain::setupDBConnection(){
    settings->beginGroup("dbinfo");
    if (settings->value("database type").toString() == "SQLITE"){
        db_data.setDatabaseName(SmartClassGlobal::getDBPath());
        SmartClassGlobal::setDatabaseType(DBManager::SQLITE);
    }
    else {
        db_data.setHostName(settings->value("host", "127.0.0.1").toString());
        db_data.setDatabaseName(settings->value("database", "default").toString());
        db_data.setPort(settings->value("port", 3306).toInt());
        db_data.setUserName(settings->value("username", "").toString());
        db_data.setPassword(settings->value("password", "").toString());
        SmartClassGlobal::setDatabaseType(DBManager::MYSQL);
    }
    SmartClassGlobal::setTablePrefix(settings->value("table prefix", "").toString());
    settings->endGroup();

    db_data.setTablePrefix(SmartClassGlobal::tablePrefix());
    db_data.setDatabaseConnectionType(SmartClassGlobal::databaseType());

    myDB = DBManager::getInstance(db_data);

    createTables();
}

void frmMain::getFirstSettings(const DBManager::DBData &sqlData, const QString &langSlug){
    db_data = sqlData;
    myDB = DBManager::getInstance(db_data);

    if (langSlug != "en") changeLanguage(langSlug);

    createTables();

    if (loginScr){
        delete loginScr;
        loginScr = NULL;
    }

    loginScr = new frmLogin(NULL);
    connect(loginScr, SIGNAL(dataReady(QVariantList)), this, SLOT(setSessionRole(QVariantList)));
    loginScr->show();

    frmAbt = new frmAbout();
    connect(ui->btAbout, SIGNAL(clicked(bool)), frmAbt, SLOT(show()));
    connect(frmAbt, SIGNAL(upgradeAvailable()), this, SLOT(setUpgradeAvailable()));
    this->hide();

    firstRunScr->close();
    delete firstRunScr;
    firstRunScr = NULL;
}

void frmMain::setSessionRole(const QVariantList &userInfo){
    sessionRole = (SmartClassGlobal::UserRoles)userInfo.at(8).toInt();
    currentUser = userInfo.at(1).toString();

    this->showMaximized();
    loginScr->close();

    setUIToRole();

    if (sessionRole != SmartClassGlobal::NEW){
        getCourses();
        getStudents();
        setBackupSettings();
    }
}

void frmMain::logOut(){
    QMessageBox confirmation;
    confirmation.setText(tr("You are going to disconnect from this session. Are you sure?"));
    confirmation.setWindowTitle(tr("Confirmation | SmartClass"));
    confirmation.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
    if (confirmation.exec() == QMessageBox::Yes){
        ui->rbStudentsTable->setChecked(true);
        ui->tableCourses->clearContents();
        ui->tableCourses->setRowCount(0);
        ui->tableStudents->clearContents();
        ui->tableStudents->setRowCount(0);
        ui->tabWidget->setCurrentIndex(0);

        if (loginScr){
            disconnect(loginScr, SIGNAL(dataReady(QVariantList)), this, SLOT(setSessionRole(QVariantList)));
            delete loginScr;
            loginScr = NULL;
        }

        loginScr = new frmLogin(NULL);
        connect(loginScr, SIGNAL(dataReady(QVariantList)), this, SLOT(setSessionRole(QVariantList)));
        loginScr->show();

        myDB->close();
        this->hide();
    }
}

void frmMain::openStudentManager(){
    if (manageStudent){
        disconnect(manageStudent, SIGNAL(updatedData(QVariantList,qlonglong)),
                   this, SLOT(receiveStudentUpdatedData(QVariantList,qlonglong)));
        disconnect(manageStudent, SIGNAL(newData(QVariantList)),
                   this, SLOT(receiveNewStudentData(QVariantList)));

        delete manageStudent;
        manageStudent = NULL;
    }

    QString senderName = sender()->objectName();
    if (senderName == "btAddStudent") manageStudent = new frmManageStudent(NULL, frmManageStudent::Create, -1);
    else if (ui->tableStudents->currentRow() < 0){
        QMessageBox::information(this, tr("Warning | SmartClass"), tr("Sorry, but you have to select a row in the students table in order to execute this command."), QMessageBox::Ok);
        return;
    }
    else if (ui->tableStudents->isHidden()){
        QMessageBox::information(this, tr("Warning | SmartClass"), tr("Sorry, but the table of students must be selected in order to execute this command."), QMessageBox::Ok);
        return;
    }
    else if (sessionRole == SmartClassGlobal::ADMIN || sessionRole == SmartClassGlobal::EDITOR)
        manageStudent = new frmManageStudent(NULL, frmManageStudent::Role::Edit, ui->tableStudents->item(ui->tableStudents->currentRow(), 0)->data(Qt::UserRole).toLongLong());
    else manageStudent = new frmManageStudent(NULL, frmManageStudent::Role::View, ui->tableStudents->item(ui->tableStudents->currentRow(), 0)->data(Qt::UserRole).toLongLong());

    connect(manageStudent, SIGNAL(updatedData(QVariantList,qlonglong)),
               this, SLOT(receiveStudentUpdatedData(QVariantList,qlonglong)));
    connect(manageStudent, SIGNAL(newData(QVariantList)),
               this, SLOT(receiveNewStudentData(QVariantList)));
    manageStudent->show();
}

void frmMain::openClassesManager(){
    if (manageClass) {
        disconnect(manageClass, SIGNAL(updatedData(QVariantList,qlonglong)),
                   this, SLOT(receiveCourseUpdatedData(QVariantList,qlonglong)));
        disconnect(manageClass, SIGNAL(newData(QVariantList)),
                   this, SLOT(receiveNewCourseData(QVariantList)));

        delete manageClass;
        manageClass = NULL;
    }

    QString senderName = sender()->objectName();
    if (senderName == "btAddCourse") manageClass = new frmManageClass(NULL, frmManageClass::Create, -1);
    else if (ui->tableCourses->currentRow() < 0){
        QMessageBox::information(this, tr("Warning | SmartClass"), tr("Sorry, but you have to select a row in the courses table in order to execute this command."), QMessageBox::Ok);
        return;
    }
    else if (ui->tableCourses->isHidden()){
        QMessageBox::information(this, tr("Warning | SmartClass"), tr("Sorry, but the table of courses must be selected in order to execute this command."), QMessageBox::Ok);
        return;
    }
    else if (sessionRole == SmartClassGlobal::ADMIN || sessionRole == SmartClassGlobal::EDITOR)
        manageClass = new frmManageClass(NULL, frmManageClass::Role::Edit, ui->tableCourses->item(ui->tableCourses->currentRow(), 0)->data(Qt::UserRole).toLongLong());
    else manageClass = new frmManageClass(NULL, frmManageClass::Role::View, ui->tableCourses->item(ui->tableCourses->currentRow(), 0)->data(Qt::UserRole).toLongLong());

    connect(manageClass, SIGNAL(updatedData(QVariantList,qlonglong)),
               this, SLOT(receiveCourseUpdatedData(QVariantList,qlonglong)));
    connect(manageClass, SIGNAL(newData(QVariantList)),
               this, SLOT(receiveNewCourseData(QVariantList)));
    manageClass->show();
}

void frmMain::openUserManager(){
    if (frmMngUsers){
        delete frmMngUsers;
        frmMngUsers = NULL;
    }

    frmMngUsers = new frmManageUsers(this, currentUser);
    frmMngUsers->show();
}

void frmMain::openContractForm(){
    if (ui->tableStudents->currentRow() < 0){
        QMessageBox::information(this, tr("Warning | SmartClass"), tr("Sorry, but you have to select a row in the students table in order to execute this command."), QMessageBox::Ok);
        return;
    }
    else if (ui->tableStudents->isHidden()){
        QMessageBox::information(this, tr("Warning | SmartClass"), tr("Sorry, but the table of students must be selected in order to execute this command."), QMessageBox::Ok);
        return;
    }

    if (frmContract){
        delete frmContract;
        frmContract = NULL;
    }

    frmContract = new frmPrintContract(NULL, ui->tableStudents->item(ui->tableStudents->currentRow(), 0)->data(Qt::UserRole).toLongLong());
    frmContract->show();
}

void frmMain::openSettingsForm(){
    if (frmConfig){
        delete frmConfig;
        frmConfig = NULL;
    }

    if (sender()->objectName() == "btInfo") frmConfig = new frmSettings(NULL, frmSettings::Info);
    else frmConfig = new frmSettings(NULL);
    frmConfig->show();
}

void frmMain::openReceiptForm(){
    if (frmPayment){
        delete frmPayment;
        frmPayment = NULL;
    }

    frmPayment = new frmReceipt(NULL);
    frmPayment->show();
}

void frmMain::openImportExportTool(){
    if (frmImportExport){
        delete frmImportExport;
        frmImportExport = NULL;
    }

    if (sender()->objectName() == "btImportDB")
        frmImportExport = new frmImportExportDB(NULL, frmImportExportDB::Import);
    else frmImportExport = new frmImportExportDB(NULL, frmImportExportDB::Export);

    frmImportExport->show();
}

void frmMain::setUIToRole(){
    bool enable = false;
    if (sessionRole == SmartClassGlobal::ADMIN) enable = true;

    ui->btManageUsers->setEnabled(enable);
    ui->btSettings->setEnabled(enable);
    ui->btGetReceipt->setEnabled(enable);

    ui->btImportDB->setEnabled(enable);
    ui->btRestoreDB->setEnabled(enable);
    ui->btExportDB->setEnabled(enable);
    ui->btBackUpDB->setEnabled(enable);
    ui->btResetDB->setEnabled(enable);
    ui->btUninstall->setEnabled(enable);

    if (sessionRole == SmartClassGlobal::EDITOR) enable = true;

    ui->btAddStudent->setEnabled(enable);
    ui->btRemoveStudent->setEnabled(enable);
    ui->btAddCourse->setEnabled(enable);

    if (sessionRole == SmartClassGlobal::VIEWER) enable = true;

    ui->btUpdateStudent->setEnabled(enable);
    ui->btPrintStudentForms->setEnabled(enable);

    if (enable) return;

    QMessageBox::information(this, tr("Information | SmartClass"), tr("Seems that you do not have enough privileges to acess any data yet. Plase, contact an administrator in order to upgrade your permissions."), QMessageBox::Ok);
}

void frmMain::getStudents(){
    QString sTableName = SmartClassGlobal::getTableName(SmartClassGlobal::STUDENT, true),
            rTableName = SmartClassGlobal::getTableName(SmartClassGlobal::RESPONSIBLE, true),
            srJoin;
    QStringList sTableSchema = SmartClassGlobal::getTableAliases(SmartClassGlobal::STUDENT),
                rTableSchema = SmartClassGlobal::getTableAliases(SmartClassGlobal::RESPONSIBLE),
                srNewTableSchema;

    srNewTableSchema << sTableName + "." + sTableSchema.at(0)
                     << sTableName + "." + sTableSchema.at(1)
                     << sTableName + "." + sTableSchema.at(5)
                     << sTableName + "." + sTableSchema.at(6)
                     << rTableName + "." + rTableSchema.at(0)
                     << rTableName + "." + rTableSchema.at(1);

    srJoin = SmartClassGlobal::getTableName(SmartClassGlobal::STUDENT) + " INNER JOIN " + rTableName + " ON " +
             sTableName + "." + sTableSchema.at(2) + " = " + srNewTableSchema.at(4);

    QList< QVariantList > srJoinRelation = myDB->retrieveAll(srJoin, srNewTableSchema, QStringList(),
                                                             QStringList() << (srNewTableSchema.at(1) + " ASC"));

    if (srJoinRelation.isEmpty()) return;

    int srSize = srJoinRelation.size();

    ui->tableStudents->setSortingEnabled(false);

    for (int i = 0; i < srSize; ++i){
        ui->tableStudents->insertRow(i);

        ui->tableStudents->setItem(i, 0, new QTableWidgetItem(srJoinRelation[i].at(1).toString()));
        ui->tableStudents->setItem(i, 1, new QTableWidgetItem(srJoinRelation[i].at(5).toString()));
        ui->tableStudents->setItem(i, 2, new QTableWidgetItem(srJoinRelation[i].at(2).toString()));
        ui->tableStudents->setItem(i, 3, new QTableWidgetItem(srJoinRelation[i].at(3).toString()));

        ui->tableStudents->item(i, 0)->setToolTip(ui->tableStudents->item(i, 0)->text());
        ui->tableStudents->item(i, 1)->setToolTip(ui->tableStudents->item(i, 1)->text());
        ui->tableStudents->item(i, 2)->setToolTip(ui->tableStudents->item(i, 2)->text());
        ui->tableStudents->item(i, 3)->setToolTip(ui->tableStudents->item(i, 3)->text());

        ui->tableStudents->item(i, 0)->setData(Qt::UserRole, srJoinRelation[i].at(0));
        ui->tableStudents->item(i, 1)->setData(Qt::UserRole, srJoinRelation[i].at(4));
    }
    ui->tableStudents->setSortingEnabled(true);
}

void frmMain::getCourses(){
    QStringList cTableSchema = SmartClassGlobal::getTableAliases(SmartClassGlobal::COURSEDETAILS),
                cNewTableSchema;
    cNewTableSchema << cTableSchema.at(0)  // ID
                    << cTableSchema.at(1)  // Course
                    << cTableSchema.at(2)  // Teacher
                    << cTableSchema.at(5)  // Classroom
                    << cTableSchema.at(6)  // Day and Time
                    << cTableSchema.at(7); // Beginning Date

    QList< QVariantList > courses = myDB->retrieveAll(SmartClassGlobal::getTableName(SmartClassGlobal::COURSEDETAILS),
                                                      cNewTableSchema, QStringList(),
                                                      QStringList() << (cNewTableSchema.at(1) + " ASC"));
    if (courses.isEmpty()) return;

    int cSize = courses.length();

    ui->tableCourses->setSortingEnabled(false);
    for (int i = 0; i < cSize; ++i){
        ui->tableCourses->insertRow(ui->tableCourses->rowCount());

        ui->tableCourses->setItem(i, 0, new QTableWidgetItem(courses[i].at(1).toString()));
        ui->tableCourses->setItem(i, 1, new QTableWidgetItem(QString::number(courses[i].at(3).toInt())));
        ui->tableCourses->setItem(i, 2, new QTableWidgetItem(courses[i].at(2).toString()));
        ui->tableCourses->setItem(i, 3, new QTableWidgetItem(courses[i].at(5).toDate().toString("dd/MM/yyyy")));
        ui->tableCourses->setItem(i, 4, new QTableWidgetItem(courses[i].at(4).toString()));

        ui->tableCourses->item(i, 0)->setToolTip(ui->tableCourses->item(i, 0)->text());
        ui->tableCourses->item(i, 1)->setToolTip(ui->tableCourses->item(i, 1)->text());
        ui->tableCourses->item(i, 2)->setToolTip(ui->tableCourses->item(i, 2)->text());
        ui->tableCourses->item(i, 3)->setToolTip(ui->tableCourses->item(i, 3)->text());
        ui->tableCourses->item(i, 4)->setToolTip(ui->tableCourses->item(i, 4)->text());

        ui->tableCourses->item(i, 0)->setData(Qt::UserRole, courses[i].at(0));
    }
    ui->tableCourses->setSortingEnabled(true);
}

void frmMain::receiveNewStudentData(const QVariantList &data){
    int oldRCount = ui->tableStudents->rowCount();
    ui->tableStudents->insertRow(oldRCount);

    ui->tableStudents->setItem(oldRCount, 0, new QTableWidgetItem(data.at(1).toString()));
    ui->tableStudents->setItem(oldRCount, 1, new QTableWidgetItem(data.at(3).toString()));
    ui->tableStudents->setItem(oldRCount, 2, new QTableWidgetItem(data.at(4).toString()));
    ui->tableStudents->setItem(oldRCount, 3, new QTableWidgetItem(data.at(5).toString()));

    ui->tableStudents->item(oldRCount, 0)->setToolTip(data.at(1).toString());
    ui->tableStudents->item(oldRCount, 1)->setToolTip(data.at(3).toString());
    ui->tableStudents->item(oldRCount, 2)->setToolTip(data.at(4).toString());
    ui->tableStudents->item(oldRCount, 3)->setToolTip(data.at(5).toString());

    ui->tableStudents->item(oldRCount, 0)->setData(Qt::UserRole, data.at(0));
    ui->tableStudents->item(oldRCount, 1)->setData(Qt::UserRole, data.at(2));
}

void frmMain::receiveStudentUpdatedData(const QVariantList &data, const qlonglong &oldStudent){
    int currentRow = ui->tableStudents->currentRow();
    if (ui->tableStudents->item(currentRow, 0)->data(Qt::UserRole).toLongLong() == oldStudent){
        ui->tableStudents->item(currentRow, 0)->setText(data.at(0).toString());
        ui->tableStudents->item(currentRow, 1)->setText(data.at(2).toString());
        ui->tableStudents->item(currentRow, 2)->setText(data.at(3).toString());
        ui->tableStudents->item(currentRow, 3)->setText(data.at(4).toString());

        ui->tableStudents->item(currentRow, 0)->setToolTip(data.at(0).toString());
        ui->tableStudents->item(currentRow, 1)->setToolTip(data.at(2).toString());
        ui->tableStudents->item(currentRow, 2)->setToolTip(data.at(3).toString());
        ui->tableStudents->item(currentRow, 3)->setToolTip(data.at(4).toString());

        ui->tableStudents->item(currentRow, 1)->setData(Qt::UserRole, data.at(1));
        return;
    }

    int rowCount = ui->tableStudents->rowCount();
    for (int i = 0; i < rowCount; ++i){
        if (ui->tableStudents->item(i, 0)->data(Qt::UserRole).toLongLong() == oldStudent){
            ui->tableStudents->item(i, 0)->setText(data.at(0).toString());
            ui->tableStudents->item(i, 1)->setText(data.at(2).toString());
            ui->tableStudents->item(i, 2)->setText(data.at(3).toString());
            ui->tableStudents->item(i, 3)->setText(data.at(4).toString());

            ui->tableStudents->item(i, 0)->setToolTip(data.at(0).toString());
            ui->tableStudents->item(i, 1)->setToolTip(data.at(2).toString());
            ui->tableStudents->item(i, 2)->setToolTip(data.at(3).toString());
            ui->tableStudents->item(i, 3)->setToolTip(data.at(4).toString());

            ui->tableStudents->item(i, 1)->setData(Qt::UserRole, data.at(1));
            return;
        }
    }
}

void frmMain::receiveNewCourseData(const QVariantList &data){
    int oldRCount = ui->tableCourses->rowCount();
    ui->tableCourses->insertRow(oldRCount);

    ui->tableCourses->setItem(oldRCount, 0, new QTableWidgetItem(data.at(0).toString()));
    ui->tableCourses->setItem(oldRCount, 1, new QTableWidgetItem(QString::number(data.at(1).toInt())));
    ui->tableCourses->setItem(oldRCount, 2, new QTableWidgetItem(data.at(2).toString()));
    ui->tableCourses->setItem(oldRCount, 3, new QTableWidgetItem(data.at(3).toDate().toString("dd/MM/yyyy")));
    ui->tableCourses->setItem(oldRCount, 4, new QTableWidgetItem(data.at(4).toString()));

    ui->tableCourses->item(oldRCount, 0)->setToolTip(data.at(0).toString());
    ui->tableCourses->item(oldRCount, 1)->setToolTip(QString::number(data.at(1).toInt()));
    ui->tableCourses->item(oldRCount, 2)->setToolTip(data.at(2).toString());
    ui->tableCourses->item(oldRCount, 3)->setToolTip(data.at(3).toDate().toString("dd/MM/yyyy"));
    ui->tableCourses->item(oldRCount, 4)->setToolTip(data.at(4).toString());

    if (oldRCount)
        ui->tableCourses->item(oldRCount, 0)->setData(Qt::UserRole, data.at(5).toLongLong() == -1 ?
                                                                    QVariant(ui->tableCourses->item(oldRCount - 1, 0)->data(Qt::UserRole).toLongLong() + 1) :
                                                      data.at(5));
    else
        ui->tableCourses->item(oldRCount, 0)->setData(Qt::UserRole, data.at(5).toLongLong() == -1 ?
                                                                    1 : data.at(5));
}

void frmMain::receiveCourseUpdatedData(const QVariantList &data, const qlonglong &oldCourse){
    int currentRow = ui->tableStudents->currentRow();
    if (ui->tableCourses->item(currentRow, 0)->data(Qt::UserRole).toLongLong() == oldCourse){
        ui->tableCourses->item(currentRow, 0)->setText(data.at(0).toString());
        ui->tableCourses->item(currentRow, 1)->setText(QString::number(data.at(1).toInt()));
        ui->tableCourses->item(currentRow, 2)->setText(data.at(2).toString());
        ui->tableCourses->item(currentRow, 3)->setText(data.at(3).toDate().toString("dd/MM/yyyy"));
        ui->tableCourses->item(currentRow, 4)->setText(data.at(4).toString());

        ui->tableCourses->item(currentRow, 0)->setToolTip(data.at(0).toString());
        ui->tableCourses->item(currentRow, 1)->setToolTip(QString::number(data.at(1).toInt()));
        ui->tableCourses->item(currentRow, 2)->setToolTip(data.at(2).toString());
        ui->tableCourses->item(currentRow, 3)->setToolTip(data.at(3).toDate().toString("dd/MM/yyyy"));
        ui->tableCourses->item(currentRow, 4)->setToolTip(data.at(4).toString());
        return;
    }

    int rowCount = ui->tableCourses->rowCount();
    for (int i = 0; i < rowCount; ++i){
        if (ui->tableCourses->item(i, 0)->data(Qt::UserRole).toLongLong() == oldCourse){
            ui->tableCourses->item(i, 0)->setText(data.at(0).toString());
            ui->tableCourses->item(i, 1)->setText(QString::number(data.at(1).toInt()));
            ui->tableCourses->item(i, 2)->setText(data.at(2).toString());
            ui->tableCourses->item(i, 3)->setText(data.at(3).toDate().toString("dd/MM/yyyy"));
            ui->tableCourses->item(i, 4)->setText(data.at(4).toString());

            ui->tableCourses->item(i, 0)->setToolTip(data.at(0).toString());
            ui->tableCourses->item(i, 1)->setToolTip(QString::number(data.at(1).toInt()));
            ui->tableCourses->item(i, 2)->setToolTip(data.at(2).toString());
            ui->tableCourses->item(i, 3)->setToolTip(data.at(3).toDate().toString("dd/MM/yyyy"));
            ui->tableCourses->item(i, 4)->setToolTip(data.at(4).toString());
            return;
        }
    }
}

void frmMain::searchPrevious(){
    QTableWidget* currentTable = ui->tableStudents->isVisible() ? ui->tableStudents : ui->tableCourses;
    int currentIndex = currentTable->currentRow() - 1,
        currentColumn = ui->tableStudents->isVisible() ? ui->cbStudentFilter->currentIndex() : ui->cbCourseFilter->currentIndex();
    for (int i = currentIndex; i >= 0; --i){
        if (currentTable->item(i, currentColumn)->text().contains(ui->edtSearch->text())){
            currentTable->selectRow(i);
            return;
        }
    }
    QMessageBox::information(this, tr("Search results | SmartClass"), tr("The search has reached the top of the table without any match.\nPlease, try again with different terms or different filters."), QMessageBox::Ok);
}

void frmMain::searchNext(){
    QTableWidget* currentTable = ui->tableStudents->isVisible() ? ui->tableStudents : ui->tableCourses;
    int currentIndex = currentTable->currentRow() + 1,
        maxIndex = currentTable->rowCount(),
        currentColumn = ui->tableStudents->isVisible() ? ui->cbStudentFilter->currentIndex() : ui->cbCourseFilter->currentIndex();
    for (int i = currentIndex; i < maxIndex; ++i){
        if (currentTable->item(i, currentColumn)->text().contains(ui->edtSearch->text())){
            currentTable->selectRow(i);
            return;
        }
    }
    QMessageBox::information(this, tr("Search results | SmartClass"), tr("The search has reached the bottom of the table without any match.\nPlease, try again with different terms or different filters."), QMessageBox::Ok);
}

void frmMain::removeStudent(){
    if (ui->tableStudents->currentRow() < 0 || ui->tableStudents->rowCount() < 1 || ui->tableStudents->isHidden()){
        QMessageBox::information(this, tr("Warning | SmartClass"), tr("Sorry, but you have to select a row in the students table in order to execute this command."), QMessageBox::Ok);
        return;
    }
    if (QMessageBox::question(this, tr("Confirmation | SmartClass"),
                              tr("You are going to erase %1 from the database.\nThis action cannot be undone. Continue?").arg(ui->tableStudents->item(ui->tableStudents->currentRow(), 0)->text()),
                              QMessageBox::Yes, QMessageBox::No)
            == QMessageBox::No) return;

    const qlonglong STUDENT_ID = ui->tableStudents->item(ui->tableStudents->currentRow(), 0)->data(Qt::UserRole).toLongLong();
    const qlonglong RESPONSIBLE_ID = ui->tableStudents->item(ui->tableStudents->currentRow(), 1)->data(Qt::UserRole).toLongLong();

    QStringList sTableSchema = SmartClassGlobal::getTableAliases(SmartClassGlobal::STUDENT),
                rTableSchema = SmartClassGlobal::getTableAliases(SmartClassGlobal::RESPONSIBLE),
                pTableSchema = SmartClassGlobal::getTableAliases(SmartClassGlobal::PAYMENTDETAILS),
                siTableSchema = SmartClassGlobal::getTableAliases(SmartClassGlobal::STUDENTIMAGES),
                riTableSchema = SmartClassGlobal::getTableAliases(SmartClassGlobal::RESPONSIBLEIMAGES),
                ceTableSchema = SmartClassGlobal::getTableAliases(SmartClassGlobal::COURSEENROLLMENTS);

    myDB->removeRow(SmartClassGlobal::getTableName(SmartClassGlobal::STUDENTIMAGES),
                    siTableSchema.at(0), STUDENT_ID);

    while (myDB->rowExists(SmartClassGlobal::getTableName(SmartClassGlobal::COURSEENROLLMENTS),
                           ceTableSchema.at(1), STUDENT_ID))
        myDB->removeRow(SmartClassGlobal::getTableName(SmartClassGlobal::COURSEENROLLMENTS),
                        ceTableSchema.at(1), STUDENT_ID);
    while (myDB->rowExists(SmartClassGlobal::getTableName(SmartClassGlobal::PAYMENTDETAILS),
                           pTableSchema.at(0), STUDENT_ID))
        myDB->removeRow(SmartClassGlobal::getTableName(SmartClassGlobal::PAYMENTDETAILS),
                        pTableSchema.at(0), STUDENT_ID);

    myDB->removeRow(SmartClassGlobal::getTableName(SmartClassGlobal::STUDENT),
                    sTableSchema.at(0), STUDENT_ID);

    if (!myDB->rowExists(SmartClassGlobal::getTableName(SmartClassGlobal::STUDENT),
                         sTableSchema.at(2), RESPONSIBLE_ID)){
        myDB->removeRow(SmartClassGlobal::getTableName(SmartClassGlobal::RESPONSIBLEIMAGES),
                        riTableSchema.at(0), RESPONSIBLE_ID);
        myDB->removeRow(SmartClassGlobal::getTableName(SmartClassGlobal::RESPONSIBLE),
                        rTableSchema.at(0), RESPONSIBLE_ID);
    }

    ui->tableStudents->removeRow(ui->tableStudents->currentRow());
}

void frmMain::removeCourse(){
    if (ui->tableCourses->currentRow() < 0 || ui->tableCourses->rowCount() < 1 || ui->tableCourses->isHidden()){
        QMessageBox::information(this, tr("Warning | SmartClass"), tr("Sorry, but you have to select a row in the students table in order to execute this command."), QMessageBox::Ok);
        return;
    }
    if (QMessageBox::question(this, tr("Confirmation | SmartClass"),
                              tr("You are going to erase %1 from the database.\nThis action cannot be undone. Continue?").arg(ui->tableCourses->item(ui->tableCourses->currentRow(), 0)->text()),
                              QMessageBox::Yes, QMessageBox::No)
            == QMessageBox::No) return;

    qlonglong COURSE_ID = ui->tableCourses->item(ui->tableCourses->currentRow(), 0)->data(Qt::UserRole).toLongLong();

    QString pTableName = SmartClassGlobal::getTableName(SmartClassGlobal::PAYMENTDETAILS),
            ceTableName = SmartClassGlobal::getTableName(SmartClassGlobal::COURSEENROLLMENTS);
    QStringList pTableSchema = SmartClassGlobal::getTableAliases(SmartClassGlobal::PAYMENTDETAILS),
                ceTableSchema = SmartClassGlobal::getTableAliases(SmartClassGlobal::COURSEENROLLMENTS);

    while (myDB->rowExists(ceTableName, ceTableSchema.at(0), COURSE_ID))
        myDB->removeRow(ceTableName, ceTableSchema.at(0), COURSE_ID);

    while (myDB->rowExists(pTableName, pTableSchema.at(1), COURSE_ID))
        myDB->removeRow(pTableName, pTableSchema.at(1), COURSE_ID);

    myDB->removeRow(SmartClassGlobal::getTableName(SmartClassGlobal::COURSEDETAILS),
                    SmartClassGlobal::getTableAliases(SmartClassGlobal::COURSEDETAILS).at(0),
                    COURSE_ID);

    ui->tableCourses->removeRow(ui->tableCourses->currentRow());
}

void frmMain::restoreDataBase(){
    if (SmartClassGlobal::databaseType() != DBManager::SQLITE){
        QMessageBox::information(this, tr("Info | SmartClass"),
                                 tr("Unfortunately you cannot restore the database from a file, since you are not using an SQLITE database."
                                    "\nPlease, use either the import tool or restore the dumped file in your host in order to restore your DB."),
                                 QMessageBox::Ok);
        return;
    }

    QFileDialog restoreDlg;
    restoreDlg.setWindowTitle(tr("Select database file | SmartClass"));
    restoreDlg.setNameFilters(QStringList() << "SQLite database file (*.db *.sqlite3)");
    restoreDlg.setAcceptMode(QFileDialog::AcceptOpen);
    restoreDlg.setDirectory(QDir::homePath());
    if (restoreDlg.exec()){
        if (QMessageBox::question(this, tr("Confirmation | SmartClass"), tr("You are going to replace the current database by a new one. This may corrupt your database system."
                                                                            "\nThis action cannot be undone. This application is going to restart after the procedure is complete."
                                                                            "\nDo you still want to proceed?"),
                                  QMessageBox::Yes, QMessageBox::No) == QMessageBox::Yes){
            if (loginScr){
                delete loginScr;
                loginScr = NULL;
            }
            if (manageStudent){
                delete manageStudent;
                manageStudent = NULL;
            }
            if (manageClass){
                delete manageClass;
                manageClass = NULL;
            }

            myDB->close();

            QFile::remove(SmartClassGlobal::getDBPath());
            QFile::copy(restoreDlg.selectedFiles().at(0), SmartClassGlobal::getDBPath());

            QMessageBox::information(this, tr("Information | SmartClass"), tr("This application is about to restart!"),
                                     QMessageBox::Ok);
            QDesktopServices::openUrl(QUrl(QCoreApplication::applicationFilePath()));
            qApp->quit();
        }
    }
}

void frmMain::backupDataBase(){
    if (SmartClassGlobal::databaseType() != DBManager::SQLITE){
        QMessageBox::information(this, tr("Info | SmartClass"),
                                 tr("Unfortunately you cannot backup the database to a file, since you are not using an SQLITE database."
                                    "\nPlease, use either the import tool or dump the database in the host in order to restore your DB."),
                                 QMessageBox::Ok);
        return;
    }

    if (loginScr){
        delete loginScr;
        loginScr = NULL;
    }
    if (manageStudent){
        delete manageStudent;
        manageStudent = NULL;
    }
    if (manageClass){
        delete manageClass;
        manageClass = NULL;
    }

    myDB->close();

    QFileDialog saveDialog;
    saveDialog.setWindowTitle(tr("Save database file | SmartClass"));
    saveDialog.setNameFilters(QStringList() << tr("SQLite database file (*.db *.sqlite3)"));
    saveDialog.setDefaultSuffix("db");
    saveDialog.setAcceptMode(QFileDialog::AcceptSave);
    saveDialog.setDirectory(QDir::homePath());

    if (saveDialog.exec())
        QFile::copy(SmartClassGlobal::getDBPath(), saveDialog.selectedFiles().at(0));

    if (!myDB->open())
        QMessageBox::critical(NULL, tr("Critical error | SmartClass"),
                              tr("The connection with the database could not be restored."
                                 "\nPlease, restart the program in order to have access its functionalities."),
                              QMessageBox::Ok, QMessageBox::NoButton);
}

void frmMain::removeDataBase(){
    if  (deleteDBStatus == 0){
        if (QMessageBox::question(this, tr("Delete database confirmation | SmartClass"),
                                  tr("Would you like to reset the database?"
                                     "\nThis is going to erase all the contents in your database (even your login information)."
                                     "\nIf you accept, you are going to be logged out and you will have to choose this option again."),
                                  QMessageBox::Yes, QMessageBox::No) == QMessageBox::Yes){
            deleteDBStatus++;
            logOut();
        }
        return;
    }
    else if (deleteDBStatus == 1){
        QMessageBox confirmation;
        confirmation.setWindowTitle(tr("Delete database confirmation | SmartClass"));
        confirmation.setText(tr("Would you like to reset the database?"
                                "\nThis action cannot be undone (be sure you have a backup of your database)."
                                "\nYou have two options to choose:"
                                "\n\n->Delete the tables (this will not remove the database file, but will erase all its contents)"
                                "\n->Delete database file (only available for SQLITE)"));
        if (SmartClassGlobal::databaseType() == DBManager::SQLITE){
            confirmation.setStandardButtons(QMessageBox::YesToAll | QMessageBox::Yes | QMessageBox::Cancel);
            confirmation.setButtonText(QMessageBox::YesToAll, tr("Detele file"));
        }
        else confirmation.setStandardButtons(QMessageBox::Yes | QMessageBox::Cancel);
        confirmation.setButtonText(QMessageBox::Yes, tr("Delete tables"));

        if (confirmation.exec() == QMessageBox::Yes){
            if (loginScr){
                delete loginScr;
                loginScr = NULL;
            }
            if (manageStudent){
                delete manageStudent;
                manageStudent = NULL;
            }
            if (manageClass){
                delete manageClass;
                manageClass = NULL;
            }

            myDB->dropTable(SmartClassGlobal::getTableName(SmartClassGlobal::COURSEENROLLMENTS));
            myDB->dropTable(SmartClassGlobal::getTableName(SmartClassGlobal::PAYMENTDETAILS));
            myDB->dropTable(SmartClassGlobal::getTableName(SmartClassGlobal::STUDENTIMAGES));
            myDB->dropTable(SmartClassGlobal::getTableName(SmartClassGlobal::RESPONSIBLEIMAGES));
            myDB->dropTable(SmartClassGlobal::getTableName(SmartClassGlobal::STUDENT));
            myDB->dropTable(SmartClassGlobal::getTableName(SmartClassGlobal::RESPONSIBLE));
            myDB->dropTable(SmartClassGlobal::getTableName(SmartClassGlobal::COURSEDETAILS));
            myDB->clearTable(SmartClassGlobal::getTableName(SmartClassGlobal::SETTINGS));
            myDB->clearTable(SmartClassGlobal::getTableName(SmartClassGlobal::USERS));
            myDB->clearTable(SmartClassGlobal::getTableName(SmartClassGlobal::ACTIVECONNECTIONS));
            myDB->close();

            settings->clear();

            QDesktopServices::openUrl(QUrl(QCoreApplication::applicationFilePath()));
            qApp->quit();
        }
        else if (confirmation.exec() == QMessageBox::YesToAll){
            if (loginScr){
                delete loginScr;
                loginScr = NULL;
            }
            if (manageStudent){
                delete manageStudent;
                manageStudent = NULL;
            }
            if (manageClass){
                delete manageClass;
                manageClass = NULL;
            }

            myDB->close();

            settings->clear();

            QFile::remove(SmartClassGlobal::getDBPath());
            QDesktopServices::openUrl(QUrl(QCoreApplication::applicationFilePath()));
            qApp->quit();
        }
    }
}

void frmMain::removeAppSettings(){
    QMessageBox uninstall;
    uninstall.setWindowTitle(tr("Maintenance Tool | SmartClass"));
    uninstall.setIcon(QMessageBox::Warning);
    uninstall.setText(tr("Are you facing any trouble with SmartClass? This tool may be helpful for you. We have three options currently available:\n"
                         "Clean Settings : Clear the initialization/standard settings and restart the application as a clean installation (Note that this option has no effect over the database).\n"
                         "Uninstall : Performs a clean uninstall by removing any settings and files which may rest after the uninstallation process (If you are using SQLite the database may be removed during the process of uninstallation. Do not forget to make a backup before uninstalling!).\n"
                         "Cancel : Cancel the current operation."));
    uninstall.setStandardButtons(QMessageBox::Yes | QMessageBox::YesToAll | QMessageBox::Cancel);
    uninstall.setButtonText(QMessageBox::Yes, tr("Clean Settings"));
    uninstall.setButtonText(QMessageBox::YesToAll, tr("Uninstall"));

    int choice = uninstall.exec();
    if (choice == QMessageBox::Cancel) return;

    settings->clear();

    QString tempDir = QDir::homePath();
    if (QSysInfo::windowsVersion() != QSysInfo::WV_None)
        tempDir += "/AppData/Roaming/Nintersoft/SmartClass/";
    else tempDir += "/.Nintersoft/SmartClass/";

    QDir downloadDir(tempDir + "Downloads/");
    if (downloadDir.exists()) downloadDir.removeRecursively();

    QDir dataDir(tempDir + "images/");
    if (dataDir.exists()) dataDir.removeRecursively();

    if (choice == QMessageBox::Yes){
        QDesktopServices::openUrl(QUrl(QCoreApplication::applicationFilePath()));
        QApplication::exit(EXIT_SUCCESS);
    }

    QMessageBox::information(this, tr("Process complete | SmartClass"),
                             tr("The process of uninstallation is going to continue right after you dismiss this message!"), QMessageBox::Ok);
    QDesktopServices::openUrl(QUrl::fromLocalFile(QCoreApplication::applicationDirPath() + "/unins000.exe"));
    QApplication::exit(EXIT_SUCCESS);
}

void frmMain::openNSDocwiki(){
    QDesktopServices::openUrl(tr("http://docwiki.nintersoft.com/en/"));
}

void frmMain::openNSWebSite(){
    QDesktopServices::openUrl(tr("http://www.nintersoft.com/en/"));
}

void frmMain::openOnlineSupport(){
    QDesktopServices::openUrl(tr("http://www.nintersoft.com/en/support/"));
}

void frmMain::openSupportEmail(){
    QDesktopServices::openUrl(tr("mailto:support@nintersoft.com?subject=SmartClass%20Support"));
}

void frmMain::alternateTable(){
    if (ui->rbStudentsTable->isChecked()) ui->rbCoursesTable->setChecked(true);
    else ui->rbStudentsTable->setChecked(true);
}

void frmMain::changeTable(bool student){
    ui->tableStudents->setVisible(student);
    ui->cbStudentFilter->setVisible(student);
    ui->tableCourses->setVisible(!student);
    ui->cbCourseFilter->setVisible(!student);

    if (sessionRole == SmartClassGlobal::NEW) return;

    ui->btPrintStudentForms->setEnabled(student);
    ui->btUpdateStudent->setEnabled(student);
    ui->btUpdateCourse->setEnabled(!student);

    if (sessionRole == SmartClassGlobal::VIEWER) return;

    ui->btRemoveStudent->setEnabled(student);
    ui->btRemoveCourse->setEnabled(!student);
}

void frmMain::openStudentTableContextMenu(const QPoint &pos){
    if (ui->tableStudents->currentRow() < 0) return;

    QMenu contextMenu(tr("Students table menu | SmartClass"), ui->tableStudents);

    QAction action1(QIcon(":/images/buttons/UpdateStudent.PNG"), tr("View/Update student data"), ui->tableStudents);
    connect(&action1, SIGNAL(triggered(bool)), this, SLOT(openStudentManager()));
    QAction action2(QIcon(":/images/buttons/RemoveStudent.PNG"), tr("Remove student"), ui->tableStudents);
    connect(&action2, SIGNAL(triggered(bool)), this, SLOT(removeStudent()));

    switch (sessionRole) {
        case SmartClassGlobal::ADMIN:
        case SmartClassGlobal::EDITOR:
            contextMenu.addActions({&action1, &action2});
            break;
        case SmartClassGlobal::VIEWER:
            contextMenu.addActions({&action1});
        default:
            break;
    }

    contextMenu.exec(ui->tableStudents->mapToGlobal(pos));
}

void frmMain::openClassTableContextMenu(const QPoint &pos){
    if (ui->tableCourses->currentRow() < 0) return;

    QMenu contextMenu(tr("Courses table menu | SmartClass"), ui->tableCourses);

    QAction action1(QIcon(":/images/buttons/UpdateClasses.PNG"), tr("View/Update course data"), ui->tableCourses);
    connect(&action1, SIGNAL(triggered(bool)), this, SLOT(openStudentManager()));
    QAction action2(QIcon(":/images/buttons/RemoveClasses.PNG"), tr("Remove course"), ui->tableCourses);
    connect(&action2, SIGNAL(triggered(bool)), this, SLOT(removeStudent()));

    switch (sessionRole) {
        case SmartClassGlobal::ADMIN:
        case SmartClassGlobal::EDITOR:
            contextMenu.addActions({&action1, &action2});
            break;
        case SmartClassGlobal::VIEWER:
            contextMenu.addActions({&action1});
        default:
            break;
    }

    contextMenu.exec(ui->tableCourses->mapToGlobal(pos));
}

void frmMain::changeLanguage(const QString &slug){
    currentLanguage = slug;
    changeTranslator(translator, QString(langPath + "SmartClass_%1.qm").arg(slug));
    changeTranslator(qtTranslator, QString(langPath + "qt_%1.qm").arg(slug));
}

void frmMain::changeTranslator(QTranslator &transl, const QString &filePath){
    QApplication::removeTranslator(&transl);
    if(transl.load(filePath)) QApplication::installTranslator(&transl);
}

void frmMain::changeEvent(QEvent *event){
    if (event != NULL && event->type() == QEvent::LanguageChange)
        ui->retranslateUi(this);
    NMainWindow::changeEvent(event);
}

void frmMain::setBackupSettings(){
    QSettings programSettings("Nintersoft", "SmartClass");
    if (programSettings.childGroups().contains("backup settings")){
        programSettings.beginGroup("backup settings");
        if (programSettings.value("enable backup", true).toBool()){
            backupPath = programSettings.value("backup path", backupPath).toString();
            if (programSettings.value("backup type", 1).toInt() == 1){
                int interval, index = programSettings.value("backup interval index", 2).toInt();
                if (index == 0) interval = 5;
                else if (index == 1) interval = 10;
                else if (index == 2) interval = 15;
                else if (index == 3) interval = 30;
                else if (index == 4) interval = 45;
                else if (index == 5) interval = 60;
                else if (index == 6) interval = 120;
                else interval = 240;
                backupTimer.setInterval(interval * 60000);
                connect(&backupTimer, SIGNAL(timeout()), this, SLOT(scheduledBackup()));
                backupTimer.start();
            }
            else {
                QStringList scheduledTimes = programSettings.value("scheduled backups", "").toString().split(';');
                if (scheduledTimes.count() != 0){
                    for (int i = 0; i < scheduledTimes.count(); ++i){
                        int time = QTime::currentTime().msecsTo(QTime::fromString(scheduledTimes.at(i)));
                        if (time > 0) QTimer::singleShot(time, this, SLOT(scheduledBackup()));
                    }
                }
            }
        }
        programSettings.endGroup();
    }
}

void frmMain::scheduledBackup(){
    if (loginScr){
        delete loginScr;
        loginScr = NULL;
    }
    if (manageStudent){
        delete manageStudent;
        manageStudent = NULL;
    }
    if (manageClass){
        delete manageClass;
        manageClass = NULL;
    }

    myDB->close();
    QString filename = backupPath + QDir::separator() +
            QDateTime::currentDateTime().toString("yyyy_MM_dd-HH_mm") + tr("_BackUp_SmartClass_DB.db");
    QFile::copy(SmartClassGlobal::getDBPath(), filename);

    if (!myDB->open())
        QMessageBox::critical(NULL, tr("Critical error | SmartClass"),
                              tr("The connection with the database could not be restored after the backup."
                                 "\nPlease, restart the program in order to have access it's functionalities."),
                              QMessageBox::Ok, QMessageBox::NoButton);
}

void frmMain::setUpgradeAvailable(){
    upgradeAvailable = true;
}
