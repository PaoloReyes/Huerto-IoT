#include <PubSubClient.h>                    //Biblioteca para usar MQTT
#include <WifiFunctions.h>                   //Biblioteca para conectar a wifi y enviar datos por MQTT

#define fotoresistencia 33                   //Pin de la fotoresistencia
#define uS_TO_S_FACTOR 1000000               //Factor de conversión de segundos a microsegundos
#define TIME_TO_SLEEP  5                     //Segundos a dormir el ESP

#define MQTT_LENGTH 20                       //Largo del dato a enviar por mqtt
char msg[MQTT_LENGTH];                       //Variable para el mensaje a enviar
const char *ssid = "chuuyas";                //Nombre de la red
const char *password = "53280077";           //Contraseña de la red
const char *broker = "192.168.0.100";        //Ip del broker o de la raspberry
const char *clientId = "ESP32Client_5";      //Cliente conectado al broker mqtt
const char *topic = "/plantas/general";      //Tópico a publicar los datos

WiFiClient espClient;                                //Objeto para manejar WiFi
PubSubClient *client = new PubSubClient(espClient);  //Objeto para manejar MQTT usando la conexión WiFi

void setup() {
  Serial.begin(9600);
  //Incia la conexión a la red
  setup_wifi(ssid, password);
  //Inicializar broker MQTT
  client->setServer(broker, 1883);
  //Configuración de deep sleep mode para ahorro de energía
  esp_sleep_enable_timer_wakeup(TIME_TO_SLEEP * uS_TO_S_FACTOR);
  Serial.print("Es de ");
  bool tiempo = (analogRead(fotoresistencia) < 1700);
  if(tiempo){
    Serial.println("Dia.");
  }else{
    Serial.println("Noche.");
  }
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
  snprintf(msg, MQTT_LENGTH, "%s,%d", planta, tiempo);
  send_message(client, clientId, topic, msg);
  client->loop();
  //Dormir durante un tiempo 
  esp_deep_sleep_start();
}

void loop() {
}
