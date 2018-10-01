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

#include "dbmanager.h"
#include "nmainwindow.h"
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

    explicit frmImportExportDB(QWidget *parent = 0, ImportMode mode = frmImportExportDB::Export,
                               const DBManager::DBData &dbData = DBManager::DBData());
    ~frmImportExportDB();

private:
    Ui::frmImportExportDB *ui;

    const ImportMode CURRENT_MODE;
    QDir currentImportDir;
    QString currentUser, sessionRole;

    DBManager* myDB;

    QList <QVariant> stringToVariant(const QStringList &sList, SmartClassGlobal::TablesSpec tableSpec);

protected slots:
    void importDB();
    void exportDB();

private slots:
    void getExportDir();
    void getImportFilePath();
    void getImportDir();
};

#endif // FRMIMPORTEXPORTDB_H
