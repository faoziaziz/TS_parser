#include "tsdata.h"

DataRaw::DataRaw()
{
    isDecompressed = false;
    isDecrypted = false;
}

void DataRaw::Decompress(void)
{
    if (!isDecompressed &&
            (fileName.contains(".CMP") || fileName.contains(".CRS"))
            )
    {
        int iterJ = 0;
        QByteArray olah;

        olah.clear();

        while(iterJ<fileContent.size()-2)
        {
            char p = fileContent.at(iterJ);
            if (p=='/'){
                int q = (unsigned char) fileContent.at(iterJ+2);
                char s = fileContent.at(iterJ+1);
                QByteArray tmpByteArray(q, s);
                olah.append(tmpByteArray);
                iterJ+=2;
            } else {
                olah.append(p);
            }
            iterJ++;
        }
        while(iterJ<fileContent.size())
        {
            olah.append(fileContent.at(iterJ));
            iterJ++;
        }

        fileContent = olah;
        isDecompressed = true;
    }
}

void DataRaw::Decrypt(void)
{
    if(!isDecrypted &&
            (fileName.contains(".CXT") || fileName.contains(".CRS"))
            )
    {
        QByteArray Hasil;

        for(int i=0;i<fileContent.size()-15;i+=16 ){
            QByteArray Olah=fileContent.mid(i,16);
            uint8_t tmpHasil[20];
            AES128_ECB_decrypt((uint8_t*)Olah.data(),(uint8_t*)"PRASIMAX01234567",tmpHasil);
            Hasil.append(QByteArray((const char*)tmpHasil,16));
        }

        fileContent = Hasil;
    }
}
