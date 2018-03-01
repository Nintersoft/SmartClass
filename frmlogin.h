#ifndef FRMLOGIN_H
#define FRMLOGIN_H

#include <QMainWindow>
#include <QCryptographicHash>
#include <QDesktopWidget>
#include <QMessageBox>
#include <QStringList>
#include <QSysInfo>
#include <QFile>
#include <QDir>

#include <QDebug>

#include "dbmanager.h"

#include "titlebar.h"

namespace Ui {
class frmLogin;
}

class frmLogin : public QMainWindow
{
    Q_OBJECT

public:
    explicit frmLogin(QWidget *parent = 0, const QStringList dbData = QStringList());
    ~frmLogin();

    static const QString getDBPath();

private slots:
    void login();
    void registerUser();
    void askQuestion();
    void changePassword();

protected slots:
    void changeTab();

protected:
    void mousePressEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);
    void undefMouseMoveEvent(QObject *object, QMouseEvent* event);
    bool eventFilter(QObject *watched, QEvent *event);

    QString randomSalt(int size);
    bool isSafePassword(const QString &pass, int &index);

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

    QStringList columns;

private:
    Ui::frmLogin *ui;
    QPoint posCursor;
    LockMoveType locked;
    DBManager *myDb;

    const int RESIZE_LIMIT;

signals:
    void dataReady(const QStringList &userInfo);
};

#endif // FRMLOGIN_H
