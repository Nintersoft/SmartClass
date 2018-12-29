#ifndef FRMMANAGESTUDENT_H
#define FRMMANAGESTUDENT_H

#include <QDesktopWidget>
#include <QMainWindow>
#include <QMessageBox>
#include <QStringList>
#include <QVariant>
#include <QDir>

#include "qlistwidgetpaymentitem.h"
#include "smartclassglobal.h"
#include "frmimageviewer.h"
#include "nmainwindow.h"
#include "dbmanager.h"

namespace Ui {
class frmManageStudent;
}

class frmManageStudent : public NMainWindow
{
    Q_OBJECT

public:
    enum Role{
        View,
        Edit,
        Create
    };

    explicit frmManageStudent(QWidget *parent = 0, Role role = Create, const qint64 &studentID = -1);
    ~frmManageStudent();

protected:
    enum CurrentImageManagement{
        Student = 0,
        StudentID,
        ParentID,
        ParentCPG,
        StudentAddress,
        NoManagement
    };

    void retrieveData();

private:
    Ui::frmManageStudent *ui;
    const Role CURRENT_ROLE;
    const qlonglong STUDENT_ID;

    DBManager* db_manager;
    frmImageViewer* frmImgViewer;

    QString imageViewerSender;
    QVariantList studentData, responsibleData;
    QList< QVariantList > courseData;

    int courseDataCount;

    CurrentImageManagement currentImg;
    QPixmap **pics;

    bool blockPaymentUpdate;

protected slots:
    void openImageViewer();
    void receiveImage(const QPixmap &image);

    void enableRegistrationDetails(int index);

    void addCourse();
    void removeCourse();
    void courseQuantityChanged();
    void courseIndexChanged();

    void saveData();
    void resetData();
    void changePaymentDetails();
    void paymentValuesChanged();
    void changeDiscountValue(double value);
    void changeDiscountPercentage(double percent);

    void updateParentInfo(const QString &pName);

signals:
    void newData(const QList<QVariant> &newData);
    void updatedData(const QList<QVariant> &newData, const qlonglong oldStudentIndex);
};

#endif // FRMMANAGESTUDENT_H
