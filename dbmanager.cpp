#include "dbmanager.h"

DBManager::DBManager(const QStringList &data, const QString &connectionName, const QString connectionType)
{
    setDBPrefix("");
    if (connectionType == "MYSQL") {
        if (connectionName == "default") myDB = QSqlDatabase::addDatabase("QMYSQL");
        else myDB = QSqlDatabase::addDatabase("QMYSQL", connectionName);
        myDB.setHostName(data.at(0));
        myDB.setDatabaseName(data.at(1));
        myDB.setPort(QString(data.at(2)).toInt());
        myDB.setUserName(data.at(3));
        myDB.setPassword(data.at(4));
        setDBPrefix(data.at(5));
    }
    else {
        if (connectionName == "default") myDB = QSqlDatabase::addDatabase("QSQLITE");
        else myDB = QSqlDatabase::addDatabase("QSQLITE", connectionName);
        myDB.setDatabaseName(data.at(0));
        setDBPrefix(data.at(1));
    }
}

DBManager::~DBManager()
{
    QString connectionName =  myDB.connectionName();
    if (myDB.isOpen()) myDB.close();
    myDB = QSqlDatabase();
    myDB.removeDatabase(connectionName);
}

bool DBManager::isOpen(){
    return myDB.isOpen();
}

bool DBManager::openDB(){
    return myDB.open();
}

void DBManager::closeDB(){
    myDB.close();
}

bool DBManager::createTable(const QString &tableName, const QStringList &columns){
    if (!myDB.isOpen()) return false;

    QString command = "CREATE TABLE IF NOT EXISTS " + prefix + tableName + "(";
    for (int i = 0; i < columns.length(); ++i){
        command += columns.at(i);
        if (i != columns.length() - 1) command += ", ";
    }
    command += ");";

    QSqlQuery query(QSqlDatabase::database(myDB.connectionName()));
    query.prepare(command);
    return query.exec();
}

bool DBManager::addLine(const QString &tableName, const QStringList &columnName, const QStringList &data){
    if (tableName.isEmpty() || tableName.isNull() || data.isEmpty()) return false;

    QString command = "INSERT INTO " + prefix + tableName + " (";
    for (int i = 0; i < columnName.length(); ++i){
        command += columnName.at(i);
        if (i != data.length() - 1) command += ", ";
    }
    command += ") VALUES (";
    for (int i = 0; i < columnName.length(); ++i){
        command += ":" + columnName.at(i);
        if (i != data.length() - 1) command += ", ";
    }
    command += ")";

    QSqlQuery queryAdd(QSqlDatabase::database(myDB.connectionName()));
    queryAdd.prepare(command);
    for (int i = 0; i < columnName.length(); ++i){
        QString newCName = ":" + columnName.at(i);
        queryAdd.bindValue(newCName, QVariant(data.at(i)));
    }

    return queryAdd.exec();
}

bool DBManager::addImage(const QString &tableName, const QString &columnName,
                         const QString &columnRelation ,const QString &relation, const QPixmap &data){
    if (tableName.isEmpty() || tableName.isNull() || columnName.isEmpty() || columnName.isNull()
            || columnRelation.isEmpty() || columnRelation.isNull() || relation.isEmpty()
            || relation.isNull() || data.isNull()) return false;

    QString command = "INSERT INTO " + prefix + tableName + " (" + columnRelation +", " + columnName + ") VALUES (:"
            + columnRelation + ", :" + columnName + ")";

    QByteArray imageData;
    QBuffer buffer(&imageData);
    buffer.open(QIODevice::WriteOnly);
    data.save(&buffer, "PNG");

    QSqlQuery queryAdd(QSqlDatabase::database(myDB.connectionName()));
    queryAdd.prepare(command);
    queryAdd.bindValue(":" + columnRelation, relation);
    queryAdd.bindValue(":" + columnName, imageData);

    return queryAdd.exec();
}

