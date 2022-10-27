#include "WizFi360.h"
#include "WizFi360Udp.h"
#include <ArduinoJson.h>
#include <Adafruit_NeoPixel.h>

// Emulate Serial1 on pins 6/7 if not present
#ifndef HAVE_HWSERIAL1
#include "SoftwareSerial.h"
SoftwareSerial Serial2(6, 7); // RX, TX
#endif

/* Baudrate */
#define SERIAL_BAUDRATE   115200
#define SERIAL2_BAUDRATE  115200

#define PIN 22 // Pin for connecting LED Strip
Adafruit_NeoPixel strip = Adafruit_NeoPixel(13, PIN, NEO_GRB + NEO_KHZ800);

/* Wi-Fi info */
char ssid[] = "SSID";       // your network SSID (name)
char pass[] = "PASWORD";   // your network password

int status = WL_IDLE_STATUS;  // the Wifi radio's status

char server1[] = "api.openweathermap.org"; //server for fetching weather data
char server2[] = "date.jsontest.com"; //server for fetching time data

unsigned long lastConnectionTime1 = 0;         // last time you connected to the server, in milliseconds
const unsigned long postingInterval1 = 900000L; // delay between updates, in milliseconds 

unsigned long lastConnectionTime2 = 0;         // last time you connected to the server, in milliseconds
const unsigned long postingInterval2 = 10000L; // delay between updates, in milliseconds 

// Initialize the Ethernet client object
WiFiClient client1;
WiFiClient client2;

String weatherMessage = " ";  //weather data came from server
String weatherData = " ";     //filtered weather data

String timeMessage = " ";  //time data came from server
String timeData = " ";     //filtered time data

int h = 0;  //hour before adding time zone
int m = 0;  //minutes before adding time zone
int hour = 0;  //final hour
int minute = 0;  //final minutes
int clouds=0;  //cloud intensity in percentage

void setup() {
  Serial.begin(SERIAL_BAUDRATE);
  // initialize serial for WizFi360 module
  Serial2.begin(SERIAL2_BAUDRATE);
  // initialize WizFi360 module
  WiFi.init(&Serial2);

  // check for the presence of the shield
  if (WiFi.status() == WL_NO_SHIELD) {
    Serial.println("WiFi shield not present");
    // don't continue
    while (true);
  }

  // attempt to connect to WiFi network
  while ( status != WL_CONNECTED) {
    Serial.print("Attempting to connect to WPA SSID: ");
    Serial.println(ssid);
    // Connect to WPA/WPA2 network
    status = WiFi.begin(ssid, pass);
  }
  Serial.println("You're connected to the network");
  
  strip.begin();
  strip.setBrightness(255);  //Brigtness from 0 to 255
  strip.show();
  
  httpRequest1();  // request weather data
}

