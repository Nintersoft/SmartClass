#include "frmlogin.h"
#include "ui_frmlogin.h"

frmLogin::frmLogin(QWidget *parent, const DBManager::DBData dbData) :
    NMainWindow(parent),
    ui(new Ui::frmLogin)
{
    ui->setupUi(this);

    // Sets the custom Widgets on the parent Class
    // Otherwise, the window resizing feature will not work
    NMainWindow::setCustomWidgets(ui->centralWidget, ui->statusBar);
    this->setMaximizeButtonEnabled(false);

    connect(ui->edtUsernameFLogin, SIGNAL(returnPressed()), this, SLOT(login()));
    connect(ui->edtPasswordFLogin, SIGNAL(returnPressed()), this, SLOT(login()));

    connect(ui->edtUsernameFRecover, SIGNAL(returnPressed()), this, SLOT(askQuestion()));
    connect(ui->edtNewPasswordFRecover, SIGNAL(returnPressed()), this, SLOT(changePassword()));
    connect(ui->edtAnswerFRecover, SIGNAL(returnPressed()), this, SLOT(changePassword()));

    connect(ui->btLoginFRegister, SIGNAL(clicked(bool)), this, SLOT(changeTab()));
    connect(ui->btLoginFRecover, SIGNAL(clicked(bool)), this, SLOT(changeTab()));
    connect(ui->btRegisterFLogin, SIGNAL(clicked(bool)), this, SLOT(changeTab()));
    connect(ui->btRegisterFRecover, SIGNAL(clicked(bool)), this, SLOT(changeTab()));
    connect(ui->btRecoverFLogin, SIGNAL(clicked(bool)), this, SLOT(changeTab()));
    connect(ui->btRecoverFRegister, SIGNAL(clicked(bool)), this, SLOT(changeTab()));

    connect(ui->btLoginFLogin, SIGNAL(clicked(bool)), this, SLOT(login()));
    connect(ui->btRegisterFRegister, SIGNAL(clicked(bool)), this, SLOT(registerUser()));
    connect(ui->btAskFRecover, SIGNAL(clicked(bool)), this, SLOT(askQuestion()));
    connect(ui->btRecoverFRecover, SIGNAL(clicked(bool)), this, SLOT(changePassword()));

    /*
     * End GUI properties
     */

    myDb = new DBManager(dbData, SmartClassGlobal::tablePrefix(),
                            SmartClassGlobal::databaseType(), DBManager::getUniqueConnectionName("login"));

    myDb->openDB();

    QSettings settings("Nintersoft", "SmartClass");

    settings.beginGroup("device settings");
    dID = settings.value("device ID", -1).toLongLong();
    settings.endGroup();

    myDb->createTable(SmartClassGlobal::getTableName(SmartClassGlobal::USERS),
                      SmartClassGlobal::getTableStructure(SmartClassGlobal::USERS));
}

frmLogin::~frmLogin()
{
    delete ui;
    if (myDb->isOpen()) myDb->closeDB();
    delete myDb;
}

void frmLogin::login(){
    QString username = ui->edtUsernameFLogin->text(),
            password = ui->edtPasswordFLogin->text();
    QStringList uTableSchema = SmartClassGlobal::getTableStructure(SmartClassGlobal::USERS);

    if (username.isEmpty() || !myDb->rowExists(SmartClassGlobal::getTableName(SmartClassGlobal::USERS), uTableSchema.at(1), username) || password.isEmpty()){
        QMessageBox::information(this, tr("Error | SmartClass"), tr("Seems that either the username or the password or both are incorrect. Please, check your data and try again."), QMessageBox::Ok);
        return;
    }

    QList<QVariant> userInfo(myDb->retrieveRow(SmartClassGlobal::getTableName(SmartClassGlobal::USERS), uTableSchema.at(1), username));
    QString tempHash = QString(QCryptographicHash::hash(QString(userInfo.at(2).toString() + ui->edtPasswordFLogin->text()).toUtf8(), QCryptographicHash::Sha512).toHex());
    if (tempHash == userInfo.at(3).toString()){
        myDb->updateRow(SmartClassGlobal::getTableName(SmartClassGlobal::ACTIVECONNECTIONS),
                        SmartClassGlobal::getTableStructure(SmartClassGlobal::ACTIVECONNECTIONS).at(0),
                        dID,
                        QStringList() << SmartClassGlobal::getTableStructure(SmartClassGlobal::ACTIVECONNECTIONS).at(4),
                        QList<QVariant>() << QDate::currentDate());
        emit dataReady(userInfo);
    }
    else QMessageBox::information(this, tr("Error | SmartClass"), tr("Seems that either the username or the password or both are incorrect. Please, check your data and try again."), QMessageBox::Ok);
}

