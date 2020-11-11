#pragma once

#include <QObject>
#include <QQueue>
#include <QMutex>
#include <QSqlQuery>
#include <QRegularExpression>

extern "C" {
    #include "aes.h"
}

class DeviceProfile
{
public:
    QByteArray deviceId;
    int parserMode;
    QString patternNomor;
    QString patternTanggal;
    QString patternJam;
    QString patternNilai;
    QString patternPajak;
    QString patternNDP;
    QString patternCustomField1;
    QString patternCustomField2;
    QString patternCustomField3;
};


class DataTransaksi
{
public:
    QString tanggal, jam, nomor, nilai, pajak, nilaiDanPajak, cF1,cF2, cF3;
};

class HasilOlah
{
public:
    int indexSumber;
    int indexTeks;
    QByteArray deviceId;
    QString fileTime;
    QByteArray refSn;
    DataTransaksi transaksiHasil;
    int parserMode;
    bool skip;
};

class HasilTransaksi
{
public:
    QQueue<HasilOlah> data;
    QMutex mutex;
};


class DataRaw
{
public:
    int indexNum;
    int indexTeks;
    QByteArray deviceId;
    QByteArray fileContent;
    QString fileName;
    QString fileTime;
    int fileSize;
    int parserMode;

    DataRaw();

    void Decrypt(void);
    void Decompress(void);

private:
    bool isDecrypted;
    bool isDecompressed;

};
