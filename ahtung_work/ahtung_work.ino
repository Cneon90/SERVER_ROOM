#define DS18B20PIN 14           // пин подключения контакта DATA 5000
#define ONE_WIRE_BUS 14
#define INTERVAL_GET_DATA 60*5*1000  // каждые 5 минут интервала измерений, мс
#define INTERVAL_GET_DATA2 1000//раз в секунду 
#define INTERVAL_GET_DATA3 10000//раз в секунду 
#define SENSOR_ID 6   //  Номер сенстора, смотреть в таблице сенсоры http://temportal.ddns.net/temper
#include <ESP8266WiFi.h>
#include "Adafruit_MQTT.h"
#include "Adafruit_MQTT_Client.h"
#include <OneWire.h>
#include <DallasTemperature.h>
// создание объекта OneWire
OneWire oneWire(ONE_WIRE_BUS);

// создадим объект для работы с библиотекой DallasTemperature
DallasTemperature sensor(&oneWire);

// переменная для интервала измерений
unsigned long millis_int1=0;


/*Получение температуры*/
float get_data_ds18b20()  {
  float temperature;
  // отправляем запрос на измерение температуры
  sensor.requestTemperatures();
  // считываем данные из регистра датчика
  temperature = sensor.getTempCByIndex(0);
  // выводим температуру в Serial-порт
 // Serial.print("Temp C: ");
  //Serial.println(temperature);
  return temperature;
}
/*Получение температуры*/





// the on off button feed turns this LED on/off
#define SVET 3  
#define STOL 2  
#define HOT  4  
// the slider feed sets the PWM output of this pin
#define PWMOUT 12

/************************* WiFi Access Point *********************************/

#define WLAN_SSID    "TP-Link_02CB"// "TP_Link_02CB"  // 
#define WLAN_PASS    "27264171" // "27264171"  //  

/************************* Adafruit.io Setup *********************************/

#define AIO_SERVER      "temportal.ddns.net"
#define AIO_SERVERPORT  1883                   // use 8883 for SSL
#define AIO_USERNAME    ""
#define AIO_KEY         ""

/************ Global State (you don't need to change this!) ******************/





// Create an ESP8266 WiFiClient class to connect to the MQTT server.
WiFiClient client;
// or... use WiFiFlientSecure for SSL
//WiFiClientSecure client;

// Setup the MQTT client class by passing in the WiFi client and MQTT server and login details.
Adafruit_MQTT_Client mqtt(&client, AIO_SERVER, AIO_SERVERPORT, AIO_USERNAME, AIO_USERNAME, AIO_KEY);
Adafruit_MQTT_Publish photocell = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "14"); //УКАЗАТЬ ID ДАТЧИКА
Adafruit_MQTT_Publish ayanserver = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "ayan/office/server"); //УКАЗАТЬ ID ДАТЧИКА
//Adafruit_MQTT_Publish photocell2 = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "38");
/****************************** Feeds ***************************************/

// Notice MQTT paths for AIO follow the form: <username>/feeds/<feedname>
Adafruit_MQTT_Subscribe onoffbutton = Adafruit_MQTT_Subscribe(&mqtt, AIO_USERNAME "/feeds/onoff");
Adafruit_MQTT_Subscribe slider = Adafruit_MQTT_Subscribe(&mqtt, AIO_USERNAME "/feeds/slider");

Adafruit_MQTT_Subscribe svet = Adafruit_MQTT_Subscribe(&mqtt, AIO_USERNAME "/KOND/SVET");
Adafruit_MQTT_Subscribe stol = Adafruit_MQTT_Subscribe(&mqtt, AIO_USERNAME "/KOND/STOL");
Adafruit_MQTT_Subscribe hot = Adafruit_MQTT_Subscribe(&mqtt, AIO_USERNAME "/KOND/HOT");
/*************************** Sketch Code ************************************/

/*************************** Настройка параметров сети ************************************/
IPAddress ip(192,168,100,7);      
IPAddress gateway(192,168,100,254);
IPAddress subnet(255,255,255,0);
IPAddress dns_server(8,8,8,8);
/*************************** Настройка параметров сети ************************************/


// Bug workaround for Arduino 1.6.6, it seems to need a function declaration
// for some reason (only affects ESP8266, likely an arduino-builder bug).
void MQTT_connect();

