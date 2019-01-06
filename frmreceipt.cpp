#include "frmreceipt.h"
#include "ui_frmreceipt.h"

frmReceipt::frmReceipt(QWidget *parent) :
    NMainWindow(parent),
    ui(new Ui::frmReceipt)
{
    ui->setupUi(this);

    // Sets the custom Widgets on the parent Class
    // Otherwise, the window resizing feature will not work
    NMainWindow::setCustomWidgets(ui->centralWidget, ui->statusBar);
    NMainWindow::setMaximizeButtonEnabled(false);

    this->setGeometry(QStyle::alignedRect(Qt::LeftToRight, Qt::AlignCenter,
            this->size(), qApp->desktop()->availableGeometry()));

    /*
     *  End of GUI implementation
     */

    totalWDiscount = 0;
    totalIntegral = 0;

    this->db_manager = DBManager::getInstance();

    connect(ui->cbShowTable, SIGNAL(toggled(bool)), this, SLOT(switchTableVisibility(bool)));
    connect(ui->btExport, SIGNAL(clicked(bool)), this, SLOT(exportCSV()));

    ui->lblDate->setText(QDate::currentDate().toString("dd/MM/yyyy"));

    for (int i = -24; i < 25; ++i){
        QDate nDate = QDate::currentDate().addMonths(i);
        ui->cbCustomMonthNYear->addItem(nDate.toString("MM/yyyy"), QVariant(nDate));
    }
    ui->cbCustomMonthNYear->setCurrentIndex(24);

    QString courseDetailsName = SmartClassGlobal::getTableName(SmartClassGlobal::COURSEDETAILS, true),
            studentInfoName = SmartClassGlobal::getTableName(SmartClassGlobal::STUDENT, true),
            paymentInfoName = SmartClassGlobal::getTableName(SmartClassGlobal::PAYMENTDETAILS, true);

    QStringList courseDetailsAliases = SmartClassGlobal::getTableAliases(SmartClassGlobal::COURSEDETAILS),
                studentInfoAliases = SmartClassGlobal::getTableAliases(SmartClassGlobal::STUDENT),
                paymentInfoAliases = SmartClassGlobal::getTableAliases(SmartClassGlobal::PAYMENTDETAILS);

    QString joinTable = SmartClassGlobal::getTableName(SmartClassGlobal::PAYMENTDETAILS) + " INNER JOIN "
            + studentInfoName + " ON " + paymentInfoName + "." + paymentInfoAliases.at(0) + " = " + studentInfoName + "." + studentInfoAliases.at(0)
            + " INNER JOIN " + courseDetailsName + " ON " + paymentInfoName + "." + paymentInfoAliases.at(1) + " = " + courseDetailsName + "." + courseDetailsAliases.at(0);

    columns << (studentInfoName + "." + studentInfoAliases.at(1))
            << (courseDetailsName + "." + courseDetailsAliases.at(1))
            << (courseDetailsName + "." + courseDetailsAliases.at(5))
            << (courseDetailsName + "." + courseDetailsAliases.at(6))
            << (courseDetailsName + "." + courseDetailsAliases.at(7))
            << (courseDetailsName + "." + courseDetailsAliases.at(9));

    for (int i = 2; i < paymentInfoAliases.size(); ++i)
        columns << (paymentInfoName + "." + paymentInfoAliases.at(i));

    pData = db_manager->retrieveAll(joinTable, columns);

    connect(ui->cbCustomMonthNYear, SIGNAL(currentIndexChanged(int)), this, SLOT(generateTable()));
    generateTable();
}

frmReceipt::~frmReceipt()
{
    delete ui;
}

void frmReceipt::switchTableVisibility(bool show){
    ui->tablePricing->setVisible(show);
    int height = 600;
    if (!show) height = 229;
    this->setMaximumHeight(height);
    this->setMinimumHeight(height);
    this->setGeometry(QRect(this->geometry().topLeft(), QSize(this->width(), this->height())));
    this->update();
}

void frmReceipt::generateTable(){
    if (db_manager == NULL) return;

    totalWDiscount = 0;
    totalIntegral = 0;

    ui->tablePricing->clearContents();
    ui->tablePricing->setRowCount(0);

    for (int i = 0; i < pData.size(); ++i){
        int installments = pData[i].at(8).toInt();

        QDate choosenDate = ui->cbCustomMonthNYear->currentData(Qt::UserRole).toDate();
        QDate initialDate = pData[i].at(7).toDate();
        QDate finalDate = initialDate.addMonths(installments - 1);

        if (choosenDate.year() < initialDate.year() || finalDate.year() < choosenDate.year()
                || (choosenDate.year() == initialDate.year() && choosenDate.month() < initialDate.month())
                || (choosenDate.year() == finalDate.year() && choosenDate.month() > finalDate.month())) continue;


        int currentRow = ui->tablePricing->rowCount();
        ui->tablePricing->insertRow(currentRow);

        ui->tablePricing->setItem(currentRow, 0, new QTableWidgetItem(pData[i].at(0).toString()));
        QString courseSyntesis = pData[i].at(1).toString() + tr(" ( class #") + pData[i].at(2).toString() + tr(" ) - ") + pData[i].at(3).toString()
                        + tr(" * starts on: ") + pData[i].at(4).toString();
        ui->tablePricing->setItem(currentRow, 1, new QTableWidgetItem(courseSyntesis));
        ui->tablePricing->setItem(currentRow, 2, new QTableWidgetItem(QString("%1/%2").arg(choosenDate.month() - initialDate.month() + 1).arg(installments)));

        double price = pData[i].at(5).toDouble();
        double discount = pData[i].at(6).toDouble();

        totalIntegral += (price / installments);
        totalWDiscount += ((price / installments) * (1 - (discount/100.0f)));

        ui->tablePricing->setItem(currentRow, 3, new QTableWidgetItem(QString("%1").arg((double)((price / installments) * (1 - (discount/100.0f))), 0, 'f', 2)));
        ui->tablePricing->setItem(currentRow, 4, new QTableWidgetItem(QString("%1").arg((price / installments), 0, 'f', 2)));
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
                     + ui->tablePricing->item(i, 2)->text() + ";"
                     //+ ui->tablePricing->item(i, 2)->text().replace("/", "|") + ";"
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
