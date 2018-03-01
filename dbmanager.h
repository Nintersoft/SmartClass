#ifndef DBMANAGER_H
#define DBMANAGER_H

/*
 * WARNING: THE IMPLEMENTATION OF THIS CLASS WORKS ONLY WITH TEXT VALUES ON THE DATABASE
 *          SO, IF YOU NEED TO WORK WITH OTHER VARIANTS, PLEASE CONVERT THEM BEFORE PASSING
 *          THE REFERENCE TO THE FUNCTIONS
 * NOTE: AFTER YOU RETRIEVE THE VALUE AS STRING YOU WILL HAVE TO CONVERT IT BACK TO ITS
 *          ORIGINAL VALUE
 */

#include <QStringList>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlRecord>
#include <QByteArray>
#include <QDateTime>
#include <QVariant>
#include <QPixmap>
#include <QBuffer>

class DBManager
{
public:
    explicit DBManager(const QStringList &data = QStringList(), const QString &connectionName = QString("default"),
                       const QString connectionType = QString("SQLITE"));
    ~DBManager();

    bool openDB();
    void closeDB();
    bool isOpen();

    bool createTable(const QString &tableName, const QStringList &columns);

    bool addLine(const QString &tableName, const QStringList &columnName, const QStringList &data);
    bool addImage(const QString &tableName, const QString &columnName,
                  const QString &columnRelation, const QString &relation, const QPixmap &data);

    bool updateLine(const QString &tableName, const QStringList &columnName, const QStringList &data,
                    const QString &columnNameCond, const QString &condition);
    bool updateLine(const QString &tableName, const QStringList &columnName, const QStringList &data,
                    const QStringList &columnNameCond, const QStringList &condition);
    bool updateImage(const QString &tableName, const QString &columnName, const QPixmap &data,
                     const QString &columnNameCond, const QString &condition);

    bool removeLine(const QString &tableName, const QString &columnName, const QString &condition);
    bool removeLine(const QString &tableName, const QStringList &columnName, const QStringList &condition);

    bool lineExists(const QString &tableName, const QString &columnName, const QString &data);
    bool lineExists(const QString &tableName, const QStringList &columnName, const QStringList &data);

    bool clearTable(const QString &tableName);
    bool dropTable(const QString &tableName);

    QStringList retrieveLine(const QString &tableName, const QString &columnName,
                             const QString &condition, const QString &operation = "=");
    QStringList retrieveLine(const QString &tableName, const QStringList &columnName,
                             const QStringList &condition, const QString &operation = "=");

    QPixmap retrieveImage(const QString &tableName, const QString &columnName,
                          const QString &columnCondition, const QString &condition);

    QStringList* retrieveAll(const QString &tableName);
    QStringList* retrieveAll(const QString &tableName, const QStringList &columns);

    QStringList* retrieveAllCondS(const QString &tableName,
                                 const QString &columnCondition, const QString &condition, const QString &operation = "=");
    QStringList* retrieveAllCond(const QString &tableName,
                                 const QStringList &columnCondition, const QStringList &condition, const QString &operation = "=");
    QStringList* retrieveAllCond(const QString &tableName, const QStringList &columns,
                                 const QString &columnCondition, const QString &condition, const QString &operation = "=");
    QStringList* retrieveAllCond(const QString &tableName, const QStringList &columns,
                                 const QStringList &columnCondition, const QStringList &condition, const QString &operation = "=");

    int rowsCount(const QString &tableName);
    int rowsCountCond(const QString &tableName,
                      const QString &columnCondition, const QString &condition, const QString &operation = "=");
    int rowsCountCond(const QString &tableName,
                      const QStringList &columnCondition, const QStringList &condition, const QString &operation = "=");

    static QString getUniqueConnectionName(const QString partName = "");

    inline void setDBPrefix(const QString &prefix) { this->prefix = prefix; }
    inline QString dbPrefix() { return this->prefix; }

private:
    QSqlDatabase myDB;
    QString prefix;
};

#endif // DBMANAGER_H