void frmLogin::registerUser(){
    QList<QVariant> userInfo;
    userInfo << ui->edtUsernameFRegister->text() << ui->edtCompleteNameFRegister->text()
             << ui->edtPasswordFRegister->text() << ui->edtPasswordConfirmationFRegister->text()
             << ui->edtQuestionFRegister->text() << ui->edtAnswerFRegister->text();

    for (int i = 0; i < userInfo.length(); ++i)
        if (userInfo.at(i).toString().isEmpty()){
            QMessageBox::warning(this, tr("Error | SmartClass"), tr("None of the fields should be empty. Please, check if there is any empty field and try again."), QMessageBox::Ok);
            return;
        }

    if (myDb->rowExists(SmartClassGlobal::getTableName(SmartClassGlobal::USERS), "username", userInfo.at(0))){
        QMessageBox::warning(this, tr("Error | SmartClass"), tr("Sorry, but unfortunately this user is already being used. Please, type a different username."), QMessageBox::Ok);
        return;
    }

    int trash = 0;
    if (!isSafePassword(userInfo.at(2).toString(), trash)){
        QMessageBox::warning(this, tr("Error | SmartClass"), tr("Your password seems to be too weak. It must contain at least one special character, one number, one uppercase letter and a lowercase letter."), QMessageBox::Ok);
        return;
    }

    if (userInfo.at(2) != userInfo.at(3).toString()){
        QMessageBox::warning(this, tr("Error | SmartClass"), tr("The password confirmation do not match to the previously typed one."), QMessageBox::Ok);
        return;
    }

    QString salt = randomSalt(25), answerSalt = randomSalt(25);
    while (salt == answerSalt) answerSalt = randomSalt(25);

    QString passwordPSalt = salt + userInfo.at(2).toString();
    QString answerPSalt = answerSalt + userInfo.at(5).toString();
    QList<QVariant> completeUserData;
    completeUserData << userInfo.at(0) << userInfo.at(1)
                     << salt << getHash(passwordPSalt)
                     << userInfo.at(4) << answerSalt
                     << getHash(answerPSalt);
    if (myDb->rowsCount(SmartClassGlobal::getTableName(SmartClassGlobal::USERS)) < 1) completeUserData << (int)SmartClassGlobal::ADMIN;
    else completeUserData << (int)SmartClassGlobal::NEW;

    if (!myDb->insertRow(SmartClassGlobal::getTableName(SmartClassGlobal::USERS),
                         SmartClassGlobal::getTableStructure(SmartClassGlobal::USERS).mid(1),
                         completeUserData)){
        QMessageBox::critical(this, tr("Database connection error | SmartClass"), tr("Unfortunately it was not possible to register this user at the database. Contact the application developer for further assistance."), QMessageBox::Ok);
        return;
    }

    ui->edtUsernameFRegister->clear();
    ui->edtCompleteNameFRegister->clear();
    ui->edtPasswordFRegister->clear();
    ui->edtPasswordConfirmationFRegister->clear();
    ui->edtQuestionFRegister->clear();
    ui->edtAnswerFRegister->clear();

    ui->edtUsernameFLogin->setText(completeUserData.at(0).toString());
    ui->edtPasswordFLogin->clear();
    ui->edtPasswordFLogin->setFocus();
    ui->tabManager->setCurrentIndex(0);

    QMessageBox::information(this, tr("Success | SmartClass"), tr("Your account has been created successfully. Remember: We do not store your passwords, so everytime you forget it, you will have to redefine it."), QMessageBox::Ok);
}

