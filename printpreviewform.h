#ifndef PRINTPREVIEWFORM_H
#define PRINTPREVIEWFORM_H

#include <QDesktopWidget>
#include <QDesktopServices>
//#include <QPrintDialog>
#include <QMainWindow>
#include <QPixmap>
#include <QWidget>
#include <QLabel>
#include <QFile>

namespace Ui {
class PrintPreviewForm;
}

class PrintPreviewForm : public QMainWindow
{
    Q_OBJECT

public:
    explicit PrintPreviewForm(QWidget *parent = 0, const QPixmap &pixmap = QPixmap());
    explicit PrintPreviewForm(QWidget *parent = 0, const QString &text = "", bool includeLogo = false);
    ~PrintPreviewForm();

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
    Ui::PrintPreviewForm *ui;
    const int RESIZE_LIMIT;

    QPoint posCursor;
    LockMoveType locked;

    QWidget* mainWidget;
};

#endif // PRINTPREVIEWFORM_H
