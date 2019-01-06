#ifndef FRMFIRSTRUN_H
#define FRMFIRSTRUN_H

#include <QDesktopWidget>
#include <QMessageBox>
#include <QMainWindow>
#include <QMouseEvent>
#include <QCloseEvent>
#include <QTranslator>
#include <QFileDialog>
#include <QSqlQuery>
#include <QDateTime>
#include <QHostInfo>
#include <QSettings>
#include <QVariant>
#include <QSysInfo>
#include <QLocale>
#include <QPixmap>
#include <QEvent>
#include <QPoint>
#include <QFile>
#include <QDir>

#include "smartclassglobal.h"
#include "nmainwindow.h"
#include "dbmanager.h"

namespace Ui {
class frmFirstRun;
}

class frmFirstRun : public NMainWindow
{
    Q_OBJECT

public:
    explicit frmFirstRun(QWidget *parent = 0);
    ~frmFirstRun();

protected:
    void closeEvent(QCloseEvent* event);
    void changeEvent(QEvent* event);

    void readLanguage(const QString &langSlug);

private:
    Ui::frmFirstRun *ui;

    DBManager* db_manager;
    DBManager::DBData db_export_data;

    int currentLangIndex; //English = 0; Portuguese = 1
    qlonglong profileID;

    QString langPath;
    QTranslator translator;
    QTranslator qtTranslator;
    QPixmap cLogo;

    bool allowed, canChangeLang, settingsExists;

    void changeTranslator(QTranslator &transl, const QString &filePath);

private slots:
    void nextStep();
    void previousStep();

    void saveDBSettings();
    void selectCompanyLogo();
    void removeCompanyLogo();
    void changeLanguage(int index);

signals:
    void sendData(const DBManager::DBData &data, const QString &langSlug);
};

#endif // FRMFIRSTRUN_H
