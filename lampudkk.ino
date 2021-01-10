#include "FirebaseESP8266.h"
#include <ESP8266WiFi.h>
#include <DHT.h>


#define FIREBASE_HOST "smart-room-45672.firebaseio.com"
#define FIREBASE_AUTH "f30uLe7zymHBvgtxy0szFN4iouSgC9yaC8zDQsiG"
#define WIFI_SSID "Kay"
#define WIFI_PASSWORD "qwerty123"
#define DHTPIN D7
#define DHTTYPE DHT11

const int relay0 = 5, 
          relay1 = 4,
          relay2 = 14,
          relay3 = 12; // D1, D2, D5, D6
int suhu, kelembapan;

//sensor dht11
DHT dht(DHTPIN, DHTTYPE);

//Define FirebaseESP8266 data object
FirebaseData firebaseData;

unsigned long sendDataPrevMillis = 0;

String path = "/12345/alat";
String nodeTemp = "/12345/sensor/suhu/kondisi";
String nodeHum = "/12345/sensor/kelembapan/kondisi";

uint16_t count = 0;

void printResult(StreamData &data);

void streamCallback(StreamData data){
  String eventPath =  data.dataPath();

  //Lampu 1
  if (eventPath== "/relay/0"){
    Serial.print("Relay 1 Berubah : ");
    Serial.println(data.stringData());
    if(data.stringData()=="on"){
      digitalWrite(relay0, LOW);
    }else if(data.stringData()=="off"){
      digitalWrite(relay0, HIGH);
    }else{
      Serial.println("Unknown");
    }
  }
  //Lampu 2
  else if(eventPath== "/relay/1"){
    Serial.print("Relay 2 Berubah : ");
    Serial.println(data.stringData());
    if(data.stringData()=="on"){
      digitalWrite(relay1, LOW);
    }else if(data.stringData()=="off"){
      digitalWrite(relay1, HIGH);
    }else{
      Serial.println("Unknown");
    }
  }
  //lampu 3
  else if(eventPath== "/relay/2"){
    Serial.print("Relay 3 Berubah : ");
    Serial.println(data.stringData());
    if(data.stringData()=="on"){
      digitalWrite(relay2, LOW);
    }else if(data.stringData()=="off"){
      digitalWrite(relay2, HIGH);
    }else{
      Serial.println("Unknown");
    }
  }
  //lampu 4
  else if(eventPath== "/relay/3"){
    Serial.print("Relay 4 Berubah : ");
    Serial.println(data.stringData());
    if(data.stringData()=="on"){
      digitalWrite(relay3, LOW);
    }else if(data.stringData()=="off"){
      digitalWrite(relay3, HIGH);
    }else{
      Serial.println("Unknown");
    }
  }else if(eventPath=="/ac/power"){
    Serial.print  ("AC power berubah : ");
    Serial.println(data.stringData());
  }else if(eventPath=="/ac/temp"){
    Serial.print("AC temp Berubah : ");
    Serial.println(data.intData());
  }else{
    if (data.dataType()=="json"){ 
      FirebaseJson &newData = data.jsonObject();
      FirebaseJsonData testData;
      newData.get(testData,"ac/power");
      Serial.println(testData.stringValue);
      //Send Data
      newData.get(testData,"ac/temp");
      Serial.println(testData.intValue);
      //
      newData.get(testData,"relay/[0]");
      Serial.println(testData.stringValue);
      //
      newData.get(testData,"relay/[1]");
      Serial.println(testData.stringValue);
      //
      newData.get(testData,"relay/[2]");
      Serial.println(testData.stringValue);
      //
      newData.get(testData,"relay/[3]");
      Serial.println(testData.stringValue);
     }
  }

}

void streamTimeoutCallback(bool timeout){
  if (timeout){
    Serial.println();
    Serial.println("Stream timeout, resume streaming...");
    Serial.println();
  }
}

void sensorFirebase(){
  //get suhu
  if(Firebase.getInt(firebaseData, nodeTemp)){
    if (firebaseData.dataType() == "int") {
      suhu = firebaseData.intData();
      Serial.print("Suhu: ");
      Serial.println(suhu);
    }
  } else {
    Serial.println(firebaseData.errorReason());
  }
  //get kelembapan
  if(Firebase.getInt(firebaseData, nodeHum)){
    if (firebaseData.dataType() == "int") {
       kelembapan = firebaseData.intData();
       Serial.print("Kelembapan: ");
       Serial.println(kelembapan);
    }
  } else {
    Serial.println(firebaseData.errorReason());
  }

}

void sensor(){
  float h = dht.readHumidity();
  float t = dht.readTemperature();
  Serial.print(F("Humidity: "));
  Serial.print(h);
  Serial.print("\n");
  Serial.print(F("Temperature: "));
  Serial.print(t);
  Serial.print(F("°C "));
  Serial.println();
  delay(2000);
}

void setup(){

  Serial.begin(9600);
  dht.begin();

  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting to Wi-Fi");
  while (WiFi.status() != WL_CONNECTED){
    Serial.print(".");
    delay(300);
  }
  Serial.println();
  Serial.print("Connected with IP: ");
  Serial.println(WiFi.localIP());
  Serial.println();

  Firebase.begin(FIREBASE_HOST, FIREBASE_AUTH);
  Firebase.reconnectWiFi(true);

  //sensorFirebase();

  //Set the size of WiFi rx/tx buffers in the case where we want to work with large data.
  firebaseData.setBSSLBufferSize(1024, 1024);

  //Set the size of HTTP response buffers in the case where we want to work with large data.
  firebaseData.setResponseSize(1024);


  if (!Firebase.beginStream(firebaseData, path)){
    Serial.println("------------------------------------");
    Serial.println("Can't begin stream connection...");
    Serial.println("REASON: " + firebaseData.errorReason());
    Serial.println("------------------------------------");
    Serial.println();
  }

  Firebase.setStreamCallback(firebaseData, streamCallback, streamTimeoutCallback);

  pinMode(relay0,OUTPUT);
  pinMode(relay1,OUTPUT);
  pinMode(relay2,OUTPUT);
  pinMode(relay3,OUTPUT);

}


void loop(){
 sensor();
}
