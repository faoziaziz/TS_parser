#include "parsing.h"
#include "aes.h"

ParsingTask::ParsingTask(QSqlQuery *queryFileRaw, int parserMode, HasilTransaksi *hasilTransaksiHost, DeviceProfile patternRegex)
{   
    dataRaw.indexNum    = queryFileRaw->value(0).toInt();
    dataRaw.deviceId    = queryFileRaw->value(1).toByteArray();
    dataRaw.fileName    = queryFileRaw->value(2).toString();
    dataRaw.fileSize    = queryFileRaw->value(3).toInt();
    dataRaw.fileContent = queryFileRaw->value(4).toByteArray();
    dataRaw.fileTime    = queryFileRaw->value(5).toString();
    dataRaw.parserMode = parserMode;

    dataRaw.Decrypt();
    dataRaw.Decompress();

    hasilTransaksi = hasilTransaksiHost;
    deviceProfile = patternRegex;

    taskNum = dataRaw.indexNum;
    setAutoDelete(true);
}

ParsingTask::ParsingTask(QSqlQuery *queryFileRaw, QSqlQuery *queryTeks, int parserMode, HasilTransaksi *hasilTransaksiHost, DeviceProfile patternRegex)
{
    if(queryFileRaw->isValid())
    {
        dataRaw.indexNum    = queryFileRaw->value(0).toInt();
        dataRaw.fileName    = queryFileRaw->value(2).toString();
        dataRaw.fileSize    = queryFileRaw->value(3).toInt();
        if(dataRaw.indexNum == lastRef) {
            repeatRef++;
            QString tempTime = queryFileRaw->value(4).toString();
            QDateTime waktuLama = QDateTime::fromString(tempTime, "yyyy-MM-dd hh:mm:ss");
            QDateTime waktuBaru = waktuLama.addSecs(repeatRef);
            if(waktuBaru.isValid()) {
                dataRaw.fileTime = waktuBaru.toString("yyyy-MM-dd hh:mm:ss");
            }
            else {
                dataRaw.fileTime = tempTime;
            }
        }
        else {
            repeatRef = 0;
            dataRaw.fileTime = queryFileRaw->value(4).toString();
            lastRef = dataRaw.indexNum;
        }
    } else {
        dataRaw.indexNum    = 0;
        dataRaw.fileName    = "";
        dataRaw.fileSize    = 0;
        dataRaw.fileTime    = "2020-04-01 00:00:00";
    }

    dataRaw.deviceId    = queryTeks->value(1).toByteArray();
    dataRaw.fileContent = queryTeks->value(3).toByteArray();
    dataRaw.indexTeks = queryTeks->value(0).toInt();

    dataRaw.parserMode = parserMode;

    hasilTransaksi = hasilTransaksiHost;
    deviceProfile = patternRegex;

    taskNum = dataRaw.indexNum;
    setAutoDelete(true);
}

ParsingTask::~ParsingTask()
{

}

void ParsingTask::run()
{
    for(int iter=0;iter<dataRaw.fileContent.size();iter++){
        if(dataRaw.fileContent.at(iter)==0)
            dataRaw.fileContent[iter]=' ';
    }

    QRegularExpression regexNomor("("+deviceProfile.patternNomor+")");
    QRegularExpression regexTanggal("("+deviceProfile.patternTanggal+")");
    QRegularExpression regexJam("("+deviceProfile.patternJam+")");
    QRegularExpression regexNilai("("+deviceProfile.patternNilai+")");
    QRegularExpression regexPajak("("+deviceProfile.patternPajak+")");
    QRegularExpression regexNilaiDanPajak("("+deviceProfile.patternNDP+")");
    QRegularExpression regexCF1("("+deviceProfile.patternCustomField1+")");
    QRegularExpression regexCF2("("+deviceProfile.patternCustomField2+")");
    QRegularExpression regexCF3("("+deviceProfile.patternCustomField3+")");

    HasilOlah tmpDataTransaksi;

    tmpDataTransaksi.transaksiHasil.nomor         = regexNomor.match(dataRaw.fileContent).captured(regexNomor.match(dataRaw.fileContent).lastCapturedIndex());
    tmpDataTransaksi.transaksiHasil.tanggal       = regexTanggal.match(dataRaw.fileContent).captured(regexTanggal.match(dataRaw.fileContent).lastCapturedIndex());
    tmpDataTransaksi.transaksiHasil.jam           = regexJam.match(dataRaw.fileContent).captured(regexJam.match(dataRaw.fileContent).lastCapturedIndex());
    tmpDataTransaksi.transaksiHasil.nilai         = regexNilai.match(dataRaw.fileContent).captured(regexNomor.match(dataRaw.fileContent).lastCapturedIndex());
    tmpDataTransaksi.transaksiHasil.pajak         = regexPajak.match(dataRaw.fileContent).captured(regexPajak.match(dataRaw.fileContent).lastCapturedIndex());
    tmpDataTransaksi.transaksiHasil.nilaiDanPajak = regexNilaiDanPajak.match(dataRaw.fileContent).captured(regexNilaiDanPajak.match(dataRaw.fileContent).lastCapturedIndex());
    tmpDataTransaksi.transaksiHasil.cF1           = regexCF1.match(dataRaw.fileContent).captured(regexCF1.match(dataRaw.fileContent).lastCapturedIndex());
    tmpDataTransaksi.transaksiHasil.cF2           = regexCF2.match(dataRaw.fileContent).captured(regexCF2.match(dataRaw.fileContent).lastCapturedIndex());
    tmpDataTransaksi.transaksiHasil.cF3           = regexCF3.match(dataRaw.fileContent).captured(regexCF3.match(dataRaw.fileContent).lastCapturedIndex());

    tmpDataTransaksi.indexSumber = dataRaw.indexNum;
    tmpDataTransaksi.indexTeks = dataRaw.indexTeks;
    tmpDataTransaksi.deviceId = dataRaw.deviceId;
    tmpDataTransaksi.parserMode = dataRaw.parserMode;
    tmpDataTransaksi.fileTime = dataRaw.fileTime;
    tmpDataTransaksi.skip = false;

    while(!hasilTransaksi->mutex.tryLock(10));

    hasilTransaksi->data.enqueue(tmpDataTransaksi);
    hasilTransaksi->mutex.unlock();
}
