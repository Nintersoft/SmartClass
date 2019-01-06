#include "frmprintcontract.h"
#include "ui_frmprintcontract.h"

frmPrintContract::frmPrintContract(QWidget *parent, qlonglong studentID) :
    NMainWindow(parent),
    ui(new Ui::frmPrintContract),
    STUDENT_ID(studentID)
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

    this->db_manager = DBManager::getInstance();

    QSettings mySettings("Nintersoft", "SmartClass");
    if (mySettings.childGroups().contains("external tools")){
        mySettings.beginGroup("external tools");

        programPath = mySettings.value("program path", "").toString();
        ui->btOpenExternal->setEnabled(mySettings.value("use external tool", false).toBool() &&
                                       !programPath.isEmpty());
        if (ui->btOpenExternal->isEnabled()){
            externalCommand = mySettings.value("command",
                                                tr("$prog_path $s_name $s_birthday $s_id $s_school $s_experimental_course  $s_experimental_date $s_address $s_parent $p_telephone $p_mobile $p_email $p_id $p_cpg $s_course $p_cost $p_discount $p_installments")).toString();
        }

        mySettings.endGroup();
    }

    sData = db_manager->retrieveRow(SmartClassGlobal::getTableName(SmartClassGlobal::STUDENT),
                                    SmartClassGlobal::getTableAliases(SmartClassGlobal::STUDENT).mid(0,1),
                                    QVariantList() << STUDENT_ID,
                                    SmartClassGlobal::getTableAliases(SmartClassGlobal::STUDENT));
    rData = db_manager->retrieveRow(SmartClassGlobal::getTableName(SmartClassGlobal::RESPONSIBLE),
                                    SmartClassGlobal::getTableAliases(SmartClassGlobal::RESPONSIBLE).mid(0,1),
                                    QVariantList() << sData.at(2),
                                    SmartClassGlobal::getTableAliases(SmartClassGlobal::RESPONSIBLE));
    QList<QVariantList> settings = db_manager->retrieveAll(SmartClassGlobal::getTableName(SmartClassGlobal::SETTINGS));
    if (settings.size())
        if (settings.at(0).size())
            companyName = settings[0][0].toString();
        else companyName = tr("Nintersoft");
    else companyName = tr("Nintersoft");

    ui->lblStudentName->setText(sData.at(1).toString());
    ui->lblParentName->setText(rData.at(1).toString());
    ui->lblParentID->setText(rData.at(6).toString());
    ui->lblParentCPG->setText(rData.at(7).toString());
    ui->lblCurrentDate->setText(QDate::currentDate().toString(tr("dd/MM/yyyy")));
    ui->edtCompanyName->setText(companyName);

    QList<QVariantList> courseAssociation = db_manager->retrieveAllCond(SmartClassGlobal::getTableName(SmartClassGlobal::COURSEENROLLMENTS),
                                                                        SmartClassGlobal::getTableAliases(SmartClassGlobal::COURSEENROLLMENTS),
                                                                        SmartClassGlobal::getTableAliases(SmartClassGlobal::COURSEENROLLMENTS).at(1),
                                                                        STUDENT_ID);

    for (int i = 0; i < courseAssociation.length(); ++i){
        QVariantList courseData = db_manager->retrieveRow(SmartClassGlobal::getTableName(SmartClassGlobal::COURSEDETAILS),
                                                          SmartClassGlobal::getTableAliases(SmartClassGlobal::COURSEDETAILS).at(0),
                                                          courseAssociation.at(i).at(0));
        QVariantList paymentData = db_manager->retrieveRow(SmartClassGlobal::getTableName(SmartClassGlobal::PAYMENTDETAILS),
                                                           SmartClassGlobal::getTableAliases(SmartClassGlobal::PAYMENTDETAILS).mid(0, 2),
                                                           QVariantList() << STUDENT_ID << courseData.at(0));

        QString courseSyntesis = courseData.at(1).toString() + tr(" ( class #") + courseData.at(5).toString() + tr(" ) - ") + courseData.at(6).toString()
                        + tr(" * starts on: ") + courseData.at(7).toString();
        ui->cbStudentCourse->addItem(courseSyntesis, courseData.at(0));
        cData << courseData;
        pData << paymentData;
    }

    connect(ui->btOpenExternal, SIGNAL(clicked(bool)), this, SLOT(openExternalTool()));
    connect(ui->btGenerateForm, SIGNAL(clicked(bool)), this, SLOT(generateContractForm()));
}

frmPrintContract::~frmPrintContract()
{
    delete ui;
}

void frmPrintContract::openExternalTool(){
    if (ui->cbStudentCourse->currentIndex() == 0){
        QMessageBox::warning(this, tr("Warning | SmartClass"),
                             tr("You have to select a course in order to continue the operation. Otherwise it will not be possible to continue."),
                             QMessageBox::Ok);
        return;
    }
    QProcess externalTool;
    externalTool.start(parseArguments());
}