void frmLogin::askQuestion(){
    QString username = ui->edtUsernameFRecover->text();
    if (!myDb->rowExists(SmartClassGlobal::getTableName(SmartClassGlobal::USERS),
                         SmartClassGlobal::getTableStructure(SmartClassGlobal::USERS).at(1),
                         username) || username.isEmpty() || username.isNull()){
        ui->lblQuestionFRecover->setText(tr("There is no question for this user..."));
        ui->lblQuestionLabelFRecover->setEnabled(false);
        ui->lblQuestionFRecover->setEnabled(false);
        ui->edtAnswerFRecover->setEnabled(false);
        ui->lblNewPasswordFRecover->setEnabled(false);
        ui->edtNewPasswordFRecover->setEnabled(false);
        ui->btRecoverFRecover->setEnabled(false);
        QMessageBox::warning(this, tr("Warning | SmartClass"), tr("This user does not exists."), QMessageBox::Ok);
    }
    else {
        ui->lblQuestionFRecover->setText(myDb->retrieveRow(SmartClassGlobal::getTableName(SmartClassGlobal::USERS),
                                                           SmartClassGlobal::getTableStructure(SmartClassGlobal::USERS).at(1),
                                                           username).at(5).toString());
        ui->lblQuestionLabelFRecover->setEnabled(true);
        ui->lblQuestionFRecover->setEnabled(true);
        ui->edtAnswerFRecover->setEnabled(true);
        ui->lblNewPasswordFRecover->setEnabled(true);
        ui->edtNewPasswordFRecover->setEnabled(true);
        ui->btRecoverFRecover->setEnabled(true);
    }
    ui->edtAnswerFRecover->clear();
}

void frmLogin::changePassword(){
    QString username = ui->edtUsernameFRecover->text();
    QList<QVariant> userInfo = myDb->retrieveRow(SmartClassGlobal::getTableName(SmartClassGlobal::USERS),
                                                 SmartClassGlobal::getTableStructure(SmartClassGlobal::USERS).at(1),
                                                 username);
    QString generatedHashAnswer = QString(QCryptographicHash::hash(QString(userInfo.at(6).toString() + ui->edtAnswerFRecover->text()).toUtf8(), QCryptographicHash::Sha512).toHex());
    if (generatedHashAnswer == userInfo.at(7).toString()){
        QString newPassword = ui->edtNewPasswordFRecover->text();
        if (newPassword.isEmpty() || newPassword.isNull()){
            QMessageBox::warning(this, tr("Error | SmartClass"), tr("The new password cannot be empty."), QMessageBox::Ok);
            return;
        }
        int trash = 0;
        if (!isSafePassword(newPassword, trash)){
            QMessageBox::warning(this, tr("Error | SmartClass"), tr("Your password seems to be too weak. It must contain at least one special character, one number, one uppercase letter and a lowercase letter."), QMessageBox::Ok);
            return;
        }
        QString newPasswordSalt = randomSalt(25);
        QString newPasswordHash = QString(QCryptographicHash::hash(QString(newPasswordSalt + newPassword).toUtf8(), QCryptographicHash::Sha512).toHex());
        userInfo.replace(3, newPasswordSalt);
        userInfo.replace(4, newPasswordHash);
        if (!myDb->updateRow(SmartClassGlobal::getTableName(SmartClassGlobal::USERS),
                             SmartClassGlobal::getTableStructure(SmartClassGlobal::USERS).at(1),
                             username,
                             SmartClassGlobal::getTableStructure(SmartClassGlobal::USERS).mid(1),
                             userInfo.mid(1))){
            QMessageBox::critical(this, tr("Database connection error | SmartClass"), tr("Unfortunately it was not possible to update the information about this user in the database. Contact the application developer for further assistance."), QMessageBox::Ok);
            return;
        }
        ui->edtAnswerFRecover->clear();
        ui->edtUsernameFRecover->clear();
        ui->edtNewPasswordFRecover->clear();

        ui->lblQuestionFRecover->setText(tr("There is no question for this user..."));
        ui->lblQuestionLabelFRecover->setEnabled(false);
        ui->lblQuestionFRecover->setEnabled(false);
        ui->edtAnswerFRecover->setEnabled(false);
        ui->lblNewPasswordFRecover->setEnabled(false);
        ui->edtNewPasswordFRecover->setEnabled(false);
        ui->btRecoverFRecover->setEnabled(false);

        ui->edtUsernameFLogin->setText(username);
        ui->edtPasswordFLogin->clear();
        ui->edtPasswordFLogin->setFocus();
        ui->tabManager->setCurrentIndex(0);

        QMessageBox::information(this, tr("Success | SmartClass"), tr("Your password has been redefined successfully. Remember: We do not store your passwords, so everytime you forget it, you will have to redefine it."), QMessageBox::Ok);
    }
    else QMessageBox::warning(this, tr("Error | SmartClass"), tr("Incorrect anwer."), QMessageBox::Ok);
}

void frmLogin::changeTab(){
    QString senderName = sender()->objectName();
    if (senderName.contains("btLogin")) ui->tabManager->setCurrentIndex(0);
    else if (senderName.contains("btRegister")) ui->tabManager->setCurrentIndex(1);
    else ui->tabManager->setCurrentIndex(2);
}
