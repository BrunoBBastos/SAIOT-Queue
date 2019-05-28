// Integrar a funcionalidade de armazenar os dados em fila ao Saiot
// Enviar os dados computados ao sistema
// Extrair dados da fila e publicá-los

/*  Não está pegando data e hora ==> CORRIGIDO
          O método não funciona dentro de interrupções. Foram criadas 'flags' que chamam as funções a partir do loop
    Está enviando apenas o último elemento da fila ==> CORRIGIDO
          A perda de dados acontecia graças ao while que tentava enviar todos os dados de uma vez
          A partir do método o método handleLoop() é possível enviar apenas um dado por loop
          O while foi substituido por um if que testa se há elementos na fila uma vez por loop
*/

#include <Arduino.h>
#include <QueueList.h>      // Fila <<< SUBSTITUIR PELA STD QUEUE
#include <Ticker.h>         // Timer
#include <SaIoTDeviceLib.h> // Saiot

#define PUSH_INTERVAL 1 // Intervalo entre pacotes de pulsos
#define SEND_INTERVAL 2 // Intervalo entre envios
#define SENSOR_PIN D3   // Porta do sensor
#define DEBOUNCE_DELAY 250 // Intervalo entre leituras válidas de cada pulsos
struct data // Conjunto de informações a serem enviadas
{
  int value;
  String time;
  //unsigned long time;
};

volatile int pulseCounter = 0; // Contador de pulsos do sensor

bool wifiSt = true; // Estado da conexão WIFI(teste)
bool ready = false;  // Indica se há dados a serem enviados
bool time2Push = false, time2Pop = false;

int total = 0; // Teste

QueueList<data> dataQ; // Lista FIFO

Ticker PushData;
Ticker SendQ;

WiFiClient espClient;
SaIoTDeviceLib hidrometro("Hidrometro", "230519Tst", "ricardo@email.com");
SaIoTSensor medidorAgua("hd01", "hidrometro_01", "Litros", "number");
String senha = "12345678910";
void callback(char *topic, byte *payload, unsigned int length);

void pulseRead()
{                                        // Leitura de pulsos do sensor
  static unsigned long lastDebounce = 0; // Registra a última marcação de tempo
  unsigned long debounce = millis();     // Tempo atual
  if (debounce - lastDebounce > DEBOUNCE_DELAY)
  { // Se intervalo entre chamadas for maior que intervalo de leituras válidas
    pulseCounter++;
    total++;
    Serial.print("\t \t \t \t"); // (Teste)
    Serial.print(total);
    Serial.print("\t");
    Serial.println(debounce);
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
    d.time = dateTime;//millis() / 1000; // (Teste) Pegar a hora do SAIOT <<< CORRIGIR
    dataQ.push(d);            // Armazena os dados na lista
    pulseCounter = 0;         // Reseta contador
    ready = true;             // Indica existência de dados prontos para envio
    time2Push = false; // Indica que o dado foi criado e depositado na fila
  }
}

void pushQflag(){
time2Push = true;
}

void popQflag(){
time2Pop = true;
}

void sendData2Saiot(int d, String t)
{ // Enviar dados
  medidorAgua.sendData((d), t);
}

void popQ()
{
 
  if (wifiSt && ready)
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
       ready = false; // Reseta a flag
    }
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

  PushData.attach(PUSH_INTERVAL, pushQflag); // Inicializa timer de armazenamento
  SendQ.attach(SEND_INTERVAL, popQflag);     // Inicializa timer de envio
}

void loop()
{
  if (Serial.available() > 0) // (Teste) Envie 'w' pela serial para simular o "status do wifi"
  {
    char c = Serial.read();
    if (c == 'w')
      wifiSt = !wifiSt;
  }
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