#include "parser.h"

void Parser::ProcessData(void)
{
    //qDebug() << "ProcessData.";

    if (!dbConnection.isOpen())
    {
        if(!dbConnection.open())
        {
            this->ReconnectDatabase();
        }
        RefreshDeviceList();
        SaveResult();
        if (parserMode == 2 || parserMode == 3)
        {
            timerProcessData_2->start(MS_PROCESS_DATA);
        }

        if (parserMode != 1 && parserMode != 3)
        {
            return;
        }
    }

    queryLimit = PA_MAX_THREAD_QUEUE - taskCreated + taskFinished;

    if (queryLimit > PA_MAX_QUERY_LIMIT)
        queryLimit = PA_MAX_QUERY_LIMIT;

    if(queryLimit>0)
    {
        int newTaskCount = 0;
        int countResult = 0;

        QSqlQuery queryFileRaw("SELECT idFileTransferStage2, DeviceId, FileName, FileSize, FileData, FileTime FROM FileTransferStage2 WHERE idFileTransferStage2>? AND FlagParser=0 ORDER BY idFileTransferStage2 ASC LIMIT ?",dbConnection);
        queryFileRaw.bindValue(0,queryLastIndex);
        queryFileRaw.bindValue(1,queryLimit);

        if(queryFileRaw.exec())
        {
            while(queryFileRaw.next())
            {
                countResult++;

                //cek parserMode
                queryLastIndex = queryFileRaw.value(0).toInt();

                int indexNum = queryFileRaw.value(0).toInt();
                int parserMode = (deviceList.contains(queryFileRaw.value(1).toByteArray()))? deviceList.value(queryFileRaw.value(1).toByteArray()).parserMode : 0;
                DeviceProfile pattern = deviceList.value(queryFileRaw.value(1).toByteArray());
                if(parserMode == 1)
                {
                    //olah parsing
                    parsingTask.insert(indexNum, new ParsingTask(&queryFileRaw, parserMode, &hasilTransaksi, pattern));

                    threadPool->start(parsingTask.value(indexNum));

                    taskCreated++;
                    newTaskCount++;

                    //qDebug()<<">>>"<<indexNum << queryFileRaw.value(1).toByteArray() << parserMode << threadPool->activeThreadCount() << taskCreated - taskFinished;
                } else {
                    {
                        //kirim skipflag ke SaveResult via Mutex;
                        HasilOlah tmpHasilOlah;
                        tmpHasilOlah.indexSumber = indexNum;
                        tmpHasilOlah.deviceId = queryFileRaw.value(1).toByteArray();
                        tmpHasilOlah.parserMode = 1;
                        tmpHasilOlah.skip = true;

                        while(hasilTransaksi.mutex.tryLock(10));
                        hasilTransaksi.data.enqueue(tmpHasilOlah);
                        hasilTransaksi.mutex.unlock();
                    }
                    taskSkipped++;

                    //qDebug()<<"***"<<indexNum << queryFileRaw.value(1).toByteArray() << 1;
                }

            }

        } // else?

        if(countResult>0)
        {
            timerProcessData->start(MS_PROCESS_DATA);
            noDataCounter=0;
        } else {
            timerProcessData->start(MS_PROCESS_NODATA_DELAY);
            noDataCounter++;
            qDebug()<< "..... 1" << queryLastIndex << noDataCounter;
        }
    } else {
        timerProcessData->start(MS_PROCESS_DATA);
    }

}