bool DBManager::updateLine(const QString &tableName, const QStringList &columnName, const QStringList &data,
                           const QString &columnNameCond, const QString &condition){
    if (tableName.isEmpty() || tableName.isNull() || condition.isEmpty()
            || condition.isNull() || columnNameCond.isEmpty() || columnNameCond.isNull()
            || columnName.isEmpty() || data.isEmpty()) return false;

    QString command = "UPDATE " + prefix + tableName + " SET (";
    for (int i = 0; i < columnName.length(); ++i){
        command += columnName.at(i);
        if (i != data.length() - 1) command += ", ";
    }
    command += ") = (";
    for (int i = 0; i < columnName.length(); ++i){
        command += ":" + columnName.at(i);
        if (i != data.length() - 1) command += ", ";
    }
    command += ") WHERE " + columnNameCond + " = (:" + columnNameCond + "c)";

    QSqlQuery queryAdd(QSqlDatabase::database(myDB.connectionName()));
    queryAdd.prepare(command);
    for (int i = 0; i < columnName.length(); ++i){
        QString newCName = ":" + columnName.at(i);
        queryAdd.bindValue(newCName, QVariant(data.at(i)));
    }
    queryAdd.bindValue(":" + columnNameCond + "c", QVariant(condition));

    return queryAdd.exec();
}

bool DBManager::updateLine(const QString &tableName, const QStringList &columnName, const QStringList &data,
                           const QStringList &columnNameCond, const QStringList &condition){
    if (tableName.isEmpty() || tableName.isNull() || condition.isEmpty()
            || columnNameCond.isEmpty() || columnName.isEmpty() || data.isEmpty()) return false;

    QString command = "UPDATE " + prefix + tableName + " SET (";
    for (int i = 0; i < columnName.length(); ++i){
        command += columnName.at(i);
        if (i != data.length() - 1) command += ", ";
    }
    command += ") = (";
    for (int i = 0; i < columnName.length(); ++i){
        command += ":" + columnName.at(i);
        if (i != data.length() - 1) command += ", ";
    }
    command += ") WHERE (";
    for (int i = 0; i < columnNameCond.length(); ++i){
        command += columnNameCond.at(i);
        if (i != columnNameCond.length() - 1) command += ", ";
    }
    command += ") = (";
    for (int i = 0; i < columnNameCond.length(); ++i){
        command += ":" + columnNameCond.at(i) + "c";
        if (i != columnNameCond.length() - 1) command += ", ";
    }
    command += ")";

    QSqlQuery queryUpdate(QSqlDatabase::database(myDB.connectionName()));
    queryUpdate.prepare(command);
    for (int i = 0; i < columnName.length(); ++i)
        queryUpdate.bindValue(":" + columnName.at(i), QVariant(data.at(i)));

    for (int i = 0; i < columnNameCond.length(); ++i)
        queryUpdate.bindValue(":" + columnNameCond.at(i) + "c", QVariant(condition.at(i)));

    return queryUpdate.exec();
}

bool DBManager::updateImage(const QString &tableName, const QString &columnName, const QPixmap &data,
                           const QString &columnNameCond, const QString &condition){
    if (tableName.isEmpty() || tableName.isNull() || condition.isEmpty()
            || condition.isNull() || columnNameCond.isEmpty() || columnNameCond.isNull()
            || columnName.isEmpty()) return false;

    QString command = "UPDATE " + prefix + tableName + " SET (" + columnName + ") = (:" + columnName + ") WHERE "
            + columnNameCond + " = (:" + columnNameCond + ")";

    QByteArray imageData;
    QBuffer buffer(&imageData);
    buffer.open(QIODevice::WriteOnly);
    data.save(&buffer, "PNG");

    QSqlQuery queryAdd(QSqlDatabase::database(myDB.connectionName()));
    queryAdd.prepare(command);
    queryAdd.bindValue(":" + columnNameCond, condition);
    queryAdd.bindValue(":" + columnName, imageData);

    return queryAdd.exec();
}


