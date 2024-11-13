
#include "Arduino.h"
#include "ConnectedLab8266.h"

    ESP8266WebServer _server(80);
    String ConnectedLab8266::_ssid="";
    String ConnectedLab8266::_password="";
    int ConnectedLab8266::_status=0;
    boolean ConnectedLab8266::_statuschange=true;  //start as true to initiate state machine
    float ConnectedLab8266::_uploadValue=0;
    String ConnectedLab8266::_uploadUnit="";
    String ConnectedLab8266::_deviceName="";
    String ConnectedLab8266::_mqttServer="";
    boolean ConnectedLab8266::_debug=false;


WiFiClient espClient;
PubSubClient mqttClient(espClient);

const int MEMPOS_SSID=0;
const int MEMPOS_PASSWORD=50;
const int MEMPOS_DEVICENAME=100;
const int MEMPOS_MQTTSERVER=150;

ConnectedLab8266::ConnectedLab8266()
{
}
void ConnectedLab8266::begin()
{
    readDataFromEEPROM();
    if (_deviceName==""){
        String mac=WiFi.macAddress();
        int len=mac.length();
        String name="cld-";
        name += mac[len-5];
        name += mac[len-4];
        name += mac[len-2];
        name += mac[len-1];
        ConnectedLab8266::setDeviceName(name);
    }
    if (_ssid!=""){
        _status=1;
        _statuschange=true;
    }
}
void ConnectedLab8266::updateStatus(){
    while(_statuschange){
        if (ConnectedLab8266::_debug){
            Serial.println("update status!!!");
            Serial.println(_status);
        }
        _statuschange=false;
        if (_status==0){
            startSetup();
        }
        else if (_status==1){
            startNetworkConnection();
        }
        else if (_status==2){
            startMQTTConnection();
        }
        else if (_status==3){
            //startSendingData();
        }
    }
}
boolean ConnectedLab8266::inSetupMode(){
    if (_status==0){return true;}
    return false;
}
boolean ConnectedLab8266::isReady(){
    if (_status==3){return true;}
    return false;
}
void ConnectedLab8266::setUploadValue(float val){
    _uploadValue=val;
}
void ConnectedLab8266::setUploadUnit(String unit){
    _uploadUnit=unit;
}
void ConnectedLab8266::uploadData(){
    if (_deviceName!="" && _uploadUnit!=""){
        String data="{\"device\":\"";
        data += _deviceName;
        data += "\", \"val\":";
        data += _uploadValue;
        data += ", \"unit\":\"";
        data += _uploadUnit;
        data += "\"}";
        mqttClient.publish("labData", data.c_str());
        _uploadValue=0;
        _uploadUnit="";
    }
}
void ConnectedLab8266::run()
{
    MDNS.update();
    if (_statuschange){updateStatus();}
    if (_status==0 || _status==2 || _status==3){_server.handleClient();}
}
void ConnectedLab8266::MQTTcallback(char* topic, byte* payload, unsigned int length) {

        if (ConnectedLab8266::_debug){
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();
  }
}
void ConnectedLab8266::startMQTTConnection(){

  if (_mqttServer!=""){
      int str_len = _mqttServer.length() + 1;
      char char_array[str_len];
      _mqttServer.toCharArray(char_array, str_len);

    mqttClient.setServer(char_array, 1883);
    //mqttClient.setCallback(callback);

        // Loop until we're reconnected
      while (!mqttClient.connected()) {
        if (ConnectedLab8266::_debug){Serial.print("Attempting MQTT connection...");}
        // Create a random client ID
        String clientId = "ESP8266Client-";
        clientId += String(random(0xffff), HEX);
        // Attempt to connect
        if (mqttClient.connect(clientId.c_str())) {
            if (ConnectedLab8266::_debug){Serial.println("mqtt broker connected");}
          _status=3;
          _statuschange=true;
        } else {
            if (ConnectedLab8266::_debug){
                Serial.print("failed, rc=");
                Serial.print(mqttClient.state());
                Serial.println(" try again in 5 seconds");
            }
          // Wait 5 seconds before retrying
          delay(5000);
        }
      }
  }
}
void handleData(){
    if (ConnectedLab8266::getDebug()){Serial.println("post data request!!!");}
    String ssid="";
    String pass="";
    String name="";
    String mqtt="";
    if(_server.hasArg("ssid")) {
      ssid=_server.arg("ssid");
    }
    if (_server.hasArg("pass")){
        pass=_server.arg("pass");
    }
    if (_server.hasArg("name")){
        name=_server.arg("name");
    }
    if (_server.hasArg("mqtt")){
        mqtt=_server.arg("mqtt");
    }

    if (ssid!="" && ConnectedLab8266::getWifiSSID()!=ssid){
      ConnectedLab8266::setNetworkCredentials(ssid,pass);
    }
    if (name!=""){
        ConnectedLab8266::setDeviceName(name);
    }
    if (mqtt!=""){
        ConnectedLab8266::setMQTTServer(mqtt);
    }
}
void handleRoot() {
  String s = "<html>";
  s += "<script>";
  s += "function sendData(){var xmlHttp = new XMLHttpRequest(); xmlHttp.onreadystatechange = function() { if (xmlHttp.readyState == 4 && xmlHttp.status == 200){ document.getElementById('data').innerHTML=xmlHttp.responseText; setTimeout(function(){getData();},1500);}}; xmlHttp.open('POST', '/data', true); xmlHttp.setRequestHeader('Content-type', 'application/x-www-form-urlencoded'); var ssid=document.getElementById('ssid').value; var pass=document.getElementById('pass').value; var name=document.getElementById('name').value; var mqtt=document.getElementById('mqtt').value;xmlHttp.send(\"name=\"+name+\"&ssid=\"+ssid+\"&pass=\"+pass+\"&mqtt=\"+mqtt);} ";
  s += "</script>";
  s += "<h2>Settings</h2>";
  s += "<table>";
  s += "<tr><td><h4>Device Name</h4></td><td><input id='name' type='input' value='"+ConnectedLab8266::getDeviceName()+"'></input></td></tr>";
  s += "<tr><td><h4>Wifi SSID</h4></td><td><input id='ssid' type='input' value='"+ConnectedLab8266::getWifiSSID()+"'></input></td></tr>";
  s += "<tr><td><h4>Wifi Password</h4></td><td><input id='pass' type='input' value='"+ConnectedLab8266::getWifiPassword()+"'></input></td></tr>";
  s += "<tr><td><h4>MQTT Server</h4></td><td><input id='mqtt' type='input' value='"+ConnectedLab8266::getMQTTServer()+"'></input></td></tr>";
  s += "</table>";
  s += "<button onclick='sendData()'>Update</button>";
  s += "</html>";
  _server.send(200, "text/html", s);   // Send HTTP status 200 (Ok) and send some text to the browser/client
}
void ConnectedLab8266::startNetworkConnection()
{
    if (ConnectedLab8266::_debug){Serial.println("starting wifi connection!!!");}
    WiFi.begin(_ssid, _password);
    long start=millis();
    while ((WiFi.status() != WL_CONNECTED) && (millis()<start+(30*1000))) {delay(500);}
    if (WiFi.status()!=WL_CONNECTED){
        if (ConnectedLab8266::_debug){Serial.println("unable to connect to wifi...");}
        WiFi.disconnect();
        ConnectedLab8266::_status=0;
        ConnectedLab8266::_statuschange=true;
    }
    else{
        if (ConnectedLab8266::_debug){
            Serial.println("connected to wifi!");
            Serial.println(WiFi.localIP());
        }
        MDNS.begin("esp8266", WiFi.localIP());
        _server.on("/", HTTP_GET, handleRoot);
        _server.begin();
        MDNS.addService("http", "tcp", 80);
        ConnectedLab8266::_status=2;
        ConnectedLab8266::_statuschange=true;
    }
}
void ConnectedLab8266::startSetup()
{
    if (ConnectedLab8266::_debug){Serial.print("Setting soft-AP ... ");}
    boolean result = WiFi.softAP("ESPsoftAP_01", "");

    _server.on("/data", HTTP_POST, handleData);
    _server.on("/", HTTP_GET, handleRoot);

    _server.begin();
}
IPAddress ConnectedLab8266::getIP(){return WiFi.softAPIP();}
String ConnectedLab8266::getWifiSSID(){return ConnectedLab8266::_ssid;}
String ConnectedLab8266::getWifiPassword(){return ConnectedLab8266::_password;}
String ConnectedLab8266::getMQTTServer(){return ConnectedLab8266::_mqttServer;}
String ConnectedLab8266::getDeviceName(){return ConnectedLab8266::_deviceName;}
boolean ConnectedLab8266::getDebug(){return ConnectedLab8266::_debug;}
void ConnectedLab8266::setDebug(boolean debug){
    _debug=debug;
}
void ConnectedLab8266::setDeviceName(const String &name)
{
    writeEEPROM(MEMPOS_DEVICENAME,name);
    _deviceName=name;
}
void ConnectedLab8266::setMQTTServer(const String &mqtt)
{
    writeEEPROM(MEMPOS_MQTTSERVER,mqtt);
    _mqttServer=mqtt;
    if (_status==2){
        _statuschange=true;   //try to start mqtt connection
    }
}
void ConnectedLab8266::setNetworkCredentials(const String &ssid, const String &password)
{
    writeEEPROM(MEMPOS_SSID,ssid);
    writeEEPROM(MEMPOS_PASSWORD,password);
    _ssid=ssid;
    _password=password;
    _status=1;
    _statuschange=true;
}
void ConnectedLab8266::writeEEPROM(int addrOffset, const String &str)
{
    byte len = str.length();
    EEPROM.begin(512);
    EEPROM.write(addrOffset, len);
    for (int i = 0; i < len; i++)
    {
        EEPROM.write(addrOffset + 1 + i, str[i]);
    }
    EEPROM.commit();
    EEPROM.end();
}
String ConnectedLab8266::readEEPROM(int addrOffset){
    EEPROM.begin(512);
    int len = EEPROM.read(addrOffset);
    if (len==0){return "";}
    char data[len + 1];
    for (int i = 0; i < len; i++)
    {
        data[i] = EEPROM.read(addrOffset + 1 + i);
    }
    data[len] = '\0';
    EEPROM.end();
    return String(data);
}
void ConnectedLab8266::clearEEPROM()
{
    EEPROM.begin(512);
    EEPROM.write(MEMPOS_SSID, 0);
    EEPROM.write(MEMPOS_PASSWORD, 0);
    EEPROM.write(MEMPOS_DEVICENAME, 0);
    EEPROM.write(MEMPOS_MQTTSERVER, 0);
    EEPROM.commit();
    EEPROM.end();
}
void ConnectedLab8266::readDataFromEEPROM()
{
    _ssid=readEEPROM(MEMPOS_SSID);
    _password=readEEPROM(MEMPOS_PASSWORD);
    _deviceName=readEEPROM(MEMPOS_DEVICENAME);
    _mqttServer=readEEPROM(MEMPOS_MQTTSERVER);
}
