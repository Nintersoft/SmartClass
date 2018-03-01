#ifndef FRMADDCLASS_H
#define FRMADDCLASS_H

#include <QMainWindow>
#include <QDesktopWidget>
#include <QListWidgetItem>
#include <QStringList>
#include <QMessageBox>

#include "frmlogin.h"
#include "dbmanager.h"

namespace Ui {
class frmAddClass;
}

class frmAddClass : public QMainWindow
{
    Q_OBJECT

public:
    enum Role{
        View,
        Edit,
        Create
    };

    explicit frmAddClass(QWidget *parent = 0, Role role = Create, QString name = NULL,
                         const QStringList &dbData = QStringList());
    ~frmAddClass();

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

    QStringList coursesTable;

private:
    Ui::frmAddClass *ui;
    const int RESIZE_LIMIT;
    const QString COURSE_NAME;
    const Role CURRENT_ROLE;

    QPoint posCursor;
    LockMoveType locked;
    DBManager* myDB;

    QStringList courseData;
    QStringList *paymentData, *studentsData;

protected slots:
    void saveNewClass();
    void resetNewClass();

    void addDayNTime();
    void removeDayNTime();

    void applyCourseData();

signals:
    void newData(const QStringList &newData);
    void updatedData(const QStringList &newData, const QString oldCourseName);
};

#endif // FRMADDCLASS_H
