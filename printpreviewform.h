#ifndef PRINTPREVIEWFORM_H
#define PRINTPREVIEWFORM_H

#include <QDesktopServices>
#include <QDesktopWidget>
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

#include "nmainwindow.h"

namespace Ui {
class PrintPreviewForm;
}

class PrintPreviewForm : public NMainWindow
{
    Q_OBJECT

public:
    explicit PrintPreviewForm(QWidget *parent = 0, const QPixmap &pixmap = QPixmap());
    explicit PrintPreviewForm(QWidget *parent = 0, const QString &text = "", const QPixmap &pixmap = QPixmap());
    explicit PrintPreviewForm(QWidget *parent = 0, const QStringList &content = QStringList(), const QPixmap &pixmap = QPixmap());
    ~PrintPreviewForm();

private:
    Ui::PrintPreviewForm *ui;

    QWidget* mainWidget;
    QLabel *contract, *companyName, *companyLogo, *parentName;

private slots:
    void exportToPDF();
    void printDocument();
};

#endif // PRINTPREVIEWFORM_H
