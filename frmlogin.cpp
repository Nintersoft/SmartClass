#include "frmlogin.h"
#include "ui_frmlogin.h"

frmLogin::frmLogin(QWidget *parent, const QStringList dbData) :
    QMainWindow(parent),
    ui(new Ui::frmLogin),
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

    setWindowTitle(tr("Login | SmartClass"));
    locked = LockMoveType::None;

    ui->titleBar->setMaximizeButtonEnabled(false);

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

    this->setGeometry(QStyle::alignedRect(Qt::LeftToRight, Qt::AlignCenter,
            this->size(), qApp->desktop()->availableGeometry()));

    /*
     * End GUI properties
     */

    myDb = new DBManager(dbData, DBManager::getUniqueConnectionName("login"),
                         dbData.length() == 2 ? "SQLITE" : "MYSQL");

    columns << "username" << "name"
            << "salt" << "hash" << "question"
            << "answerSalt" << "hashAnswer" << "role";

    myDb->openDB();
    QStringList newColumns;
    QString id = (dbData.length() == 2) ? "id INTEGER PRIMARY KEY" : "id INTEGER(64) UNSIGNED AUTO_INCREMENT PRIMARY KEY";
    newColumns << id << "username TEXT NOT NULL" << "name TEXT NOT NULL"
                << "salt  TEXT NOT NULL" << "hash TEXT NOT NULL" << "question TEXT"
                << "answerSalt TEXT NOT NULL" << "hashAnswer TEXT NOT NULL" << "role TEXT";
    myDb->createTable("myclass_users", newColumns);
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
    if (username.isEmpty() || !myDb->lineExists("myclass_users", "username", username) || password.isEmpty()){
        QMessageBox::information(this, tr("Error | SmartClass"), tr("Seems that either the username or the password or both are incorrect. Please, check your data and try again."), QMessageBox::Ok);
        return;
    }

    QStringList userInfo(myDb->retrieveLine("myclass_users", "username", username));
    QString tempHash = QString(QCryptographicHash::hash(QString(userInfo.at(2) + ui->edtPasswordFLogin->text()).toUtf8(), QCryptographicHash::Sha512).toHex());
    if (tempHash == userInfo.at(3)) emit dataReady(userInfo);
    else QMessageBox::information(this, tr("Error | SmartClass"), tr("Seems that either the username or the password or both are incorrect. Please, check your data and try again."), QMessageBox::Ok);
}

void frmLogin::registerUser(){
    QStringList userInfo;
    userInfo << ui->edtUsernameFRegister->text() << ui->edtCompleteNameFRegister->text()
             << ui->edtPasswordFRegister->text() << ui->edtPasswordConfirmationFRegister->text()
             << ui->edtQuestionFRegister->text() << ui->edtAnswerFRegister->text();

    for (int i = 0; i < userInfo.length(); ++i)
        if (QString(userInfo.at(i)).isEmpty()){
            QMessageBox::warning(this, tr("Error | SmartClass"), tr("None of the fields should be empty. Please, check if there is any empty field and try again."), QMessageBox::Ok);
            return;
        }

    if (myDb->lineExists("myclass_users", "username", userInfo.at(0))){
        QMessageBox::warning(this, tr("Error | SmartClass"), tr("Sorry, but unfortunately this user is already being used. Please, type a different username."), QMessageBox::Ok);
        return;
    }

    int trash = 0;
    if (!isSafePassword(userInfo.at(2), trash)){
        QMessageBox::warning(this, tr("Error | SmartClass"), tr("Your password seems to be too weak. It must contain at least one special character, one number, one uppercase letter and a lowercase letter."), QMessageBox::Ok);
        return;
    }

    if (userInfo.at(2) != userInfo.at(3)){
        QMessageBox::warning(this, tr("Error | SmartClass"), tr("The password confirmation do not match to the previously typed one."), QMessageBox::Ok);
        return;
    }

    QString salt = randomSalt(25);
    QString passwordPSalt = salt + userInfo.at(2);
    QString answerSalt = randomSalt(25);
    QString answerPSalt = answerSalt + userInfo.at(5);
    QStringList completeUserData;
    completeUserData << userInfo.at(0) << userInfo.at(1)
                     << salt << QString(QCryptographicHash::hash(passwordPSalt.toUtf8(), QCryptographicHash::Sha512).toHex())
                     << userInfo.at(4) << answerSalt
                     << QString(QCryptographicHash::hash(answerPSalt.toUtf8(), QCryptographicHash::Sha512).toHex());
    if (myDb->rowsCount("myclass_users") < 1) completeUserData << "ADMIN";
    else completeUserData << "NEW";

    if (!myDb->addLine("myclass_users", columns, completeUserData)){
        QMessageBox::critical(this, tr("Database connection error | SmartClass"), tr("Unfortunately it was not possible to register this user at the database. Contact the application developer for further assistance."), QMessageBox::Ok);
        return;
    }

    ui->edtUsernameFRegister->clear();
    ui->edtCompleteNameFRegister->clear();
    ui->edtPasswordFRegister->clear();
    ui->edtPasswordConfirmationFRegister->clear();
    ui->edtQuestionFRegister->clear();
    ui->edtAnswerFRegister->clear();

    ui->edtUsernameFLogin->setText(completeUserData.at(0));
    ui->edtPasswordFLogin->clear();
    ui->edtPasswordFLogin->setFocus();
    ui->tabManager->setCurrentIndex(0);

    QMessageBox::information(this, tr("Success | SmartClass"), tr("Your account has been created successfully. Remember: We do not store your passwords, so everytime you forget it, you will have to redefine it."), QMessageBox::Ok);
}

