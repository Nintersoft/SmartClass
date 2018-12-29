#ifndef FRMMANAGECLASS_H
#define FRMMANAGECLASS_H

#include <QListWidgetItem>
#include <QDesktopWidget>
#include <QMainWindow>
#include <QStringList>
#include <QMessageBox>
#include <QSqlQuery>
#include <QVariant>

#include "nmainwindow.h"
#include "dbmanager.h"
#include "frmlogin.h"

namespace Ui {
class frmManageClass;
}

class frmManageClass : public NMainWindow
{
    Q_OBJECT

public:
    enum Role{
        View,
        Edit,
        Create
    };

    explicit frmManageClass(QWidget *parent = 0, Role role = Create, qint64 courseID = -1);
    ~frmManageClass();

private:
    Ui::frmManageClass *ui;
    const qint64 COURSE_ID;
    const Role CURRENT_ROLE;

    DBManager* myDB;

    QList<QVariant> courseData;

protected slots:
    void saveNewClass();
    void resetNewClass();

    void addDayNTime();
    void removeDayNTime();

    void applyCourseData();

signals:
    void newData(const QList<QVariant> &newData);
    void updatedData(const QList<QVariant> &newData, const qlonglong oldIndex);
};

#endif // FRMMANAGECLASS_H
