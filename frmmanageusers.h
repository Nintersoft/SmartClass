#ifndef FRMMANAGEUSERS_H
#define FRMMANAGEUSERS_H

#include <QListWidgetItem>
#include <QMainWindow>
#include <QStringList>
#include <QListWidget>
#include <QMessageBox>
#include <QSqlError>
#include <QVariant>
#include <QList>

#include "smartclassglobal.h"
#include "frmmanageuser.h"
#include "nmainwindow.h"
#include "dbmanager.h"
#include "frmlogin.h"

namespace Ui {
class frmManageUsers;
}

class frmManageUsers : public NMainWindow
{
    Q_OBJECT

public:
    explicit frmManageUsers(QWidget *parent = 0, DBManager *dbManager = 0,
                                const QString &currentUser = "");
    ~frmManageUsers();

private:
    Ui::frmManageUsers *ui;

    QVariant currentID;
    QList< QList<QVariant> > userDBData;

    frmManageUser *userForm;
    DBManager *dbManager;

    QString CURRENT_USER;

private slots:
    void setToolsEnabled(int row);

    void removeUser();
    void openUserManager();
    void getModifiedData(const QList<QVariant> newData);
};

#endif // FRMMANAGEUSERS_H
