#include "frmmanageuser.h"
#include "ui_frmmanageuser.h"

frmManageUser::frmManageUser(QWidget *parent, const QVariantList &data,
                             const QString &currentUser) :
    NMainWindow(parent),
    ui(new Ui::frmManageUser),
    IS_CURRENT_USER(currentUser == data.at(0).toString())
{
    ui->setupUi(this);

    // Sets the custom Widgets on the parent Class
    // Otherwise, the window resizing feature will not work
    NMainWindow::setCustomWidgets(ui->centralWidget, ui->statusBar);
    NMainWindow::setMaximizeButtonEnabled(false);
    NMainWindow::setMinimizeButtonEnabled(false);

    this->setGeometry(QStyle::alignedRect(Qt::LeftToRight, Qt::AlignCenter,
            this->size(), qApp->desktop()->availableGeometry()));

    /*
     *  End of GUI implementation
     */

    ui->edtUsername->setText(data.at(0).toString());
    ui->edtName->setText(data.at(1).toString());
    ui->edtQuestion->setText(data.at(2).toString());
    ui->cbRole->setCurrentIndex(data.at(3).toInt());

    connect(ui->btSave, SIGNAL(clicked(bool)), this, SLOT(saveData()));
    connect(ui->btPassHelp, SIGNAL(clicked(bool)), this, SLOT(showCHelp()));
    connect(ui->btAnswerHelp, SIGNAL(clicked(bool)), this, SLOT(showCHelp()));
    connect(ui->btCancel, SIGNAL(clicked(bool)), this, SLOT(close()));
}

frmManageUser::~frmManageUser()
{
    delete ui;
}

void frmManageUser::showCHelp(){
    QString arg1 = sender()->objectName() == "btPassHelp" ? tr("password") : tr("answer");
    QMessageBox::information(NULL, tr("Information | SmartClass"), tr("In order to keep the current %1 you just have to leave this field empty.").arg(arg1), QMessageBox::Ok);
}

void frmManageUser::saveData(){
    int trash = 0;
    if (!ui->edtPassword->text().isEmpty() &&
            !frmLogin::isSafePassword(ui->edtPassword->text(), trash)){
        QMessageBox::warning(NULL, tr("Error | SmartClass"), tr("The new password seems to be too weak. It must contain at least one special character, one number, one uppercase letter and a lowercase letter."), QMessageBox::Ok);
        return;
    }

    if (IS_CURRENT_USER && ui->cbRole->currentIndex()){
        QMessageBox::information(NULL, tr("Information | SmartClass"), tr("Due to safety purposes, you are not able to downgrade your own permissions. If you really need to change your permissions, please, ask to another administrator do it."), QMessageBox::Ok);
        return;
    }

    QVariantList newData;
    newData << ui->edtUsername->text()
            << ui->edtName->text()
            << ui->edtPassword->text()
            << ui->edtQuestion->text()
            << ui->edtAnswer->text()
            << ui->cbRole->currentIndex();
    emit sendNewData(newData);
    close();
}
