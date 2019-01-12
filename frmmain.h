#ifndef FRMMAIN_H
#define FRMMAIN_H

#include <QDesktopServices>
#include <QCoreApplication>
#include <QResizeEvent>
#include <QMouseEvent>
#include <QCloseEvent>
#include <QMainWindow>
#include <QTranslator>
#include <QMessageBox>
#include <QStringList>
#include <QClipboard>
#include <QSettings>
#include <QDateTime>
#include <QSettings>
#include <QVariant>
#include <QAction>
#include <QTimer>
#include <QPoint>
#include <QTimer>
#include <QMenu>
#include <QFile>
#include <QTime>
#include <QIcon>
#include <QDir>

#include "smartclassglobal.h"
#include "nmainwindow.h"
#include "dbmanager.h"

#include "frmimportexportdb.h"
#include "frmmanagestudent.h"
#include "frmprintcontract.h"
#include "frmmanageclass.h"
#include "frmmanageusers.h"
#include "frmfirstrun.h"
#include "frmsettings.h"
#include "frmreceipt.h"
#include "titlebar.h"
#include "frmlogin.h"
#include "frmabout.h"

namespace Ui {
class frmMain;
}

class frmMain : public NMainWindow
{
    Q_OBJECT

public:
    explicit frmMain(QWidget *parent = 0);
    ~frmMain();

    inline int getReturnCode() { return this->returnCode; }

private slots:
    void setSessionRole(const QVariantList &userInfo);
    void logOut();

    void openStudentManager();
    void openClassesManager();
    void openUserManager();
    void openContractForm();
    void openSettingsForm();
    void openReceiptForm();
    void openImportExportTool();
    void openAboutForm();

    void removeStudent();
    void removeCourse();

    void backupDataBase();
    void restoreDataBase();
    void removeDataBase();

    void removeAppSettings();

    void searchPrevious();
    void searchNext();

    void alternateTable();
    void changeTable(bool student);

    void openStudentTableContextMenu(const QPoint &pos);
    void openClassTableContextMenu(const QPoint &pos);

    void changeLanguage(const QString &slug);
    void setUpgradeAvailable();

protected slots:
    void receiveNewStudentData(const QVariantList &data);
    void receiveStudentUpdatedData(const QVariantList &data, const qlonglong &oldStudent);
    void updateResponsibleName(qlonglong pID, const QString &newName);

    void receiveNewCourseData(const QVariantList &data);
    void receiveCourseUpdatedData(const QVariantList &data, const qlonglong &oldCourse);

    void getFirstSettings(const DBManager::DBData &sqlData, const QString &langSlug);

    void openNSWebSite();
    void openNSDocwiki();
    void openSupportEmail();
    void openOnlineSupport();

    void scheduledBackup();

protected:
    void resizeEvent(QResizeEvent *event);
    void changeEvent(QEvent *event);

    void setBackupSettings();

    QTimer backupTimer;
    QString backupPath;

    int backupTimersSize;
    int returnCode;

private:
    Ui::frmMain *ui;

    frmFirstRun *firstRunScr;
    frmLogin *loginScr;
    frmManageStudent *manageStudent;
    frmAbout *frmAbt;
    frmManageClass *manageClass;
    frmSettings *frmConfig;
    frmReceipt *frmPayment;
    frmPrintContract *frmContract;
    frmImportExportDB *frmImportExport;
    frmManageUsers *frmMngUsers;

    QString currentUser;
    SmartClassGlobal::UserRoles sessionRole;

    DBManager *myDB;
    DBManager::DBData db_data;

    int deleteDBStatus;

    QString currentLanguage;
    QString langPath;
    QTranslator translator;
    QTranslator qtTranslator;
    QSettings* settings;

    bool upgradeAvailable;

    void getStudents();
    void getCourses();
    void setUIToRole();

    void createTables();
    void setupDBConnection();

    void closeEvent(QCloseEvent* event);

    void changeTranslator(QTranslator &transl, const QString &filePath);
};

#endif // FRMMAIN_H
