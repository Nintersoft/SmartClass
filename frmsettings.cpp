#include "frmsettings.h"
#include "ui_frmsettings.h"

frmSettings::frmSettings(QWidget *parent, SmartClassGlobal::UserRoles role, OpenMode mode) :
    NMainWindow(parent),
    ui(new Ui::frmSettings),
    programSettings("Nintersoft", "SmartClass"),
    CURRENT_ROLE(role)
{
    ui->setupUi(this);

    // Sets the custom Widgets on the parent Class
    // Otherwise, the window resizing feature will not work
    NMainWindow::setCustomWidgets(ui->centralWidget, ui->statusBar);
    this->setMaximizeButtonEnabled(false);

    this->setGeometry(QStyle::alignedRect(Qt::LeftToRight, Qt::AlignCenter,
            this->size(), qApp->desktop()->availableGeometry()));

    /*
     *  End of GUI implementation
     */

    db_manager = DBManager::getInstance();

    this->db_manager = db_manager;

    if (mode == frmSettings::Info) ui->tabWidget->setCurrentIndex(2);
    this->setMaximumSize(this->size());
    this->setMinimumSize(this->size());

    if (CURRENT_ROLE != SmartClassGlobal::ADMIN){
        ui->grpContract->setEnabled(false);
        ui->grpBranding->setEnabled(false);
        ui->grpThirdParty->setEnabled(false);
        ui->grpBackupSettings->setEnabled(false);
    }

    ui->lblScheduleTime->setVisible(false);
    ui->edtSchedule->setVisible(false);
    ui->btAddSchedule->setVisible(false);
    ui->btRemoveSchedule->setVisible(false);
    ui->listSchedules->setVisible(false);

    ui->edtBackupPath->setText(QDir::homePath() + "/.SmartClassBKP/");

    connect(ui->btSearchBackupPath, SIGNAL(clicked(bool)), this, SLOT(selectBackupPath()));
    connect(ui->btSearchCompanyLogo, SIGNAL(clicked(bool)), this, SLOT(selectNewLogo()));
    connect(ui->btSearchExternalProgram, SIGNAL(clicked(bool)), this, SLOT(selectExternalProgram()));

    connect(ui->btClArgsHelper, SIGNAL(clicked(bool)), this, SLOT(openExternalProgramHelper()));

    connect(ui->btAddSchedule, SIGNAL(clicked(bool)), this, SLOT(addSchedule()));
    connect(ui->btRemoveSchedule, SIGNAL(clicked(bool)), this, SLOT(removeSchedule()));

    connect(ui->cbEnableDBBackup, SIGNAL(toggled(bool)), ui->rbScheduleInterval, SLOT(setEnabled(bool)));
    connect(ui->cbEnableDBBackup, SIGNAL(toggled(bool)), ui->rbSheduleTime, SLOT(setEnabled(bool)));
    connect(ui->cbEnableDBBackup, SIGNAL(toggled(bool)), ui->lblScheduleTime, SLOT(setEnabled(bool)));
    connect(ui->cbEnableDBBackup, SIGNAL(toggled(bool)), ui->edtSchedule, SLOT(setEnabled(bool)));
    connect(ui->cbEnableDBBackup, SIGNAL(toggled(bool)), ui->btAddSchedule, SLOT(setEnabled(bool)));
    connect(ui->cbEnableDBBackup, SIGNAL(toggled(bool)), ui->listSchedules, SLOT(setEnabled(bool)));
    connect(ui->cbEnableDBBackup, SIGNAL(toggled(bool)), ui->lblBackupInterval, SLOT(setEnabled(bool)));
    connect(ui->cbEnableDBBackup, SIGNAL(toggled(bool)), ui->cbBackupInterval, SLOT(setEnabled(bool)));
    connect(ui->cbEnableDBBackup, SIGNAL(toggled(bool)), ui->lblBackupIntervalDesc, SLOT(setEnabled(bool)));
    connect(ui->cbEnableDBBackup, SIGNAL(toggled(bool)), ui->lblBackupPath, SLOT(setEnabled(bool)));
    connect(ui->cbEnableDBBackup, SIGNAL(toggled(bool)), ui->edtBackupPath, SLOT(setEnabled(bool)));
    connect(ui->cbEnableDBBackup, SIGNAL(toggled(bool)), ui->btSearchBackupPath, SLOT(setEnabled(bool)));

    connect(ui->cbUseExternalProgram, SIGNAL(toggled(bool)), ui->edtExternalProgramPath, SLOT(setEnabled(bool)));
    connect(ui->cbUseExternalProgram, SIGNAL(toggled(bool)), ui->lblExternalProgramPath, SLOT(setEnabled(bool)));
    connect(ui->cbUseExternalProgram, SIGNAL(toggled(bool)), ui->btSearchExternalProgram, SLOT(setEnabled(bool)));
    connect(ui->cbUseExternalProgram, SIGNAL(toggled(bool)), ui->lblClArgs, SLOT(setEnabled(bool)));
    connect(ui->cbUseExternalProgram, SIGNAL(toggled(bool)), ui->btClArgsHelper, SLOT(setEnabled(bool)));
    connect(ui->cbUseExternalProgram, SIGNAL(toggled(bool)), ui->edtClArgs, SLOT(setEnabled(bool)));

    connect(ui->rbScheduleInterval, SIGNAL(toggled(bool)), ui->lblBackupInterval, SLOT(setVisible(bool)));
    connect(ui->rbScheduleInterval, SIGNAL(toggled(bool)), ui->lblBackupIntervalDesc, SLOT(setVisible(bool)));
    connect(ui->rbScheduleInterval, SIGNAL(toggled(bool)), ui->cbBackupInterval, SLOT(setVisible(bool)));

    connect(ui->rbScheduleInterval, SIGNAL(toggled(bool)), ui->lblScheduleTime, SLOT(setHidden(bool)));
    connect(ui->rbScheduleInterval, SIGNAL(toggled(bool)), ui->edtSchedule, SLOT(setHidden(bool)));
    connect(ui->rbScheduleInterval, SIGNAL(toggled(bool)), ui->btAddSchedule, SLOT(setHidden(bool)));
    connect(ui->rbScheduleInterval, SIGNAL(toggled(bool)), ui->btRemoveSchedule, SLOT(setHidden(bool)));
    connect(ui->rbScheduleInterval, SIGNAL(toggled(bool)), ui->listSchedules, SLOT(setHidden(bool)));

    retrieveSettings();
    connect(ui->btSaveSettings, SIGNAL(clicked(bool)), this, SLOT(saveOptions()));
    connect(ui->btResetSettings, SIGNAL(clicked(bool)), this, SLOT(resetSettings()));
    connect(ui->btCancel, SIGNAL(clicked(bool)), this, SLOT(close()));

    langSettings = ui->cbLanguageSelector->currentIndex();
}