bool DBManager::removeLine(const QString &tableName, const QString &columnName, const QString &condition){
    if (tableName.isEmpty() || tableName.isNull()
            || condition.isEmpty() || condition.isNull()
            || columnName.isEmpty() || columnName.isNull()) return false;

    if (!lineExists(tableName, columnName, condition)) return false;

    QSqlQuery removeQuery(QSqlDatabase::database(myDB.connectionName()));
    QString command = "DELETE FROM " + prefix + tableName + " WHERE " + columnName + " = (:" + columnName + ")";
    removeQuery.prepare(command);
    removeQuery.bindValue(":" + columnName, QVariant(condition));

    return removeQuery.exec();
}

bool DBManager::removeLine(const QString &tableName, const QStringList &columnName, const QStringList &condition){
    if (tableName.isEmpty() || tableName.isNull()
            || condition.isEmpty() || columnName.isEmpty()) return false;

    if (!lineExists(tableName, columnName, condition)) return false;

    QSqlQuery removeQuery(QSqlDatabase::database(myDB.connectionName()));
    QString command = "DELETE FROM " + prefix + tableName + " WHERE (";
    for (int i = 0; i < columnName.length(); ++i){
        command += columnName.at(i);
        if (i != condition.length() - 1) command += ", ";
    }
    command += ") = (";
    for (int i = 0; i < columnName.length(); ++i){
        command += ":" + columnName.at(i);
        if (i != condition.length() - 1) command += ", ";
    }
    command += ")";

    removeQuery.prepare(command);
    for (int i = 0; i < columnName.length(); ++i)
        removeQuery.bindValue((":" + columnName.at(i)), QVariant(condition.at(i)));

    return removeQuery.exec();
}

bool DBManager::lineExists(const QString &tableName, const QString &columnName, const QString &data){
    if (tableName.isEmpty() || tableName.isNull()
            || data.isEmpty() || data.isNull()
            || columnName.isEmpty() || columnName.isNull()) return false;

    QSqlQuery checkQuery(QSqlDatabase::database(myDB.connectionName()));
    QString command = "SELECT " + columnName + " FROM " + prefix + tableName + " WHERE " + columnName + " = (:" + columnName + ")";
    checkQuery.prepare(command);
    checkQuery.bindValue(":" + columnName, QVariant(data));

    bool exists = false;
    if (checkQuery.exec())
        exists = checkQuery.next();

    return exists;
}

bool DBManager::lineExists(const QString &tableName, const QStringList &columnName, const QStringList &data){
    if (tableName.isEmpty() || tableName.isNull()
            || data.isEmpty()  || columnName.isEmpty()) return false;

    QString command = "SELECT * FROM " + prefix + tableName + " WHERE (";
    for (int i = 0; i < columnName.length(); ++i){
        command += columnName.at(i);
        if (i != data.length() - 1) command += ", ";
    }
    command += ") = (";
    for (int i = 0; i < columnName.length(); ++i){
        command += ":" + columnName.at(i);
        if (i != data.length() - 1) command += ", ";
    }
    command += ")";

    QSqlQuery checkQuery(QSqlDatabase::database(myDB.connectionName()));
    checkQuery.prepare(command);
    for (int i = 0; i < columnName.length(); ++i){
        QString newCName = ":" + columnName.at(i);
        checkQuery.bindValue(newCName, QVariant(data.at(i)));
    }
    bool exists = false;
    if (checkQuery.exec())
        exists = checkQuery.next();
    return exists;
}

bool DBManager::clearTable(const QString &tableName){
    if (tableName.isEmpty() || tableName.isNull()) return false;

    QSqlQuery clearQuery(QSqlDatabase::database(myDB.connectionName()));
    QString command = "DELETE FROM " + prefix + tableName;
    clearQuery.prepare(command);

    return clearQuery.exec();
}

bool DBManager::dropTable(const QString &tableName){
    if (tableName.isEmpty() || tableName.isNull()) return false;

    QSqlQuery dropQuery(QSqlDatabase::database(myDB.connectionName()));
    QString command = "DROP TABLE " + prefix + tableName;
    dropQuery.prepare(command);

    return dropQuery.exec();
}

