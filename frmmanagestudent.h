#ifndef FRMMANAGESTUDENT_H
#define FRMMANAGESTUDENT_H

#include <QMainWindow>
#include <QDesktopWidget>
#include <QMessageBox>
#include <QStringList>
#include <QDir>

#include "dbmanager.h"
#include "frmimageviewer.h"
#include "qlistwidgetpaymentitem.h"

namespace Ui {
class frmManageStudent;
}

class frmManageStudent : public QMainWindow
{
    Q_OBJECT

public:
    enum Role{
        View,
        Edit,
        Create
    };

    explicit frmManageStudent(QWidget *parent = 0, Role role = Create, const QString &studentName = NULL,
                              const QStringList &dbData = QStringList());
    ~frmManageStudent();

    const QString getDBPath();

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

    enum CurrentImageManagement{
        Student = 0,
        StudentID,
        ParentID,
        ParentCPG,
        StudentAddress,
        NoManagement
    };

    void retrieveData();
    void setParentGroupEnabled(bool enable);

private:
    Ui::frmManageStudent *ui;
    const int RESIZE_LIMIT, CURRENT_ROLE;
    const QString STUDENT_NAME;

    QPoint posCursor;
    LockMoveType locked;

    DBManager* myDB;
    frmImageViewer* frmImgViewer;

    QString imageViewerSender;
    QStringList studentData, parentalData;
    QStringList *courseData, *paymentData;
    int courseDataCount, paymentDataCount;

    CurrentImageManagement currentImg;
    QPixmap **pics;

    bool blockPaymentUpdate;

    QStringList studentsTable, parentsTable, coursesTable, pricingTable, studentImagesTable, parentImagesTable;

protected slots:
    void openImageViewer();
    void receiveImage(const QPixmap &image);

    void enableRegistrationDetails(int index);

    void retrieveParentData();
    void parentNameChanged();

    void addCourse();
    void removeCourse();
    void courseQuantityChanged();
    void courseIndexChanged();

    void saveData();
    void resetData();
    void changePaymentDetails();
    void paymentValuesChanged();

    /*
     * FIX PAYMENT DETAILS AND COURSE SHORT DESC
     * URGENT!
     */

signals:
    void newData(const QStringList &newData);
    void updatedData(const QStringList &newData, const QString oldStudentName);
};

#endif // FRMMANAGESTUDENT_H
