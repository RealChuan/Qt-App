#include "datbasesql.h"

#include <QSqlDatabase>
#include <QSqlError>
#include <QDebug>

namespace Utils {

struct DatabaseSQL::DatabaseSQLPrivate
{
    QSqlDatabase database;
    QString error;
};

DatabaseSQL::DatabaseSQL(QObject *parent)
    : QObject(parent)
    , d_ptr(new DatabaseSQLPrivate)
{
    qInfo() << tr("Qt currently supports database drivers:")
            << QSqlDatabase::drivers();
}

DatabaseSQL::~DatabaseSQL()
{
}

bool DatabaseSQL::openSQL(const DatabaseParam &param)
{
    if ("MySQL" == param.type) {
        return openMySQL(param.ip, param.port, param.databaseName, param.uesrname, param.password);
    } else if ("SQLite" == param.type) {
        return openSQLite(param.databaseName);
    }
    d_ptr->error = tr("Unknow SQL type!");
    return false;
}

QSqlQuery DatabaseSQL::query(const QString &sql)
{
    QSqlQuery q;
    if(!q.exec(sql)){
        d_ptr->error = (tr("DataBase Query Error: %1 !").arg(q.lastError().text()));
        qWarning() << d_ptr->error << sql;
        return q;
    }
    return q;
}

QString DatabaseSQL::errorString() const
{
    return d_ptr->error;
}

bool DatabaseSQL::tableContains(const QString &tableName)
{
    return d_ptr->database.tables().contains(tableName);
}

bool DatabaseSQL::openMySQL(const QString &ip,
                            int port,
                            const QString &databaseName,
                            const QString &username,
                            const QString &password)
{
    if (!QSqlDatabase::drivers().contains("QMYSQL")){
        qWarning() <<tr("Unable to load database, "
                         "This program needs the MYSQL driver!");
        return false;
    }

    if (ip.isEmpty()) {
        d_ptr->error = tr("MySQL IP Empty!");
        return false;
    }
    if(port<=0 || port >65536){
        d_ptr->error = tr("MySQL Port Error!");
        return false;
    }
    if(databaseName.isEmpty()){
        d_ptr->error = tr("MySQL Database Name Empty!");
        return false;
    }
    if(username.isEmpty()){
        d_ptr->error = tr("MySQL Username Empty!");
        return false;
    }
    if(password.isEmpty()){
        d_ptr->error = tr("MySQL Password Empty!");
        return false;
    }

    d_ptr->database = QSqlDatabase::addDatabase("QMYSQL");
    //???????????????????????????????????????????????????????????????127.0.0.1????????????????????????????????????localhost)
    d_ptr->database.setHostName(ip);
    d_ptr->database.setPort(port);                 //????????????????????????
    d_ptr->database.setDatabaseName(databaseName); //??????????????????
    d_ptr->database.setUserName(username);         //??????????????????
    d_ptr->database.setPassword(password);         //???????????????

    if (d_ptr->database.open())
        return true;

    d_ptr->error = tr("MySQL Open Error: %1 !").arg(d_ptr->database.lastError().text());
    return false;
}

bool DatabaseSQL::openSQLite(const QString &databaseName)
{
    if (!QSqlDatabase::drivers().contains("QSQLITE")){
        qWarning() << tr("Unable to load database, "
                         "This program needs the SQLITE driver");
        return false;
    }

    if (databaseName.isEmpty()) {
        d_ptr->error = tr("SQLite Database Name Empty!");
        return false;
    }
    /*---------------------------------------------------------/
     *      ???navicat?????????????????????????????????setPassWord()??????,     /
     *      ??????????????????????????????????????????,                         /
     *      QT???sql????????????????????????sqlite???                      /
     *      ?????????????????????????????????????????????????????????                 /
     *--------------------------------------------------------*/
    d_ptr->database = QSqlDatabase::addDatabase("QSQLITE");
    d_ptr->database.setDatabaseName(databaseName);
    if (d_ptr->database.open())
        return true;

    d_ptr->error = tr("SQLite Open Error: %1 !").arg(d_ptr->database.lastError().text());
    return false;
}

}