QStringList DBManager::retrieveLine(const QString &tableName, const QString &columnName, const QString &condition, const QString &operation){
    QStringList retrievedData;

    QSqlQuery retrieveQuery(QSqlDatabase::database(myDB.connectionName()));
    QString command = "SELECT * FROM " + prefix + tableName + " WHERE " + columnName + " " + operation + " (:" + columnName + ")";
    retrieveQuery.prepare(command);
    retrieveQuery.bindValue(":" + columnName, condition);

    if (retrieveQuery.exec()){
        if (retrieveQuery.next()){
            int count = 1;
            while (retrieveQuery.value(count).isValid()){
                retrievedData << retrieveQuery.value(count).toString();
                ++count;
            }
        }
    }

    return retrievedData;
}

QStringList DBManager::retrieveLine(const QString &tableName, const QStringList &columnName, const QStringList &condition,
                                    const QString &operation){

    QSqlQuery retrieveQuery(QSqlDatabase::database(myDB.connectionName()));
    QString command = "SELECT * FROM " + prefix + tableName + " WHERE (";
    for (int i = 0; i < columnName.length(); ++i){
        command += columnName.at(i);
        if (i != condition.length() - 1) command += ", ";
    }
    command += ") "+ operation + " (";
    for (int i = 0; i < columnName.length(); ++i){
        command += ":" + columnName.at(i);
        if (i != condition.length() - 1) command += ", ";
    }
    command += ")";

    retrieveQuery.prepare(command);
    for (int i = 0; i < columnName.length(); ++i){
        QString newCName = ":" + columnName.at(i);
        retrieveQuery.bindValue(newCName, QVariant(condition.at(i)));
    }

    QStringList retrievedData;

    if (retrieveQuery.exec()){
        if (retrieveQuery.next()){
            int count = 1;
            while (retrieveQuery.value(count).isValid()){
                retrievedData << retrieveQuery.value(count).toString();
                ++count;
            }
        }
    }

    return retrievedData;
}

QPixmap DBManager::retrieveImage(const QString &tableName, const QString &columnName,
                                const QString &columnCondition, const QString &condition){
    QPixmap retrievedImage;

    QSqlQuery retrieveQuery(QSqlDatabase::database(myDB.connectionName()));
    QString command = "SELECT " + columnName + " FROM " + prefix + tableName + " WHERE " + columnCondition + "=(:" + columnCondition + ")";
    retrieveQuery.prepare(command);
    retrieveQuery.bindValue(":" + columnCondition, condition);

    if (retrieveQuery.exec()){
        retrieveQuery.first();
        QByteArray output = retrieveQuery.value(0).toByteArray();
        if (output.isEmpty() || output.isNull()) retrievedImage = QPixmap();
        else retrievedImage.loadFromData(output);
    }
    else retrievedImage = QPixmap();

    return retrievedImage;
}

QStringList* DBManager::retrieveAll(const QString &tableName){
    int size = this->rowsCount(tableName);
    if (size <= 0) return NULL;
    QStringList* results = new QStringList[size];

    QSqlQuery retrieveQuery(QSqlDatabase::database(myDB.connectionName()));
    QString command = "SELECT * FROM " + prefix + tableName;
    retrieveQuery.prepare(command);

    if (retrieveQuery.exec()){
        for (int i = 0; i < size; ++i){
            retrieveQuery.next();
            int count  = 0;
            while (retrieveQuery.value(count).isValid()){
                results[i] << retrieveQuery.value(count).toString();
                ++count;
            }
        }
    }

    return results;
}

