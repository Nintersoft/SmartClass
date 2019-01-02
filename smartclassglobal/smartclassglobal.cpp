#include "smartclassglobal.h"

QString SmartClassGlobal::databaseTablePrefix = "";
QString SmartClassGlobal::globalConnection = "";

DBManager::DBConnectionType SmartClassGlobal::databaseTypeS = DBManager::SQLITE;
