#include <QCoreApplication>
#include <QDebug>
#include <QThread>
#include <QTime>

#include "config.h"
#include "parser.h"

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    qDebug() << APP_NAME << APP_VER;
    qDebug() << PRASIMAX << APP_DATE;

    QDateTime deadline (QDate (EXPIRED_DATE));
    QDateTime sekarang = QDateTime::currentDateTime();
    if (sekarang.daysTo(deadline)<0)
    {
        qDebug() << "Application error, code 88: need update";
        return 0;
    }

    if(argc!=7){
        qDebug()<<"Usage: Parser <DB_Addr> <DB_Name> <DB_Port> <User> <Pass> <Mode>";
        return 0;
    }

    QThread* threadParser = new QThread;
    Parser* parser = new Parser;

    parser->SetDatabase(argc,argv);

    parser->moveToThread(threadParser);
    QObject::connect(threadParser, SIGNAL(started()), parser, SLOT(ProcessData()));

    threadParser->start();


    return a.exec();
}