QStringList* DBManager::retrieveAll(const QString &tableName, const QStringList &columns){
    int size = this->rowsCount(tableName);
    if (size <= 0) return NULL;
    QStringList* results = new QStringList[size];

    QSqlQuery retrieveQuery(QSqlDatabase::database(myDB.connectionName()));
    QString command = "SELECT ";
    for (int i = 0; i < columns.length(); ++ i) command += (i == (columns.length() - 1) ? columns.at(i) : columns.at(i) + ", ");
    command += " FROM " + prefix + tableName;
    retrieveQuery.prepare(command);

    if (retrieveQuery.exec()){
        for (int i = 0; i < size; ++i){
            retrieveQuery.next();
            for (int j = 0; j < columns.length(); ++j)
                results[i] << retrieveQuery.value(j).toString();
        }
    }

    return results;
}

QStringList* DBManager::retrieveAllCondS(const QString &tableName, const QString &columnCondition,
                                         const QString &condition, const QString &operation){
    int size = this->rowsCountCond(tableName, columnCondition, condition, operation);
    if (size <= 0) return NULL;
    QStringList* results = new QStringList[size];

    QSqlQuery retrieveQuery(QSqlDatabase::database(myDB.connectionName()));
    QString command = "SELECT * FROM " + prefix + tableName + " WHERE (" + columnCondition + ") " + operation + " (:" + columnCondition + ");";
    retrieveQuery.prepare(command);
    retrieveQuery.bindValue(":" + columnCondition, condition);

    if (retrieveQuery.exec()){
        for (int i = 0; i < size; ++i){
            retrieveQuery.next();
            int count  = 1;
            while (retrieveQuery.value(count).isValid()){
                results[i] << retrieveQuery.value(count).toString();
                ++count;
            }
        }
    }

    return results;
}

QStringList* DBManager::retrieveAllCond(const QString &tableName, const QStringList &columnCondition, const QStringList &condition, const QString &operation){
    int size = this->rowsCountCond(tableName, columnCondition, condition, operation);
    if (size <= 0) return NULL;
    QStringList* results = new QStringList[size];

    QSqlQuery retrieveQuery(QSqlDatabase::database(myDB.connectionName()));
    QString command = "SELECT * FROM " + prefix + tableName + " WHERE (";
    for (int i = 0; i < columnCondition.length(); ++i){
        command += columnCondition.at(i);
        if (i != condition.length() - 1) command += ", ";
    }
    command += ") "+ operation + " (";
    for (int i = 0; i < columnCondition.length(); ++i){
        command += ":" + columnCondition.at(i);
        if (i != condition.length() - 1) command += ", ";
    }
    command += ")";

    retrieveQuery.prepare(command);
    for (int i = 0; i < columnCondition.length(); ++i){
        QString newCName = ":" + columnCondition.at(i);
        retrieveQuery.bindValue(newCName, QVariant(condition.at(i)));
    }

    if (retrieveQuery.exec()){
        for (int i = 0; i < size; ++i){
            retrieveQuery.next();
            int count  = 1;
            while (retrieveQuery.value(count).isValid()){
                results[i] << retrieveQuery.value(count).toString();
                ++count;
            }
        }
    }

    return results;
}

QStringList* DBManager::retrieveAllCond(const QString &tableName, const QStringList &columns,
                                         const QString &columnCondition, const QString &condition, const QString &operation){
    int size = this->rowsCountCond(tableName, columnCondition, condition, operation);
    if (size <= 0) return NULL;
    QStringList* results = new QStringList[size];

    QSqlQuery retrieveQuery(QSqlDatabase::database(myDB.connectionName()));
    QString command = "SELECT ";
    for (int i = 0; i < columns.length(); ++ i) command += (i == (columns.length() - 1) ? columns.at(i) : columns.at(i) + ", ");
    command += " FROM " + prefix + tableName + " WHERE " + columnCondition + " " + operation + " (:" + columnCondition + ")";
    retrieveQuery.prepare(command);
    retrieveQuery.bindValue(":" + columnCondition, condition);

    if (retrieveQuery.exec()){
        for (int i = 0; i < size; ++i){
            retrieveQuery.next();
            for (int j = 0; j < columns.length(); ++j)
                results[i] << retrieveQuery.value(j).toString();
        }
    }

    return results;
}