void frmLogin::askQuestion(){
    QString username = ui->edtUsernameFRecover->text();
    if (!myDb->lineExists("myclass_users", "username", username)
            || username.isEmpty() || username.isNull()){
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
        ui->lblQuestionFRecover->setText(myDb->retrieveLine("myclass_users", "username", username).at(4));
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
    QStringList userInfo = myDb->retrieveLine("myclass_users", "username", username);
    QString generatedHashAnswer = QString(QCryptographicHash::hash(QString(userInfo.at(5) + ui->edtAnswerFRecover->text()).toUtf8(), QCryptographicHash::Sha512).toHex());
    if (generatedHashAnswer == userInfo.at(6)){
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
        userInfo.replace(2, newPasswordSalt);
        userInfo.replace(3, newPasswordHash);
        if (!myDb->updateLine("myclass_users", columns, userInfo, "username", username)){
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

/*
 * Safety Functions (don't change, unless necessary)
 */

bool frmLogin::isSafePassword(const QString &pass, int &index){
    bool isSafe = true, safetyItems[4];
    for (unsigned int i = 0; i < sizeof(safetyItems)/sizeof(bool); ++i)
        safetyItems[i] = false;

    for (int i = 0; i < pass.length(); ++i){
        if (pass[i].isNumber()) safetyItems[0] = true;
        else if (!pass[i].isLetterOrNumber()) safetyItems[1] = true;
        else if (pass[i].isLetter() && pass[i].isUpper()) safetyItems[2] = true;
        else if (pass[i].isLetter() && pass[i].isLower()) safetyItems[3] = true;
    }

    for (unsigned int i = 0; i < sizeof(safetyItems)/sizeof(bool); ++i)
        if (!safetyItems[i]){
            index = i;
            isSafe = false;
            break;
        }

    return isSafe;
}

QString frmLogin::randomSalt(int size){
    qsrand((uint)QTime::currentTime().msec());
    QString salt = "";
    for (int i = 0; i < size; ++i){
        unsigned char currentChar = (unsigned char)((qrand() % 223) + 33);
        if (currentChar == ';' || (currentChar >= 127 && currentChar <= 160)){
            i--;
            continue;
        }
        salt += currentChar;
    }
    return salt;
}


/*
 * GUI Functions (don't change, unless necessary)
 */

void frmLogin::mousePressEvent(QMouseEvent *event)
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

void frmLogin::undefMouseMoveEvent(QObject* object, QMouseEvent* event){
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

void frmLogin::mouseReleaseEvent(QMouseEvent *event){
    locked = LockMoveType::None;
    event->accept();
}

bool frmLogin::eventFilter(QObject* object, QEvent* event)
{
    if(event->type() == QEvent::MouseMove)
        undefMouseMoveEvent(object, static_cast<QMouseEvent*>(event));
    else if (event->type() == QEvent::MouseButtonPress && object->objectName() == "titleBar"){
        mousePressEvent(static_cast<QMouseEvent*>(event));
    }
    return false;
}


const QString frmLogin::getDBPath(){
    QString tempDir = QDir::homePath();
    if (QSysInfo::windowsVersion() != QSysInfo::WV_None)
        tempDir += "/AppData/Roaming/Nintersoft/SmartClass/";
    else tempDir += "/.Nintersoft/SmartClass/";

    QString dataDir = tempDir + "data/";
    if (!QDir(dataDir).exists()) QDir(dataDir).mkpath(dataDir);

    return tempDir + "data/classInfo.db";
}