void Parser::ProcessData_2(void)
{
    //qDebug() << "ProcessData_2.";

    queryLimit=PA_MAX_THREAD_QUEUE - taskCreated + taskFinished;

    if (queryLimit > PA_MAX_QUERY_LIMIT)
        queryLimit = PA_MAX_QUERY_LIMIT;

    if(queryLimit>0)
    {
        int newTaskCount = 0;
        int countResult = 0;

        QSqlQuery queryTeks("SELECT SeqNum, DeviceId, RefSN, Data FROM Teks WHERE SeqNum > ? AND FlagParser = 0 ORDER BY SeqNum ASC LIMIT ?", dbConnection);
        queryTeks.bindValue(0,queryLastIndex_2);
        queryTeks.bindValue(1,queryLimit);


        if(queryTeks.exec())
        {
            while(queryTeks.next())
            {
                queryLastIndex_2 = queryTeks.value(0).toInt();

                int refSnImage = queryTeks.value(2).toInt();
                QSqlQuery queryImage("SELECT RefSN FROM Image WHERE SeqNum = ?", dbConnection);
                queryImage.bindValue(0, refSnImage);
                countResult++;

                //cek parserMode
                QSqlQuery queryFileRaw("SELECT idFileTransferStage2, DeviceId, FileName, FileSize, FileTime FROM FileTransferStage2 WHERE idFileTransferStage2 = ?",dbConnection);
                int indexNum = 0;// = queryTeks.value(0).toInt();

                int parserMode = (deviceList.contains(queryTeks.value(1).toByteArray()))? deviceList.value(queryTeks.value(1).toByteArray()).parserMode : 0;
                DeviceProfile pattern = deviceList.value(queryTeks.value(1).toByteArray());
                if(parserMode == 2 || parserMode == 3)
                {
                    while(!queryImage.exec())
                    {
                        ReconnectDatabase();
                    }

                    int refSnFTS2 = 0;
                    if (queryImage.next())
                    {
                        refSnFTS2 = queryImage.value(0).toInt();

                    }//else ?

                    queryFileRaw.bindValue(0, refSnFTS2);

                    while(!queryFileRaw.exec())
                    {
                        ReconnectDatabase();
                    }

                    if (queryFileRaw.next())
                    {
                        indexNum = queryImage.value(0).toInt();
                    }//else ?

                    //olah parsing
                    parsingTask.insert(indexNum, new ParsingTask(&queryFileRaw, &queryTeks, parserMode, &hasilTransaksi, pattern));

                    threadPool->start(parsingTask.value(indexNum));

                    taskCreated++;
                    newTaskCount++;

                    //qDebug()<<">>>"<<indexNum << queryFileRaw.value(1).toByteArray() << parserMode << threadPool->activeThreadCount() << taskCreated - taskFinished;
                } else {
                    {
                        //kirim skipflag ke SaveResult via Mutex;
                        HasilOlah tmpHasilOlah;
                        tmpHasilOlah.indexSumber = indexNum;
                        tmpHasilOlah.deviceId = queryFileRaw.value(1).toByteArray();
                        tmpHasilOlah.parserMode = 2;
                        tmpHasilOlah.skip = true;

                        while(hasilTransaksi.mutex.tryLock(10));
                        hasilTransaksi.data.enqueue(tmpHasilOlah);
                        hasilTransaksi.mutex.unlock();
                    }
                    taskSkipped++;

                    //qDebug()<<"***"<<indexNum << queryFileRaw.value(1).toByteArray() << 1;
                }

            }

        } // else?

        if(countResult>0)
        {
            timerProcessData_2->start(MS_PROCESS_DATA);
            noDataCounter_2=0;
        } else {
            timerProcessData_2->start(MS_PROCESS_NODATA_DELAY);
            noDataCounter_2++;
            qDebug()<< "..... 2" << queryLastIndex_2 << noDataCounter_2;
        }
    } else {
        timerProcessData_2->start(MS_PROCESS_DATA);
    }

}



