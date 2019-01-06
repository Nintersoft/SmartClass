#include "printpreviewform.h"
#include "ui_printpreviewform.h"

PrintPreviewForm::PrintPreviewForm(QWidget *parent, const QPixmap &pixmap) :
    NMainWindow(parent),
    ui(new Ui::PrintPreviewForm)
{
    ui->setupUi(this);

    // Sets the custom Widgets on the parent Class
    // Otherwise, the window resizing feature will not work
    NMainWindow::setCustomWidgets(ui->centralWidget, ui->statusBar);

    this->setGeometry(QStyle::alignedRect(Qt::LeftToRight, Qt::AlignCenter,
            this->size(), qApp->desktop()->availableGeometry()));

    /*
     *  End of GUI implementation
     */

    connect(ui->btExportToPdf, SIGNAL(clicked(bool)), this, SLOT(exportToPDF()));
    connect(ui->btPrint, SIGNAL(clicked(bool)), this, SLOT(printDocument()));

    parentName = NULL;
    contract = NULL;
    companyLogo = NULL;
    companyName = NULL;

    QGridLayout *paperLayout = static_cast<QGridLayout*>(ui->paperPreview->layout());

    ui->paperPreview->setMinimumSize(QSize(210 * 3.779527559, 297 * 3.779527559));
    ui->paperPreview->setMaximumSize(QSize(210 * 3.779527559, 297 * 3.779527559));

    if (!pixmap.isNull()){
        int heightDifference, widthDifference;
        heightDifference = ui->paperPreview->geometry().height() - pixmap.height();
        widthDifference = ui->paperPreview->geometry().width() - pixmap.width();

        contract = new QLabel(this);
        if (heightDifference < 0 || widthDifference < 0){
            contract->setPixmap(pixmap.scaled(QSize(210 * 3.779527559 - 30, 297 * 3.779527559 - 30), Qt::KeepAspectRatio, Qt::SmoothTransformation));
            heightDifference = ui->paperPreview->geometry().height() - contract->pixmap()->height();
            widthDifference = ui->paperPreview->geometry().width() - contract->pixmap()->width();
        }
        else contract->setPixmap(pixmap);
        contract->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);

        paperLayout->addWidget(contract, 0, 0);
        contract->show();
    }
}

PrintPreviewForm::PrintPreviewForm(QWidget *parent, const QString &text, const QPixmap &pixmap) :
    NMainWindow(parent),
    ui(new Ui::PrintPreviewForm)
{
    ui->setupUi(this);

    // Sets the custom Widgets on the parent Class
    // Otherwise, the window resizing feature will not work
    NMainWindow::setCustomWidgets(ui->centralWidget, ui->statusBar);

    /*
     *  End of GUI implementation
     */

    connect(ui->btExportToPdf, SIGNAL(clicked(bool)), this, SLOT(exportToPDF()));
    connect(ui->btPrint, SIGNAL(clicked(bool)), this, SLOT(printDocument()));

    parentName = NULL;
    contract = NULL;
    companyLogo = NULL;
    companyName = NULL;

    QGridLayout *paperLayout = static_cast<QGridLayout*>(ui->paperPreview->layout());
    paperLayout->setContentsMargins(20, 30, 20, 30);
    paperLayout->setHorizontalSpacing(10);
    paperLayout->setVerticalSpacing(10);

    ui->paperPreview->setMinimumSize(QSize(210 * 3.779527559, 297 * 3.779527559));
    ui->paperPreview->setMaximumSize(QSize(210 * 3.779527559, 297 * 3.779527559));

    QStringList availableFonts(QFontDatabase().families());

    QFont defaultFont;
    defaultFont.setPointSize(14);
    if (availableFonts.contains("Arial", Qt::CaseInsensitive))
        defaultFont.setFamily("Arial");

    contract = new QLabel(this);
    contract->setText(text);
    contract->setFont(defaultFont);
    contract->setWordWrap(true);

    QString dateStr = QDate::currentDate().toString(tr("dddd, dd of MMMM of yyyy\n"));
    dateStr[0] = dateStr[0].toUpper();

    parentName = new QLabel(this);
    parentName->setText("\n"
                      "\n"
                      "\n"
                      "\n"
                      "______________________________________________________\n"
                      + dateStr);
    parentName->setFont(defaultFont);
    parentName->setWordWrap(true);

    if (!pixmap.isNull()){
        companyLogo = new QLabel(this);
        companyLogo->setPixmap(pixmap.scaled(QSize(400, 200), Qt::KeepAspectRatio, Qt::SmoothTransformation));
        companyLogo->setMaximumHeight(200);
        companyLogo->setMinimumHeight(200);
        companyLogo->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
    }

    if (companyLogo) paperLayout->addWidget(companyLogo, 0, 0);
    paperLayout->addWidget(contract, 1, 0);
    paperLayout->addWidget(parentName, 2, 0);

    contract->show();
    companyLogo->show();
    parentName->show();
}

