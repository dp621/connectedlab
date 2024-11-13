
#include <SoftwareSerial.h>
#include <WiFi.h>
#include <WebServer.h>
#include <ESPmDNS.h>
#include <Update.h>


#define RX (13)
#define TX (12)


EspSoftwareSerial::UART testSerial;
bool sendData=false;
String cmdLog[10];
String respLog[10];
String cmdType[10];
int logcnt=0;


const char* host = "esp32";
const char* ssid = "<ssid>";
const char* password = "<pass>";
const char* sap_ssid = "serialmonitor";
const char* sap_password = "serialmonitor";
boolean connectToNetwork=false;
int BAUD_RATE=9600;

WebServer server(80);

const char* serverIndex =
//"<script src='https://ajax.googleapis.com/ajax/libs/jquery/3.2.1/jquery.min.js'></script>"
"<h3>Serial 2 Wifi Tool</h3>"
"<input id='cmd' style='width:200px;'></input>"
"<button id='sendhex'>Send Hex</button>"
"<button id='sendstr'>Send String</button>"
"<h5>Output:</h5>"
"<div id='data' style='width:100%; height:150px; overflow-y:auto; border:1px solid black;'>"
"</div>"
"<hr>"
"<h4>System Update</h4>"
"<p>Current Version: <span id='ver'>1.0</span></p>"
"<form method='POST' action='#' enctype='multipart/form-data' id='upload_form'>"
   "<input type='file' name='update'>"
        "<input type='submit' value='Update'>"
    "</form>"
 "<div id='prg'>progress: 0%</div>"
 "<script>"
  "document.getElementById('upload_form').onsubmit = function (e){"
  "e.preventDefault();"
  "var form = document.querySelector(\"#upload_form\");"
  "var data = new FormData(form);"
  "const xhr = new XMLHttpRequest();"
  "xhr.upload.addEventListener('progress', function(evt) {"
        "if (evt.lengthComputable) {"
          "var per = evt.loaded / evt.total;"
          "document.getElementById('prg').innerHTML ='progress: ' + Math.round(per*100) + '%';"
        "}"
      "}, false);"
  "xhr.open(\"POST\", \"/update\");"
  "xhr.send(data);"
  "xhr.responseType = \"json\";"
  "xhr.onload = () => {"
    "if (xhr.readyState == 4 && xhr.status == 200) {"
      
    "} else {"
      "console.log(`Error: ${xhr.status}`);"
    "}"
  "};"
  "}; "
 "document.getElementById('sendhex').onclick = function (){"
  "var data=document.getElementById('cmd').value;"
  "const xhr = new XMLHttpRequest();"
  "xhr.open(\"POST\", \"/sendhex\");"
  "xhr.send(data);"
  "xhr.responseType = \"json\";"
  "xhr.onload = () => {"
    "if (xhr.readyState == 4 && xhr.status == 200) {"
      "const data = xhr.response;"
      "console.log(data);"
    "} else {"
      "console.log(`Error: ${xhr.status}`);"
    "}"
    "document.getElementById('cmd').value=\"\";"
  "};"
 "}; "
 "document.getElementById('sendstr').onclick = function (){"
  "var data=document.getElementById('cmd').value;"
  "const xhr = new XMLHttpRequest();"
  "xhr.open(\"POST\", \"/sendstr\");"
  "xhr.send(data);"
  "xhr.responseType = \"json\";"
  "xhr.onload = () => {"
    "if (xhr.readyState == 4 && xhr.status == 200) {"
      "const data = xhr.response;"
      "console.log(data);"
    "} else {"
      "console.log(`Error: ${xhr.status}`);"
    "}"
    "document.getElementById('cmd').value=\"\";"
  "};"
 "}; "
 "window.onload = function(){"
  "const xhr = new XMLHttpRequest();"
  "xhr.open(\"GET\", \"/data\");"
  "xhr.send();"
  "xhr.responseType = \"json\";"
  "xhr.onload = () => {"
    "if (xhr.readyState == 4 && xhr.status == 200) {"
      "const data = xhr.response;"
      "console.log(data);"
      "if (data.log.length>0){"
              "var i=0;"
              "var s=\"\";"
              "while (i<data.log.length){"
              "s=s+\"<div>\";"
              "s=s+\"<p style='margin-bottom:0px;'>\"+data.log[i].cmd+\"</p>\";"
              "s=s+\"<p style='margin-top:0px;'>\"+data.log[i].resp+\"</p>\";"
              "s=s+\"</div>\";"
              "s=s+\"<hr>\";"
              "i++;"
              "}"
              "document.getElementById('data').innerHTML=s;"
           "}"
    "} else {"
      "console.log(`Error: ${xhr.status}`);"
    "}"
  "};"
 "};"
 "</script>";