frmSettings::~frmSettings()
{
    delete ui;
}

void frmSettings::retrieveSettings(){
    gSettings = db_manager->retrieveAll(SmartClassGlobal::getTableName(SmartClassGlobal::SETTINGS),
                                        SmartClassGlobal::getTableAliases(SmartClassGlobal::SETTINGS));
    if (gSettings.size()){
        if (gSettings.at(0).size()){
            if (gSettings[0][0].isValid() && !gSettings[0][0].isNull())
                ui->edtCompanyName->setText(gSettings[0][0].toString());
            else ui->edtCompanyName->setText(tr("Nintersoft Team"));

            if (gSettings[0][1].isValid() && !gSettings[0][1].isNull())
                ui->edtContractText->setPlainText(gSettings[0][1].toString());
            else ui->edtContractText->setPlainText(tr("I, %1, owner of the ID %2 and CPG %3,"
                                                      " hereby confirm that I am enrolling %4, who is under my tutelage, on the following course:"
                                                      "\n"
                                                      "\n - %5."
                                                      "\n"
                                                      "\nI am aware that this course is going to be ministred at %6."));

            if (gSettings[0][2].isValid() && !gSettings[0][2].isNull()){
                defLogo = DBManager::variantToPixmap(gSettings[0][2]);
                ui->lblLogoImage->setPixmap(defLogo);
            }
        }
    }

    if (programSettings.childGroups().contains("language options")){
        programSettings.beginGroup("language options");
        ui->cbLanguageSelector->setCurrentIndex(programSettings.value("current language index", 0).toInt());
        programSettings.endGroup();
    }
    if (programSettings.childGroups().contains("backup settings")){
        programSettings.beginGroup("backup settings");
        ui->cbEnableDBBackup->setChecked(programSettings.value("enable backup", true).toBool());
        if (programSettings.value("backup type").toInt() == 1) ui->rbScheduleInterval->setChecked(true);
        else ui->rbSheduleTime->setChecked(true);
        ui->cbBackupInterval->setCurrentIndex(programSettings.value("backup interval index", 7).toInt());

        QStringList scheduledTimes = programSettings.value("scheduled backups", "").toString().split(';', QString::SkipEmptyParts);
        if (!scheduledTimes.isEmpty()){
            ui->listSchedules->addItems(scheduledTimes);
            ui->btRemoveSchedule->setEnabled(true);
        }

        QString backupPath = QDir::homePath() + QDir::separator() + ".SmartClassBKP" + QDir::separator();
        ui->edtBackupPath->setText(programSettings.value("backup path", backupPath).toString());
        programSettings.endGroup();
    }
    if (programSettings.childGroups().contains("external tools")){
        programSettings.beginGroup("external tools");
        ui->cbUseExternalProgram->setChecked(programSettings.value("use external tool", false).toBool());
        ui->edtExternalProgramPath->setText(programSettings.value("program path", "").toString());
        ui->edtClArgs->setPlainText(programSettings.value("command", tr("$prog_path $s_name $s_birthday $s_id $s_school $s_experimental_course $s_experimental_date $s_parent $p_telephone $p_mobile $p_email $p_id $p_cpg $s_course $p_address $p_cost $p_discount $p_installments")).toString());
        programSettings.endGroup();
    }
    if (programSettings.childGroups().contains("dbinfo")){
        programSettings.beginGroup("dbinfo");
        ui->lblDbType->setText(programSettings.value("database type", "-").toString());
        if (ui->lblDbType->text() != "SQLITE"){
            ui->lblDBName->setText(programSettings.value("database", "-").toString());
            ui->lblDBHost->setText(programSettings.value("host", "-").toString());
            ui->lblDBPort->setText(programSettings.value("port", "-").toString());
            ui->lblDBUsername->setText(programSettings.value("username", "-").toString());
            ui->lblDBPassword->setText(programSettings.value("password", "-").toString());
        }
        else ui->lblBackupUnavailable->setVisible(false);
        ui->lblDBPrefix->setText(programSettings.value("table prefix", "-").toString());
        programSettings.endGroup();
    }
}

