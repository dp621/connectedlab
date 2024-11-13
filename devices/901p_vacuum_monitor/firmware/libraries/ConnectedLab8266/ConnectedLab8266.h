#ifndef ConnectedLab8266_h
#define ConnectedLab8266_h

#include "Arduino.h"
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <EEPROM.h>
#include <ESP8266mDNS.h>
#include <PubSubClient.h>

class ConnectedLab8266
{
  public:
    ConnectedLab8266();
    void begin();
    void run();
    static void startNetworkConnection();
    static void startSetup();
    static void startMQTTConnection();
    IPAddress getIP();
    static String getWifiSSID();
    static String getWifiPassword();
    static String getMQTTServer();
    static String getDeviceName();
    void setUploadValue(float val);
    void setUploadUnit(String unit);
    void uploadData();
    boolean inSetupMode();
    boolean isReady();
    static void setNetworkCredentials(const String &ssid, const String &password);
    static void setDeviceName(const String &name);
    static void setMQTTServer(const String &mqtt);
    static void setDebug(boolean debug);
    static boolean getDebug();
  private:
    static void writeEEPROM(int addrOffset, const String &str);
    String readEEPROM(int addrOffset);
    void clearEEPROM();
    void readDataFromEEPROM();
    static String _ssid;
    static String _password;
    static String _deviceName;
    static String _mqttServer;
    static int _status;
    static boolean _debug;
    static float _uploadValue;
    static String _uploadUnit;
    static boolean _statuschange;
    static void updateStatus();
    static void MQTTcallback(char* topic, byte* payload, unsigned int length);
};

#endif