void returnOK() {
  server.send(200, "text/plain", "");
}

void handleSendHex() {
  int numArgs = server.args();
  Serial.print("Number of args: ");
  Serial.println(numArgs);
  for (int i=0; i< numArgs; i++){
    Serial.print(server.argName(i));
    Serial.print(" : ");
    Serial.println(server.arg(i));
  }
  if (numArgs>0){
    cmdLog[logcnt]=server.arg(0);
    cmdType[logcnt]="hex";
  }

sendData=true;


  returnOK();
}

void handleSendString() {
  int numArgs = server.args();
  Serial.print("Number of args: ");
  Serial.println(numArgs);
  for (int i=0; i< numArgs; i++){
    Serial.print(server.argName(i));
    Serial.print(" : ");
    Serial.println(server.arg(i));
  }
  if (numArgs>0){
    cmdLog[logcnt]=server.arg(0);
    cmdType[logcnt]="str";
  }

sendData=true;


  returnOK();
}
void handleData() {
    String data="{\"log\":[";
    int i=0;
    while (i<logcnt){
      data += "{\"cmd\":\"";
      data += cmdLog[i];
      Serial.println(cmdLog[i]);
      Serial.println(respLog[i]);
      data += "\", \"resp\":\"";
      data += respLog[i];
      data += "\"}";
      i++;
      if (i<logcnt){
        data += ",";
      }
    }
    data += "]}";
    server.send(200, "application/json", data);
}
void hexCharacterStringToBytes(byte *byteArray, const char *hexString)
{
  bool oddLength = strlen(hexString) & 1;

  byte currentByte = 0;
  byte byteIndex = 0;

  for (byte charIndex = 0; charIndex < strlen(hexString); charIndex++)
  {
    bool oddCharIndex = charIndex & 1;

    if (oddLength)
    {
      // If the length is odd
      if (oddCharIndex)
      {
        // odd characters go in high nibble
        currentByte = nibble(hexString[charIndex]) << 4;
      }
      else
      {
        // Even characters go into low nibble
        currentByte |= nibble(hexString[charIndex]);
        byteArray[byteIndex++] = currentByte;
        currentByte = 0;
      }
    }
    else
    {
      // If the length is even
      if (!oddCharIndex)
      {
        // Odd characters go into the high nibble
        currentByte = nibble(hexString[charIndex]) << 4;
      }
      else
      {
        // Odd characters go into low nibble
        currentByte |= nibble(hexString[charIndex]);
        byteArray[byteIndex++] = currentByte;
        currentByte = 0;
      }
    }
  }
}
void dumpByteArray(const byte * byteArray, const byte arraySize)
{

for (int i = 0; i < arraySize; i++)
{
  Serial.print("0x");
  if (byteArray[i] < 0x10)
    Serial.print("0");
  Serial.print(byteArray[i], HEX);
  Serial.print(", ");
}
Serial.println();
}

byte nibble(char c)
{
  if (c >= '0' && c <= '9')
    return c - '0';

  if (c >= 'a' && c <= 'f')
    return c - 'a' + 10;

  if (c >= 'A' && c <= 'F')
    return c - 'A' + 10;

  return 0;  // Not a valid hexadecimal character
}

