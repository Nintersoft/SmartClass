#include "frmmain.h"
#include "ui_frmmain.h"

frmMain::frmMain(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::frmMain),
    RESIZE_LIMIT(2)
{
    ui->setupUi(this);
    setWindowFlags(Qt::Widget | Qt::FramelessWindowHint);
    centralWidget()->installEventFilter(this);
    ui->titleBar->installEventFilter(this);
    ui->statusBar->installEventFilter(this);

    centralWidget()->setMouseTracking(true);
    ui->titleBar->setMouseTracking(true);
    ui->statusBar->setMouseTracking(true);

    setWindowTitle(tr("SmartClass | Nintersoft"));
    locked = LockMoveType::None;

    /*
     *  End of GUI implementation
     */

    returnCode = -1;

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

    bool isMySQL = false, dbData;
    if (settings->childGroups().contains("dbinfo", Qt::CaseInsensitive)){
        settings->beginGroup("dbinfo");
        isMySQL = (settings->value("database type", "").toString() == "MYSQL");
        settings->endGroup();
        dbData = true;
    }

    if (dbData && (QFile::exists(frmLogin::getDBPath()) || isMySQL)){
        setupDBConnection();
        if (returnCode == -1){
            if (settings->childGroups().contains("language options", Qt::CaseInsensitive)){
                settings->beginGroup("language options");
                if (settings->value("current language index").toInt() != 0) changeLanguage("pt");
                settings->endGroup();
            }

            loginScr = new frmLogin(NULL, db_SETTINGS);
            connect(loginScr, SIGNAL(dataReady(QStringList)), this, SLOT(setSessionRole(QStringList)));
            loginScr->show();

            createTables();

            frmAbt = new frmAbout();
            connect(ui->btAbout, SIGNAL(clicked(bool)), frmAbt, SLOT(show()));
        }
    }
    else {
        firstRunScr = new frmFirstRun();
        connect(firstRunScr, SIGNAL(sendData(QStringList, QString)), this, SLOT(getFirstSettings(QStringList, QString)));
        firstRunScr->show();
    }

    if (returnCode == -1){
        this->hide();

        connect(ui->btLogOff, SIGNAL(clicked(bool)), this, SLOT(logOut()));
        connect(ui->rbStudentsTable, SIGNAL(toggled(bool)), this, SLOT(changeTable(bool)));

        connect(ui->btAddStudent, SIGNAL(clicked(bool)), this, SLOT(openStudentManager()));
        connect(ui->btUpdateStudent, SIGNAL(clicked(bool)), this, SLOT(openStudentManager()));
        connect(ui->btRemoveStudent, SIGNAL(clicked(bool)), this, SLOT(removeStudent()));
        connect(ui->btAddCourse, SIGNAL(clicked(bool)), this, SLOT(openClassesManager()));
        connect(ui->btUpdateCourse, SIGNAL(clicked(bool)), this, SLOT(openClassesManager()));
        connect(ui->btRemoveCourse, SIGNAL(clicked(bool)), this, SLOT(removeCourse()));

        connect(ui->btSettings, SIGNAL(clicked(bool)), this, SLOT(openSettingsForm()));
        connect(ui->btInfo, SIGNAL(clicked(bool)), this, SLOT(openSettingsForm()));

        connect(ui->btPrintStudentForms, SIGNAL(clicked(bool)), this, SLOT(openContractForm()));
        connect(ui->btGetReceipt, SIGNAL(clicked(bool)), this, SLOT(openReceiptForm()));

        deleteDBStatus = 0;

        connect(ui->btRemoveUser, SIGNAL(clicked(bool)), this, SLOT(removeUser()));
        connect(ui->btUpgradeRole, SIGNAL(clicked(bool)), this, SLOT(upgradeUserPermissions()));
        connect(ui->btRevokeRole, SIGNAL(clicked(bool)), this, SLOT(revokeUserPermissions()));

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
        frmContract = NULL;
        frmPayment = NULL;
        frmConfig= NULL;
        addClass = NULL;
    }
}

frmMain::~frmMain()
{
    if (myDB){
        if (myDB->isOpen()) myDB->closeDB();
        delete myDB;
    }

    if (firstRunScr) delete firstRunScr;
    if (settings) delete settings;

    delete ui;
}

void frmMain::closeEvent(QCloseEvent *event){
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
    QMainWindow::resizeEvent(event);
}

