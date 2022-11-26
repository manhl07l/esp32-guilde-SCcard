#include <Arduino.h>
#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include "FS.h"
#include "SD.h"
#include "SPI.h"

// Ham nhap dia chi wifi
const char* ssid = "Dai";
const char* password = "12345679";
//Tao truong ghi du lieu
const char* PARAM_STRING = "inputString";
// Tao webserver tai port80
AsyncWebServer server(80);
//Ham Khoi tao the nho
void initSDCard(){
  if(!SD.begin()){
    Serial.println("khoi tao that bai");
    return;
  }
  uint8_t cardType = SD.cardType();

  if(cardType == CARD_NONE){
    Serial.println("khong ton tai the SD");
    return;
  }

  Serial.print("Loai the SD: ");
  if(cardType == CARD_MMC){
    Serial.println("MMC");
  } else if(cardType == CARD_SD){
    Serial.println("SDSC");
  } else if(cardType == CARD_SDHC){
    Serial.println("SDHC");
  } else {
    Serial.println("UNKNOWN");
  }
  uint64_t cardSize = SD.cardSize() / (1024 * 1024);
  Serial.printf("Kich co cua the: %lluMB\n", cardSize);
}
//Ham khoi tao wifi
void initWiFi() {
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi ..");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print('.');
    delay(1000);
  }
  Serial.println(WiFi.localIP());
}
//Ham doc du lieu tu SD
String readFile(fs::FS &fs, const char * path){
  Serial.printf("Reading file: %s\r\n", path);
  File file = fs.open(path, "r");
  if(!file || file.isDirectory()){
    Serial.println("- empty file or failed to open file");
    return String();
  }
  Serial.println("- read from file:");
  String fileContent;
  while(file.available()){
    fileContent+=String((char)file.read());
  }
  file.close();
  Serial.println(fileContent);
  return fileContent;
}
//Ham ghi du lieu vao SD
void writeFile(fs::FS &fs, const char * path, const char * message){
  Serial.printf("Writing file: %s\r\n", path);
  File file = fs.open(path, "w");
  if(!file){
    Serial.println("- failed to open file for writing");
    return;
  }
  if(file.print(message)){
    Serial.println("- file written");
  } else {
    Serial.println("- write failed");
  }
  file.close();
}

void setup() {
  Serial.begin(115200);
  initWiFi();
  initSDCard();

  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SD, "/profolio.html", "text/html");
  });
  server.on("/get", HTTP_GET, [] (AsyncWebServerRequest *request) {
    String inputMessage;//tao bien dau vao
    if (request->hasParam(PARAM_STRING)) {
      inputMessage = request->getParam(PARAM_STRING)->value();//luu bien vua nhap vao truong ghi du lieu
      writeFile(SD, "/inputString.txt", inputMessage.c_str());//luu vao du lieu vao file InputString trong SD
    }
    else {
      inputMessage = "No message sent";
    }
    Serial.println(inputMessage);
    //sau khi gui du lieu se chuyen den 1 trang khac
    request->send(200, "text/html", "thanks to contact us"
                                     "<br><a href=\"/\">Return to Home Page</a>");
  });

  server.serveStatic("/", SD, "/");

  server.begin();
}
// Ham loop sau moi 5s se cap nhat du lieu moi nhap
void loop() {
  String yourInputString = readFile(SD, "/inputString.txt");
  Serial.print("*** Your inputString: ");
  Serial.println(yourInputString);
  delay(5000);
}
