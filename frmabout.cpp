#include "frmabout.h"
#include "ui_frmabout.h"

frmAbout::frmAbout(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::frmAbout),
    RESIZE_LIMIT(2)
 {
    ui->setupUi(this);

    this->setWindowFlags(Qt::Widget | Qt::FramelessWindowHint);
    this->centralWidget()->installEventFilter(this);
    ui->titleBar->installEventFilter(this);
    ui->statusBar->installEventFilter(this);

    this->centralWidget()->setMouseTracking(true);
    ui->titleBar->setMouseTracking(true);
    ui->statusBar->setMouseTracking(true);

    this->setWindowTitle(tr("About | SmartClass"));
    this->locked = LockMoveType::None;

    ui->titleBar->setMaximizeButtonEnabled(false);

    this->setGeometry(QStyle::alignedRect(Qt::LeftToRight, Qt::AlignCenter,
            this->size(), qApp->desktop()->availableGeometry()));

    /*
     * End of GUI operations
     */

    ui->pbUpgrade->setVisible(false);
    ui->lblProgStep->setVisible(false);

    connect(ui->btOpenNSWebsite, SIGNAL(clicked(bool)), this, SLOT(openNSWebsite()));
    connect(ui->btShowLicence, SIGNAL(clicked(bool)), this, SLOT(showLicences()));

    isCommercial = true;

    QString logoPath  = QDir::homePath() + ((QSysInfo::windowsVersion() != QSysInfo::WV_None) ?
                "/AppData/Roaming/Nintersoft/SmartClass/images/Logo.png" :
                "/.Nintersoft/SmartClass/images/Logo.png");
    if (QFile::exists(logoPath)) ui->lblCompanyLogo->setPixmap(QPixmap(logoPath, "PNG").scaled(100, 100, Qt::KeepAspectRatio));

    QSettings settings("Nintersoft", "SmartClass");
    if (settings.childGroups().contains("company info", Qt::CaseInsensitive)){
        settings.beginGroup("company info");
        ui->lblLicencedTo->setText(settings.value("name", tr("Nintersoft Team")).toString());
        settings.endGroup();
    }


#ifdef _WIN32
    connect(ui->btUpgrade, SIGNAL(clicked(bool)), this, SLOT(upgradeProgram()));
    upgradeStep = 0;
    upgradeVersion = 3;

    tempDir = QDir::homePath() + "/AppData/Roaming/Nintersoft/SmartClass/Downloads/";

    QDir tempDirA(tempDir);
    if (tempDirA.exists(tempDir)) tempDirA.removeRecursively();
    tempDirA.mkpath(tempDir);
#else
    ui->btUpgrade->setEnabled(false);
#endif
}

frmAbout::~frmAbout()
{
    delete ui;
}

void frmAbout::openNSWebsite(){
    QDesktopServices::openUrl(QUrl(tr("http://www.nintersoft.com/en/")));
}


void frmAbout::showLicences(){
    if (isCommercial){
        QMessageBox chooseLicence;
        chooseLicence.setWindowTitle("Choose licence | SmartClass");
        chooseLicence.setText("Please, choose the licence to be displayed (You must have a PDF reader in order to open the licence file).");
        chooseLicence.setStandardButtons(QMessageBox::Ok | QMessageBox::Yes | QMessageBox::Cancel);
        chooseLicence.setButtonText(QMessageBox::Ok, tr("Nintersoft OSL"));
        chooseLicence.setButtonText(QMessageBox::Yes, tr("Commercial"));
        int execValue = chooseLicence.exec();
        if (execValue == QMessageBox::Ok) QDesktopServices::openUrl(QUrl::fromLocalFile((QCoreApplication::applicationDirPath() + "/Licença de Código Aberto Nintersoft rev1.pdf")));
        else if (execValue == QMessageBox::Yes) QDesktopServices::openUrl(QUrl::fromLocalFile((QCoreApplication::applicationDirPath() + "/Licença de Comercial Nintersoft rev1.pdf")));
    }
    else QDesktopServices::openUrl(QUrl::fromLocalFile((QCoreApplication::applicationDirPath() + "/Licença de Código Aberto Nintersoft rev1.pdf")));
}

#ifdef _WIN32
void frmAbout::upgradeProgram(){
    ui->lblProgStep->setText(tr("Beginning upgrade process..."));
    ui->lblProgStep->setVisible(true);
    ui->pbUpgrade->setVisible(true);

    upgradeStep = 0;
    downloadArchive(QSysInfo::buildCpuArchitecture().contains("64") ?
                        "http://download.nintersoft.com/smartclass/at-sc-x64.txt" :
                        "http://download.nintersoft.com/smartclass/at-sc.txt", tempDir, "at-sc.txt");
    ui->btUpgrade->setEnabled(false);
}

