#ifndef FRMPRINTCONTRACT_H
#define FRMPRINTCONTRACT_H

#include <QDesktopServices>
#include <QDesktopWidget>
#include <QMainWindow>
#include <QMouseEvent>
#include <QMessageBox>
#include <QSettings>
#include <QVariant>
#include <QProcess>
#include <QEvent>
#include <QDate>

#include "smartclassglobal.h"
#include "printpreviewform.h"
#include "nmainwindow.h"
#include "dbmanager.h"

namespace Ui {
class frmPrintContract;
}

class frmPrintContract : public NMainWindow
{
    Q_OBJECT

public:
    explicit frmPrintContract(QWidget *parent = 0, DBManager *db_manager = NULL, qlonglong studentID = -1);
    ~frmPrintContract();

private:
    Ui::frmPrintContract *ui;

    qlonglong STUDENT_ID;
    QString externalCommand, programPath, companyName;
    PrintPreviewForm *frmPrintPrev;

    QList<QVariant> sData, rData;
    QList< QList<QVariant> > pData, cData;

    QString parseArguments();
    DBManager *db_manager;

protected slots:
    void generateContractForm();
    void openExternalTool();
};

#endif // FRMPRINTCONTRACT_H