void setup() {
  
	Serial.begin(115200);
	testSerial.begin(BAUD_RATE, EspSoftwareSerial::SWSERIAL_8N1, RX, TX);
	// Only half duplex this way, but reliable TX timings for high bps

	testSerial.println(PSTR("\nSoftware serial onReceive() event test started"));

//setup wifi
// Connect to WiFi network
if (!connectToNetwork){
  Serial.println("\n[*] Creating AP");
    WiFi.mode(WIFI_AP);
    WiFi.softAP(sap_ssid, sap_password);
    Serial.print("[+] AP Created with IP Gateway ");
    Serial.println(WiFi.softAPIP());
}
else{
  WiFi.begin(ssid, password);
  Serial.println("");

  // Wait for connection
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("Connected to ");
  Serial.println(ssid);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
}

  /*use mdns for host name resolution*/
  if (!MDNS.begin(host)) { //http://esp32.local
    Serial.println("Error setting up MDNS responder!");
    while (1) {
      delay(1000);
    }
  }
  Serial.println("mDNS responder started");
  server.on("/", HTTP_GET, []() {
    server.sendHeader("Connection", "close");
    server.send(200, "text/html", serverIndex);
  });
  server.on("/sendhex", HTTP_POST, handleSendHex);
  server.on("/sendstr", HTTP_POST, handleSendString);
  server.on("/data", HTTP_GET, handleData);
  /*handling uploading firmware file */
  server.on("/update", HTTP_POST, []() {
    server.sendHeader("Connection", "close");
    server.send(200, "text/plain", (Update.hasError()) ? "FAIL" : "OK");
    ESP.restart();
  }, []() {
    HTTPUpload& upload = server.upload();
    if (upload.status == UPLOAD_FILE_START) {
      Serial.printf("Update: %s\n", upload.filename.c_str());
      if (!Update.begin(UPDATE_SIZE_UNKNOWN)) { //start with max available size
        Update.printError(Serial);
      }
    } else if (upload.status == UPLOAD_FILE_WRITE) {
      /* flashing firmware to ESP*/
      if (Update.write(upload.buf, upload.currentSize) != upload.currentSize) {
        Update.printError(Serial);
      }
    } else if (upload.status == UPLOAD_FILE_END) {
      if (Update.end(true)) { //true to set the size to the current progress
        Serial.printf("Update Success: %u\nRebooting...\n", upload.totalSize);
      } else {
        Update.printError(Serial);
      }
    }
  });
  server.begin();


delay(1000);
/*
	for (char ch = ' '; ch <= 'z'; ch++) {
		testSerial.write(ch);
	}
	testSerial.println();*/
}

void loop() {
  if (sendData){

    if (cmdType[logcnt]=="str"){ //send string
      Serial.print("sending string: ");
      Serial.println(cmdLog[logcnt]);
      testSerial.println(cmdLog[logcnt]);
      testSerial.flush();
      delay(100);
    }
    else {//sendHex
      //parse the message
      Serial.print("parsing: ");
      Serial.println(cmdLog[logcnt]);
      const byte MaxByteArraySize = cmdLog[logcnt].length()/2;
      byte byteArray[MaxByteArraySize] = {0};
      hexCharacterStringToBytes(byteArray, cmdLog[logcnt].c_str());
      dumpByteArray(byteArray, MaxByteArraySize);

      int i=0;
      Serial.println("testing serial send....");
      while (i<MaxByteArraySize){
        testSerial.write(byteArray[i]);
        Serial.print("wrote byte: ");
        Serial.println(byteArray[i]);
        i++;
      }
      testSerial.println();
      testSerial.flush();
      delay(100);

    //send test message
    /*testSerial.write('0xCA');
    testSerial.write('0x00');
    testSerial.write('0x01');
    testSerial.write('0x70');
    testSerial.write('0x00');
    testSerial.write('0x8E');
    testSerial.println();
    testSerial.flush();*/
    //cmdLog[logcnt]="CA 00 01 70 00 8E";
  }

  respLog[logcnt]="no response";

  logcnt++;
  sendData=false;
  }
  // put your main code here, to run repeatedly:
  if (testSerial.available() > 0) {
    // get incoming byte:
    Serial.print("got byte: ");
    String rs="";
    char* resp="";
    while (testSerial.available() > 0){
      byte inByte = testSerial.read();
      Serial.print(inByte);
      Serial.print(' ');
      rs += inByte;
      rs += " ";
      delay(10);
    }
    Serial.println();
    if (logcnt>0){
      Serial.println(rs);
      respLog[logcnt-1]=rs;
    }
  }

  //reset logcount
  if (logcnt==9){
    logcnt=0;
  }

  server.handleClient();
  delay(20);
}

