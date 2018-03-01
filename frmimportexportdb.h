#ifndef FRMIMPORTEXPORTDB_H
#define FRMIMPORTEXPORTDB_H

#include <QDesktopWidget>
#include <QDesktopServices>
#include <QFileDialog>
#include <QMainWindow>
#include <QMouseEvent>
#include <QStringList>
#include <QRegExp>
#include <QFileInfo>
#include <QPixmap>
#include <QEvent>
#include <QFile>
#include <QDir>

#include "dbmanager.h"
#include "frmlogin.h"

namespace Ui {
class frmImportExportDB;
}

class frmImportExportDB : public QMainWindow
{
    Q_OBJECT

public:
    enum ImportMode{
        Import,
        Export
    };

    explicit frmImportExportDB(QWidget *parent = 0, ImportMode mode = frmImportExportDB::Export,
                               const QStringList &dbData = QStringList());
    ~frmImportExportDB();

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
    Ui::frmImportExportDB *ui;
    const int RESIZE_LIMIT;
    const ImportMode CURRENT_MODE;

    QPoint posCursor;
    LockMoveType locked;
    QString currentUser, sessionRole;

    DBManager* myDB;
    QStringList* tableColumns;

    QDir currentImportDir;

protected slots:
    void importDB();
    void exportDB();

private slots:
    void getExportDir();
    void getImportFilePath();
    void getImportDir();
};

#endif // FRMIMPORTEXPORTDB_H
