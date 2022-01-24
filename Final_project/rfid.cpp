#include "rfid.h"

RFID_Reader::RFID_Reader()
{
    _mfrc522 = 0;
    _id = 0;
}

RFID_Reader::RFID_Reader(MFRC522 *mfrc522)
{
    _mfrc522 = mfrc522;
    _mfrc522->PCD_Init();
    _id = new int[10];
}

bool RFID_Reader::read()
{
    if ( ! _mfrc522->PICC_IsNewCardPresent()) return false;
    if ( ! _mfrc522->PICC_ReadCardSerial()) return false;
    _idSize = _mfrc522->uid.size;
    for(int i = 0; i < _idSize; ++i) _id[i] = _mfrc522->uid.uidByte[i];
    _mfrc522->PICC_HaltA();
    return true;
}

void RFID_Reader::getCharID(char *idName)
{
    char idDigit[2];
    for(int i = 0; i < _idSize; ++i){
        _hex2str(_id[i], idDigit);
        idName[2*i] = idDigit[0];
        idName[2*i+1] = idDigit[1];
    }
}

void RFID_Reader::_hex2str(int num, char* id)
{
    id[0] = _hex2char(num/16);
    id[1] = _hex2char(num%16);
}

char RFID_Reader::_hex2char(int num)
{
    char digit;
    switch(num){
        case 1: digit = '1'; break;
        case 2: digit = '2'; break;
        case 3: digit = '3'; break;
        case 4: digit = '4'; break;
        case 5: digit = '5'; break;
        case 6: digit = '6'; break;
        case 7: digit = '7'; break;
        case 8: digit = '8'; break;
        case 9: digit = '9'; break;
        case 10: digit = 'A'; break;
        case 11: digit = 'B'; break;
        case 12: digit = 'C'; break;
        case 13: digit = 'D'; break;
        case 14: digit = 'E'; break;
        case 15: digit = 'F'; break;
        case 0: digit = '0'; break;
        default: digit = 'g';
    }
    return digit;
}