#include "frmmanageusers.h"
#include "ui_frmmanageusers.h"

frmManageUsers::frmManageUsers(QWidget *parent, DBManager *dbManager, const QString &currentUser) :
    NMainWindow(parent),
    ui(new Ui::frmManageUsers),
    CURRENT_USER(currentUser)
{
    ui->setupUi(this);

    // Sets the custom Widgets on the parent Class
    // Otherwise, the window resizing feature will not work
    NMainWindow::setCustomWidgets(ui->centralWidget, ui->statusBar);
    NMainWindow::setMaximizeButtonEnabled(false);
    NMainWindow::setMinimizeButtonEnabled(false);

    /*
     *  End of GUI implementation
     */

    userForm = NULL;

    this->dbManager = dbManager;
    if (dbManager){
        userDBData = dbManager->retrieveAll(SmartClassGlobal::getTableName(SmartClassGlobal::USERS),
                                            SmartClassGlobal::getTableAliases(SmartClassGlobal::USERS));
        for (int i = 0; i < userDBData.length(); ++i){
            QListWidgetItem *newItem = new QListWidgetItem(QIcon(":/images/buttons/User.PNG"),
                                                           userDBData.at(i).at(2).toString() + "[" + userDBData.at(i).at(1).toString() + "]");
            newItem->setData(Qt::UserRole, userDBData.at(i).at(0));
            ui->lwUsers->addItem(newItem);
        }
    }

    connect(ui->btExit, SIGNAL(clicked(bool)), this, SLOT(close()));
    connect(ui->btManageUser, SIGNAL(clicked(bool)), this, SLOT(openUserManager()));
    connect(ui->btRemoveUser, SIGNAL(clicked(bool)), this, SLOT(removeUser()));
    connect(ui->lwUsers, SIGNAL(currentRowChanged(int)), this, SLOT(setToolsEnabled(int)));
}

frmManageUsers::~frmManageUsers()
{
    ui->lwUsers->clear();
    delete ui;
}

void frmManageUsers::setToolsEnabled(int row){
    bool enable = row >= 0;

    ui->btRemoveUser->setEnabled(enable);
    ui->btManageUser->setEnabled(enable);
}

void frmManageUsers::removeUser(){
    int currentRow = ui->lwUsers->currentRow();
    QListWidgetItem* item = ui->lwUsers->item(currentRow);

    if (item->text().contains(CURRENT_USER)){
        QMessageBox::information(this, tr("Information | SmartClass"), tr("Due to safety purposes, you are not able to remove yourself. If you really need to remove your account, please, ask to another administrator do it."), QMessageBox::Ok);
        return;
    }

    //QString username = QString(item->text().split(" [ ").at(1)).split(" ] ").at(0);
    if (dbManager->removeRow(SmartClassGlobal::getTableName(SmartClassGlobal::USERS),
                             SmartClassGlobal::getTableAliases(SmartClassGlobal::USERS).at(0),
                             item->data(Qt::UserRole))){
        dbManager->removeRow(SmartClassGlobal::getTableName(SmartClassGlobal::ACTIVECONNECTIONS),
                             SmartClassGlobal::getTableAliases(SmartClassGlobal::ACTIVECONNECTIONS).at(0),
                             item->data(Qt::UserRole));
        userDBData.removeAt(currentRow);
        QMessageBox::information(this, tr("Information | SmartClass"), tr("The user %1 has been removed successfully.").arg(item->text().split("[").at(1).split("]").at(0)), QMessageBox::Ok);
        delete item;
        item = NULL;
        ui->lwUsers->setCurrentRow(currentRow - 1);
    }
    else QMessageBox::information(this, tr("Information | SmartClass"), tr("An error has ocurred during the attempt of user removal operation. Try again later (after restarting SmartClass)."), QMessageBox::Ok);
}

void frmManageUsers::openUserManager(){
    if (userForm){
        disconnect(userForm, SIGNAL(sendNewData(QStringList)), this, SLOT(getModifiedData(QStringList)));
        delete userForm;
    }

    int currentRow = ui->lwUsers->currentRow();
    currentID = ui->lwUsers->item(currentRow)->data(Qt::UserRole);

    QList<QVariant> exportData;
    exportData << userDBData.at(currentRow).at(1)
               << userDBData.at(currentRow).at(2)
               << userDBData.at(currentRow).at(5)
               << userDBData.at(currentRow).at(8);

    userForm = new frmManageUser(this, exportData, CURRENT_USER);
    connect(userForm, SIGNAL(sendNewData(QStringList)), this, SLOT(getModifiedData(QStringList)));
    userForm->show();
}

void frmManageUsers::getModifiedData(const QList<QVariant> newData){
    int currentRow = ui->lwUsers->currentRow();

    QVariantList currentRData = userDBData.at(currentRow).mid(1);
    currentRData[0] = newData.at(0);
    currentRData[1] = newData.at(1);
    currentRData[4] = newData.at(3);
    currentRData[7] = newData.at(5);

    if (!newData.at(2).toString().isEmpty()){
        currentRData[2] = frmLogin::randomSalt(25);
        currentRData[3] = frmLogin::getHash(currentRData.at(2).toString() + newData.at(2).toString());
    }
    if (!newData.at(5).toString().isEmpty()){
        currentRData[5] = frmLogin::randomSalt(25);
        currentRData[6] = frmLogin::getHash(currentRData.at(5).toString() + newData.at(4).toString());
    }

    bool opResult = dbManager->updateRow(SmartClassGlobal::getTableName(SmartClassGlobal::USERS),
                                         SmartClassGlobal::getTableAliases(SmartClassGlobal::USERS).at(0),
                                         currentID,
                                         SmartClassGlobal::getTableAliases(SmartClassGlobal::USERS).mid(1),
                                         currentRData);

    if (!opResult){
        while (QMessageBox::warning(this, tr("Error | SmartClass"),
                                            tr("An error has ocurred while we tried to update the user data.\n"
                                                "Error details: %1\n"
                                                "Would you like to try again?").arg(dbManager->lastError().text()),
                                        QMessageBox::Retry, QMessageBox::Abort) != QMessageBox::Abort
               && !opResult){
            opResult = dbManager->updateRow(SmartClassGlobal::getTableName(SmartClassGlobal::USERS),
                                                SmartClassGlobal::getTableAliases(SmartClassGlobal::USERS).at(0),
                                                currentID,
                                                SmartClassGlobal::getTableAliases(SmartClassGlobal::USERS).mid(1),
                                                currentRData);
        }
    }
}
