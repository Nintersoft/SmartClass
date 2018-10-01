#ifndef FRMABOUT_H
#define FRMABOUT_H

#include <QNetworkAccessManager>
#include <QDesktopServices>
#include <QDesktopWidget>
#include <QNetworkReply>
#include <QMainWindow>
#include <QMessageBox>
#include <QFileInfo>
#include <QSettings>
#include <QPixmap>
#include <QFile>
#include <QDir>
#include <QUrl>

#include "nmainwindow.h"

namespace Ui {
class frmAbout;
}

class frmAbout : public NMainWindow
{
    Q_OBJECT

public:
    explicit frmAbout(QWidget *parent = 0);
    ~frmAbout();

protected slots:
#ifdef _WIN32
    void upgradeProgram();

    /*
    Method taken from StackOverflow [Endauriel]
    URL:http://stackoverflow.com/questions/14989135/qt-downloading-a-file-doesnt-work
    */
    bool downloadArchive(QString archiveUrl, QString saveToPath, QString archiveName);
#endif
    void openNSWebsite();
    void showLicences();

private:
    Ui::frmAbout *ui;
    const QString STUDENT_NAME;

    int isCommercial;
#ifdef _WIN32
    QNetworkAccessManager manager;
    QFile *file;
    QNetworkReply *reply;

    int upgradeStep, upgradeVersion;
    QString tempDir;

    /*
    Slots taken from StackOverflow [Endauriel]
    URL:http://stackoverflow.com/questions/14989135/qt-downloading-a-file-doesnt-work
    */
private slots:
    void downloadReadyRead();
    void downloadProgress(qint64 bytesReceived, qint64 bytesTotal);
    void downloadFinished();

    void setDefaultStatus();

signals:
    void upgradeAvailable();
#endif
};

#endif // FRMABOUT_H
