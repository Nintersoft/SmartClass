#ifndef FRMADDCLASS_H
#define FRMADDCLASS_H

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
class frmAddClass;
}

class frmAddClass : public NMainWindow
{
    Q_OBJECT

public:
    enum Role{
        View,
        Edit,
        Create
    };

    explicit frmAddClass(QWidget *parent = 0, Role role = Create, qint64 courseID = -1,
                         const DBManager::DBData &dbData = DBManager::DBData());
    ~frmAddClass();

private:
    Ui::frmAddClass *ui;
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

#endif // FRMADDCLASS_H