void loop() {
    // if there's incoming data from the net connection send it out the serial port
    // this is for debugging purposes only
    while (client1.available()) {
      char c = client1.read();
      weatherMessage = weatherMessage + c;
  //    Serial.write(c);
    }
    if(weatherMessage != " "){
      int indexS = weatherMessage.indexOf('{');
      int indexE = weatherMessage.lastIndexOf('}');
      weatherData = weatherMessage.substring(indexS, indexE+1);
      StaticJsonDocument<1024> doc1;
      DeserializationError error = deserializeJson(doc1, weatherData);
      
      JsonObject main = doc1["main"];
      float temp = main["temp"]; 
      int humidity = main["humidity"]; 
      
      int cloud = doc1["clouds"]["all"]; 
      
      Serial.println(">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>><<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<");
      Serial.println(temp);  //temperature
      Serial.println(cloud); //local variable from cloud%
      clouds = cloud;
      Serial.println(clouds);  //global variable from cloud%
      Serial.println(">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>><<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<");

//------------------------Changing Base Color acording to temperature--------------------
      if(temp <150){
        BaseChange(strip.Color(0, 0, 255), 50);
      }
      else if(temp >=150 && temp<270){
        BaseChange(strip.Color(0, 255, 255), 50);
      }
      else if(temp >=270 && temp<350){
        BaseChange(strip.Color(255, 70, 0), 50);
      }
      else if(temp >=350 && temp<500){
        BaseChange(strip.Color(255, 0, 0), 50);
      }
      else{
        BaseChange(strip.Color(255, 0, 255), 50);
      }
    }
    weatherMessage = " ";
  
    // if 10 seconds have passed since your last connection,
    // then connect again and send data
    if (millis() - lastConnectionTime1 > postingInterval1) {
      httpRequest1();
    }

    while (client2.available()) {
      char c = client2.read();
      timeMessage = timeMessage + c;
  //    Serial.write(c);
    }
    if(timeMessage != " "){
      int indexS = timeMessage.indexOf('{');
      int indexE = timeMessage.lastIndexOf('}');
      timeData = timeMessage.substring(indexS, indexE+1);
    
      StaticJsonDocument<128> doc2;
    
      DeserializationError error = deserializeJson(doc2, timeData);
    
      //const char* date = doc2["date"]; // "08-22-2022"
      //long long milliseconds_since_epoch = doc2["milliseconds_since_epoch"]; // 1661178826155
      const char* time = doc2["time"]; // "02:33:46 PM"
      String times = time;
      h = ((times.substring(0,2)).toInt());
      m = ((times.substring(3, 5)).toInt());
      h = (h + 5) % 24;
      Serial.println(h);
      minute = (m + 30) % 60;

      if(minute < 31 && minute >= 0){
        hour = h+1;
      }
      else{
        hour = h;
      }
      
      Serial.println("***************************************************************************");
      Serial.println(time);
      Serial.println(hour);
      Serial.println(minute);
      Serial.println("***************************************************************************");

          /*------------------------Morning clouds-------------------------*/
      if(hour >= 5 && hour < 9 && clouds < 20 )
      {
       CloudChange(strip.Color(255, 70, 255), 50);
      }
      else if(hour >= 5 && hour < 9 && clouds >= 20 && clouds < 50)
      {
       CloudChange(strip.Color(255, 70, 255), 50);
      }
      else if(hour >= 5 && hour < 9 && clouds >= 50 && clouds < 85)
      {
       CloudChange(strip.Color(200, 40, 255), 50);
      }
      else if(hour >= 5 && hour<9 && clouds >= 85)
      {
       thunder(strip.Color(150, 40, 255), 50);
      }
    
    
      /*------------------------Afternoon clouds-------------------------*/
      if(hour >= 9 && hour < 17 && clouds < 20)
      {
       CloudChange(strip.Color(255, 70, 40), 50);
      }
      if(hour >= 9 && hour < 17 && clouds >= 20 && clouds < 50)
      {
       CloudChange(strip.Color(255, 70, 60), 50);
      }
      if(hour >= 9 && hour <17 && clouds >= 50 && clouds < 85)
      {
       CloudChange(strip.Color(200, 40, 255), 50);
      }
      if(hour >= 9 && hour < 17 && clouds >= 85)
      {
       thunder(strip.Color(150, 150, 255), 50);
      }
    
      /*------------------------Evening clouds-------------------------*/
      if(hour >= 17 && hour < 19 && clouds < 20)
      {
       CloudChange(strip.Color(255, 100, 255), 50);
      }
      if(hour >= 17 && hour < 19 && clouds >= 20 && clouds < 50)
      {
       CloudChange(strip.Color(255, 70, 255), 50);
      }
      if(hour >= 17 && hour < 19 && clouds >= 50 && clouds < 85)
      {
        CloudChange(strip.Color(255, 10, 255), 50);
      }
      if(hour >= 17 && hour < 19 && clouds >= 85)
      {
       thunder(strip.Color(100, 0, 255), 50);
      }
    
      /*------------------------Nighourt clouds-------------------------*/
      if((hour >= 19 || hour < 5)&& clouds < 20)
      {
       CloudChange(strip.Color(100, 100, 120), 50);
      }
      if((hour >= 19 || hour < 5)&& clouds >= 20 && clouds < 50)
      {
       CloudChange(strip.Color(100, 100, 150), 50);
      }
      if((hour >= 19 || hour < 5) && clouds >= 50 && clouds < 85)
      {
       CloudChange(strip.Color(100, 100, 255), 50);
      }
      if((hour >= 19 || hour < 5) && clouds >= 85)
      {
        thunder(strip.Color(70, 0, 255), 10);
      }

//-----------------------Changing Sun / Moon Color according to time---------------------------------
      if(hour >= 6 && hour <= 8){
        SunMoonChange(strip.Color(245, 30, 129), 50);
      }
      else if(hour > 8 && hour <= 11){
        SunMoonChange(strip.Color(255, 70, 0), 50);
      }
      else if(hour > 11 && hour <= 16){
        SunMoonChange(strip.Color(255, 70, 30), 50);
      }
      else if(hour > 16 && hour <= 18){
        SunMoonChange(strip.Color(245, 60, 129), 50);
      }
      else{
        SunMoonChange(strip.Color(240, 240, 255), 50);
      }
    }
    timeMessage = " ";
    
    // if 10 seconds have passed since your last connection,
    // then connect again and send data
    if (millis() - lastConnectionTime2 > postingInterval2) {
      httpRequest2();
    }
}

