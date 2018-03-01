#include "frmreceipt.h"
#include "ui_frmreceipt.h"

frmReceipt::frmReceipt(QWidget *parent, QStringList *paymentData, QStringList *courseData,
                       int paymentDataSize, int courseDataSize) :
    QMainWindow(parent),
    ui(new Ui::frmReceipt),
    RESIZE_LIMIT(2)
{
    ui->setupUi(this);
    setWindowFlags(Qt::Widget | Qt::FramelessWindowHint);
    centralWidget()->installEventFilter(this);
    ui->titleBar->installEventFilter(this);
    ui->statusBar->installEventFilter(this);

    centralWidget()->setMouseTracking(true);
    ui->titleBar->setMouseTracking(true);
    ui->statusBar->setMouseTracking(true);

    setWindowTitle(tr("Receipt viewer | SmartClass"));
    locked = LockMoveType::None;

    this->setGeometry(QStyle::alignedRect(Qt::LeftToRight, Qt::AlignCenter,
            this->size(), qApp->desktop()->availableGeometry()));

    /*
     *  End of GUI implementation
     */

    totalWDiscount = 0;
    totalIntegral = 0;

    ui->titleBar->setMaximizeButtonEnabled(false);
    connect(ui->cbShowTable, SIGNAL(toggled(bool)), this, SLOT(switchTableVisibility(bool)));
    connect(ui->btExport, SIGNAL(clicked(bool)), this, SLOT(exportCSV()));

    ui->lblDate->setText(QDate::currentDate().toString("dd/MM/yyyy"));

    for (int i = 0; i < 25; ++i)
        ui->cbCustomMonthNYear->addItem(QDate::currentDate().addMonths(i).toString("MM/yyyy"));
    ui->cbCustomMonthNYear->setCurrentIndex(0);

    connect(ui->cbCustomMonthNYear, SIGNAL(currentIndexChanged(int)), this, SLOT(generateTable()));

    this->paymentData = paymentData;
    this->paymentDataSize = paymentDataSize;
    this->courseData = courseData;
    this->courseDataSize = courseDataSize;
    generateTable();
}

frmReceipt::~frmReceipt()
{
    delete ui;
}

void frmReceipt::switchTableVisibility(bool show){
    ui->tablePricing->setVisible(show);
    int height = 600;
    if (!show) height = 259;
    this->setMaximumHeight(height);
    this->setMinimumHeight(height);
    this->setGeometry(QRect(this->geometry().topLeft(), QSize(this->width(), this->height())));
    this->update();
}

void frmReceipt::generateTable(){
    if (paymentData == NULL) return;

    totalWDiscount = 0;
    totalIntegral = 0;

    ui->tablePricing->clearContents();
    ui->tablePricing->setRowCount(0);

    for (int i = 0; i < paymentDataSize; ++i){
        int installments = paymentData[i].at(4).toInt();

        QDate choosenDate = QDate::fromString("01/" + ui->cbCustomMonthNYear->currentText(), "dd/MM/yyyy");
        QDate initialDate = QDate::fromString(paymentData[i].at(3), "dd/MM/yyyy");
        QDate finalDate = initialDate.addMonths(installments - 1);

        if (choosenDate.year() < initialDate.year() || finalDate.year() < choosenDate.year()
                || (choosenDate.year() == initialDate.year() && choosenDate.month() < initialDate.month())
                || (choosenDate.year() == finalDate.year() && choosenDate.month() > finalDate.month())) continue;

        for (int k = 0; k < installments; ++k){
            if (choosenDate.year() == initialDate.addMonths(k).year()
                    && choosenDate.month() == initialDate.addMonths(k).month()){
                int currentRow = ui->tablePricing->rowCount();
                ui->tablePricing->insertRow(currentRow);
                ui->tablePricing->setItem(currentRow, 0, new QTableWidgetItem(paymentData[i].at(0)));
                ui->tablePricing->setItem(currentRow, 1, new QTableWidgetItem(paymentData[i].at(1)));
                ui->tablePricing->setItem(currentRow, 2, new QTableWidgetItem(QString::number(k + 1) + "/" + paymentData[i].at(4)));

                double price = 0;
                int discount = QString(paymentData[i].at(2)).toInt();
                for (int j = 0; j < courseDataSize; ++j){
                    QString courseSyntesis = courseData[j].at(0) + " [ " + courseData[j].at(4) + " ] - " + courseData[j].at(2)
                                    + tr(" * starts on: ") + courseData[j].at(3);
                    if (paymentData[i].at(1) == courseSyntesis){
                        price = QString(courseData[j].at(1)).toDouble();
                        break;
                    }
                }

                totalIntegral += (price / installments);
                totalWDiscount += ((price / installments) * (1 - (discount/100.0f)));

                ui->tablePricing->setItem(currentRow, 3, new QTableWidgetItem(QString("%1").arg((double)((price / installments) * (1 - (discount/100.0f))), 0, 'f', 2)));
                ui->tablePricing->setItem(currentRow, 4, new QTableWidgetItem(QString("%1").arg((price / installments), 0, 'f', 2)));
                break;
            }
        }
    }

    ui->lblReceiptWithDiscount->setText(QString("%1").arg(totalWDiscount, 0, 'f', 2));
    ui->lblReceiptWithoutDiscount->setText(QString("%1").arg(totalIntegral, 0, 'f', 2));
}

