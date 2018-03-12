#ifndef FRMMAIN_H
#define FRMMAIN_H

#include <QMainWindow>
#include <QDesktopServices>
#include <QCoreApplication>
#include <QMouseEvent>
#include <QCloseEvent>
#include <QResizeEvent>
#include <QSettings>
#include <QTranslator>
#include <QMessageBox>
#include <QStringList>
#include <QDateTime>
#include <QSettings>
#include <QTimer>
#include <QPoint>
#include <QTimer>
#include <QFile>
#include <QTime>
#include <QDir>

#include "dbmanager.h"

#include "frmimportexportdb.h"
#include "frmmanagestudent.h"
#include "frmprintcontract.h"
#include "frmfirstrun.h"
#include "frmaddclass.h"
#include "frmsettings.h"
#include "frmreceipt.h"
#include "titlebar.h"
#include "frmlogin.h"
#include "frmabout.h"

namespace Ui {
class frmMain;
}

class frmMain : public QMainWindow
{
    Q_OBJECT

public:
    explicit frmMain(QWidget *parent = 0);
    ~frmMain();

    int returnCode;

private slots:
    void setSessionRole(const QStringList &userInfo);
    void logOut();

    void openStudentManager();
    void openClassesManager();
    void openContractForm();
    void openSettingsForm();
    void openReceiptForm();
    void openImportExportTool();

    void removeStudent();
    void removeCourse();

    void revokeUserPermissions();
    void upgradeUserPermissions();
    void removeUser();

    void backupDataBase();
    void restoreDataBase();
    void removeDataBase();

    void searchPrevious();
    void searchNext();

    void alternateTable();
    void changeTable(bool student);

    void changeLanguage(const QString &slug);

protected slots:
    void receiveNewStudentData(const QStringList &data);
    void receiveStudentUpdatedData(const QStringList &data, const QString &oldName);

    void receiveNewCourseData(const QStringList &data);
    void receiveCourseUpdatedData(const QStringList &data, const QString &oldCourse);

    void getFirstSettings(const QStringList &sqlData, const QString &langSlug);

    void openNSWebSite();
    void openNSDocwiki();
    void openSupportEmail();
    void openOnlineSupport();

    void scheduledBackup();

protected:
    void mousePressEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);
    void resizeEvent(QResizeEvent *event);
    void undefMouseMoveEvent(QObject *object, QMouseEvent* event);
    bool eventFilter(QObject *watched, QEvent *event);

    void changeEvent(QEvent *event);
    
    enum LockMoveType{
        Left,
        Right,
        Top,
        Bottom,
        TopLeft,
        TopRight,
        BottomLeft,
        BottomRight,
        None
    };

    QTimer backupTimer;
    int backupTimersSize;
    QString backupPath;

    void setBackupSettings();

private:
    Ui::frmMain *ui;
    const int RESIZE_LIMIT;

    frmFirstRun *firstRunScr;
    frmLogin *loginScr;
    frmManageStudent *manageStudent;
    frmAbout *frmAbt;
    frmAddClass *addClass;
    frmSettings *frmConfig;
    frmReceipt *frmPayment;
    frmPrintContract *frmContract;
    frmImportExportDB *frmImportExport;

    QPoint posCursor;
    LockMoveType locked;
    QString currentUser, sessionRole;

    DBManager *myDB;

    int deleteDBStatus;

    QString currentLanguage;
    QString langPath;
    QTranslator translator;
    QTranslator qtTranslator;
    QSettings* settings;
    QStringList db_SETTINGS;

    void getUsers();
    void getStudents();
    void getCourses();
    void setUIToRole();

    void createTables();
    void setupDBConnection();

    void closeEvent(QCloseEvent* event);

    void changeTranslator(QTranslator &transl, const QString &filePath);
};

#endif // FRMMAIN_H
