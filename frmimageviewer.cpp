#include "frmimageviewer.h"
#include "ui_frmimageviewer.h"

frmImageViewer::frmImageViewer(QWidget *parent, const QPixmap &image) :
    QMainWindow(parent),
    ui(new Ui::frmImageViewer),
    RESIZE_LIMIT(2),
    INITIAL_IMAGE(image)
{
    ui->setupUi(this);

    setWindowFlags(Qt::Widget | Qt::FramelessWindowHint);
    centralWidget()->installEventFilter(this);
    ui->titleBar->installEventFilter(this);
    ui->statusBar->installEventFilter(this);

    centralWidget()->setMouseTracking(true);
    ui->titleBar->setMouseTracking(true);
    ui->statusBar->setMouseTracking(true);

    setWindowTitle(tr("Image viewer | SmartClass"));
    locked = LockMoveType::None;

    ui->titleBar->setMaximizeButtonEnabled(false);

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
        QFile imageFile(dialog.selectedFiles().at(0));
        if (imageFile.open(QIODevice::ReadOnly)){
            QPixmap image = QPixmap::fromImage(QImage::fromData(imageFile.readAll()));
            if (!image.isNull()){
                ui->lblPicture->setText("");
                ui->lblPicture->setPixmap(image.scaled(QSize(390, 254), Qt::KeepAspectRatio, Qt::SmoothTransformation));
                currentImage = image;
            }
            else {
                ui->lblPicture->setText(tr("There is no picture to display here!"));
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
    ui->lblPicture->setText(tr("There is no picture to display here!"));
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
/*
 * GUI Functions (don't change, unless necessary)
 */

void frmImageViewer::mousePressEvent(QMouseEvent *event)
{
    if(event->button() == Qt::LeftButton)
    {
        int x = event->x(), y = event->y(), bottom = this->height() - RESIZE_LIMIT, right = this->width() - RESIZE_LIMIT;
        if (x < RESIZE_LIMIT && y < RESIZE_LIMIT){
            posCursor = event->globalPos() - this->geometry().topLeft();
            locked = LockMoveType::TopLeft;
        }
        else if (x < RESIZE_LIMIT && y > bottom){
            posCursor = event->globalPos() - this->geometry().bottomLeft();
            locked = LockMoveType::BottomLeft;
        }
        else if (x > right && y < RESIZE_LIMIT){
            posCursor = event->globalPos() - this->geometry().topRight();
            locked = LockMoveType::TopRight;
        }
        else if (x > right && y > bottom){
            posCursor = event->globalPos() - this->geometry().bottomRight();
            locked = LockMoveType::BottomRight;
        }
        else if (x < RESIZE_LIMIT || y < RESIZE_LIMIT){
            posCursor = event->globalPos() - this->geometry().topLeft();
            locked = x < RESIZE_LIMIT ? LockMoveType::Left : LockMoveType::Top;
        }
        else if (x > right || y > bottom){
            posCursor = event->globalPos() - this->geometry().bottomRight();
            locked = x > right ? LockMoveType::Right : LockMoveType::Bottom;
        }
        event->accept();
    }
}

void frmImageViewer::undefMouseMoveEvent(QObject* object, QMouseEvent* event){
    if (locked != LockMoveType::None){
        switch (locked) {
        case LockMoveType::TopLeft:
            this->setGeometry(QRect(QPoint(event->globalPos().x() - posCursor.x(), event->globalPos().y() - posCursor.y()),
                                    this->geometry().bottomRight()));
            break;
        case LockMoveType::TopRight:
            this->setGeometry(QRect(QPoint(this->geometry().left(), event->globalPos().y() - posCursor.y()),
                                    QPoint(event->globalPos().x() - posCursor.x(), this->geometry().bottom())));
            break;
        case LockMoveType::BottomLeft:
            this->setGeometry(QRect(QPoint(event->globalPos().x() - posCursor.x(), this->geometry().top()),
                                    QPoint(this->geometry().right(), event->globalPos().y() - posCursor.y())));
            break;
        case LockMoveType::BottomRight:
            this->setGeometry(QRect(this->geometry().topLeft(),
                                    QPoint(event->globalPos().x() - posCursor.x(), event->globalPos().y() - posCursor.y())));
            break;
        case LockMoveType::Left:
            this->setGeometry(QRect(QPoint(event->globalPos().x() - posCursor.x(), this->geometry().top()),
                                    this->geometry().bottomRight()));
            break;
        case LockMoveType::Right:
            this->setGeometry(QRect(this->geometry().topLeft(),
                                    QPoint(event->globalPos().x() - posCursor.x(), this->geometry().bottom())));
            break;
        case LockMoveType::Top:
            this->setGeometry(QRect(QPoint(this->geometry().left(), event->globalPos().y() - posCursor.y()),
                                    this->geometry().bottomRight()));
            break;
        default:
            this->setGeometry(QRect(this->geometry().topLeft(),
                                    QPoint(this->geometry().right(), event->globalPos().y() - posCursor.y())));
            break;
        }
        return;
    }

    int x = event->x(), y = event->y(), right = this->width() - RESIZE_LIMIT;
    if (object->objectName() == "statusBar"){
        if (x < RESIZE_LIMIT && y > (19 - RESIZE_LIMIT)){
            this->setCursor(QCursor(Qt::SizeBDiagCursor));
            return;
        }
        else if (x > right && y > (19 - RESIZE_LIMIT)){
            this->setCursor(QCursor(Qt::SizeFDiagCursor));
            return;
        }
        else if (y > (19 - RESIZE_LIMIT)){
            this->setCursor(QCursor(Qt::SizeVerCursor));
            return;
        }
    }
    else if (object->objectName() == "titleBar"){
        if (x < RESIZE_LIMIT && y < RESIZE_LIMIT){
            this->setCursor(QCursor(Qt::SizeFDiagCursor));
            return;
        }
        if (x > right && y < RESIZE_LIMIT){
            this->setCursor(QCursor(Qt::SizeBDiagCursor));
            return;
        }
        else if (y < RESIZE_LIMIT){
            this->setCursor(QCursor(Qt::SizeVerCursor));
            return;
        }
    }
    if (x < RESIZE_LIMIT || x > right){
        this->setCursor(QCursor(Qt::SizeHorCursor));
    }
    else {
        this->setCursor(QCursor(Qt::ArrowCursor));
    }
}

void frmImageViewer::mouseReleaseEvent(QMouseEvent *event){
    locked = LockMoveType::None;
    event->accept();
}

bool frmImageViewer::eventFilter(QObject* object, QEvent* event)
{
    if(event->type() == QEvent::MouseMove)
        undefMouseMoveEvent(object, static_cast<QMouseEvent*>(event));
    else if (event->type() == QEvent::MouseButtonPress && object->objectName() == "titleBar"){
        mousePressEvent(static_cast<QMouseEvent*>(event));
    }
    return false;
}