QStringList* DBManager::retrieveAllCond(const QString &tableName, const QStringList &columns,
                                        const QStringList &columnCondition, const QStringList &condition, const QString &operation){
    int size = this->rowsCountCond(tableName, columnCondition, condition, operation);
    if (size <= 0) return NULL;
    QStringList* results = new QStringList[size];

    QSqlQuery retrieveQuery(QSqlDatabase::database(myDB.connectionName()));
    QString command = "SELECT ";
    for (int i = 0; i < columns.length(); ++ i) command += (i == (columns.length() - 1) ? columns.at(i) : columns.at(i) + ", ");
    command += " FROM " + prefix + tableName + " WHERE (";
    for (int i = 0; i < columnCondition.length(); ++i){
        command += columnCondition.at(i);
        if (i != columnCondition.length() - 1) command += ", ";
    }
    command += ") "+ operation + " (";
    for (int i = 0; i < columnCondition.length(); ++i){
        command += ":" + columnCondition.at(i);
        if (i != condition.length() - 1) command += ", ";
    }
    command += ")";

    retrieveQuery.prepare(command);
    for (int i = 0; i < columnCondition.length(); ++i){
        QString newCName = ":" + columnCondition.at(i);
        retrieveQuery.bindValue(newCName, QVariant(condition.at(i)));
    }

    if (retrieveQuery.exec()){
        for (int i = 0; i < size; ++i){
            retrieveQuery.next();
            for (int j = 0; j < columns.length(); ++j)
                results[i] << retrieveQuery.value(j).toString();
        }
    }

    return results;
}

int DBManager::rowsCount(const QString &tableName){
    if (tableName.isEmpty() || tableName.isNull()) return -1;

    QSqlQuery countQuery(QSqlDatabase::database(myDB.connectionName()));
    QString command = "SELECT * FROM " + prefix + tableName;
    countQuery.prepare(command);

    int counter = 0;
    if (countQuery.exec()){
        while (countQuery.next()) counter++;
        return counter;
    }
    return -1;
}

int DBManager::rowsCountCond(const QString &tableName,
                             const QString &columnCondition, const QString &condition, const QString &operation){
    if (tableName.isEmpty() || tableName.isNull() || columnCondition.isEmpty() || columnCondition.isNull()
            || condition.isNull() || condition.isEmpty()) return -1;

    QSqlQuery countQuery(QSqlDatabase::database(myDB.connectionName()));
    QString command = "SELECT * FROM " + prefix + tableName + " WHERE (" + columnCondition + ") " + operation + " (:" + columnCondition + ");";
    countQuery.prepare(command);
    countQuery.bindValue(":" + columnCondition, condition);

    int counter = 0;
    if (countQuery.exec()){
        while (countQuery.next()) counter++;
        return counter;
    }
    return -1;
}

int DBManager::rowsCountCond(const QString &tableName,
                             const QStringList &columnCondition, const QStringList &condition, const QString &operation){

    QSqlQuery countQuery(QSqlDatabase::database(myDB.connectionName()));
    QString command = "SELECT * FROM " + tableName  + " WHERE (";
    for (int i = 0; i < columnCondition.length(); ++i){
        command += columnCondition.at(i);
        if (i != condition.length() - 1) command += ", ";
    }
    command += ") "+ operation + " (";
    for (int i = 0; i < columnCondition.length(); ++i){
        command += ":" + columnCondition.at(i);
        if (i != condition.length() - 1) command += ", ";
    }
    command += ")";

    countQuery.prepare(command);
    for (int i = 0; i < columnCondition.length(); ++i){
        QString newCName = ":" + columnCondition.at(i);
        countQuery.bindValue(newCName, QVariant(condition.at(i)));
    }

    int counter = 0;
    if (countQuery.exec()){
        while (countQuery.next()) counter++;
        return counter;
    }
    return -1;
}

QString DBManager::getUniqueConnectionName(const QString partName){
    QString connectionName = partName;
    connectionName += QDateTime::currentDateTime().toString("dd.MM.yyyy-hh:mm:ss:zzz");
    return connectionName;
}
