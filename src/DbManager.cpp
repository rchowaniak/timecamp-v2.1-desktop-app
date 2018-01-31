#include "DbManager.h"
#include "Settings.h"

#include <QSqlQuery>
#include <QSqlError>
#include <QSqlRecord>
#include <QDebug>

DbManager &DbManager::instance()
{
    static DbManager _instance;
    return _instance;
}

DbManager::DbManager(QObject *parent)
    : QObject(parent)
{
    qInfo() << "[DB] Starting DB manager!";
    m_db = QSqlDatabase::addDatabase("QSQLITE");
    m_db.setDatabaseName(DB_FILENAME);

    if (!m_db.open())
    {
        qInfo() << "[DB] Error: connection with database fail";
    }
    else
    {
        qDebug() << "[DB] Database: connection ok";
        createTable();
    }
}

DbManager::~DbManager()
{
    if (m_db.isOpen())
    {
        m_db.close();
    }
}

bool DbManager::isOpen() const
{
    return m_db.isOpen();
}

bool DbManager::createTable()
{
    bool success = false;

    QSqlQuery query;
    query.prepare("CREATE TABLE \"apps\" ( `ID` INTEGER PRIMARY KEY AUTOINCREMENT, `app_name` TEXT, `window_name` TEXT, `additional_info` TEXT, `start_time` INTEGER NOT NULL, `end_time` INTEGER NOT NULL )");

    if (!query.exec())
    {
        qDebug() << "Couldn't create the table: one might already exist.";
        success = false;
    }

    return success;
}

bool DbManager::saveToDb(AppData *app, qint64 start, qint64 end)
{
    bool success = false;
    QString appName = app->getAppName();
    QString windowName = app->getWindowName();
    QString additionalInfo = app->getAdditionalInfo();

    if (start > 0 && end > 0)
    {
        QSqlQuery queryAdd;
        queryAdd.prepare("INSERT INTO apps (ID, app_name, window_name, additional_info, start_time, end_time) VALUES (NULL, ?, ?, ?, ?, ?)");
        queryAdd.addBindValue(appName);
        queryAdd.addBindValue(windowName);
        queryAdd.addBindValue(additionalInfo);
        queryAdd.addBindValue(start);
        queryAdd.addBindValue(end);

        if(queryAdd.exec())
        {
            qDebug() << "[DB] app added successfully";
            success = true;
        }
        else
        {
            qInfo() << "[DB] adding failed: " << queryAdd.lastError();
        }
    }
    else
    {
        qInfo() << "[DB] adding failed: missing values!";
    }

    return success;
}