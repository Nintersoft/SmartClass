#include "frmabout.h"
#include "ui_frmabout.h"

frmAbout::frmAbout(QWidget *parent) :
    NMainWindow(parent),
    ui(new Ui::frmAbout)
 {
    ui->setupUi(this);

    // Sets the custom Widgets on the parent Class
    // Otherwise, the window resizing feature will not work
    NMainWindow::setCustomWidgets(ui->centralWidget, ui->statusBar);
    this->setMaximizeButtonEnabled(false);

    /*
     * End of GUI operations
     */

    ui->pbUpgrade->setVisible(false);
    ui->lblProgStep->setVisible(false);

    connect(ui->btOpenNSWebsite, SIGNAL(clicked(bool)), this, SLOT(openNSWebsite()));
    connect(ui->btShowLicence, SIGNAL(clicked(bool)), this, SLOT(showLicences()));

    isCommercial = true;

    QList< QVariantList > settingsRow = DBManager::getInstance()->retrieveAll(SmartClassGlobal::getTableName(SmartClassGlobal::SETTINGS),
                                                                              SmartClassGlobal::getTableAliases(SmartClassGlobal::SETTINGS));
    if (settingsRow.size()){
        QPixmap logo(DBManager::variantToPixmap(settingsRow.at(0).at(2)));
        if (!logo.isNull())
            ui->lblCompanyLogo->setPixmap(logo.scaled(100, 100, Qt::KeepAspectRatio));

        ui->lblLicencedTo->setText(settingsRow.at(0).at(0).isNull() ?
                                     tr("Nintersoft Team") :
                                     settingsRow.at(0).at(0).toString());
    }


#ifdef _WIN32
    connect(ui->btUpgrade, SIGNAL(clicked(bool)), this, SLOT(upgradeProgram()));
    upgradeStep = 0;
    upgradeVersion = 4;

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
        chooseLicence.setStandardButtons(QMessageBox::Ok | QMessageBox::Yes | QMessageBox::Open | QMessageBox::Cancel);
        chooseLicence.setButtonText(QMessageBox::Ok, tr("Nintersoft OSL"));
        chooseLicence.setButtonText(QMessageBox::Yes, tr("Commercial"));
        chooseLicence.setButtonText(QMessageBox::Open, tr("Privacy Policy"));
        int execValue = chooseLicence.exec();
        if (execValue == QMessageBox::Ok) QDesktopServices::openUrl(QUrl::fromLocalFile((QCoreApplication::applicationDirPath() + "/Licença de Código Aberto Nintersoft rev1.pdf")));
        else if (execValue == QMessageBox::Yes) QDesktopServices::openUrl(QUrl::fromLocalFile((QCoreApplication::applicationDirPath() + "/Licença de Comercial Nintersoft rev1.pdf")));
        else if (execValue == QMessageBox::Open) QDesktopServices::openUrl(QUrl::fromLocalFile((QCoreApplication::applicationDirPath() + "/Política de privacidade SmartClass.pdf")));
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
                downloadLink += upData[i];

            if (upData.last().toInt() > upgradeVersion) downloadArchive(downloadLink, tempDir, "SmartClass.exe");
            else{
                QMessageBox::information(this, tr("Already updated | SmartClass"), tr("Congratulations, you already have the latest version of SmartClass installed in your machine!"));
                setDefaultStatus();
            }
        }
    }
    else{
        setDefaultStatus();
        QMessageBox::information(this, tr("Upgrade info | SmartClass"), tr("The upgrade process is going to start as soon as you quit the program."
                                                                           "\nYou can continue working, since the installer will be launched right after you close SmartClass."), QMessageBox::Ok);
        ui->btUpgrade->setEnabled(false);
        emit upgradeAvailable();
    }
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
