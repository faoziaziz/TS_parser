#pragma once

#include <QObject>
#include <QDebug>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QThreadPool>
#include <QTimer>
#include <QQueue>
#include <QMutex>

#include "config.h"
#include "tsdata.h"
#include "parsing.h"

class Parser : public QObject
{
    Q_OBJECT
public:
    explicit Parser(QObject *parent = 0);
    ~Parser();
    bool SetDatabase(int argc, char *argv[]);



private:
    QSqlDatabase dbConnection;
    int parserMode;

    bool ReconnectDatabase(void);
    int queryLimit;
    int queryLastIndex;
    int queryLastIndex_2;
    int maxThread;

    int noDataCounter;
    int noDataCounter_2;

    void InsertTransaksi(int indexSumber, QByteArray deviceId, QByteArray fileTime, DataTransaksi transaksiToInsert);

    void UpdateFlagParser(int indexToUpdate, int nilaiFlag);
    void UpdateFlagParser_2(int indexToUpdate, int nilaiFlag);

    QMap<QByteArray, DeviceProfile> deviceList;

    HasilTransaksi hasilTransaksi;

    QTimer *timerDeviceList;
    QTimer *timerProcessData;
    QTimer *timerProcessData_2;
    QTimer *timerSaveResult;

    QThreadPool *threadPool;
    int taskCreated;
    int taskSkipped;
    int taskFinished;

    QMap<int, ParsingTask*> parsingTask;

signals:

public slots:
    void ProcessData(void);
    void ProcessData_2(void);
    void SaveResult(void);
    void RefreshDeviceList(void);

};
