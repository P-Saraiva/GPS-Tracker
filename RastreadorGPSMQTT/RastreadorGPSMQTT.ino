

#include "Adafruit_MQTT.h"                                  // Adafruit MQTT library
#include "Adafruit_MQTT_Client.h"                           // Adafruit MQTT library

#include "ESP8266WiFi.h"                                    // ESP8266 library     


#include <TinyGPS++.h>                                      // Tiny GPS Plus Library
#include <LiquidCrystal.h>             

LiquidCrystal lcd(5, 4, 2, 14, 12, 13);                        //Define os pinos do lcd                    

TinyGPSPlus gps;                                            

const double HOME_LAT = ****;                          // Latitude e Longitude do ponto de partida 
const double HOME_LNG = ****;                        

// Dados WiFi 

#define WLAN_SSID       "****"                             // SSID da rede
#define WLAN_PASS       "********"                        // Senha da rede

// Dados Adafruit.io 

#define AIO_SERVER      "io.adafruit.com"
#define AIO_SERVERPORT  1883                                // use 8883 for SSL
#define AIO_USERNAME    "*****"                          // Username Adafruit
#define AIO_KEY         "*****************************"  // Key da Adafruit


/***************** Interação entre o código e o MQTT Server ******************/

WiFiClient client;                                         

Adafruit_MQTT_Client mqtt(&client, AIO_SERVER, AIO_SERVERPORT, AIO_USERNAME, AIO_KEY);

Adafruit_MQTT_Publish gpslat = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/gpslat");

Adafruit_MQTT_Publish gpslng = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/gpslng");

Adafruit_MQTT_Publish gpslatlng = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/gpslatlng/csv"); 


//setup
void setup() 
{
  Serial.begin(9600);                                 
  WiFi.mode(WIFI_STA);                                  
  WiFi.disconnect();                                   
  delay(100);                                           
  
  //Printa informações no LCD
  
  lcd.begin(16, 2);
  lcd.setCursor(0,0);                                  
  lcd.print("Rastreador GPS"); 
  lcd.setCursor(0,1);    
  lcd.print("PET MECATRONICA"); 
                                 

  delay(3000);                                                                                

   
  Serial.print("Connecting to WiFi");
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Conectando WiFi"); 
  lcd.setCursor(0,1);

  WiFi.begin(WLAN_SSID, WLAN_PASS);                     
      while (WiFi.status() != WL_CONNECTED) 
         {                                              
           delay(500);
           Serial.print(".");
           lcd.print("."); 
           
         } 
           Serial.println("Connected");
           lcd.clear();
           lcd.print("Conectado!");
}       

//Loop                                                
void loop() {
    
  smartDelay(500);         //Atualiza os dados GPS                             
  MQTT_connect();          //Coenecta com o servidor via MQTT       

  float Distance_To_Home;                              
  float GPSlat = (gps.location.lat());                 
  float GPSlng = (gps.location.lng());                  
  float GPSalt = (gps.altitude.feet());                  
  Distance_To_Home = (unsigned long)TinyGPSPlus::distanceBetween(gps.location.lat(),gps.location.lng(),HOME_LAT, HOME_LNG);  //Biblioteca do GPS calcula a distancia ate o ponto de origem  
  Serial.println(F("  GPS Tracking"));
  Serial.print("----------------");
  Serial.print("GPS Lat: ");
  Serial.println(gps.location.lat(), 6);               
  Serial.print("GPS Lon: ");
  Serial.println(gps.location.lng(), 6);               
  Serial.print("Distance: ");
  Serial.println(Distance_To_Home);                     
 
  lcd.clear();
  lcd.print("LAT: ");
  lcd.print(gps.location.lat(), 6);
  lcd.setCursor(0,1);
  lcd.print("LNG: ");
  lcd.print(gps.location.lng(), 6);
  delay(10000);
  lcd.clear();
  lcd.print("DISTANCIA: ");
  lcd.print(Distance_To_Home);

 
                                                            
            char gpsbuffer[30];                        
            char *p = gpsbuffer;                                            

            dtostrf(Distance_To_Home, 3, 4, p);         
            p += strlen(p);
            p[0] = ','; p++;                      
            
            dtostrf(GPSlat, 3, 6, p);                   
            p += strlen(p);
            p[0] = ','; p++;
                                                            
            dtostrf(GPSlng, 3, 6, p);                   
            p += strlen(p);
            p[0] = ','; p++;  
                                                            
            dtostrf(GPSalt, 2, 1, p);                   
            p += strlen(p);
                                                                        
            p[0] = 0;                                  
            if ((GPSlng != 0) && (GPSlat != 0))         
              {
              Serial.println("Sending GPS Data ");     
              gpslatlng.publish(gpsbuffer);             
              Serial.println(gpsbuffer);  
              }
            
            gpslng.publish(GPSlng,6);                                

            //Verifica se dados foram recebidos
            
            if (! gpslat.publish(GPSlat,6))           
               {
                 Serial.println(F("Failed"));          
               } else 
                  {
                   //display.println(gpsbuffer);
                   Serial.println(F("Data Sent!"));                   
                   }  
             
    
  if (millis() > 5000 && gps.charsProcessed() < 10)
    Serial.println(F("No GPS data received: check wiring"));
  
  // Pequena pausa
  Serial.print("Pausing...");
  smartDelay(500);                                     
  delay(10000);
}



static void smartDelay(unsigned long ms)                 
{
  unsigned long start = millis();
  do 
  {
    while (Serial.available())
      gps.encode(Serial.read());
  } while (millis() - start < ms);
}


void MQTT_connect() {
  
  int8_t ret;
  if (mqtt.connected()) { return; }                     
  Serial.print("Connecting to MQTT... ");
  lcd.clear();
  lcd.print("Status do server");
  delay(3000);
  uint8_t retries = 3;
 
     
  while ((ret = mqtt.connect()) != 0) {                 
       mqtt.disconnect();
       delay(5000);                                   
       retries--;
       if (retries == 0) {                                                                                      
         while (1);         
       }
  }
  Serial.println("MQTT Connected!");
  lcd.setCursor(0,1);
  lcd.print("Server conectado!");
  delay(1000);
}
