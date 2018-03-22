#ifndef COMMS_H
#define COMMS_H

#include <QObject>
#include <QSettings>
#include <QNetworkReply>

#include "AppData.h"

class Comms : public QObject {
    Q_OBJECT
    Q_DISABLE_COPY(Comms)

public:

    static Comms &instance();
    explicit Comms(QObject *parent = nullptr);
    virtual ~Comms(){}

    void saveApp(AppData *app);
    void sendAppData(QList<AppData*> *appList);

    qint64 getCurrentTime() const;
    void setCurrentTime(qint64 current_time);
    void timedUpdates();

    bool isApiKeyOK();

private:
    AppData *lastApp;
    QSettings settings;
    qint64 lastSync;
    qint64 currentTime;
    QString apiKey;

signals:

public slots:
    void serviceRequestFinished(QNetworkReply* reply);
};

#endif // COMMS_H
