#include "frmsettings.h"
#include "ui_frmsettings.h"

frmSettings::frmSettings(QWidget *parent, OpenMode mode) :
    QMainWindow(parent),
    ui(new Ui::frmSettings),
    RESIZE_LIMIT(2),
    programSettings("Nintersoft", "SmartClass")
{
    ui->setupUi(this);
    setWindowFlags(Qt::Widget | Qt::FramelessWindowHint);
    centralWidget()->installEventFilter(this);
    ui->titleBar->installEventFilter(this);
    ui->statusBar->installEventFilter(this);

    centralWidget()->setMouseTracking(true);
    ui->titleBar->setMouseTracking(true);
    ui->statusBar->setMouseTracking(true);

    setWindowTitle(tr("Settings | SmartClass"));
    locked = LockMoveType::None;

    ui->titleBar->setMaximizeButtonEnabled(false);

    this->setGeometry(QStyle::alignedRect(Qt::LeftToRight, Qt::AlignCenter,
            this->size(), qApp->desktop()->availableGeometry()));
    /*
     *  End of GUI implementation
     */

    if (mode == frmSettings::Info) ui->tabWidget->setCurrentIndex(2);
    this->setMaximumSize(this->size());
    this->setMinimumSize(this->size());

    ui->lblScheduleTime->setVisible(false);
    ui->edtSchedule->setVisible(false);
    ui->btAddSchedule->setVisible(false);
    ui->btRemoveSchedule->setVisible(false);
    ui->listSchedules->setVisible(false);

    ui->edtBackupPath->setText(QDir::homePath());

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

}

frmSettings::~frmSettings()
{
    delete ui;
}

void frmSettings::retrieveSettings(){
    if (programSettings.childGroups().contains("contract form")){
        programSettings.beginGroup("contract form");
        ui->edtContractText->setPlainText(programSettings.value("text", "").toString());
        programSettings.endGroup();
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
        ui->cbBackupInterval->setCurrentIndex(programSettings.value("backup interval index").toInt());

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
        ui->edtClArgs->setPlainText(programSettings.value("command", tr("$prog_path $s_name $s_birthday $s_id $s_school $s_experimental_course  $s_experimental_date $s_parent $p_telephone $p_mobile $p_email $p_id $p_cpg $s_course $p_address $p_cost $p_discount $p_installments")).toString());
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
    if (programSettings.childGroups().contains("company info")){
        programSettings.beginGroup("company info");

        QString logoPath  = QDir::homePath() + ((QSysInfo::windowsVersion() != QSysInfo::WV_None) ?
                    "/AppData/Roaming/Nintersoft/SmartClass/images/" :
                    "/.Nintersoft/SmartClass/images/");
        if (!QDir(logoPath).exists()) QDir(logoPath).mkpath(logoPath);
        logoPath += "Logo.png";
        if (QFile::exists(logoPath)) ui->lblLogoImage->setPixmap(QPixmap(logoPath));

        ui->edtCompanyName->setText(programSettings.value("name", tr("Nintersoft Team")).toString());
        programSettings.endGroup();
    }
}

void frmSettings::resetSettings(){
    if (QMessageBox::question(this, tr("Confirmation | SmartClass"), tr("You are going to reset the settings to the previously saved"
                                                                        " state.\nAre you sure?"), QMessageBox::Yes, QMessageBox::No)
            == QMessageBox::Yes){
        retrieveSettings();
    }
}

void frmSettings::saveOptions(){
    programSettings.beginGroup("contract form");
    programSettings.setValue("text", ui->edtContractText->toPlainText().isEmpty() ? tr("") : ui->edtContractText->toPlainText());
    programSettings.endGroup();

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
                                 tr("$prog_path $s_name $s_birthday $s_id $s_school $s_experimental_course  $s_experimental_date $s_parent $p_telephone $p_mobile $p_email $p_id $p_cpg $s_course $p_address $p_cost $p_discount $p_installments") :
                                 ui->edtClArgs->toPlainText());
    programSettings.endGroup();

    programSettings.beginGroup("company info");
    programSettings.setValue("name", ui->edtCompanyName->text().isEmpty() ? tr("Nintersoft Team") : ui->edtCompanyLogo->text());

    QString logoPath  = QDir::homePath() + ((QSysInfo::windowsVersion() != QSysInfo::WV_None) ?
                "/AppData/Roaming/Nintersoft/SmartClass/images/" :
                "/.Nintersoft/SmartClass/images/");
    if (!QDir(logoPath).exists()) QDir(logoPath).mkpath(logoPath);
    logoPath += "Logo.png";
    if (!ui->edtCompanyLogo->text().isEmpty() && QFile::exists(ui->edtCompanyLogo->text())){
        QFile::remove(logoPath);
        QFile::copy(ui->edtCompanyLogo->text(), logoPath);
    }
    programSettings.endGroup();

    this->close();
}

void frmSettings::selectNewLogo(){
    QFileDialog selectLogoDialog;
    selectLogoDialog.setWindowTitle(tr("Select company logo | SmartClass"));
    selectLogoDialog.setAcceptMode(QFileDialog::AcceptOpen);
    selectLogoDialog.setNameFilters(QStringList() << tr("Image files (*.png *.jpeg *.jpg)") << tr("All files (*.*)"));
    selectLogoDialog.setDirectory(QDir::homePath());
    if (selectLogoDialog.exec()) ui->edtCompanyLogo->setText(selectLogoDialog.selectedFiles().at(0));
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
            QMessageBox::warning(this, tr("Warning | SmartClass"), tr("Sorry, but you cannot schedule two backups to the same period."), QMessageBox::Ok);
            return;
        }
    ui->listSchedules->addItem(time);
    ui->btRemoveSchedule->setEnabled(true);
}

void frmSettings::removeSchedule(){
    int currentRow = ui->listSchedules->currentRow();
    if (currentRow < 0) return;

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
    QMessageBox::information(this, tr("External contract program helper | SmartClass"),
                             tr("The following box is the field containing the command line command which will be executed (it supports prefix and sulfix words).\n"
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
                                "Be careful when running scripts. The program/command is always the first one in the command line."),
                             QMessageBox::Ok, QMessageBox::NoButton);
}

/*
 * GUI Functions (don't change, unless necessary)
 */

void frmSettings::mousePressEvent(QMouseEvent *event)
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

void frmSettings::undefMouseMoveEvent(QObject* object, QMouseEvent* event){
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

void frmSettings::mouseReleaseEvent(QMouseEvent *event){
    locked = LockMoveType::None;
    event->accept();
}

bool frmSettings::eventFilter(QObject* object, QEvent* event)
{
    if(event->type() == QEvent::MouseMove)
        undefMouseMoveEvent(object, static_cast<QMouseEvent*>(event));
    else if (event->type() == QEvent::MouseButtonPress && object->objectName() == "titleBar"){
        mousePressEvent(static_cast<QMouseEvent*>(event));
    }
    return false;
}
