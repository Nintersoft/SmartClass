#ifndef FRMSETTINGS_H
#define FRMSETTINGS_H

#include <QDesktopServices>
#include <QDesktopWidget>
#include <QMessageBox>
#include <QMainWindow>
#include <QMouseEvent>
#include <QFileDialog>
#include <QSettings>
#include <QVariant>
#include <QPixmap>
#include <QEvent>
#include <QFile>
#include <QList>
#include <QDir>

#include "smartclassglobal.h"
#include "nmainwindow.h"
#include "dbmanager.h"
#include "frmlogin.h"

namespace Ui {
class frmSettings;
}

class frmSettings : public NMainWindow
{
    Q_OBJECT

public:
    enum OpenMode{
        Normal,
        Info
    };

    explicit frmSettings(QWidget *parent = 0, SmartClassGlobal::UserRoles role = SmartClassGlobal::VIEWER, OpenMode mode = frmSettings::Normal);
    ~frmSettings();

private:
    Ui::frmSettings *ui;

    QList< QList<QVariant> > gSettings;
    QString currentUser, sessionRole;
    QSettings programSettings;

    DBManager *db_manager;

    int langSettings;
    QPixmap defLogo;

    SmartClassGlobal::UserRoles CURRENT_ROLE;   

protected slots:
    void saveOptions();
    void resetSettings();
    void retrieveSettings();
    void openExternalProgramHelper();

private slots:
    void selectNewLogo();
    void selectBackupPath();
    void selectExternalProgram();
    void addSchedule();
    void removeSchedule();
};

#endif // FRMSETTINGS_H