void Parser::SaveResult(void)
{
    //qDebug() << "SaveResult.";
    int counter = 0;
    bool saveTransaksi = false;
    bool updateFlag = false;
    bool updateFlag_2 = false;

    QSqlQuery queryUpdateFlag_2("UPDATE Teks SET FlagParser=? WHERE SeqNum=?");
    QSqlQuery queryUpdateFlag("UPDATE FileTransferStage2 SET FlagParser=? WHERE idFileTransferStage2=?");
    QSqlQuery querySaveTransaksi("INSERT INTO Transaksi (DeviceId, RefSN, FileTime, Nomor, Tanggal, Jam, Nilai, Pajak, NilaiDanPajak, CustomField1, CustomField2, CustomField3) VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)");
    while(!hasilTransaksi.data.isEmpty() )
    {
        HasilOlah tmpHasilOlah;
        tmpHasilOlah = hasilTransaksi.data.dequeue();
        counter++;
        if(!tmpHasilOlah.skip)
        {
            //InsertTransaksi(tmpHasilOlah.indexSumber, tmpHasilOlah.deviceId, tmpHasilOlah.fileTime, tmpHasilOlah.transaksiHasil);
            {
                querySaveTransaksi.bindValue(0, tmpHasilOlah.deviceId.isEmpty()?"":tmpHasilOlah.deviceId);
                querySaveTransaksi.bindValue(1, tmpHasilOlah.indexSumber);
                querySaveTransaksi.bindValue(2, tmpHasilOlah.fileTime.isEmpty()?"2010-01-01 00:00:00":tmpHasilOlah.fileTime);
                querySaveTransaksi.bindValue(3, tmpHasilOlah.transaksiHasil.nomor.isEmpty()?"":tmpHasilOlah.transaksiHasil.nomor);
                querySaveTransaksi.bindValue(4, tmpHasilOlah.transaksiHasil.tanggal.isEmpty()?"":tmpHasilOlah.transaksiHasil.tanggal);
                querySaveTransaksi.bindValue(5, tmpHasilOlah.transaksiHasil.jam.isEmpty()?"":tmpHasilOlah.transaksiHasil.jam);
                querySaveTransaksi.bindValue(6, tmpHasilOlah.transaksiHasil.nilai.isEmpty()?"":tmpHasilOlah.transaksiHasil.nilai);
                querySaveTransaksi.bindValue(7, tmpHasilOlah.transaksiHasil.pajak.isEmpty()?"":tmpHasilOlah.transaksiHasil.pajak);
                querySaveTransaksi.bindValue(8, tmpHasilOlah.transaksiHasil.nilaiDanPajak.isEmpty()?"":tmpHasilOlah.transaksiHasil.nilaiDanPajak);
                querySaveTransaksi.bindValue(9, tmpHasilOlah.transaksiHasil.cF1.isEmpty()?"":tmpHasilOlah.transaksiHasil.cF1);
                querySaveTransaksi.bindValue(10, tmpHasilOlah.transaksiHasil.cF2.isEmpty()?"":tmpHasilOlah.transaksiHasil.cF2);
                querySaveTransaksi.bindValue(11, tmpHasilOlah.transaksiHasil.cF3.isEmpty()?"":tmpHasilOlah.transaksiHasil.cF3);

                saveTransaksi = true;
            }
            taskFinished++;
        }

        if (tmpHasilOlah.parserMode == 1)
        {
            //UpdateFlagParser(tmpHasilOlah.indexSumber, tmpHasilOlah.parserMode);
            if (!tmpHasilOlah.skip)
                queryUpdateFlag.bindValue(0, tmpHasilOlah.parserMode);
            else
                queryUpdateFlag.bindValue(0, DB_FLAG_SKIP);
            queryUpdateFlag.bindValue(1, tmpHasilOlah.indexSumber);

            updateFlag = true;
        }
        if (tmpHasilOlah.parserMode == 2)
        { //parserMode == 2
            //UpdateFlagParser_2(tmpHasilOlah.indexTeks, tmpHasilOlah.parserMode);
            if (!tmpHasilOlah.skip)
                queryUpdateFlag_2.bindValue(0, tmpHasilOlah.parserMode);
            else
                queryUpdateFlag_2.bindValue(0, DB_FLAG_SKIP);
            queryUpdateFlag_2.bindValue(1, tmpHasilOlah.indexTeks);

            updateFlag_2 = true;
        }


        qDebug()<<"<<"
               << tmpHasilOlah.indexSumber
               << tmpHasilOlah.deviceId
               << tmpHasilOlah.parserMode
               << taskFinished
               << tmpHasilOlah.transaksiHasil.nomor
               << tmpHasilOlah.transaksiHasil.tanggal
               << tmpHasilOlah.transaksiHasil.nilai
               //<< hasilTransaksi.data.count()
                  ;
                  //<< taskCreated - taskFinished << threadPool->activeThreadCount();
        if (counter == PA_INSERT_QUERY_LIMIT)
        {
            if (saveTransaksi){
                while(!querySaveTransaksi.exec())
                    ReconnectDatabase();
            }
            if (updateFlag)
            {
                while(!queryUpdateFlag.exec())
                    ReconnectDatabase();

            }

            if (updateFlag_2) {
                while(!queryUpdateFlag_2.exec())
                    ReconnectDatabase();
            }

            counter = 0;
            saveTransaksi = false;
            updateFlag = false;
            updateFlag_2 = false;
        }
    }

    if (counter > 0)
    {
        if (saveTransaksi) querySaveTransaksi.exec();
        if (updateFlag) queryUpdateFlag.exec();
        if (updateFlag_2) queryUpdateFlag_2.exec();
        counter = 0;
    }
    timerSaveResult->start(MS_SAVE_RESULT);
}


