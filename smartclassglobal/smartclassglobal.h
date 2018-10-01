#ifndef SMARTCLASSGLOBAL_H
#define SMARTCLASSGLOBAL_H

#include <QOperatingSystemVersion>
#include <QString>
#include <QFile>
#include <QDir>

#include "dbmanager.h"

class SmartClassGlobal
{
public:
    inline static void setTablePrefix(const QString &prefix) { databaseTablePrefix =  prefix; }
    inline static QString tablePrefix() { return databaseTablePrefix; }

    inline static void setDatabaseType(const DBManager::DBConnectionType &db_type) { databaseTypeS =  db_type; }
    inline static DBManager::DBConnectionType databaseType() { return databaseTypeS; }

    inline static const QString getDBPath(){
        QString tempDir = QDir::homePath();
        if (QOperatingSystemVersion::currentType() == QOperatingSystemVersion::Windows)
            tempDir += "/AppData/Roaming/Nintersoft/SmartClass/";
        else tempDir += "/.Nintersoft/SmartClass/";

        QString dataDir = tempDir + "data/";
        if (!QDir(dataDir).exists()) QDir(dataDir).mkpath(dataDir);

        return tempDir + "data/classInfo.db";
    }

    enum TablesSpec{
        USERS = 0,
        STUDENT,
        RESPONSIBLE,
        STUDENTIMAGES,
        RESPONSIBLEIMAGES,
        COURSEDETAILS,
        COURSEENROLLMENTS,
        PAYMENTDETAILS,
        SETTINGS,
        ACTIVECONNECTIONS
    };

    inline static const QString getTableName(TablesSpec table){
        QString tableName = databaseTablePrefix;
        switch (table) {
            case USERS:
                tableName += "smartclass_users";
                break;
            case STUDENT:
                tableName += "smartclass_students";
                break;
            case RESPONSIBLE:
                tableName += "smartclass_responsibles";
                break;
            case STUDENTIMAGES:
                tableName += "smartclass_simages";
                break;
            case RESPONSIBLEIMAGES:
                tableName += "smartclass_pimages";
                break;
            case COURSEDETAILS:
                tableName += "smartclass_courses";
                break;
            case COURSEENROLLMENTS:
                tableName += "smartclass_enrollments";
                break;
            case PAYMENTDETAILS:
                tableName += "smartclass_payment";
                break;
            case SETTINGS:
                tableName += "smartclass_gsettings";
                break;
            //activeConnections
            default:
                tableName += "smartclass_connections";
                break;
        }
        return tableName;
    }
    inline static const QStringList getTableStructure(TablesSpec table){
        bool isSQLite = SmartClassGlobal::databaseType() == DBManager::SQLITE;
        QString id, blobType;

        if (isSQLite){
            id = "id INTEGER PRIMARY KEY";
            blobType = "BLOB";
        }
        else{
            id = "id INTEGER(64) UNSIGNED AUTO_INCREMENT PRIMARY KEY";
            blobType = "MEDIUMBLOB";
        }

        QStringList tableStructure;
        switch (table) {
            case USERS:
                tableStructure << id << "username TE    XT NOT NULL" << "name TEXT NOT NULL"
                                << "salt  TEXT NOT NULL" << "hash TEXT NOT NULL" << "question TEXT"
                                << "answerSalt TEXT NOT NULL" << "answerHash TEXT NOT NULL" << "role INTEGER";
                break;
            case STUDENT:
                tableStructure << id << "name TEXT NOT NULL" << ("parent INTEGER" + isSQLite ? "" : "(64)")
                                << "birthday DATE" << "studentID TEXT" << "school TEXT" << "observations TEXT"
                                << "experimentalClass TEXT" << "experimentalClassDate DATETIME"
                                << "experimentalClassObservations TEXT";
                break;
            case RESPONSIBLE:
                tableStructure << id << "name TEXT NOT NULL" << "phone TEXT" << "mobileOperator INTEGER"
                                << "mobile TEXT" << "email TEXT" << "responsibleID TEXT" << "responsibleCPG TEXT"
                                << "meeting TEXT" << "address TEXT";
                break;
            case STUDENTIMAGES:
                tableStructure << ("id INTEGER" + isSQLite ? "" : "(64)") << ("picture " + blobType)
                                << ("studentID " + blobType);
                break;
            case RESPONSIBLEIMAGES:
                tableStructure << ("id INTEGER" + isSQLite ? "" : "(64)") << ("responsibleID " + blobType)
                                << ("responsibleCPG " + blobType) << ("addressComprobation " + blobType);
                break;
            case COURSEDETAILS:
                tableStructure << id << "course TEXT NOT NULL" << "teacher TEXT NOT NULL"
                                << "shortDescription TEXT NOT NULL" << "longDescription TEXT NOT NULL" << "class INTEGER"
                                << "dayNTime TEXT NOT NULL" << "beginningDate DATE" << "endDate DATE"
                                << "price DOUBLE";
                break;
            case COURSEENROLLMENTS:
                tableStructure << ("cid INTEGER" + isSQLite ? "" : "(64)") << ("sid INTEGER" + isSQLite ? "" : "(64)");
                break;
            case PAYMENTDETAILS:
                tableStructure << ("sid INTEGER" + isSQLite ? "" : "(64)") << ("cid INTEGER" + isSQLite ? "" : "(64)")
                                << "discount DOUBLE" << "beginningDate DATE" << "installments INTEGER";
                break;
            case SETTINGS:
                tableStructure << "companyName TEXT" << "contract TEXT" << ("logo " + blobType);
                break;
                //activeConnections
            default:
                tableStructure << id << "deviceName TEXT" << "OS TEXT" << "OSVersion TEXT"
                                << "lastAccess DATE";
                break;
        }
        return tableStructure;
    }

    enum UserRoles{
        ADMIN = 0,
        EDITOR = 1,
        VIEWER = 2,
        NEW = 3
    };

private:
    static QString databaseTablePrefix;
    static DBManager::DBConnectionType databaseTypeS;
};

#endif // SMARTCLASSGLOBAL_H
