#ifndef FRMRECEIPT_H
#define FRMRECEIPT_H

#include <QDesktopWidget>
#include <QDesktopServices>
#include <QMainWindow>
#include <QFileDialog>
#include <QFile>
#include <QDate>
#include <QDir>

namespace Ui {
class frmReceipt;
}

class frmReceipt : public QMainWindow
{
    Q_OBJECT

public:
    explicit frmReceipt(QWidget *parent = 0, QStringList* paymentData = NULL, QStringList* courseData = NULL,
                                            int paymentDataSize = 0, int courseDataSize = 0);
    ~frmReceipt();

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
    Ui::frmReceipt *ui;
    const int RESIZE_LIMIT;

    QPoint posCursor;
    LockMoveType locked;
    QString currentUser, sessionRole;

    QStringList *paymentData, *courseData;
    int paymentDataSize, courseDataSize;
    double totalWDiscount, totalIntegral;

private slots:
    void switchTableVisibility(bool show);
    void exportCSV();

protected slots:
    void generateTable();
};

#endif // FRMRECEIPT_H