QString frmPrintContract::parseArguments(){
    QString commandLine = externalCommand;
    int currentLine = ui->cbStudentCourse->currentIndex();

    QString cost = "0.00";
    cost = QString::number(cData[currentLine].at(9).toDouble(), 'f', 2);
    cost.replace('.', tr(","));

    return commandLine.replace(tr("$prog_path"), programPath)
            .replace(tr("$s_name"), "\"" + sData.at(1).toString() + "\"")
            .replace(tr("$s_birthday"), "\"" + sData.at(3).toDate().toString("dd/MM/yyyy") + "\"")
            .replace(tr("$s_id"), "\"" + sData.at(4).toString() + "\"")
            .replace(tr("$s_school"), "\"" + sData.at(5).toString() + "\"")
            .replace(tr("$s_experimental_course"), "\"" + sData.at(7).toString() + "\"")
            .replace(tr("$s_experimental_date"), "\"" + sData.at(8).toDateTime().toString("dd/MM/yyyy - HH:mm") + "\"")
            // Parental details
            .replace(tr("$s_parent"), "\"" + rData.at(1).toString() + "\"")
            .replace(tr("$p_telephone"), "\"" + rData.at(2).toString() + "\"")
            .replace(tr("$p_mobile"), "\"" + rData.at(4).toString() + "\"")
            .replace(tr("$p_email"), "\"" + rData.at(5).toString() + "\"")
            .replace(tr("$p_id"), "\"" + rData.at(6).toString() + "\"")
            .replace(tr("$p_cpg"), "\"" + rData.at(7).toString() + "\"")
            .replace(tr("$s_address"), "\"" + rData.at(9).toString() + "\"")
            // Course and payment details
            .replace(tr("$s_course"), "\"" + ui->cbStudentCourse->currentText() + "\"")
            // Payment details
            .replace(tr("$p_cost"), "\"" + cost + "\"")
            .replace(tr("$p_discount"), pData.size() ? ("\"" + QString::number(pData[currentLine].at(2).toDouble(), 'f', 2).replace('.', tr(",")) + "\"") : "\"\"")
            .replace(tr("$p_installments"), pData.size() ? ("\"" + QString::number(pData[currentLine].at(4).toInt()) + "\"") : "\"\"");
}

void frmPrintContract::generateContractForm(){
    QString errorMsg = tr("Unfortunately we cannot proceed. Some data are either inconsistent or inexistent."
                          " Please, fix the folowing issues before trying to generate the form again:\n");
    bool error = false;

    if (ui->edtCompanyName->text().isEmpty()){
        errorMsg += tr("\n->The name of the companny cannot be empty;");
        error = true;
    }
    if (ui->cbStudentCourse->currentIndex() == 0){
        errorMsg += tr("\n->You must select a course in order to generate the form;");
        error = true;
    }

    if (error){
        QMessageBox::warning(this, tr("Warning | SmartClass"), errorMsg, QMessageBox::Ok, QMessageBox::Ok);
        return;
    }

    QString defaultTemplate = tr("I, %1, owner of the ID %2 and CPG %3,"
                                 " hereby confirm that I am enrolling my son/daugther %4 on the following course:"
                                 "\n"
                                 "\n - %5."
                                 "\n"
                                 "\nI am aware that this course is going to be ministred at %6.");

    QList< QList<QVariant> > settings = db_manager->retrieveAll(SmartClassGlobal::getTableName(SmartClassGlobal::SETTINGS));
    if (settings.size())
        if (settings[0].size())
            if (!settings[0][1].isNull() && settings[0][1].isValid())
                defaultTemplate = settings[0][1].toString();

    defaultTemplate = defaultTemplate.arg(ui->lblParentName->text())
                                     .arg(ui->lblParentID->text())
                                     .arg(ui->lblParentCPG->text())
                                     .arg(ui->lblStudentName->text())
                                     .arg(ui->cbStudentCourse->currentText())
                                     .arg(ui->edtCompanyName->text());

    bool hasImage = ui->cbIncludeCompanyLogo->isChecked() && settings.size();
    if (hasImage) hasImage &= (bool)settings[0].size();
    if (hasImage) hasImage &= (settings[0][2].isNull() && settings[0][2].isValid());
    if (hasImage) frmPrintPrev = new PrintPreviewForm(NULL, QStringList() << defaultTemplate
                                                                            << ui->lblParentName->text()
                                                                            << ui->edtCompanyName->text(),
                                                        db_manager->variantToPixmap(settings[0][2]));
    else frmPrintPrev = new PrintPreviewForm(NULL, QStringList() << defaultTemplate
                                                    << ui->lblParentName->text()
                                                    << ui->edtCompanyName->text());
    frmPrintPrev->showMaximized();
}