void frmAbout::downloadReadyRead(){
    if (file) file->write(reply->readAll());
}

void frmAbout::downloadProgress(qint64 bytesReceived, qint64 bytesTotal){
    ui->pbUpgrade->setMaximum(bytesTotal);
    ui->pbUpgrade->setValue(bytesReceived);
}

void frmAbout::downloadFinished(){
    downloadReadyRead();
    file->flush();
    file->close();

    bool error = false;

    if (reply->error()){
        error = true;
        QMessageBox::information(this, tr("Upgrade failed | SmartClass"), tr("Failed to download file: %1").arg(reply->errorString()));
    }

    reply->deleteLater();
    reply = NULL;
    delete file;
    file = NULL;

    if (error) setDefaultStatus();
    else if (upgradeStep == 0){
        upgradeStep++;
        QFile updateInformation(tempDir + QDir::separator() + "at-sc.txt");
        QStringList versions;

        if (!updateInformation.open(QIODevice::ReadOnly)){
            QMessageBox::information(this, tr("Upgrade failed | SmartClass"), tr("Error while opening the file which contains the repository information.").arg(reply->errorString()));
            setDefaultStatus();
        }
        else{
            while (!updateInformation.atEnd()) versions.append(updateInformation.readLine());
            updateInformation.close();

            QString infoLine = versions.at(0), downloadLink = "";
            QStringList upData = infoLine.split("=");

            for (int i = 1; i < (upData.length() - 1); ++i)
                downloadLink += infoLine[i];

            if (upData.last().toInt() > upgradeVersion) downloadArchive(downloadLink, tempDir, "SmartClass.exe");
            else{
                QMessageBox::information(this, tr("Already updated | SmartClass"), tr("Congratulations, you already have the latest version of SmartClass installed in your machine!"));
                setDefaultStatus();
            }
        }
    }
    else emit upgradeAvailable();
}

bool frmAbout::downloadArchive(QString archiveUrl, QString saveToPath, QString archiveName){
    QUrl url(archiveUrl);

    if (archiveName.count() <= 0) return false;

    if(QFile::exists(saveToPath + QDir::separator() + archiveName))
        QFile::remove(saveToPath + QDir::separator() + archiveName);

    file = new QFile(saveToPath + QDir::separator() + archiveName);

    if (upgradeStep == 0){
        if(!file->open(QIODevice::WriteOnly | QIODevice::Text)){
            delete file;
            file = NULL;
            return false;
        }
    }
    else {
        if(!file->open(QIODevice::WriteOnly)){
            delete file;
            file = NULL;
            return false;
        }
    }

    reply = manager.get(QNetworkRequest(url));

    connect(reply, SIGNAL(finished()), this, SLOT(downloadFinished()));
    connect(reply, SIGNAL(readyRead()), this, SLOT(downloadReadyRead()));
    connect(reply, SIGNAL(downloadProgress(qint64,qint64)), this, SLOT(downloadProgress(qint64,qint64)));

    if (upgradeStep == 0) ui->lblProgStep->setText(tr("Updating repository information..."));
    else ui->lblProgStep->setText(tr("Downloading %1...").arg(archiveName));

    return true;
}

void frmAbout::setDefaultStatus(){
    ui->lblProgStep->setText(tr("Upgrade process..."));
    ui->lblProgStep->setVisible(false);
    ui->pbUpgrade->setMaximum(0);
    ui->pbUpgrade->setValue(-1);
    ui->pbUpgrade->setVisible(false);
    ui->btUpgrade->setEnabled(true);
}
#endif

/*
 * GUI Functions (don't change, unless necessary)
 */

void frmAbout::mousePressEvent(QMouseEvent *event)
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

void frmAbout::undefMouseMoveEvent(QObject* object, QMouseEvent* event){
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

void frmAbout::mouseReleaseEvent(QMouseEvent *event){
    locked = LockMoveType::None;
    event->accept();
}

bool frmAbout::eventFilter(QObject* object, QEvent* event)
{
    if(event->type() == QEvent::MouseMove)
        undefMouseMoveEvent(object, static_cast<QMouseEvent*>(event));
    else if (event->type() == QEvent::MouseButtonPress && object->objectName() == "titleBar"){
        mousePressEvent(static_cast<QMouseEvent*>(event));
    }
    return false;
}
