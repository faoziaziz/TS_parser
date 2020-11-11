#pragma once
#include <QRunnable>
#include <QThread>
#include <QQueue>
#include <QMutex>
#include <QSqlQuery>
#include <QDebug>
#include <QRegularExpression>
#include <QDateTime>

#include "tsdata.h"
#include "config.h"
//#include "parser.h"

class ParsingTask : public QRunnable
{
public:
    ParsingTask(QSqlQuery *queryFileRaw, int parserMode, HasilTransaksi *HasilTransaksi, DeviceProfile patternRegex);
    ParsingTask(QSqlQuery *queryFileRaw, QSqlQuery *queryTeks, int parserMode, HasilTransaksi *HasilTransaksi, DeviceProfile patternRegex);
    ~ParsingTask();

    void run();

    int taskNum;

    HasilTransaksi *hasilTransaksi;
    DeviceProfile deviceProfile;

private:
    DataRaw dataRaw;
    int lastRef = 0;
    int repeatRef = 0;
};