void frmSettings::resetSettings(){
    if (QMessageBox::question(NULL, tr("Confirmation | SmartClass"), tr("You are going to reset the settings to the previously saved"
                                                                        " state.\nAre you sure?"), QMessageBox::Yes, QMessageBox::No)
            == QMessageBox::Yes){
        retrieveSettings();
    }
}

void frmSettings::saveOptions(){
    QVariantList globalSettings;
    globalSettings << (ui->edtCompanyName->text().isEmpty() ? tr("Nintersoft Team") : ui->edtCompanyName->text())
                   << ui->edtContractText->toPlainText();

    if (!ui->edtCompanyLogo->text().isEmpty() && QFile::exists(ui->edtCompanyLogo->text()))
        globalSettings << DBManager::pixmapToVariant(QPixmap(ui->edtCompanyLogo->text()));
    else if (!defLogo.isNull())
        globalSettings << DBManager::pixmapToVariant(defLogo);
    else globalSettings << QVariant();

    if (gSettings.size() && gSettings[0].size())
        db_manager->updateRow(SmartClassGlobal::getTableName(SmartClassGlobal::SETTINGS),
                              SmartClassGlobal::getTableAliases(SmartClassGlobal::SETTINGS).at(0),
                              gSettings[0][0],
                              SmartClassGlobal::getTableAliases(SmartClassGlobal::SETTINGS),
                              globalSettings);
    else db_manager->insertRow(SmartClassGlobal::getTableName(SmartClassGlobal::SETTINGS),
                               SmartClassGlobal::getTableAliases(SmartClassGlobal::SETTINGS),
                               globalSettings);

    programSettings.beginGroup("language options");
    programSettings.setValue("current language index", ui->cbLanguageSelector->currentIndex());
    programSettings.endGroup();

    programSettings.beginGroup("backup settings");
    programSettings.setValue("enable backup", ui->cbEnableDBBackup->isChecked());
    programSettings.setValue("backup type", ui->rbScheduleInterval->isChecked() ? 1 : 2);
    programSettings.setValue("backup interval index", ui->cbBackupInterval->currentIndex());
    QStringList scheduledTimes;
    for (int i = 0; i < ui->listSchedules->count(); ++i)
        scheduledTimes << ui->listSchedules->item(i)->text();
    programSettings.setValue("scheduled backups", scheduledTimes.join(';'));
    programSettings.setValue("backup path", ui->edtBackupPath->text());
    programSettings.endGroup();

    programSettings.beginGroup("external tools");
    programSettings.setValue("use external tool", ui->cbUseExternalProgram->isChecked());
    programSettings.setValue("program path", ui->edtExternalProgramPath->text());
    programSettings.setValue("command", ui->edtClArgs->toPlainText().isEmpty() ?
                                 tr("$prog_path $s_name $s_birthday $s_id $s_school $s_experimental_course $s_experimental_date $s_address $s_parent $p_telephone $p_mobile $p_email $p_id $p_cpg $s_course $p_cost $p_discount $p_installments") :
                                 ui->edtClArgs->toPlainText());
    programSettings.endGroup();

    if (langSettings != ui->cbLanguageSelector->currentIndex())
        QMessageBox::information(NULL,
                                 tr("Info | SmartClass"),
                                 tr("You have to restart SmartClass in order to the language change take effect!"),
                                 QMessageBox::Ok, QMessageBox::NoButton);

    this->close();
}

