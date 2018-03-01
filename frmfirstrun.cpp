#include "frmfirstrun.h"
#include "ui_frmfirstrun.h"

frmFirstRun::frmFirstRun(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::frmFirstRun),
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

    ui->titleBar->setMaximizeButtonEnabled(false);

    this->setGeometry(QStyle::alignedRect(Qt::LeftToRight, Qt::AlignCenter,
            this->size(), qApp->desktop()->availableGeometry()));

    /*
     *  End of GUI implementation
     */

    allowed = false;
    canChangeLang = true;

    currentLangIndex = 0;
    ui->grpMySQLSettings->setEnabled(false);

    connect(ui->btSearchCompanyLogo, SIGNAL(clicked(bool)), this, SLOT(selectCompanyLogo()));
    connect(ui->btRemoveLogo, SIGNAL(clicked(bool)), ui->edtCompanyLogoPath, SLOT(clear()));

    connect(ui->btSaveData, SIGNAL(clicked(bool)), this, SLOT(saveDBSettings()));
    connect(ui->rbMySQL, SIGNAL(toggled(bool)), ui->grpMySQLSettings, SLOT(setEnabled(bool)));

    connect(ui->cbLanguage, SIGNAL(currentIndexChanged(int)), this, SLOT(changeLanguage(int)));

    langPath = QApplication::applicationDirPath().append("/lang/");

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
    confirmation.setText(tr("You are going to quit without save the database settings. This may corrupt the data. Do you want to proceed?"));
    confirmation.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
    confirmation.setIcon(QMessageBox::Critical);

    if (confirmation.exec() == QMessageBox::Yes) qApp->quit();
    else event->ignore();
}

void frmFirstRun::saveDBSettings(){
    if (db_manager){
        delete db_manager;
        db_manager = NULL;
    }

    QStringList dbData;

    if (ui->rbSQLite->isChecked()){
        dbData << getDBPath()
               << ui->edtDatabasePrefix->text();
        db_manager = new DBManager(dbData, DBManager::getUniqueConnectionName("firstRun"));
    }
    else{
        dbData << ui->edtHost->text() << ui->edtDatabase->text()
               << QString::number(ui->edtPortNumber->value()) << ui->edtUsername->text()
               << ui->edtPassword->text()
               << ui->edtDatabasePrefix->text();
        db_manager = new DBManager(dbData, DBManager::getUniqueConnectionName("firstRun"), "MYSQL");
    }

    if (!db_manager->openDB()){
        QMessageBox::critical(this, tr("Error | SmartClass"), tr("An error has occurred while we tried to connect to the database. Please, check the input and try again."), QMessageBox::Ok, QMessageBox::NoButton);
        delete db_manager;
        db_manager = NULL;
        return;
    }

    db_manager->closeDB();
    delete db_manager;
    db_manager = NULL;

    QSettings settings("Nintersoft", "SmartClass");
    settings.beginGroup("company info");
    settings.setValue("name", ui->edtCompanyName->text().isEmpty() ? tr("Nintersoft Team") : ui->edtCompanyName->text());
    settings.endGroup();

    settings.beginGroup("language options");
    settings.setValue("current language index", currentLangIndex);
    settings.endGroup();

    QString logoPath  = QDir::homePath() + ((QSysInfo::windowsVersion() != QSysInfo::WV_None) ?
                "/AppData/Roaming/Nintersoft/SmartClass/images/" :
                "/.Nintersoft/SmartClass/images/");
    if (!QDir(logoPath).exists()) QDir(logoPath).mkpath(logoPath);
    logoPath += "Logo.png";

    if (QFile::exists(ui->edtCompanyLogoPath->text()) && !ui->edtCompanyLogoPath->text().isEmpty()){
        QFile newFile(logoPath);
        if (newFile.exists()) newFile.remove();
        QPixmap openFile(ui->edtCompanyLogoPath->text());
        newFile.open(QIODevice::WriteOnly);
        openFile.save(&newFile, "PNG");
        newFile.close();
    }

    allowed = true;
    sendData(dbData, currentLangIndex == 0 ? "en" : "pt");
}

void frmFirstRun::selectCompanyLogo(){
    QFileDialog selectLogoDialog;
    selectLogoDialog.setWindowTitle(tr("Select company logo | SmartClass"));
    selectLogoDialog.setAcceptMode(QFileDialog::AcceptOpen);
    selectLogoDialog.setNameFilters(QStringList() << tr("Image files (*.png *.jpeg *.jpg)") << tr("All files (*.*)"));
    selectLogoDialog.setDirectory(QDir::homePath());
    if (selectLogoDialog.exec()) ui->edtCompanyLogoPath->setText(selectLogoDialog.selectedFiles().at(0));
}

void frmFirstRun::changeLanguage(int index){
    if (index == currentLangIndex) return;
    if (!canChangeLang){
        ui->cbLanguage->setCurrentIndex(currentLangIndex);
        return;
    }

    canChangeLang = false;
    currentLangIndex = index;
    if (index == 0) readLanguage("en");
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
    QMainWindow::changeEvent(event);
}

/*
 * GUI Functions (don't change, unless necessary)
 */

void frmFirstRun::mousePressEvent(QMouseEvent *event)
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

void frmFirstRun::undefMouseMoveEvent(QObject* object, QMouseEvent* event){
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

void frmFirstRun::mouseReleaseEvent(QMouseEvent *event){
    locked = LockMoveType::None;
    event->accept();
}

bool frmFirstRun::eventFilter(QObject* object, QEvent* event)
{
    if(event->type() == QEvent::MouseMove)
        undefMouseMoveEvent(object, static_cast<QMouseEvent*>(event));
    else if (event->type() == QEvent::MouseButtonPress && object->objectName() == "titleBar"){
        mousePressEvent(static_cast<QMouseEvent*>(event));
    }
    return false;
}

const QString frmFirstRun::getDBPath(){
    QString tempDir = QDir::homePath();
    if (QSysInfo::windowsVersion() != QSysInfo::WV_None)
        tempDir += "/AppData/Roaming/Nintersoft/SmartClass/";
    else tempDir += "/.Nintersoft/SmartClass/";

    QString dataDir = tempDir + "data/";
    if (!QDir(dataDir).exists()) QDir(dataDir).mkpath(dataDir);

    return tempDir + "data/classInfo.db";
}
