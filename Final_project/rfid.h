#include "mbed.h"
#include "MFRC522.h"

class RFID_Reader{
    public:
        RFID_Reader();
        RFID_Reader(MFRC522 *mfrc522);
        bool read();
        int getSize() { return _idSize; }
        int* getID() { return _id; }
        void getCharID(char*);
    private:
        MFRC522 *_mfrc522;
        int *_id;
        int _idSize;
        void _hex2str(int, char*);
        char _hex2char(int);
};