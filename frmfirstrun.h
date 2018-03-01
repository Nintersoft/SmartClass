#ifndef FRMFIRSTRUN_H
#define FRMFIRSTRUN_H

#include <QMessageBox>
#include <QMainWindow>
#include <QDesktopWidget>
#include <QMouseEvent>
#include <QCloseEvent>
#include <QTranslator>
#include <QFileDialog>
#include <QSettings>
#include <QPixmap>
#include <QEvent>
#include <QPoint>
#include <QFile>
#include <QDir>

#include "dbmanager.h"

namespace Ui {
class frmFirstRun;
}

class frmFirstRun : public QMainWindow
{
    Q_OBJECT

public:
    explicit frmFirstRun(QWidget *parent = 0);
    ~frmFirstRun();

    static const QString getDBPath();

protected:
    void mousePressEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);
    void undefMouseMoveEvent(QObject *object, QMouseEvent* event);
    bool eventFilter(QObject *watched, QEvent *event);

    void closeEvent(QCloseEvent* event);
    void changeEvent(QEvent* event);

    void readLanguage(const QString &langSlug);

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
    Ui::frmFirstRun *ui;

    QPoint posCursor;
    LockMoveType locked;
    DBManager* db_manager;

    const int RESIZE_LIMIT;
    int currentLangIndex; //English = 0; Portuguese = 1

    QString langPath;
    QTranslator translator;
    QTranslator qtTranslator;

    bool allowed, canChangeLang;

    void changeTranslator(QTranslator &transl, const QString &filePath);

private slots:
    void saveDBSettings();
    void selectCompanyLogo();
    void changeLanguage(int index);

signals:
    void sendData(const QStringList &data, const QString &langSlug);
};

#endif // FRMFIRSTRUN_H
