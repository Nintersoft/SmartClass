#ifndef FRMIMPORTEXPORTDB_H
#define FRMIMPORTEXPORTDB_H

#include <QDesktopServices>
#include <QDesktopWidget>
#include <QFileDialog>
#include <QMainWindow>
#include <QMouseEvent>
#include <QStringList>
#include <QFileInfo>
#include <QVariant>
#include <QRegExp>
#include <QPixmap>
#include <QEvent>
#include <QDate>
#include <QList>
#include <QFile>
#include <QDir>

#include "nmainwindow.h"
#include "dbmanager.h"
#include "frmlogin.h"

namespace Ui {
class frmImportExportDB;
}

class frmImportExportDB : public NMainWindow
{
    Q_OBJECT

public:
    enum ImportMode{
        Import,
        Export
    };

    explicit frmImportExportDB(QWidget *parent = 0, ImportMode mode = frmImportExportDB::Export);
    ~frmImportExportDB();

private:
    Ui::frmImportExportDB *ui;

    const ImportMode CURRENT_MODE;
    QDir currentImportDir;
    QString currentUser, sessionRole;

    DBManager* db_manager;

    QVariantList stringToVariant(const QStringList &sList, SmartClassGlobal::TablesSpec tableSpec);

protected:

    enum ImportOperation{
        Insert,
        Update,
        NoOperation
    };

private slots:
    void getExportDir();
    void getImportFilePath();
    void getImportDir();

protected slots:
    void importDB();
    void importCheckingErrors();
    void importWithoutCheckErrors();
    void exportDB();
};

#endif // FRMIMPORTEXPORTDB_H
