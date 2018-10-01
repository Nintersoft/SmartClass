#ifndef FRMRECEIPT_H
#define FRMRECEIPT_H

#include <QDesktopServices>
#include <QDesktopWidget>
#include <QMainWindow>
#include <QFileDialog>
#include <QVariant>
#include <QList>
#include <QFile>
#include <QDate>
#include <QDir>

#include "smartclassglobal.h"
#include "nmainwindow.h"
#include "dbmanager.h"

namespace Ui {
class frmReceipt;
}

class frmReceipt : public NMainWindow
{
    Q_OBJECT

public:
    explicit frmReceipt(QWidget *parent = 0, DBManager *db_manager = NULL);
    ~frmReceipt();

private:
    Ui::frmReceipt *ui;

    DBManager *db_manager;
    double totalWDiscount, totalIntegral;

    QList< QList<QVariant> > sData, pData, cData;

private slots:
    void switchTableVisibility(bool show);
    void exportCSV();

protected slots:
    void generateTable();
};

#endif // FRMRECEIPT_H
