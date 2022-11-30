#include <Adafruit_Sensor.h>                 //Biblioteca para sensor de humedad
#include <DHT.h>                             //Biblioteca para sensor de humedad
#include <DHT_U.h>                           //Biblioteca para sensor de humedad
#include <Temperatura.h>                     //Biblioteca propia para sensor de temperatura
#include <WiFi.h>                            //Biblioteca para usar WiFi
#include <PubSubClient.h>                    //Biblioteca para usar MQTT

#define DHTPIN 22                            //Pin del sensor de humedad
#define TERPIN 33                            //Pin del sensor de temperatura
#define SERIESRES 10000                      //Resistencia en serie con termistor
#define DHTTYPE DHT11                        //Tipo de sensor de humedad
#define uS_TO_S_FACTOR 1000000               //Factor de conversión de segundos a microsegundos
#define TIME_TO_SLEEP  5                     //Segundos a dormir el ESP
#define HUMEDAD_MINIMA 50                    //Humedad para regar la planta
#define MQTT_LENGTH 20                       //Largo del dato a enviar por mqtt
char msg[MQTT_LENGTH];
const char *ssid = "chuuyas";                //Nombre de la red
const char *password = "53280077";           //Contraseña de la red
const char *broker = "192.168.0.100";        //Ip del broker o de la raspberry
const char *clientId = "ESP32Client_3";      //Cliente conectado al broker mqtt
const char *topic = "/plantas/albahaca";     //Tópico a publicar los datos

DHT_Unified dht(DHTPIN, DHTTYPE);            //Creación del objeto dht para manejo de sensor de humedad
Termistor termistor(TERPIN, SERIESRES);      //Creación del objeto termistor para manejo de sensor de temperatura
WiFiClient espClient;                        //Objeto para manejar WiFi
PubSubClient client(espClient);              //Objeto para manejar MQTT usando la conexión WiFi
uint32_t delayMS;                            //Variable global de tiempo de espera para la lectura del sensor de humedad para evitar saturarlo de peticiones

void setup_wifi();                                      //Declaración de función para conectar WiFi
void send_message(const char *clientId, char msg);      //Declaración de función para conectar a mqtt

void setup() {
  Serial.begin(9600);
  //Incia la conexión a la red
  setup_wifi();
  //Inicializar broker MQTT
  client.setServer(broker, 1883);
  //Inicialización de sensor de humedad
  dht.begin();
  sensor_t sensor;
  dht.temperature().getSensor(&sensor);
  dht.humidity().getSensor(&sensor);
  delayMS = sensor.min_delay / 1000;
  //Configuración de deep sleep mode para ahorro de energía
  esp_sleep_enable_timer_wakeup(TIME_TO_SLEEP * uS_TO_S_FACTOR);
  //Delay para no sobrepedir datos al sensor de humedad
  delay(delayMS);
  //Lectura de humedad
  sensors_event_t event;
  dht.humidity().getEvent(&event);
  int humedad = 0;
  if (!isnan(event.relative_humidity)) {
    humedad = event.relative_humidity;
    if (humedad < HUMEDAD_MINIMA) {
      //Regar
      //Mandar a la base que se regó
    }
  }
  //Lectura de temperatura
  float temp = termistor.get_temperature();
  //Obtención del nombre de la planta
  int it = 0;
  String planta = "";
  bool first = true;
  bool start_name = false;
  while(topic[it] != '\0') {
    if (topic[it] == '/') {
      if (first)
        first = false;
      else 
        start_name = true;
    }  
    it++;
    if (start_name){
      planta+=topic[it];
    }  
  }
  //Publicar datos
  snprintf(msg, MQTT_LENGTH, "%s,%d,%.2f", planta, humedad, temp);
  send_message(clientId, msg);
  client.loop();
  //Dormir durante un tiempo 
  esp_deep_sleep_start();
}

void loop() {
}

void setup_wifi() {
  delay(10);
  Serial.println();
  Serial.print("Conectando a ");
  Serial.println(ssid);
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  randomSeed(micros());
  Serial.println("");
  Serial.println("WiFi conectado");
  Serial.println("Dirección IP: ");
  Serial.println(WiFi.localIP());
}

void send_message(const char *clientId, char *msg) {
  while (!client.connected()) {
    Serial.print("Intentando establecer conexión MQTT...\n");
    if (client.connect(clientId)) {
      Serial.print(clientId);
      Serial.println(": Conectado");
      Serial.print("Enviando mensaje: ");
      Serial.println(msg);
      client.publish(topic, msg);
    }
  }
}
