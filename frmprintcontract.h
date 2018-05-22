#ifndef FRMPRINTCONTRACT_H
#define FRMPRINTCONTRACT_H

#include <QDesktopWidget>
#include <QDesktopServices>
#include <QMainWindow>
#include <QMouseEvent>
#include <QMessageBox>
#include <QSettings>
#include <QProcess>
#include <QEvent>
#include <QDate>

#include "dbmanager.h"
#include "printpreviewform.h"

namespace Ui {
class frmPrintContract;
}

class frmPrintContract : public QMainWindow
{
    Q_OBJECT

public:
    explicit frmPrintContract(QWidget *parent = 0, QStringList studentData = QStringList(),
                              QStringList parentData = QStringList(), QStringList* paymentData = NULL,
                              int paymentDataSize = 0, QStringList* coursesData = NULL,
                              int coursesDataSize = 0, const QString &companyName = NULL);
    ~frmPrintContract();

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
    Ui::frmPrintContract *ui;
    const int RESIZE_LIMIT;

    QPoint posCursor;
    LockMoveType locked;
    QString currentUser, sessionRole;

    QString parseArguments();

    QStringList studentData, parentalData;
    QStringList *paymentData, *coursesData;
    QString externalCommand, programPath;

    const int PAYMENT_DATA_SIZE, COURSES_DATA_SIZE;

    PrintPreviewForm *frmPrintPrev;

protected slots:
    void generateContractForm();
    void openExternalTool();
};

#endif // FRMPRINTCONTRACT_H
