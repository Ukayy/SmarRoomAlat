#include <ESP8266WiFi.h>
#include "FirebaseESP8266.h"
#include <SPI.h>
#include <MFRC522.h>

#define FIREBASE_HOST "smart-room-45672.firebaseio.com"                //Firebase Project URL
#define FIREBASE_AUTH "f30uLe7zymHBvgtxy0szFN4iouSgC9yaC8zDQsiG"       //Firebase Key Auth
#define FIREBASE_FCM_KEY "AAAAjw2EVGM:APA91bFdiENI8VAFK5SjwP4Cw5NfQRjNekQ7hOlLpwpxgNELla5gNq5rkAuW5NbJIvLQweiC2fxzoIlIFU81SFbVR__HUP3Y-jpefxh_TpxOKEqzjDQq2eUZ2nZyqLTOW8JXJnG2qiD4"
#define WIFI_SSID "Kay"
#define WIFI_PASSWORD "qwerty123"
#define SS_PIN D4
#define RST_PIN D2

const int relay = 5;

String id_card = "";

MFRC522 rfid(SS_PIN,RST_PIN);


FirebaseData firebaseData;
unsigned long sendDataPrevMillis = 0;
String path = "/12345/pintu/kondisi";
uint16_t count = 0;


//Fungsi untuk koneksi ke Internet
void connectInternet(){
  WiFi.begin(WIFI_SSID,WIFI_PASSWORD);
  Serial.print("connecting");
  while (WiFi.status()!=WL_CONNECTED){
    Serial.print(".");
    delay(500);
  }
  Serial.println();
  Serial.print("connected:");
  Serial.println(WiFi.localIP());
}

//Fungsi RFID
void fungsiRFID(){
   if(rfid.PICC_IsNewCardPresent()){
    if(rfid.PICC_ReadCardSerial()){
      Serial.print("Tag UID : ");
      for (byte i = 0; i<rfid.uid.size; i++){
        //Serial.print(rfid.uid.uidByte[i]<0x10? " 0" : " ");
        Serial.print(rfid.uid.uidByte[i],HEX);
        id_card.concat(String(rfid.uid.uidByte[i],HEX));
      }
      Serial.println();
      id_card.toUpperCase();
      Serial.print("id_card : ");
      Serial.println(id_card);
      rfid.PICC_HaltA();
      sendNotif(id_card);
      waktu(id_card);
      sendNotif(id_card);
      id_card = "";
    }
  }
}

//fungsi stream
void streamCallback(StreamData data){

  String eventPath =  data.dataPath();

  //Pintu
  if (eventPath){
    Serial.print("Pintu Berubah : ");
    Serial.println(data.stringData());
    if(data.stringData()=="on"){
      digitalWrite(relay, LOW);
    }else if(data.stringData()=="off"){
      digitalWrite(relay, HIGH);
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


void waktu(String id_card){
  FirebaseJson json;
  json.set("tag",id_card);
  if (Firebase.pushJSON(firebaseData,"/12345/pintu/riwayat",json)){
     if (Firebase.setTimestamp(firebaseData,firebaseData.dataPath()+"/"+firebaseData.pushName()+"/timestamp")){}
    
  }    
}

void sendNotif(String id_card){ 
  if(Firebase.getString(firebaseData, "/12345/rfid/"+id_card+"/name")){
    if(firebaseData.dataType() == "String"){
      Serial.println(firebaseData.stringData());    
    }
  }else{
     Serial.println(firebaseData.errorReason());
  }
  firebaseData.fcm.setDataMessage("{\"id_tag\":\""+id_card+"\"}");//data
  Firebase.sendTopic(firebaseData);
}

void setup() {
  Serial.begin(115200); 
  connectInternet();   // Memanggil fungsi connect internet

  Firebase.begin(FIREBASE_HOST, FIREBASE_AUTH);
  Firebase.reconnectWiFi(true);

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

  //RFID
  SPI.begin();
  rfid.PCD_Init();
  Serial.println("RFID reading UID");

  //relay
  pinMode(relay, OUTPUT);
  digitalWrite(relay,HIGH);

  //FCM
  firebaseData.fcm.begin(FIREBASE_FCM_KEY);
  firebaseData.fcm.setPriority("high");
  firebaseData.fcm.setTopic("12345");
}

void loop() {

  fungsiRFID();
  

}