void frmMain::createTables(){
    QStringList studentsTable, parentsTable, studentImagesTable, pricingTable, coursesTable, parentImagesTable;
    QString id = (db_SETTINGS.length() == 2) ? "id INTEGER PRIMARY KEY" : "id INTEGER(64) UNSIGNED AUTO_INCREMENT PRIMARY KEY";
    studentsTable << id << "student TEXT NOT NULL" << "birthday TEXT NOT NULL"
                    << "studentID TEXT" << "school TEXT" << "observations TEXT"
                    << "experimentalCourse TEXT" << "experimentalCourseDate TEXT"
                    << "experimentalCourseObservations TEXT" << "courses TEXT" << "address TEXT";
    parentsTable << id << "parent TEXT NOT NULL" << "students TEXT NOT NULL"
                 << "phone TEXT" << "mobileOperator TEXT" << "mobile TEXT" << "email TEXT"
                    << "parentID TEXT" << "parentCPG TEXT" << "meeting TEXT";
    studentImagesTable << id << "student TEXT NOT NULL" << "studentImage BLOB"
                       << "studentID BLOB";
    parentImagesTable << id << "parent TEXT NOT NULL" << "parentID BLOB"
                      << "parentCPG BLOB" << "addressComprobation BLOB";
    pricingTable << id << "student TEXT NOT NULL" << "course TEXT NOT NULL" << "discount TEXT"
                 << "beginningDate TEXT NOT NULL" << "installments TEXT NOT NULL";
    coursesTable << id << "course TEXT NOT NULL" << "teacher TEXT NOT NULL"
                 << "shortDescription TEXT NOT NULL" << "longDescription TEXT NOT NULL" << "class TEXT NOT NULL"
                 << "dayNTime TEXT NOT NULL" << "beginningDate TEXT NOT NULL" << "endDate TEXT NOT NULL"
                 << "price TEXT NOT NULL" << "students TEXT";

    if (!myDB->openDB()){
        QMessageBox dbWarning;
        dbWarning.setWindowTitle(tr("Connection issue | SmartClass"));
        dbWarning.setIcon(QMessageBox::Critical);
        dbWarning.setStandardButtons(QMessageBox::Discard | QMessageBox::Abort);
        dbWarning.setButtonText(QMessageBox::Discard, tr("Clean settings"));
        dbWarning.setButtonText(QMessageBox::Abort, tr("Quit"));
        dbWarning.setText(tr("It was not possible to establish connection to the database. Unfortunately it will not be possible to continue with the execution of this program. Please, try again later."
                             "\nIf you keep having trouble while trying to connect, please, consider to clean the program settings (the database is not going to be affected)."));
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

    myDB->createTable("myclass_parents", parentsTable);
    myDB->createTable("myclass_students", studentsTable);
    myDB->createTable("myclass_simages", studentImagesTable);
    myDB->createTable("myclass_pimages", parentImagesTable);
    myDB->createTable("myclass_pricing", pricingTable);
    myDB->createTable("myclass_courses", coursesTable);
}

void frmMain::setupDBConnection(){
    if (!settings->childGroups().contains("dbinfo")){
        settings->beginGroup("dbinfo");
        settings->setValue("database type", "SQLITE");
        db_SETTINGS = QStringList() << frmLogin::getDBPath()
                                    << settings->value("table prefix", "").toString();
        myDB = new DBManager(db_SETTINGS, DBManager::getUniqueConnectionName("mainConnection"));
    }
    else {
        settings->beginGroup("dbinfo");
        if (settings->value("database type").toString() == "SQLITE"){
            db_SETTINGS = QStringList() << frmLogin::getDBPath()
                                        << settings->value("table prefix", "").toString();
            myDB = new DBManager(db_SETTINGS, DBManager::getUniqueConnectionName("mainConnection"));
        }
        else {
            db_SETTINGS << settings->value("host").toString()
                        << settings->value("database").toString()
                        << QString::number(settings->value("port").toInt())
                        << settings->value("username").toString()
                        << settings->value("password").toString()
                        << settings->value("table prefix", "").toString();
            myDB = new DBManager(db_SETTINGS, DBManager::getUniqueConnectionName("mainConnection"), "MYSQL");
        }
    }
    settings->endGroup();
    createTables();
}

void frmMain::getFirstSettings(const QStringList &sqlData, const QString &langSlug){
    db_SETTINGS = sqlData;

    settings->beginGroup("dbinfo");
    int size = sqlData.length();
    if (size == 2){
        settings->setValue("database type", QString("SQLITE"));
        settings->setValue("table prefix", QString(sqlData.at(1)));
        myDB = new DBManager(sqlData, DBManager::getUniqueConnectionName("mainConnection"), "SQLITE");
    }
    else {
        settings->setValue("database type", QString("MYSQL"));
        settings->setValue("host", QString(sqlData.at(0)));
        settings->setValue("database", QString(sqlData.at(1)));
        settings->setValue("port", QString(sqlData.at(2)).toInt());
        settings->setValue("username", QString(sqlData.at(3)));
        settings->setValue("password", QString(sqlData.at(4)));
        settings->setValue("table prefix", QString(sqlData.at(5)));
        myDB = new DBManager(sqlData, DBManager::getUniqueConnectionName("mainConnection"), "MYSQL");
    }

    settings->endGroup();
    if (langSlug != "en") changeLanguage(langSlug);

    createTables();

    if (loginScr){
        delete loginScr;
        loginScr = NULL;
    }

    loginScr = new frmLogin(NULL, sqlData);
    connect(loginScr, SIGNAL(dataReady(QStringList)), this, SLOT(setSessionRole(QStringList)));
    loginScr->show();

    frmAbt = new frmAbout();
    connect(ui->btAbout, SIGNAL(clicked(bool)), frmAbt, SLOT(show()));
    this->hide();

    firstRunScr->close();
    delete firstRunScr;
    firstRunScr = NULL;
}

void frmMain::setSessionRole(const QStringList &userInfo){
    sessionRole = userInfo.at(7);
    currentUser = userInfo.at(0);

    this->showMaximized();
    loginScr->close();

    setUIToRole();

    getUsers();
    if (sessionRole != "NEW"){
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
        ui->lwUsers->clear();
        ui->tabWidget->setCurrentIndex(1);

        if (loginScr){
            disconnect(loginScr, SIGNAL(dataReady(QStringList)), this, SLOT(setSessionRole(QStringList)));
            delete loginScr;
            loginScr = NULL;
        }

        loginScr = new frmLogin(NULL, db_SETTINGS);
        connect(loginScr, SIGNAL(dataReady(QStringList)), this, SLOT(setSessionRole(QStringList)));
        loginScr->show();

        myDB->closeDB();
        this->hide();
    }
}

void frmMain::openStudentManager(){
    if (manageStudent){
        disconnect(manageStudent, SIGNAL(updatedData(QStringList,QString)),
                   this, SLOT(receiveStudentUpdatedData(QStringList,QString)));
        disconnect(manageStudent, SIGNAL(newData(QStringList)),
                   this, SLOT(receiveNewStudentData(QStringList)));

        delete manageStudent;
        manageStudent = NULL;
    }

    QString senderName = sender()->objectName();
    if (senderName == "btAddStudent") manageStudent = new frmManageStudent(NULL, frmManageStudent::Create, NULL, db_SETTINGS);
    else if (ui->tableStudents->currentRow() < 0){
        QMessageBox::information(this, tr("Warning | SmartClass"), tr("Sorry, but you have to select a row in the students table in order to execute this command."), QMessageBox::Ok);
        return;
    }
    else if (ui->tableStudents->isHidden()){
        QMessageBox::information(this, tr("Warning | SmartClass"), tr("Sorry, but the table of students must be selected in order to execute this command."), QMessageBox::Ok);
        return;
    }
    else if (senderName == "btUpdateStudent" && (sessionRole == "ADMIN" || sessionRole == "EDITOR"))
        manageStudent = new frmManageStudent(NULL, frmManageStudent::Role::Edit, ui->tableStudents->item(ui->tableStudents->currentRow(), 0)->text(), db_SETTINGS);
    else manageStudent = new frmManageStudent(NULL, frmManageStudent::Role::View, ui->tableStudents->item(ui->tableStudents->currentRow(), 0)->text(), db_SETTINGS);

    connect(manageStudent, SIGNAL(updatedData(QStringList,QString)),
               this, SLOT(receiveStudentUpdatedData(QStringList,QString)));
    connect(manageStudent, SIGNAL(newData(QStringList)),
               this, SLOT(receiveNewStudentData(QStringList)));
    manageStudent->show();
}

void frmMain::openClassesManager(){
    if (addClass) {
        disconnect(addClass, SIGNAL(updatedData(QStringList,QString)),
                   this, SLOT(receiveCourseUpdatedData(QStringList,QString)));
        disconnect(addClass, SIGNAL(newData(QStringList)),
                   this, SLOT(receiveNewCourseData(QStringList)));

        delete addClass;
        addClass = NULL;
    }

    QString senderName = sender()->objectName();
    if (senderName == "btAddCourse") addClass = new frmAddClass(NULL, frmAddClass::Create, NULL, db_SETTINGS);
    else if (ui->tableCourses->currentRow() < 0){
        QMessageBox::information(this, tr("Warning | SmartClass"), tr("Sorry, but you have to select a row in the courses table in order to execute this command."), QMessageBox::Ok);
        return;
    }
    else if (ui->tableCourses->isHidden()){
        QMessageBox::information(this, tr("Warning | SmartClass"), tr("Sorry, but the table of courses must be selected in order to execute this command."), QMessageBox::Ok);
        return;
    }
    else if (senderName == "btUpdateCourse" && (sessionRole == "ADMIN" || sessionRole == "EDITOR"))
        addClass = new frmAddClass(NULL, frmAddClass::Role::Edit, QString(ui->tableCourses->item(ui->tableCourses->currentRow(), 0)->text()
                                                                          + "|" + ui->tableCourses->item(ui->tableCourses->currentRow(), 1)->text()
                                                                          + "|" + ui->tableCourses->item(ui->tableCourses->currentRow(), 4)->text()
                                                                          + "|" + ui->tableCourses->item(ui->tableCourses->currentRow(), 3)->text()),
                                                                  db_SETTINGS);
    else addClass = new frmAddClass(NULL, frmAddClass::Role::View, QString(ui->tableCourses->item(ui->tableCourses->currentRow(), 0)->text()
                                                                           + "|" + ui->tableCourses->item(ui->tableCourses->currentRow(), 1)->text()
                                                                           + "|" + ui->tableCourses->item(ui->tableCourses->currentRow(), 4)->text()
                                                                           + "|" + ui->tableCourses->item(ui->tableCourses->currentRow(), 3)->text()),
                                                                  db_SETTINGS);

    connect(addClass, SIGNAL(updatedData(QStringList,QString)),
               this, SLOT(receiveCourseUpdatedData(QStringList,QString)));
    connect(addClass, SIGNAL(newData(QStringList)),
               this, SLOT(receiveNewCourseData(QStringList)));
    addClass->show();
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

    QString companyName = NULL;
    if (settings->childGroups().contains("company info")){
        settings->beginGroup("company info");
        companyName = settings->value("company info", "").toString();
        settings->endGroup();
    }
    frmContract = new frmPrintContract(NULL,
                                       myDB->retrieveLine("myclass_students", "student", ui->tableStudents->item(ui->tableStudents->currentRow(), 0)->text()),
                                       myDB->retrieveLine("myclass_parents", "parent", ui->tableStudents->item(ui->tableStudents->currentRow(), 1)->text()),
                                       myDB->retrieveAllCondS("myclass_pricing", "student", ui->tableStudents->item(ui->tableStudents->currentRow(), 0)->text()),
                                       myDB->rowsCountCond("myclass_pricing", "student", ui->tableStudents->item(ui->tableStudents->currentRow(), 0)->text()),
                                       myDB->retrieveAllCondS("myclass_courses", "students", QString("%" + ui->tableStudents->item(ui->tableStudents->currentRow(), 0)->text() + "%"), "LIKE"),
                                       myDB->rowsCountCond("myclass_courses", "students", QString("%" + ui->tableStudents->item(ui->tableStudents->currentRow(), 0)->text() + "%"), "LIKE"),
                                       companyName);
    frmContract->show();
}

void frmMain::openSettingsForm(){
    if (frmConfig){
        delete frmConfig;
        frmConfig = NULL;
    }

    if (sender()->objectName() == "btInfo") frmConfig = new frmSettings(NULL, frmSettings::Info);
    else frmConfig = new frmSettings();
    frmConfig->show();
}

void frmMain::openReceiptForm(){
    if (frmPayment){
        delete frmPayment;
        frmPayment = NULL;
    }

    frmPayment = new frmReceipt(NULL, myDB->retrieveAll("myclass_pricing", QStringList() << "student" << "course" << "discount" << "beginningDate"
                                                                                        << "installments"),
                                myDB->retrieveAll("myclass_courses", QStringList() << "course" << "price" << "dayNTime" << "beginningDate" << "class"),
                                myDB->rowsCount("myclass_pricing"), myDB->rowsCount("myclass_courses"));
    frmPayment->show();
}

void frmMain::openImportExportTool(){
    if (frmImportExport){
        delete frmImportExport;
        frmImportExport = NULL;
    }

    if (sender()->objectName() == "btImportDB")
        frmImportExport = new frmImportExportDB(NULL, frmImportExportDB::Import, db_SETTINGS);
    else frmImportExport = new frmImportExportDB(NULL, frmImportExportDB::Export, db_SETTINGS);

    frmImportExport->show();
}

void frmMain::getUsers(){
    QStringList* users = myDB->retrieveAll("myclass_users", QStringList() << "username" << "name" << "role");
    if (users == NULL) return;
    int size = myDB->rowsCount("myclass_users");

    for (int i = 0; i < size; ++i){
        QString itemCaption = users[i].at(1) + " [ " + users[i].at(0) + " ] ~ " + users[i].at(2);
        ui->lwUsers->addItem(itemCaption);
    }

    delete[] users;
}

void frmMain::upgradeUserPermissions(){
    int currentRow = ui->lwUsers->currentRow();
    if (currentRow < 0){
        QMessageBox::warning(this, tr("Information | SmartClass"), tr("You have not selected/specified any user to upgrade its permissions."), QMessageBox::Ok);
        return;
    }
    QListWidgetItem* item = ui->lwUsers->item(currentRow);

    if (item->text().contains(currentUser)){
        QMessageBox::information(this, tr("Information | SmartClass"), tr("Due to safety purposes, you are not able to update your own permissions. If you really need to change your permissions, please, ask to another administrator do it."), QMessageBox::Ok);
        return;
    }

    if (item->text().contains("ADMIN")){
        QMessageBox::information(this, tr("Information | SmartClass"), tr("This user already have the maximum level of permission. Operation aborted."), QMessageBox::Ok);
        return;
    }

    QString username = QString(item->text().split(" [ ").at(1)).split(" ] ").at(0);
    QString oldRole = item->text().split(" ] ~ ").at(1), newRole = "NEW";
    if (oldRole == "NEW") newRole = "VIEWER";
    else if (oldRole == "VIEWER") newRole = "EDITOR";
    else if (oldRole == "EDITOR") newRole = "ADMIN";
    if (myDB->updateLine("myclass_users", QStringList() << "role", QStringList() << newRole, "username", username)){
        item->setText(item->text().replace(oldRole, newRole));
        QMessageBox::information(this, tr("Information | SmartClass"), tr("The user ") + username + tr(" has been granted ") + newRole + tr(" permissions."), QMessageBox::Ok);
    }
    else QMessageBox::information(this, tr("Information | SmartClass"), tr("An error has ocurred during the attempt of user permission upgrade operation. Try again later (after restarting SmartClass)."), QMessageBox::Ok);
}

void frmMain::revokeUserPermissions(){
    int currentRow = ui->lwUsers->currentRow();
    if (currentRow < 0){
        QMessageBox::warning(this, tr("Information | SmartClass"), tr("You have not selected/specified any user to revoke its permissions."), QMessageBox::Ok);
        return;
    }
    QListWidgetItem* item = ui->lwUsers->item(currentRow);

    if (item->text().contains(currentUser)){
        QMessageBox::information(this, tr("Information | SmartClass"), tr("Due to safety purposes, you are not able to downgrade your own permissions. If you really need to change your permissions, please, ask to another administrator do it."), QMessageBox::Ok);
        return;
    }

    if (item->text().contains("NEW")){
        QMessageBox::information(this, tr("Information | SmartClass"), tr("This user already have the minimal level of permission. Operation aborted."), QMessageBox::Ok);
        return;
    }

    QString username = QString(item->text().split(" [ ").at(1)).split(" ] ").at(0);
    QString oldRole = item->text().split(" ] ~ ").at(1), newRole = "NEW";
    if (oldRole == "ADMIN") newRole = "EDITOR";
    else if (oldRole == "EDITOR") newRole = "VIEWER";
    else if (oldRole == "VIEWER") newRole = "NEW";
    if (myDB->updateLine("myclass_users", QStringList() << "role", QStringList() << newRole, "username", username)){
        item->setText(item->text().replace(oldRole, newRole));
        QMessageBox::information(this, tr("Information | SmartClass"), tr("The user ") + username + tr(" has been granted ") + newRole + tr(" permissions."), QMessageBox::Ok);
    }
    else QMessageBox::information(this, tr("Information | SmartClass"), tr("An error has ocurred during the attempt of user permission downgrade operation. Try again later (after restarting SmartClass)."), QMessageBox::Ok);
}

void frmMain::removeUser(){
    int currentRow = ui->lwUsers->currentRow();if (currentRow < 0){
        QMessageBox::warning(this, tr("Information | SmartClass"), tr("You have not selected/specified any user to be removed."), QMessageBox::Ok);
        return;
    }
    QListWidgetItem* item = ui->lwUsers->item(currentRow);

    if (item->text().contains(currentUser)){
        QMessageBox::information(this, tr("Information | SmartClass"), tr("Due to safety purposes, you are not able to remove yourself. If you really need to remove your account, please, ask to another administrator do it."), QMessageBox::Ok);
        return;
    }

    QString username = QString(item->text().split(" [ ").at(1)).split(" ] ").at(0);
    if (myDB->removeLine("myclass_users", "username", username)){
        QMessageBox::information(this, tr("Information | SmartClass"), tr("The user ") + username + tr(" has been removed successfully."), QMessageBox::Ok);
        delete item;
        item = NULL;
        ui->lwUsers->setCurrentRow(currentRow == 0 ? 0 : (currentRow - 1));
    }
    else QMessageBox::information(this, tr("Information | SmartClass"), tr("An error has ocurred during the attempt of user removal operation. Try again later (after restarting SmartClass)."), QMessageBox::Ok);
}

void frmMain::setUIToRole(){
    bool enable = false;
    if (sessionRole == "ADMIN") enable = true;

    ui->lwUsers->setEnabled(enable);
    ui->btUpgradeRole->setEnabled(enable);
    ui->btRevokeRole->setEnabled(enable);
    ui->btRemoveUser->setEnabled(enable);
    ui->btSettings->setEnabled(enable);
    ui->btGetReceipt->setEnabled(enable);

    ui->btImportDB->setEnabled(enable);
    ui->btRestoreDB->setEnabled(enable);
    ui->btExportDB->setEnabled(enable);
    ui->btBackUpDB->setEnabled(enable);
    ui->btResetDB->setEnabled(enable);
    ui->btUninstall->setEnabled(enable);

    if (sessionRole == "EDITOR") enable = true;

    ui->btAddStudent->setEnabled(enable);
    ui->btRemoveStudent->setEnabled(enable);
    ui->btAddCourse->setEnabled(enable);
    ui->btRemoveCourse->setEnabled(enable);

    if (sessionRole == "VIEWER") enable = true;

    ui->btUpdateStudent->setEnabled(enable);
    ui->btUpdateCourse->setEnabled(enable);

    ui->btPrintStudentForms->setEnabled(enable);

    if (enable) return;

    QMessageBox::information(this, tr("Information | SmartClass"), tr("Seems that you do not have enough privileges to acess any data here. Plase, contact the administrator in order to upgrade your privileges."), QMessageBox::Ok);
}

void frmMain::getStudents(){
    QStringList* students = myDB->retrieveAll("myclass_students", QStringList() << "student" << "school" << "observations");
    QStringList* parents = myDB->retrieveAll("myclass_parents", QStringList() << "parent" << "students");
    if (parents == NULL || students == NULL) return;
    int sSize = myDB->rowsCount("myclass_students");
    int pSize = myDB->rowsCount("myclass_parents");

    ui->tableStudents->setSortingEnabled(false);
    for (int i = 0; i < sSize; ++i){
        QStringList studentTempData = QStringList() << students[i].at(0);
        for (int k = 0; k < pSize; ++k){
            if (QString(parents[k].at(1)).contains(students[i].at(0))){
                studentTempData << parents[k].at(0);
                break;
            }
        }
        studentTempData << students[i].at(1) << students[i].at(2);

        ui->tableStudents->insertRow(ui->tableStudents->rowCount());

        ui->tableStudents->setItem(i, 0, new QTableWidgetItem(studentTempData.at(0)));
        ui->tableStudents->setItem(i, 1, new QTableWidgetItem(studentTempData.at(1)));
        ui->tableStudents->setItem(i, 2, new QTableWidgetItem(studentTempData.at(2)));
        ui->tableStudents->setItem(i, 3, new QTableWidgetItem(studentTempData.at(3)));
    }
    ui->tableStudents->setSortingEnabled(true);

    delete[] students;
    delete[] parents;
}

void frmMain::getCourses(){
    QStringList* courses = myDB->retrieveAll("myclass_courses", QStringList() << "course" << "class" << "teacher" << "beginningDate" << "dayNTime");
    if (courses == NULL) return;
    int cSize = myDB->rowsCount("myclass_courses");

    ui->tableCourses->setSortingEnabled(false);
    for (int i = 0; i < cSize; ++i){
        ui->tableCourses->insertRow(ui->tableCourses->rowCount());

        ui->tableCourses->setItem(i, 0, new QTableWidgetItem(courses[i].at(0)));
        ui->tableCourses->setItem(i, 1, new QTableWidgetItem(courses[i].at(1)));
        ui->tableCourses->setItem(i, 2, new QTableWidgetItem(courses[i].at(2)));
        ui->tableCourses->setItem(i, 3, new QTableWidgetItem(courses[i].at(3)));
        ui->tableCourses->setItem(i, 4, new QTableWidgetItem(courses[i].at(4)));
    }
    ui->tableCourses->setSortingEnabled(true);

    delete[] courses;
}

void frmMain::receiveNewStudentData(const QStringList &data){
    int oldRCount = ui->tableStudents->rowCount();
    ui->tableStudents->insertRow(oldRCount);

    ui->tableStudents->setItem(oldRCount, 0, new QTableWidgetItem(data.at(0)));
    ui->tableStudents->setItem(oldRCount, 1, new QTableWidgetItem(data.at(1)));
    ui->tableStudents->setItem(oldRCount, 2, new QTableWidgetItem(data.at(2)));
    ui->tableStudents->setItem(oldRCount, 3, new QTableWidgetItem(data.at(3)));
}

void frmMain::receiveStudentUpdatedData(const QStringList &data, const QString &oldName){
    int rowCount = ui->tableStudents->rowCount();
    for (int i = 0; i < rowCount; ++i){
        if (ui->tableStudents->item(i, 0)->text() == oldName){
            ui->tableStudents->item(i, 0)->setText(data.at(0));
            ui->tableStudents->item(i, 1)->setText(data.at(1));
            ui->tableStudents->item(i, 2)->setText(data.at(2));
            ui->tableStudents->item(i, 3)->setText(data.at(3));
        }
    }
}

void frmMain::receiveNewCourseData(const QStringList &data){
    int oldRCount = ui->tableCourses->rowCount();
    ui->tableCourses->insertRow(oldRCount);

    ui->tableCourses->setItem(oldRCount, 0, new QTableWidgetItem(data.at(0)));
    ui->tableCourses->setItem(oldRCount, 1, new QTableWidgetItem(data.at(1)));
    ui->tableCourses->setItem(oldRCount, 2, new QTableWidgetItem(data.at(2)));
    ui->tableCourses->setItem(oldRCount, 3, new QTableWidgetItem(data.at(3)));
    ui->tableCourses->setItem(oldRCount, 4, new QTableWidgetItem(data.at(4)));
}

void frmMain::receiveCourseUpdatedData(const QStringList &data, const QString &oldCourse){
    int rowCount = ui->tableCourses->rowCount();
    for (int i = 0; i < rowCount; ++i){
        if (ui->tableCourses->item(i, 0)->text() == oldCourse){
            ui->tableCourses->item(i, 0)->setText(data.at(0));
            ui->tableCourses->item(i, 1)->setText(data.at(1));
            ui->tableCourses->item(i, 2)->setText(data.at(2));
            ui->tableCourses->item(i, 3)->setText(data.at(3));
            ui->tableCourses->item(i, 4)->setText(data.at(4));
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
    QMessageBox::information(this, tr("Search results | SmartClass"), tr("The search has reached the top of the table without any match.\nPlease, try again with different terms or filters."), QMessageBox::Ok);
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
    QMessageBox::information(this, tr("Search results | SmartClass"), tr("The search has reached the bottom of the table without any match.\nPlease, try again with different terms or filters."), QMessageBox::Ok);
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

    const QString STUDENT_NAME = ui->tableStudents->item(ui->tableStudents->currentRow(), 0)->text();
    const QString PARENT_NAME = ui->tableStudents->item(ui->tableStudents->currentRow(), 1)->text();

    myDB->removeLine("myclass_students", "student", STUDENT_NAME);
    myDB->removeLine("myclass_simages", "student", STUDENT_NAME);
    QStringList selectedParentData = myDB->retrieveLine("myclass_parents", "parent", PARENT_NAME);
    QStringList studentsFromParent = QString(selectedParentData.at(1)).split("|", QString::SkipEmptyParts);

    if (studentsFromParent.length() <= 1){
        myDB->removeLine("myclass_parents", "parent", PARENT_NAME);
        myDB->removeLine("myclass_pimages", "parent", PARENT_NAME);
    }
    else {
        studentsFromParent.removeOne(STUDENT_NAME);
        myDB->updateLine("myclass_parents", QStringList() << "students", QStringList() << studentsFromParent.join("|"),
                        "parent", PARENT_NAME);
    }

    QStringList courseColumns;
    courseColumns << "course" << "teacher" << "shortDescription" << "longDescription" << "class"
                    << "dayNTime" << "beginningDate" << "endDate" << "price" << "students";
    QStringList currentCourseData = myDB->retrieveLine("myclass_courses", "students", "%" + STUDENT_NAME + "%", "LIKE");
    while (!currentCourseData.isEmpty()){
        QStringList studentsInCourse = QString(currentCourseData.at(9)).split("|", QString::SkipEmptyParts);
        studentsInCourse.removeOne(STUDENT_NAME);
        myDB->updateLine("myclass_courses", QStringList() << "students", QStringList() << studentsInCourse.join("|"),
                        courseColumns, currentCourseData);

        currentCourseData = myDB->retrieveLine("myclass_courses", "students", "%" + STUDENT_NAME + "%", "LIKE");
    }

    while (myDB->lineExists("myclass_pricing", "student", STUDENT_NAME))
        myDB->removeLine("myclass_pricing", "student", STUDENT_NAME);

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

    int cRow = ui->tableCourses->currentRow();
    QStringList COURSE_DATA = myDB->retrieveLine("myclass_courses", QStringList() << "course" << "class" << "dayNTime" << "beginningDate",
                                                QStringList() << ui->tableCourses->item(cRow, 0)->text()
                                                              << ui->tableCourses->item(cRow, 1)->text()
                                                              << ui->tableCourses->item(cRow, 4)->text()
                                                              << ui->tableCourses->item(cRow, 3)->text());
    myDB->removeLine("myclass_courses", QStringList() << "course" << "class" << "dayNTime" << "beginningDate",
                    QStringList() << ui->tableCourses->item(cRow, 0)->text()
                                  << ui->tableCourses->item(cRow, 1)->text()
                                  << ui->tableCourses->item(cRow, 4)->text()
                                  << ui->tableCourses->item(cRow, 3)->text());
    QString courseSyntesis = COURSE_DATA.at(0) + " [ " + COURSE_DATA.at(4) + " ] - " + COURSE_DATA.at(5)
                    + tr(" * starts on: ") + COURSE_DATA.at(6);

    while (myDB->lineExists("myclass_pricing", QStringList() << "course", QStringList() << courseSyntesis))
        myDB->removeLine("myclass_pricing", QStringList() << "course", QStringList() << courseSyntesis);

    QStringList studentData = myDB->retrieveLine("myclass_students", "courses", "%" + courseSyntesis + "%", "LIKE");
    QStringList studentsTable = QStringList() << "student" << "birthday" << "studentID" << "school" << "observations"
                                              << "experimentalCourse" << "experimentalCourseDate" << "experimentalCourseObservations"
                                              << "courses" << "address";
    while (!studentData.isEmpty()){
        QStringList coursesOfStudent = QString(studentData.at(8)).split("|", QString::SkipEmptyParts);
        coursesOfStudent.removeOne(courseSyntesis);
        myDB->updateLine("myclass_students", QStringList() << "courses", QStringList() << coursesOfStudent.join("|"),
                        studentsTable, studentData);

        studentData = myDB->retrieveLine("myclass_students", "courses", "%" + courseSyntesis + "%", "LIKE");
    }

    ui->tableCourses->removeRow(cRow);
}

void frmMain::restoreDataBase(){
    if (db_SETTINGS.length() != 1){
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
            if (addClass){
                delete addClass;
                addClass = NULL;
            }

            myDB->closeDB();

            QFile::remove(frmLogin::getDBPath());
            QFile::copy(restoreDlg.selectedFiles().at(0), frmLogin::getDBPath());

            QMessageBox::question(this, tr("Information | SmartClass"), tr("This application is about to restart!"),
                                              QMessageBox::Ok);
            QDesktopServices::openUrl(QUrl(QCoreApplication::applicationFilePath()));
            qApp->quit();
        }
    }
}

void frmMain::backupDataBase(){
    if (db_SETTINGS.length() != 1){
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
    if (addClass){
        delete addClass;
        addClass = NULL;
    }

    myDB->closeDB();

    QFileDialog saveDialog;
    saveDialog.setWindowTitle(tr("Save database file | SmartClass"));
    saveDialog.setNameFilters(QStringList() << "SQLite database file (*.db *.sqlite3)");
    saveDialog.setDefaultSuffix("db");
    saveDialog.setAcceptMode(QFileDialog::AcceptOpen);
    saveDialog.setDirectory(QDir::homePath());

    if (saveDialog.exec())
        QFile::copy(frmLogin::getDBPath(), saveDialog.selectedFiles().at(0));

    if (!myDB->openDB())
        QMessageBox::critical(NULL, tr("Critical error | SmartClass"),
                              tr("The connection with the database could not be restored."
                                 "\nPlease, restart the program in order to have access its functionalities."),
                              QMessageBox::Ok, QMessageBox::NoButton);
}

void frmMain::removeDataBase(){
    if  (deleteDBStatus == 0){
        if (QMessageBox::question(this, tr("Delete database confirmation | SmartClass"),
                                  tr("Would you like to reset the database?"
                                     "\nThis is going to erase all the contents in your database (even your login)."
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
        if (db_SETTINGS.length() == 1){
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
            if (addClass){
                delete addClass;
                addClass = NULL;
            }

            myDB->dropTable("myclass_users");
            myDB->dropTable("myclass_students");
            myDB->dropTable("myclass_parents");
            myDB->dropTable("myclass_courses");
            myDB->dropTable("myclass_simages");
            myDB->dropTable("myclass_pimages");
            myDB->dropTable("myclass_pricing");
            myDB->closeDB();

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
            if (addClass){
                delete addClass;
                addClass = NULL;
            }

            myDB->closeDB();

            QFile::remove(frmLogin::getDBPath());
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
                         "Clear Settings : Clear the initialization/standard settings and restart the application as a clean installation (Note that this option has no effect over the database).\n"
                         "Uninstall : Performs a clean uninstall by removing any settings and files which may rest after the uninstallation process (If you are using SQLite the database may be removed during the process of uninstallation. Do not forget to make a backup before uninstalling!).\n"
                         "Cancel : Cancel the current operation."));
    uninstall.setStandardButtons(QMessageBox::Yes | QMessageBox::YesToAll | QMessageBox::Cancel);
    uninstall.setButtonText(QMessageBox::Yes, tr("Clear Settings"));
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
    QDesktopServices::openUrl(QUrl("./unins000.exe"));
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
    QMainWindow::changeEvent(event);
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
    if (addClass){
        delete addClass;
        addClass = NULL;
    }

    myDB->closeDB();
    QString filename = backupPath + QDir::separator() +
            QDateTime::currentDateTime().toString("yyyy_MM_dd-HH_mm") + tr("_BackUp_SmartClass_DB.db");
    QFile::copy(frmLogin::getDBPath(), filename);

    if (!myDB->openDB())
        QMessageBox::critical(NULL, tr("Critical error | SmartClass"),
                              tr("The connection with the database could not be restored after the backup."
                                 "\nPlease, restart the program in order to have access it's functionalities."),
                              QMessageBox::Ok, QMessageBox::NoButton);
}

/*
 * GUI Functions (don't change, unless necessary)
 */

void frmMain::mousePressEvent(QMouseEvent *event)
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

void frmMain::undefMouseMoveEvent(QObject* object, QMouseEvent* event){
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

void frmMain::mouseReleaseEvent(QMouseEvent *event){
    locked = LockMoveType::None;
    event->accept();
}

bool frmMain::eventFilter(QObject* object, QEvent* event)
{
    if(event->type() == QEvent::MouseMove)
        undefMouseMoveEvent(object, static_cast<QMouseEvent*>(event));
    else if (event->type() == QEvent::MouseButtonPress && object->objectName() == "titleBar"){
        mousePressEvent(static_cast<QMouseEvent*>(event));
    }
    return false;
}