PrintPreviewForm::PrintPreviewForm(QWidget *parent, const QStringList &content, const QPixmap &pixmap) :
    NMainWindow(parent),
    ui(new Ui::PrintPreviewForm)
{
    ui->setupUi(this);

    // Sets the custom Widgets on the parent Class
    // Otherwise, the window resizing feature will not work
    NMainWindow::setCustomWidgets(ui->centralWidget, ui->statusBar);

    /*
     *  End of GUI implementation
     */

    connect(ui->btExportToPdf, SIGNAL(clicked(bool)), this, SLOT(exportToPDF()));
    connect(ui->btPrint, SIGNAL(clicked(bool)), this, SLOT(printDocument()));

    parentName = NULL;
    contract = NULL;
    companyLogo = NULL;
    companyName = NULL;

    QGridLayout *paperLayout = static_cast<QGridLayout*>(ui->paperPreview->layout());
    paperLayout->setContentsMargins(20, 30, 20, 30);
    paperLayout->setHorizontalSpacing(10);
    paperLayout->setVerticalSpacing(10);

    ui->paperPreview->setMinimumSize(QSize(210 * 3.779527559, 297 * 3.779527559));
    ui->paperPreview->setMaximumSize(QSize(210 * 3.779527559, 297 * 3.779527559));

    QStringList availableFonts(QFontDatabase().families());

    QFont defaultFont;
    defaultFont.setPointSize(14);
    if (availableFonts.contains("Arial", Qt::CaseInsensitive))
        defaultFont.setFamily("Arial");

    if (content.size() >= 2){
        contract = new QLabel(this);
        contract->setText(content[0]);
        contract->setFont(defaultFont);
        contract->setWordWrap(true);

        QString dateStr = QDate::currentDate().toString(tr("dddd, dd of MMMM of yyyy\n"));
        dateStr[0] = dateStr[0].toUpper();

        parentName = new QLabel(this);
        parentName->setText("\n"
                          "\n"
                          "\n"
                          "\n"
                          "______________________________________________________\n"
                          + dateStr
                          + content[1] + "\n");
        parentName->setFont(defaultFont);
        parentName->setWordWrap(true);

        if (!pixmap.isNull()){
            companyLogo = new QLabel(this);
            companyLogo->setPixmap(pixmap.scaled(QSize(400, 200), Qt::KeepAspectRatio, Qt::SmoothTransformation));
            companyLogo->setMaximumHeight(200);
            companyLogo->setMinimumHeight(200);
            companyLogo->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
            paperLayout->addWidget(companyLogo, 0, 0);
        }

        if (content.size() == 3){
            companyName = new QLabel(this);
            companyName ->setText(content[2]);
            companyName ->setFont(defaultFont);
            companyName->setAlignment(Qt::AlignHCenter | Qt::AlignTop);
            if (companyLogo) companyName->setMaximumHeight(100);
            paperLayout->addWidget(companyName, 1, 0);
        }

        paperLayout->addWidget(contract, 2, 0);
        paperLayout->addWidget(parentName, 3, 0);

        if (companyLogo) companyLogo->show();
        if (companyName) companyName->show();
        contract->show();
        parentName->show();
    }
    else {
        contract = new QLabel(this);
        contract->setText(content[0]);
        contract->setFont(defaultFont);
        paperLayout->addWidget(contract, 0, 0);
        contract->show();
    }
}

PrintPreviewForm::~PrintPreviewForm()
{
    if (parentName) delete parentName;
    if (contract) delete contract;
    if (companyName) delete companyName;
    if (companyLogo) delete companyLogo;
    delete ui;
}

void PrintPreviewForm::exportToPDF(){
    QFileDialog saveDialog;
    saveDialog.setDirectory(QDir::homePath());
    saveDialog.setWindowTitle(tr("Export content to PDF | SmartClass"));
    saveDialog.setConfirmOverwrite(true);
    saveDialog.selectFile(tr("SmartClass - document_") + QDate::currentDate().toString("dd_MM_yyyy"));
    saveDialog.setDefaultSuffix("pdf");
    saveDialog.setNameFilter(tr("Portable Document Format files (*.pdf)"));
    saveDialog.setAcceptMode(QFileDialog::AcceptSave);

    if (saveDialog.exec()){
        QString savePath = saveDialog.selectedFiles().at(0);
        if (QFile::exists(savePath)) QFile::remove(savePath);

        QPrinter pdfExporter(QPrinter::ScreenResolution);
        pdfExporter.setOutputFormat(QPrinter::PdfFormat);
        pdfExporter.setPdfVersion(QPrinter::PdfVersion_A1b);
        pdfExporter.setPageSize(QPrinter::A4);
        pdfExporter.setFullPage(true);
        pdfExporter.setPageOrientation(QPageLayout::Portrait);
        pdfExporter.setOutputFileName(savePath);

        QPainter painter(&pdfExporter);
        double xscale = pdfExporter.pageRect().width()/double(ui->paperPreview->width());
        double yscale = pdfExporter.pageRect().height()/double(ui->paperPreview->height());
        painter.scale(xscale, yscale);

        QString ssBackup = ui->paperPreview->styleSheet();
        ui->paperPreview->setStyleSheet("QWidget#paperPreview{"
                                        "    background-color: white;"
                                        "}");
        ui->paperPreview->render(&painter);
        ui->paperPreview->setStyleSheet(ssBackup);

        QDesktopServices::openUrl(QUrl::fromLocalFile(savePath));
    }
}


void PrintPreviewForm::printDocument(){
    QPrinter printer(QPrinter::ScreenResolution);
    printer.setOutputFormat(QPrinter::NativeFormat);
    printer.setPageSize(QPrinter::A4);
    printer.setFullPage(true);
    printer.setPageOrientation(QPageLayout::Portrait);

    QPrintDialog printDialog(&printer);
    printDialog.setWindowTitle(tr("Print Document | SmartClass"));

    if (printDialog.exec() != QPrintDialog::Accepted) return;

    QPainter painter(&printer);
    double xscale = printer.pageRect().width()/double(ui->paperPreview->width());
    double yscale = printer.pageRect().height()/double(ui->paperPreview->height());
    painter.scale(xscale, yscale);

    QString ssBackup = ui->paperPreview->styleSheet();
    ui->paperPreview->setStyleSheet("QWidget#paperPreview{"
                                    "    background-color: white;"
                                    "}");
    ui->paperPreview->render(&painter);
    ui->paperPreview->setStyleSheet(ssBackup);
}