void frmReceipt::exportCSV(){
    QFileDialog saveDialog;
    saveDialog.setDirectory(QDir::homePath());
    saveDialog.setWindowTitle(tr("Export receipt to CSV | SmartClass"));
    saveDialog.setConfirmOverwrite(true);
    saveDialog.selectFile(tr("SmartClass - receipt_") + QDate::currentDate().toString("dd_MM_yyyy"));
    saveDialog.setDefaultSuffix("csv");
    saveDialog.setNameFilter(tr("Comma Separated Values files (*.csv)"));
    saveDialog.setAcceptMode(QFileDialog::AcceptSave);

    if (saveDialog.exec()){
        QString savePath = saveDialog.selectedFiles().at(0);
        if (QFile::exists(savePath)) QFile::remove(savePath);

        QString data = tr("Monthy income description\n\n")
                        + ui->lblDateDesc->text() + ";" + ui->cbCustomMonthNYear->currentText() + "\n"
                        + ui->lblReceiptWithDiscountDesc->text() + ";" + ui->lblReceiptWithDiscount->text() + "\n"
                        + ui->lblReceiptWithoutDiscountDesc->text() + ";" + ui->lblReceiptWithoutDiscount->text() + "\n\n"
                        + tr("Student") + ";" + tr("Course") + ";" + tr("Installment") + ";" + tr("Price (with discount)") + ";" + tr("Price (integral)") + "\n";

        for (int i = 0; i < ui->tablePricing->rowCount(); ++i){
            data += (ui->tablePricing->item(i, 0)->text() + ";"
                     + ui->tablePricing->item(i, 1)->text() + ";"
                     + ui->tablePricing->item(i, 2)->text().replace("/", "|") + ";"
                     + ui->tablePricing->item(i, 3)->text() + ";"
                     + ui->tablePricing->item(i, 4)->text() + "\n");
        }

        data += tr("\n\nSmartClass autogenerated data");

        QFile saveFile(savePath);
        saveFile.open(QIODevice::WriteOnly);
        saveFile.write(data.toLocal8Bit());
        saveFile.close();
    }
}

/*
 * GUI Functions (don't change, unless necessary)
 */

void frmReceipt::mousePressEvent(QMouseEvent *event)
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

void frmReceipt::undefMouseMoveEvent(QObject* object, QMouseEvent* event){
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

void frmReceipt::mouseReleaseEvent(QMouseEvent *event){
    locked = LockMoveType::None;
    event->accept();
}

bool frmReceipt::eventFilter(QObject* object, QEvent* event)
{
    if(event->type() == QEvent::MouseMove)
        undefMouseMoveEvent(object, static_cast<QMouseEvent*>(event));
    else if (event->type() == QEvent::MouseButtonPress && object->objectName() == "titleBar"){
        mousePressEvent(static_cast<QMouseEvent*>(event));
    }
    return false;
}
