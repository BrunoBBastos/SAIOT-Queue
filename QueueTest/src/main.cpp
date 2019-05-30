/*
O programa lê pulsos digitais, incrementa um contador e periodicamente, armazena o número de leituras e a hora
da sua ocorrência numa fila. Se houver conexão Wi-Fi, aqui simulada por uma flag, o programa publica os dados armazenados
na fila no SAIOT em ordem de chegada, caso contrário, os dados permanecem guardados em sequência até o momento da reconexão.

CONCLUÍDO >> Integrar a funcionalidade de armazenar os dados em fila ao Saiot
CONCLUÍDO >> Enviar os dados computados ao sistema
CONCLUÍDO >> Extrair dados da fila e publicá-los

CORRIGIDO >> Não está pegando data e hora (SaIoTCom::getDateNow())
          O método não funciona dentro de interrupções. Foram criadas 'flags' que chamam as funções a partir do loop
CORRIGIDO >> Está enviando apenas o último elemento da fila
          A perda de dados acontecia graças ao while que tentava enviar todos os dados de uma vez
          A partir do método handleLoop() é possível enviar apenas um dado por loop
          O while foi substituido por um if que testa se há elementos na fila uma vez por loop

 As linhas e funções marcadas como 'Teste' não são essenciais para o funcionamento
*/


#include <Arduino.h>
#include <QueueList.h>      // Fila <<< SUBSTITUIR PELA STD QUEUE
#include <Ticker.h>         // Timer
#include <SaIoTDeviceLib.h> // Saiot

#define PUSH_INTERVAL 1 // Intervalo entre pacotes de pulsos
#define SEND_INTERVAL 2 // Intervalo entre envios
#define SENSOR_PIN D3   // Porta do sensor
#define DEBOUNCE_DELAY 250 // Intervalo entre leituras válidas de cada pulsos

//Variáveis Globais ***************************** Variáveis Globais
struct data // Conjunto de informações a serem enviadas
{
  int value;
  String time;
};

unsigned long debounce = 0;
volatile int pulseCounter = 0; // Contador de pulsos do sensor

bool wifiSt = true; // Estado da conexão WIFI (Teste)
bool dReady = false;  // Indica se há dados a serem enviados
bool time2Push = false, time2Pop = false; // Flags das interrupções internas

int total = 0; // Teste

// Objetos *********************************************** Objetos
QueueList<data> dataQ; // Fila FIFO

Ticker PushData;
Ticker SendQ;

WiFiClient espClient;
SaIoTDeviceLib hidrometro("Hidrometro", "230519Tst", "ricardo@email.com");
SaIoTSensor medidorAgua("hd01", "hidrometro_01", "Litros", "number");
String senha = "12345678910";
void callback(char *topic, byte *payload, unsigned int length);

// Funções *********************************************** Funções
void pulseRead();
void pushQ();
void pushQflag();
void popQflag();
void sendData2Saiot(int d, String t);
void popQ();
void toggleWifi();
void printPulses();

// ********************************************************* SETUP
void setup()
{
  Serial.begin(115200); // (Teste)
  Serial.println();
  Serial.println();
  Serial.println("Comecou");

  pinMode(SENSOR_PIN, INPUT_PULLUP); // Configura porta do sensor
  // Associa interrupção ext ao pino do sensor
  attachInterrupt(digitalPinToInterrupt(SENSOR_PIN), pulseRead, RISING);

  hidrometro.addSensor(medidorAgua);
  hidrometro.preSetCom(espClient, callback); 
  hidrometro.start(senha);
  
  PushData.attach(PUSH_INTERVAL, pushQflag); // Inicializa timer de armazenamento
  SendQ.attach(SEND_INTERVAL, popQflag);     // Inicializa timer de envio
}

// ********************************************************** LOOP
void loop()
{
  toggleWifi(); // Envie 'w' pela Serial para 'ligar/desligar'
  if(time2Push) pushQ();
  if(time2Pop) popQ();
  hidrometro.handleLoop();
}

void callback(char* topic, byte* payload, unsigned int length){
  String payloadS;
  Serial.print("Topic: ");
  Serial.println(topic);
  for (unsigned int i=0;i<length;i++) {
    payloadS += (char)payload[i];
  }
  if(strcmp(topic,hidrometro.getSerial().c_str()) == 0){
    Serial.println("SerialLog: " + payloadS);
  }
}

void pulseRead() 
{ // << Aprimorar forma do debounce
  static unsigned long lastDebounce = 0; // Registra a última marcação de tempo
  debounce = millis();     // Tempo atual
  if (debounce - lastDebounce > DEBOUNCE_DELAY)
  { // Se intervalo entre chamadas for maior que intervalo de leituras válidas
    pulseCounter++;
    total++; // Teste
    printPulses(); // Teste
  }
  lastDebounce = debounce; // Reseta a última marcação de tempo
}

void pushQ()
{ // Preenche a fila
  if (pulseCounter > 0)
  { // Apenas se houverem dados
    String dateTime;
    dateTime = SaIoTCom::getDateNow();
    data d;
    d.value = pulseCounter;
    d.time = dateTime;
    dataQ.push(d);            // Armazena os dados na lista
    pulseCounter = 0;         // Reseta contador
    dReady = true;             // Indica existência de dados prontos para envio
    time2Push = false; // Indica que o dado foi criado e depositado na fila
  }
}

void popQ()
{
  if (wifiSt && dReady)
  { // Se houverem dados prontos e WIFI disponível
    if (dataQ.count() > 0)
    {                          // Enquanto houverem elementos na fila
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
}

void sendData2Saiot(int d, String t)
{ // Enviar dados
  medidorAgua.sendData((d), t);
}

void pushQflag(){
time2Push = true;
}

void popQflag(){
time2Pop = true;
}

void toggleWifi(){
if (Serial.available() > 0) 
  { // (Teste) Envie 'w' pela serial para simular o "status do wifi"
    char c = Serial.read();
    if (c == 'w')
      wifiSt = !wifiSt;
  Serial.print("Status do Wifi: ");
  Serial.println(wifiSt);
  }
}

void printPulses(){
    Serial.print("\t \t \t \t"); // (Teste)
    Serial.print(total);
    Serial.print("\t");
    Serial.println(debounce);
}