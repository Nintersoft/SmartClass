#ifndef FRMIMAGEVIEWER_H
#define FRMIMAGEVIEWER_H

#include <QMainWindow>
#include <QCloseEvent>
#include <QFileDialog>
#include <QDesktopWidget>
#include <QStandardPaths>
#include <QStringList>
#include <QPixmap>
#include <QImage>
#include <QFile>

#include "printpreviewform.h"

namespace Ui {
class frmImageViewer;
}

class frmImageViewer : public QMainWindow
{
    Q_OBJECT

public:
    explicit frmImageViewer(QWidget *parent = 0, const QPixmap &image = QPixmap());
    ~frmImageViewer();

protected:
    void mousePressEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);
    void undefMouseMoveEvent(QObject *object, QMouseEvent* event);
    bool eventFilter(QObject *watched, QEvent *event);

    void closeEvent(QCloseEvent *event);

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
    Ui::frmImageViewer *ui;
    const int RESIZE_LIMIT;
    const QString STUDENT_NAME;

    QPoint posCursor;
    LockMoveType locked;
    const QPixmap INITIAL_IMAGE;

    PrintPreviewForm* printForm;

private slots:
    void openImage();
    void removeImage();
    void openPrintForm();

signals:
    void exec(const QPixmap &image);
};

#endif // FRMIMAGEVIEWER_H