void setup() {
  pinMode(SVET, OUTPUT);
  pinMode(STOL, OUTPUT);
  pinMode(HOT, OUTPUT);
  
  pinMode(PWMOUT, OUTPUT);

  Serial.begin(115200);
  delay(10);

  Serial.println(F("Adafruit MQTT demo"));

  // Connect to WiFi access point.
  Serial.println(); Serial.println();
  Serial.print("Connecting to ");
  Serial.println(WLAN_SSID);
  
  WiFi.config(ip,dns_server, gateway, subnet);//Применяем настройки (статический IP)
  WiFi.begin(WLAN_SSID, WLAN_PASS);
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

   
  Serial.println();
  Serial.println("WiFi connected");
  Serial.println("IP address: "); Serial.println(WiFi.localIP());

 /* Поиск доступных сетей
   Serial.println("WiFi SCAN  ");  
           int n = WiFi.scanNetworks();//Узнаем количество доступных сетей
             Serial.println("scan done");
             if (n == 0) {//Если нету, то выводим сообщение
            Serial.println("no networks found");
          } else {
            Serial.print(n);//Количество найденых сетей выводим
            Serial.println(" networks found");
            for (int i = 0; i < n; ++i) {
              // Print SSID and RSSI for each network found
              Serial.print(i + 1);
              Serial.print(":   ");
              Serial.print(WiFi.SSID(i));
              Serial.print(" (");
              Serial.print(WiFi.RSSI(i));
              Serial.print(")");
              Serial.println((WiFi.encryptionType(i) == ENC_TYPE_NONE) ? " " : "*");
              delay(10);
            }
          }
  Serial.println("");
*/
  
  // Setup MQTT subscription for onoff & slider feed.
  mqtt.subscribe(&svet);
  mqtt.subscribe(&stol);
  mqtt.subscribe(&hot);
  //mqtt.subscribe(&onoffbutton);
  //mqtt.subscribe(&slider);


  sensor.begin();
  // устанавливаем разрешение датчика от 9 до 12 бит
  sensor.setResolution(12);
}

 uint32_t x=0;
 int   lastNotifyTime;
 int   lastNotifyTime2;
 int   lastNotifyTime3;
 int    r;



 
  void loop() {
     MQTT_connect();//Проверка соединения            
                
                if (millis() - lastNotifyTime > INTERVAL_GET_DATA) 
                {
                   float r1 = get_data_ds18b20();//Узнаем температуру
                     Serial.print("TEMPERATURA1 ="); //вывод в консоль
                     Serial.println(r1);
                   char x[5];                                                
                   photocell.publish(dtostrf(r1, 5, 2/*Количество_символов_после_запятой*/, x));  
                   r1=0;
                   
                   lastNotifyTime = millis();
                };



      if (millis() - lastNotifyTime2 > INTERVAL_GET_DATA2) 
                {
                   float r1 = get_data_ds18b20();
                                       Serial.print("server =");
                                       Serial.println(r1);
                   char x[5];                                                   //
                   ayanserver.publish(dtostrf(r1, 5, 2/*Количество_символов_после_запятой*/, x));
                   r1=0;
                   //Serial.println(lastNotifyTime2/1000);
                   lastNotifyTime2 = millis();
                };



        if (millis() - lastNotifyTime3 > INTERVAL_GET_DATA3) 
                {
                   
                   
                     // ping the server to keep the mqtt connection alive
                   
                   lastNotifyTime2 = millis();
                };



                


  Adafruit_MQTT_Subscribe *subscription;
  while ((subscription = mqtt.readSubscription(3000))) {
    // Check if its the onoff button feed
/*
    if (subscription == &svet) {
      Serial.print(F("SVET button: "));
      Serial.println((char *)svet.lastread);
      
              if (strcmp((char *)svet.lastread, "1") == 0) {
                digitalWrite(SVET, LOW); 
              }
              if (strcmp((char *)svet.lastread, "0") == 0) {
                digitalWrite(SVET, HIGH); 
              }


    }



    if (subscription == &stol) {
      Serial.print(F("STOL button: "));
      Serial.println((char *)stol.lastread);
      
              if (strcmp((char *)stol.lastread, "1") == 0) {
                digitalWrite(STOL, LOW); 
              }
              if (strcmp((char *)stol.lastread, "0") == 0) {
                digitalWrite(STOL, HIGH); 
              }

    }

     if (subscription == &hot) {
      Serial.print(F("HOT button: "));
      Serial.println((char *)hot.lastread);
      
              if (strcmp((char *)hot.lastread, "1") == 0) {
                digitalWrite(HOT, LOW); 
              }
              if (strcmp((char *)hot.lastread, "0") == 0) {
                digitalWrite(HOT, HIGH); 
              }

    }
    
    // check if its the slider feed
    if (subscription == &slider) {
      Serial.print(F("Slider: "));
      Serial.println((char *)slider.lastread);
      uint16_t sliderval = atoi((char *)slider.lastread);  // convert to a number
      analogWrite(PWMOUT, sliderval);
    }
    */
  }

 
 if(! mqtt.ping()) {
                      mqtt.disconnect();
                    }
}

// Function to connect and reconnect as necessary to the MQTT server.
// Should be called in the loop function and it will take care if connecting.
void MQTT_connect() {
  int8_t ret;
       
  // Stop if already connected.
  if (mqtt.connected()) {
    return;
  }

  Serial.print("Connecting to MQTT... ");

  uint8_t retries = 3;
  while ((ret = mqtt.connect()) != 0) { // connect will return 0 for connected
       Serial.println(mqtt.connectErrorString(ret));
       Serial.println("Retrying MQTT connection in 5 seconds...");
       mqtt.disconnect();
       delay(5000);  // wait 5 seconds
       retries--;
       if (retries == 0) {
         // basically die and wait for WDT to reset me
         while (1);
       }
  }
  Serial.println("MQTT Connected!");
}
