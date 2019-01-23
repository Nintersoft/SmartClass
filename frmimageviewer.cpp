#include "frmimageviewer.h"
#include "ui_frmimageviewer.h"

frmImageViewer::frmImageViewer(QWidget *parent, const QPixmap &image) :
    NMainWindow(parent),
    ui(new Ui::frmImageViewer),
    INITIAL_IMAGE(image)
{
    ui->setupUi(this);

    // Sets the custom Widgets on the parent Class
    // Otherwise, the window resizing feature will not work
    NMainWindow::setCustomWidgets(ui->centralWidget, ui->statusBar);
    this->setMaximizeButtonEnabled(false);

    this->setGeometry(QStyle::alignedRect(Qt::LeftToRight, Qt::AlignCenter,
            this->size(), qApp->desktop()->availableGeometry()));

    /*
     * End of GUI operations
     */

    if (!image.isNull()) ui->lblPicture->setPixmap(image.scaled(QSize(390, 254), Qt::KeepAspectRatio, Qt::SmoothTransformation));

    connect(ui->btLoadImage, SIGNAL(clicked(bool)), this, SLOT(openImage()));
    connect(ui->btCancel, SIGNAL(clicked(bool)), this, SLOT(close()));
    connect(ui->btPrintImage, SIGNAL(clicked(bool)), this, SLOT(openPrintForm()));
    connect(ui->btRemoveImage, SIGNAL(clicked(bool)), this, SLOT(removeImage()));
    connect(ui->btSave, SIGNAL(clicked(bool)), this, SLOT(close()));

    currentImage = image;
    printForm = NULL;
}

frmImageViewer::~frmImageViewer()
{
    if (printForm) delete printForm;
    delete ui;
}

void frmImageViewer::closeEvent(QCloseEvent *event){
    if (sender()->objectName() == "btSave") emit exec(ui->lblPicture->pixmap() == NULL ? QPixmap() : currentImage);
    else emit exec(INITIAL_IMAGE);

    event->accept();
}

void frmImageViewer::openImage(){
    QFileDialog dialog;
    dialog.setWindowTitle(tr("Manage image file | SmartClass"));
    dialog.setFileMode(QFileDialog::ExistingFile);
    dialog.setDirectory(QStandardPaths::standardLocations(QStandardPaths::PicturesLocation).first());

    if (dialog.exec()){
        if (SmartClassGlobal::databaseType() == DBManager::MYSQL &&
                QFileInfo(dialog.selectedFiles().at(0)).size() > 16777215){
            QMessageBox::warning(NULL, tr("Warning | SmartClass"),
                                 tr("Unfortunately it is not possible to open the selected image, since"
                                    "the current database system does not support its size."),
                                 QMessageBox::Ok, QMessageBox::NoButton);
            return;
        }

        QFile imageFile(dialog.selectedFiles().at(0));
        if (imageFile.open(QIODevice::ReadOnly)){
            QPixmap image = QPixmap::fromImage(QImage::fromData(imageFile.readAll()));
            if (!image.isNull()){
                ui->lblPicture->setText("");
                ui->lblPicture->setPixmap(image.scaled(QSize(390, 254), Qt::KeepAspectRatio, Qt::SmoothTransformation));
                currentImage = image;
            }
            else {
                ui->lblPicture->setText(tr("There is no image to display here!"));
                ui->lblPicture->setPixmap(QPixmap());
                currentImage = QPixmap();
            }
            imageFile.close();
        }
    }
}

void frmImageViewer::removeImage(){
    ui->lblPicture->clear();
    ui->lblPicture->setPixmap(QPixmap());
    ui->lblPicture->setText(tr("There is no image to display here!"));
    ui->lblPicture->update();

    currentImage = QPixmap();
}

void frmImageViewer::openPrintForm(){
    if (printForm){
        delete printForm;
        printForm = NULL;
    }

    printForm = new PrintPreviewForm(NULL, currentImage);
    printForm->showMaximized();
}
