// Integrar a funcionalidade de armazenar os dados em fila ao Saiot
// Enviar os dados computados ao sistema
// Extrair dados da fila e publicá-los

// Não está pegando data e hora

#include <Arduino.h>
#include <QueueList.h>      // Fila <<< SUBSTITUIR PELA STD QUEUE
#include <Ticker.h>         // Timer
#include <SaIoTDeviceLib.h> // Saiot

#define PUSH_INTERVAL 2 // Intervalo entre pacotes de pulsos
#define SEND_INTERVAL 4 // Intervalo entre envios
#define SENSOR_PIN D3   // Porta do sensor
#define DEBOUNCE_DELAY 250 // Intervalo entre leituras válidas de cada pulsos
struct data // Conjunto de informações a serem enviadas
{
  int value;
  String time;
  //unsigned long time;
};

volatile int pulseCounter = 0; // Contador de pulsos do sensor

bool wifiSt = false; // Estado da conexão WIFI(teste)
bool ready = false;  // Indica se há dados a serem enviados

QueueList<data> dataQ; // Lista FIFO

Ticker PushData;
Ticker SendQ;

WiFiClient espClient;
SaIoTDeviceLib hidrometro("Hidrometro", "230519Tst", "ricardo@email.com");
SaIoTSensor medidorAgua("hd01", "hidrometro_01", "Litros", "number");
String senha = "12345678910";
void callback(char *topic, byte *payload, unsigned int length);

void sendData2Saiot(int d, String t)
{ // Enviar dados
  //String dateTime = SaIoTCom::getDateNow();
  medidorAgua.sendData((d), t);
}

void pulseRead()
{                                        // Leitura de pulsos do sensor
  static unsigned long lastDebounce = 0; // Registra a última marcação de tempo
  unsigned long debounce = millis();     // Tempo atual
  if (debounce - lastDebounce > DEBOUNCE_DELAY)
  { // Se intervalo entre chamadas for maior que intervalo de leituras válidas
    pulseCounter++;
    Serial.print("\t \t \t +1 \t"); // (Teste)
    Serial.println(debounce);
  }
  lastDebounce = debounce; // Reseta a última marcação de tempo
}

void pushQ()
{ // Preenche a fila
  if (pulseCounter > 0)
  { // Apenas se houverem dados
  String dateTime = SaIoTCom::getDateNow();
    data d;
    d.value = pulseCounter;
    d.time = dateTime;//millis() / 1000; // (Teste) Pegar a hora do SAIOT <<< CORRIGIR
    dataQ.push(d);            // Armazena os dados na lista
    pulseCounter = 0;         // Reseta contador
    ready = true;             // Indica existência de dados prontos para envio
  }
}

void popQ()
{
  //if (ready && !wifiSt) // (Teste)
   // Serial.println("waiting for wifi");
  if (wifiSt && ready)
  { // Se houverem dados prontos e WIFI disponível
    while (dataQ.count() > 0)
    {                          // Enquanto houverem elementos na fila
      data out = dataQ.pop();  // Retira o primeiro dado da fila
      //Serial.print(out.value); // Envia
      //Serial.print('\t');
      //Serial.println(out.time);
      sendData2Saiot(out.value, out.time);
    }
    ready = false; // Reseta a flag
  }
}

void setup()
{
  pinMode(SENSOR_PIN, INPUT_PULLUP); // Configura porta do sensor

  hidrometro.addSensor(medidorAgua);
  hidrometro.preSetCom(espClient, callback); // Função padrão obrigatória (?)
  hidrometro.start(senha);

  Serial.begin(115200); // (Teste)
  Serial.println();
  Serial.println("Comecou");
  // Associa interrupção ext ao pino do sensor
  attachInterrupt(digitalPinToInterrupt(SENSOR_PIN), pulseRead, RISING);

  PushData.attach(PUSH_INTERVAL, pushQ); // Inicializa timer de armazenamento
  SendQ.attach(SEND_INTERVAL, popQ);     // Inicializa timer de envio
}

void loop()
{
  if (Serial.available() > 0) // (Teste) Envie 'w' pela serial para simular o "status do wifi"
  {
    char c = Serial.read();
    if (c == 'w')
      wifiSt = !wifiSt;
  }

  //sendData2Saiot();
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