#ifndef PRINTPREVIEWFORM_H
#define PRINTPREVIEWFORM_H

#include <QDesktopWidget>
#include <QDesktopServices>
#include <QFontDatabase>
#include <QPrintDialog>
#include <QFileDialog>
#include <QMainWindow>
#include <QGridLayout>
#include <QPrinter>
#include <QPainter>
#include <QPixmap>
#include <QWidget>
#include <QLabel>
#include <QFile>
#include <QDate>
#include <QRect>
#include <QDir>

namespace Ui {
class PrintPreviewForm;
}

class PrintPreviewForm : public QMainWindow
{
    Q_OBJECT

public:
    explicit PrintPreviewForm(QWidget *parent = 0, const QPixmap &pixmap = QPixmap());
    explicit PrintPreviewForm(QWidget *parent = 0, const QString &text = "", const QPixmap &pixmap = QPixmap());
    explicit PrintPreviewForm(QWidget *parent = 0, const QStringList &content = QStringList(), const QPixmap &pixmap = QPixmap());
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
    QLabel *contract, *companyName, *companyLogo, *parentName;

private slots:
    void exportToPDF();
    void printDocument();
};

#endif // PRINTPREVIEWFORM_H
