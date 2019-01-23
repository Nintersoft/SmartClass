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

    inline static void setGlobalConnectionName(const QString &cName) { globalConnection = cName; }
    inline static QString connectionName() { return DBManager::getUniqueConnectionName(globalConnection); }

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
        PAYMENTDETAILS,
        SETTINGS,
        ACTIVECONNECTIONS
    };

    inline static const QString getTableName(TablesSpec table, bool includePrefix = false){
        QString tableName = (includePrefix ? databaseTablePrefix : "");
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
                tableName += "smartclass_rimages";
                break;
            case COURSEDETAILS:
                tableName += "smartclass_courses";
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
            id = "id INTEGER PRIMARY KEY AUTOINCREMENT";
            blobType = "BLOB";
        }
        else{
            id = "id INTEGER(64) UNSIGNED AUTO_INCREMENT PRIMARY KEY";
            blobType = "MEDIUMBLOB";
        }

        QStringList tableStructure;
        switch (table) {
            case USERS:
                tableStructure << id << "username VARCHAR(255) NOT NULL UNIQUE" << "name TEXT NOT NULL"
                                << "salt TEXT NOT NULL" << "hash TEXT NOT NULL" << "question TEXT"
                                << "answerSalt TEXT NOT NULL" << "answerHash TEXT NOT NULL" << "role INTEGER";
                break;
            case STUDENT:
                tableStructure << id << "name TEXT NOT NULL" << (isSQLite ? "rID INTEGER" : "rID INTEGER(64) UNSIGNED")
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
                tableStructure << (isSQLite ? "id INTEGER" : "id INTEGER(64) UNSIGNED") << ("picture " + blobType)
                                << ("studentID " + blobType);
                break;
            case RESPONSIBLEIMAGES:
                tableStructure << (isSQLite ? "id INTEGER" : "id INTEGER(64) UNSIGNED") << ("responsibleID " + blobType)
                                << ("responsibleCPG " + blobType) << ("addressComprobation " + blobType);
                break;
            case COURSEDETAILS:
                tableStructure << id << "course TEXT NOT NULL" << "teacher TEXT NOT NULL"
                                << "shortDescription TEXT NOT NULL" << "longDescription TEXT NOT NULL" << "class INTEGER"
                                << "dayNTime TEXT NOT NULL" << "beginningDate DATE" << "endDate DATE"
                                << "price DOUBLE";
                break;
            case PAYMENTDETAILS:
                tableStructure << (isSQLite ? "sid INTEGER" : "sid INTEGER(64) UNSIGNED")
                                << (isSQLite ? "cid INTEGER" : "cid INTEGER(64) UNSIGNED")
                                << "discount DOUBLE" << "beginningDate DATE" << "installments INTEGER";
                break;
            case SETTINGS:
                tableStructure << "companyName TEXT" << "contract TEXT" << ("logo " + blobType);
                break;
                //activeConnections
            default:
                tableStructure << id << "deviceName TEXT" << "OS TEXT" << "OSVersion TEXT"
                                << "lastAccess TEXT";
                break;
        }
        return tableStructure;
    }
    inline static const QStringList getTableAliases(TablesSpec table){
        QStringList tableStructure;
        switch (table) {
            case USERS:
                tableStructure << "id" << "username" << "name"
                                << "salt" << "hash" << "question"
                                << "answerSalt" << "answerHash" << "role";
                break;
            case STUDENT:
                tableStructure << "id" << "name" << "rID"
                                << "birthday" << "studentID" << "school" << "observations"
                                << "experimentalClass" << "experimentalClassDate"
                                << "experimentalClassObservations";
                break;
            case RESPONSIBLE:
                tableStructure << "id" << "name" << "phone" << "mobileOperator"
                                << "mobile" << "email" << "responsibleID" << "responsibleCPG"
                                << "meeting" << "address";
                break;
            case STUDENTIMAGES:
                tableStructure << "id" << "picture" << "studentID";
                break;
            case RESPONSIBLEIMAGES:
                tableStructure << "id" << "responsibleID"
                                << "responsibleCPG" << "addressComprobation";
                break;
            case COURSEDETAILS:
                tableStructure << "id" << "course" << "teacher"
                                << "shortDescription" << "longDescription" << "class"
                                << "dayNTime" << "beginningDate" << "endDate" << "price";
                break;
            case PAYMENTDETAILS:
                tableStructure << "sid" << "cid" << "discount" << "beginningDate"
                               << "installments";
                break;
            case SETTINGS:
                tableStructure << "companyName" << "contract" << "logo";
                break;
                //activeConnections
            default:
                tableStructure << "id" << "deviceName" << "OS" << "OSVersion"
                                << "lastAccess";
                break;
        }
        return tableStructure;
    }
    inline static const QStringList getTableConstraints(TablesSpec spec){
        QStringList tableConstraints;
        switch (spec) {
        case STUDENT:
            tableConstraints << "CONSTRAINT FK_Rid FOREIGN KEY (rID) REFERENCES " +
                                tablePrefix() + getTableName(RESPONSIBLE) + "(id)";
            break;
        case STUDENTIMAGES:
            tableConstraints << "CONSTRAINT FK_SIid FOREIGN KEY (id) REFERENCES " +
                                tablePrefix() + getTableName(STUDENT) + "(id)";
            break;
        case RESPONSIBLEIMAGES:
            tableConstraints << "CONSTRAINT FK_RIid FOREIGN KEY (id) REFERENCES " +
                                tablePrefix() + getTableName(RESPONSIBLE) + "(id)";
            break;
        case PAYMENTDETAILS:
            tableConstraints << "CONSTRAINT FK_SPid FOREIGN KEY (sid) REFERENCES " +
                                tablePrefix() + getTableName(STUDENT) + "(id)";
            tableConstraints << "CONSTRAINT FK_CPid FOREIGN KEY (cid) REFERENCES " +
                                tablePrefix() + getTableName(COURSEDETAILS) + "(id)";
            tableConstraints << "CONSTRAINT PK_Payment PRIMARY KEY (sid, cid)";
            break;
        default:
            break;
        }
        return tableConstraints;
    }

    enum UserRoles{
        ADMIN = 0,
        EDITOR = 1,
        VIEWER = 2,
        NEW = 3
    };

protected:
    static QString globalConnection;
    static QString databaseTablePrefix;
    static DBManager::DBConnectionType databaseTypeS;
};

#endif // SMARTCLASSGLOBAL_H