void frmSettings::selectNewLogo(){
    QFileDialog selectLogoDialog;
    selectLogoDialog.setWindowTitle(tr("Select company logo | SmartClass"));
    selectLogoDialog.setAcceptMode(QFileDialog::AcceptOpen);
    selectLogoDialog.setFileMode(QFileDialog::ExistingFile);
    selectLogoDialog.setNameFilters(QStringList() << tr("Image files (*.png *.jpeg *.jpg)") << tr("All files (*.*)"));
    selectLogoDialog.setDirectory(QDir::homePath());
    if (!selectLogoDialog.exec()) return;
    ui->edtCompanyLogo->setText(selectLogoDialog.selectedFiles().at(0));
    ui->lblLogoImage->setPixmap(QPixmap(ui->edtCompanyLogo->text()));
}

void frmSettings::selectBackupPath(){
    QFileDialog selectBackupPathDialog;
    selectBackupPathDialog.setWindowTitle(tr("Select backup path | SmartClass"));
    selectBackupPathDialog.setAcceptMode(QFileDialog::AcceptOpen);
    selectBackupPathDialog.setFileMode(QFileDialog::DirectoryOnly);
    selectBackupPathDialog.setNameFilters(QStringList() << tr("All files (*.*)"));
    selectBackupPathDialog.setDirectory(QDir::homePath());
    if (selectBackupPathDialog.exec()) ui->edtBackupPath->setText(selectBackupPathDialog.selectedFiles().at(0));
}

void frmSettings::selectExternalProgram(){
    QFileDialog selectExternalProgramDialog;
    selectExternalProgramDialog.setWindowTitle(tr("Select company logo | SmartClass"));
    selectExternalProgramDialog.setAcceptMode(QFileDialog::AcceptOpen);
    selectExternalProgramDialog.setNameFilters(QStringList() << tr("Executable files (*.bat *.bin *.cmd *.com *.cpl *csh *.exe *.inx *.ksh *.msi *.out *.paf *.reg *.run *.rgs *.vb *.vbe *.vbs *.vbscript *.ws *.wsf)")
                                               << tr("All files (*.*)"));
    selectExternalProgramDialog.setDirectory(QDir::homePath());
    if (selectExternalProgramDialog.exec()) ui->edtExternalProgramPath->setText(selectExternalProgramDialog.selectedFiles().at(0));
}

void frmSettings::addSchedule(){
    QString time = ui->edtSchedule->time().toString("HH:mm");
    for (int i = 0; i < ui->listSchedules->count(); ++i)
        if (ui->listSchedules->item(i)->text() == time){
            QMessageBox::warning(NULL, tr("Warning | SmartClass"), tr("Sorry, but you cannot schedule two backups to the same period."), QMessageBox::Ok);
            return;
        }
    ui->listSchedules->addItem(time);
    ui->btRemoveSchedule->setEnabled(true);
}

void frmSettings::removeSchedule(){
    int currentRow = ui->listSchedules->currentRow();

    if (currentRow < 0){
        ui->btRemoveSchedule->setEnabled(false);
        return;
    }

    delete ui->listSchedules->item(currentRow);
    if (currentRow > 0){
        ui->listSchedules->setCurrentRow(currentRow - 1);
        return;
    }

    if (ui->listSchedules->count() > 0)
        ui->listSchedules->setCurrentRow(0);
    else
        ui->btRemoveSchedule->setEnabled(false);
}

void frmSettings::openExternalProgramHelper(){
    QMessageBox::information(NULL, tr("External contract program helper | SmartClass"),
                             tr("The following box is the field containing the command line command which will be executed (it supports prefix and suffix words).\n"
                                "This is a list of the special supported arguments:\n\n\n"
                                "$prog_path: Path to the external program/tool.\n"
                                "$s_name: Name of the student.\n"
                                "$s_birthday: Birthday of the student.\n"
                                "$s_id: Student ID.\n"
                                "$s_school: Student school.\n"
                                "$s_experimental_course: Name of the experimental course.\n"
                                "$s_experimental_date: Date and time of the experimental course.\n"
                                "$p_address: Parent/responsible address.\n"
                                "$s_parent: Name of the parent/responsible of the student.\n"
                                "$p_telephone: Telephone number of the parent/responsible of the student.\n"
                                "$p_mobile: Mobile phone number of the parent/responsible of the student.\n"
                                "$p_email: Email of the parent/responsible.\n"
                                "$p_id: ID of the parent/responsible.\n"
                                "$p_cpg: CPG of the parent/responsible.\n"
                                "$s_course: Registered course (Student course).\n"
                                "$p_cost: Cost of the selected course.\n"
                                "$p_discount: Discount of the selected course (percentage).\n"
                                "$p_installments: Installments of the selected course (number).\n\n"
                                "WARNING: Be careful when running scripts. The program/command is always the first one in the command line."),
                             QMessageBox::Ok, QMessageBox::NoButton);
}
