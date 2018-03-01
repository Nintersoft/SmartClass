#ifndef FRMSETTINGS_H
#define FRMSETTINGS_H

#include <QDesktopWidget>
#include <QDesktopServices>
#include <QMessageBox>
#include <QMainWindow>
#include <QMouseEvent>
#include <QFileDialog>
#include <QSettings>
#include <QPixmap>
#include <QEvent>
#include <QFile>
#include <QDir>

#include "frmlogin.h"

namespace Ui {
class frmSettings;
}

class frmSettings : public QMainWindow
{
    Q_OBJECT

public:
    enum OpenMode{
        Normal,
        Info
    };

    explicit frmSettings(QWidget *parent = 0, OpenMode mode = frmSettings::Normal);
    ~frmSettings();

protected:
    void mousePressEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);
    void undefMouseMoveEvent(QObject *object, QMouseEvent* event);
    bool eventFilter(QObject *watched, QEvent *event);

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

private:
    Ui::frmSettings *ui;
    const int RESIZE_LIMIT;

    QPoint posCursor;
    LockMoveType locked;
    QString currentUser, sessionRole;
    QSettings programSettings;

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
