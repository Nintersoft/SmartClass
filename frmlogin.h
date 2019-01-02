#ifndef FRMLOGIN_H
#define FRMLOGIN_H

#include <QCryptographicHash>
#include <QDesktopWidget>
#include <QMainWindow>
#include <QMessageBox>
#include <QStringList>
#include <QSettings>
#include <QDateTime>
#include <QHostInfo>
#include <QVariant>
#include <QSysInfo>
#include <QFile>
#include <QDir>

#include <QDebug>

#include "smartclassglobal.h"
#include "nmainwindow.h"
#include "dbmanager.h"

#include "titlebar.h"

namespace Ui {
class frmLogin;
}

class frmLogin : public NMainWindow
{
    Q_OBJECT

public:
    explicit frmLogin(QWidget *parent = 0);
    ~frmLogin();

    inline static QString randomSalt(int size){
        qsrand((uint)QTime::currentTime().msec());
        QString salt = "";
        for (int i = 0; i < size; ++i){
            unsigned char currentChar = (unsigned char)((qrand() % 223) + 33);
            if (currentChar == ';' || (currentChar >= 127 && currentChar <= 160)){
                i--;
                continue;
            }
            salt += currentChar;
        }
        return salt;
    }

    inline static QString getHash(const QString &password){
        return QString(QCryptographicHash::hash(password.toUtf8(), QCryptographicHash::Sha512).toHex());
    }

    inline static bool isSafePassword(const QString &pass, int &index){
        bool isSafe = true, safetyItems[4];
        for (unsigned int i = 0; i < sizeof(safetyItems)/sizeof(bool); ++i)
            safetyItems[i] = false;

        for (int i = 0; i < pass.length(); ++i){
            if (pass[i].isNumber()) safetyItems[0] = true;
            else if (!pass[i].isLetterOrNumber()) safetyItems[1] = true;
            else if (pass[i].isLetter() && pass[i].isUpper()) safetyItems[2] = true;
            else if (pass[i].isLetter() && pass[i].isLower()) safetyItems[3] = true;
        }

        for (unsigned int i = 0; i < sizeof(safetyItems)/sizeof(bool); ++i)
            if (!safetyItems[i]){
                index = i;
                isSafe = false;
                break;
            }

        return isSafe;
    }

private slots:
    void login();
    void registerUser();
    void askQuestion();
    void changePassword();

protected slots:
    void changeTab();

private:
    Ui::frmLogin *ui;

    DBManager *db_manager;
    qlonglong dID;

signals:
    void dataReady(const QVariantList &userInfo);
};

#endif // FRMLOGIN_H
