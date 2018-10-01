#ifndef FRMMANAGEUSER_H
#define FRMMANAGEUSER_H

#include <QMainWindow>
#include <QVariant>
#include <QList>

#include "nmainwindow.h"
#include "dbmanager.h"
#include "frmlogin.h"

namespace Ui {
class frmManageUser;
}

class frmManageUser : public NMainWindow
{
    Q_OBJECT

public:
    explicit frmManageUser(QWidget *parent = 0, const QList<QVariant> &data = QList<QVariant>(),
                           const QString &currentUser = QString());
    ~frmManageUser();

private:
    Ui::frmManageUser *ui;

    const bool IS_CURRENT_USER;

private slots:
    void showCHelp();
    void saveData();

signals:
    void sendNewData(QList<QVariant> newData);
};

#endif // FRMMANAGEUSER_H
