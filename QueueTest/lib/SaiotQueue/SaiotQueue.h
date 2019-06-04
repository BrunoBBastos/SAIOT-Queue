/*
    Biblioteca de filas de dados

    Autor: Bruno B. Bastos
    Data: 06/2019
*/

#ifndef SaIoTQueue_h
#define SaIoTQueue_h

#include "Arduino.h"
#include "QueueList.h"

class SaiotQueue
{
public:
void init();
void saveData();
void popData();
bool dataReady();



private:


};



#endif