// this method makes a HTTP connection to the server
void httpRequest1() {
  Serial.println();
    
  // close any connection before send a new request
  // this will free the socket on the WiFi shield
  client1.stop();

  // if there's a successful connection
  if (client1.connect(server1, 80)) {
    Serial.println("Connecting...");
    
    // send the HTTP PUT request
    client1.println(F("GET /data/2.5/weather?lat=17.6795&lon=77.6051&appid=43f22249d3d42ec***********ca809b HTTP/1.1"));
    client1.println(F("Host: api.openweathermap.org"));
    client1.println("Connection: close");
    client1.println();

    // note the time that the connection was made
    lastConnectionTime1 = millis();
  }
  else {
    // if you couldn't make a connection
    Serial.println("Connection failed");
  }
}

void httpRequest2() {
  Serial.println();
    
  // close any connection before send a new request
  // this will free the socket on the WiFi shield
  client2.stop();

  // if there's a successful connection
  if (client2.connect(server2, 80)) {
    Serial.println("Connecting...");
    
    // send the HTTP PUT request
    client2.println(F("GET /asciilogo.txt HTTP/1.1"));
    client2.println(F("Host: date.jsontest.com"));
    client2.println("Connection: close");
    client2.println();

    // note the time that the connection was made
    lastConnectionTime2 = millis();
  }
  else {
    // if you couldn't make a connection
    Serial.println("Connection failed");
  }
}

void BaseChange(uint32_t c, uint8_t wait) {
  for(uint16_t i=0; i<8 ; i++) {
    strip.setPixelColor(i, c);
    strip.show();
    delay(wait);
  }
}

void CloudChange(uint32_t c, uint8_t wait) {
  for(uint16_t i=8; i<12 ; i++) {
    strip.setPixelColor(i, c);
    strip.show();
    delay(wait);
  }
}

void thunder(uint32_t c, uint8_t wait) {
  for(uint16_t i=8; i<12 ; i++) {
    strip.setPixelColor(i, c);
    strip.show();
    delay(wait);
  }
  delay(300);
  for(uint16_t i=8; i<12 ; i++) {
    strip.setPixelColor(i, strip.Color(250, 250, 250));
    strip.show();
    delay(wait);
  }
  delay(200);
  for(uint16_t i=8; i<12 ; i++) {
    strip.setPixelColor(i, c);
    strip.show();
    delay(wait);
  }
  delay(150);
  for(uint16_t i=8; i<12 ; i++) {
    strip.setPixelColor(i, strip.Color(250, 250, 250));
    strip.show();
    delay(wait);
  }
}

void SunMoonChange(uint32_t c, uint8_t wait) {
    strip.setPixelColor(12, c);
    strip.show();
    delay(wait);
}
