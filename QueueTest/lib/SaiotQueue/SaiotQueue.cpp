/*
    Biblioteca de filas de dados

    Autor: Bruno B. Bastos
    Data: 06/2019

1) Criar um timer (ou 2) durante a declaração da nova lista
    > O timer deve gerenciar o intervalo de armazenagem e envio de dados

2)



*/

#include "SaiotQueue.h"
#include "QueueList.h"
using namespace std


SaiotQueue::SaiotQueue(){ // Construtor : criar timer de armazenamento, envio, gerar uma fila no tipo desejado
struct data
{
    typename value;
    String instant;
}

}

void SaiotQueue::init(){ // 

}

void SaiotQueue::saveData(){ // Armazena o dado coletado pelo sensor
    
}

void SaiotQueue::popData(){ // Remove o primeiro dado da fila e envia
    // Verificar o status do Wi-Fi e se há dados na fila
    //      




}
/*
if (wifiSt && dReady)
  { // Se houverem dados prontos e WIFI disponível
    if (dataQ.count() > 0)
    {                          // Se houverem elementos na fila
      data out = dataQ.pop();  // Retira o primeiro dado da fila
      sendData2Saiot(out.value, out.time); // Envia
      Serial.print(out.value); 
      Serial.print('\t');
      Serial.println(out.time);
    }
    else {
       time2Pop = false;
       dReady = false; // Reseta a flag
    }
  }
*/
bool SaiotQueue::dataReady(){ // Indica se há dados prontos para envio

}