//Constructor
Parser::Parser(QObject *parent) : QObject(parent)
{
    maxThread=PA_MAX_THREAD;
    queryLastIndex=0;
    queryLastIndex_2=0;

    timerProcessData = new QTimer(this);
    timerProcessData->setSingleShot(true);
    connect(timerProcessData, SIGNAL(timeout()), this, SLOT(ProcessData()));

    timerProcessData_2 = new QTimer(this);
    timerProcessData_2->setSingleShot(true);
    connect(timerProcessData_2, SIGNAL(timeout()), this, SLOT(ProcessData_2()));

    timerDeviceList = new QTimer(this);
    timerDeviceList->setSingleShot(true);
    connect(timerDeviceList, SIGNAL(timeout()), this, SLOT(RefreshDeviceList()));

    timerSaveResult = new QTimer(this);
    timerSaveResult->setSingleShot(true);
    connect(timerSaveResult, SIGNAL(timeout()), this, SLOT(SaveResult()));


    threadPool = new QThreadPool(this);
    threadPool->setMaxThreadCount(maxThread);
    taskCreated = 0;
    taskFinished = 0;
    taskSkipped = 0;

    noDataCounter = 0;
    noDataCounter_2 = 0;

}

//Destructor
Parser::~Parser()
{

}

//Databases;
bool Parser::SetDatabase(int argc, char *argv[])
{
    if(argc<7)
        return false;

    bool isDriver = dbConnection.isDriverAvailable("QMYSQL");
    dbConnection = QSqlDatabase::addDatabase("QMYSQL");

    dbConnection.setHostName(argv[1]);
    dbConnection.setDatabaseName(argv[2]);
    dbConnection.setPort(QString::fromLocal8Bit(argv[3]).toInt());
    dbConnection.setUserName(argv[4]);
    dbConnection.setPassword(argv[5]);

    parserMode = QString::fromLocal8Bit(argv[6]).toInt();

    return isDriver;
}

bool Parser::ReconnectDatabase(void)
{
    qDebug()<< "DB connection is lost!";
    dbConnection.close();
    int ReconnectCount=0;
    while(!dbConnection.isOpen()){
        QThread::sleep(10);
        qDebug()<<"Reconnect... " << ++ReconnectCount;
        dbConnection.open();
    }
    qDebug() << "DB connection is back.";

    return true;
}

void Parser::RefreshDeviceList(void)
{
    //qDebug()<<"Refresh."; ///

    QSqlQuery queryDevice("SELECT DeviceId, ParserMode, PatternNomor, PatternTanggal, PatternJam, PatternNilai, PatternPajak, PatternNDP, PatternCustomField1, PatternCustomField2, PatternCustomField3 from DeviceTable WHERE ParserMode>0 ORDER BY id ASC Limit ? ",dbConnection);
    queryDevice.bindValue(0, DEVICE_LIMIT);

    while(!queryDevice.exec())
        ReconnectDatabase();

    while(queryDevice.next())
    {
        DeviceProfile tmpProfile;
        tmpProfile.deviceId = queryDevice.value("DeviceId").toByteArray();
        tmpProfile.parserMode = queryDevice.value("ParserMode").toInt();
        tmpProfile.patternNomor = queryDevice.value("PatternNomor").toString();
        tmpProfile.patternTanggal = queryDevice.value("PatternTanggal").toString();
        tmpProfile.patternJam = queryDevice.value("PatternJam").toString();
        tmpProfile.patternNilai = queryDevice.value("PatternNilai").toString();
        tmpProfile.patternPajak = queryDevice.value("PatternPajak").toString();
        tmpProfile.patternNDP = queryDevice.value("PatternNDP").toString();
        tmpProfile.patternCustomField1 = queryDevice.value("PatternCustomField1").toString();
        tmpProfile.patternCustomField2 = queryDevice.value("PatternCustomField2").toString();
        tmpProfile.patternCustomField3 = queryDevice.value("PatternCustomField3").toString();

        deviceList.insert(tmpProfile.deviceId,tmpProfile);
    }

    //todo: cleanup untuk deviceList entry yang sudah terhapus di DB

    timerDeviceList->start(MS_REFRESH_DEVICE_LIST); //refresh tiap 30 detik

}
