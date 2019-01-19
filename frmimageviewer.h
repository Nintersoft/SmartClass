#ifndef FRMIMAGEVIEWER_H
#define FRMIMAGEVIEWER_H

#include <QDesktopWidget>
#include <QStandardPaths>
#include <QMainWindow>
#include <QCloseEvent>
#include <QFileDialog>
#include <QStringList>
#include <QMessageBox>
#include <QFileInfo>
#include <QPixmap>
#include <QImage>
#include <QFile>

#include "smartclassglobal.h"
#include "printpreviewform.h"
#include "nmainwindow.h"
#include "dbmanager.h"

namespace Ui {
class frmImageViewer;
}

class frmImageViewer : public NMainWindow
{
    Q_OBJECT

public:
    explicit frmImageViewer(QWidget *parent = 0, const QPixmap &image = QPixmap());
    ~frmImageViewer();

protected:
    void closeEvent(QCloseEvent *event);

private:
    Ui::frmImageViewer *ui;

    const QPixmap INITIAL_IMAGE;
    QPixmap currentImage;

    PrintPreviewForm* printForm;

private slots:
    void openImage();
    void removeImage();
    void openPrintForm();

signals:
    void exec(const QPixmap &image);
};

#endif // FRMIMAGEVIEWER_H
