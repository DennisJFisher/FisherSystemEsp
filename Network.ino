#include <ESP8266WiFi.h>
// DJF
#include <ESP8266mDNS.h>        // Include the mDNS library
#include "HTML.h"

inline bool WifiConnected()
{
    return (WL_CONNECTED == WiFi.status());
}

//////////////////////////////////////////
// Wifi Network
//////////////////////////////////////////
const char WiFiPassword[] = "salicia1";
IPAddress LastIPAddress;
int LastWifiSuccessTime_ms = 0;

// Network static IPs here for ALL the devices so they're coherant..
// If this is false, use DHCP.
bool UseStaticIP = true;
IPAddress Gateway(192, 168, 1, 1);
IPAddress DNS1     (8, 8, 8, 8);
IPAddress DNS2     (8, 8, 4, 4);
IPAddress Subnet   (255, 255, 255, 0);
#if defined TEST
IPAddress StaticIP  (192, 168, 1, 181);
#elif defined KITCHEN
IPAddress StaticIP  (192, 168, 1, 183);
#elif defined GARAGE_UP
IPAddress StaticIP  (192, 168, 1, 184);
#elif defined GARAGE_DOWN
IPAddress StaticIP  (192, 168, 1, 185);
#elif defined GARAGE_DOOR
IPAddress StaticIP  (192, 168, 1, 186);
#elif defined CABIN_BASEMENT_NORTH
IPAddress StaticIP  (192, 168, 1, 187);
#elif defined CABIN_BASEMENT_SOUTH
IPAddress StaticIP  (192, 168, 1, 188);
#elif defined RIVER
IPAddress StaticIP  (192, 168, 1, 189);
#elif defined HOTTUB
IPAddress StaticIP  (192, 168, 1, 193);
#else
ERROR... not defined!!!
#endif

//////////////////////////////////////////
// NTP
//////////////////////////////////////////
#include <NTPClient.h>
#include <WiFiUdp.h>

WiFiUDP ntpUDP;

// You can specify the time server pool and the offset (in seconds, can be
// changed later with setTimeOffset() ). Additionally you can specify the
// update interval (in milliseconds, can be changed using setUpdateInterval()).
// NOTE that the offset seems to offect even NTP_GetEpochTime.
#if 0
void NTP_Setup(){}
void NTP_Loop(){}
#else
const int NTPOffset_s = -5 * 60 * 60; // -6 hours
const int NTPInterval_ms = 10 * 60 * 1000; // 10 minutes
NTPClient timeClient(ntpUDP, "time.nist.gov", NTPOffset_s, NTPInterval_ms);

void NTP_Setup()
{
    Serial.println("Starting NTP");
    timeClient.begin();
    //causes an exception 9 after Wifi redo???  timeClient.forceUpdate();
}

void NTP_Loop()
{
    timeClient.update();
    //  Serial.println("NTP " + timeClient.getFormattedTime() + " " + String(timeClient.getEpochTime()));
}

void NTP_GetTime(int &Day, int &Hours, int &Minutes, int &Seconds)
{
    Day     = timeClient.getDay();
    Hours   = timeClient.getHours();
    Minutes = timeClient.getMinutes();
    Seconds = timeClient.getSeconds();

    //Serial.printf("Day = %d, Hours = %d, Minutes = %d, Seconds = %d\n", Day, Hours, Minutes, Seconds);
}

unsigned long NTP_GetEpochTime()
{
    return timeClient.getEpochTime();
}
#endif
//////////////////////////////////////////
// OTA
//////////////////////////////////////////
#include <ArduinoOTA.h>

void Ota_Setup()
{
    Serial.println("Starting OTA");

    // Port defaults to 8266
    // ArduinoOTA.setPort(8266);

    // This is the name returned to the Port selection in the Arduino IDE.
    ArduinoOTA.setHostname(DeviceConfiguration.ClientName.c_str());

    // No authentication by default
    //ArduinoOTA.setPassword((const char *)"123");

    ArduinoOTA.onStart([]() {
        Serial.println("Start OTA process");
    });

    ArduinoOTA.onEnd([]() {
        Serial.println("\nEnd OTA process");
    });

    ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
        unsigned int Percent = (progress / (total / 100));
        Serial.printf("Progress OTA: %u%%", Percent);
        Serial.println("");
        char str[30];
        //sendStrPadXY(" Remote program", 3, 0);
        sprintf(str, "Remote program %3u%%", Percent);
        //sendStrPadXY(str, 5, 6);
    });

    ArduinoOTA.onError([](ota_error_t error) {
        Serial.printf("OTA Error[%u]: ", error);
        if (error == OTA_AUTH_ERROR) Serial.println("OTA Auth Failed");
        else if (error == OTA_BEGIN_ERROR) Serial.println("OTA Begin Failed");
        else if (error == OTA_CONNECT_ERROR) Serial.println("OTA Connect Failed");
        else if (error == OTA_RECEIVE_ERROR) Serial.println("OTA Receive Failed");
        else if (error == OTA_END_ERROR) Serial.println("OTA End Failed");
    });

    ArduinoOTA.begin();
}
void Ota_Loop(void)
{
    ArduinoOTA.handle();
}

///////////////////////////////////////////////////////
// Webserver (and WebSocket)
///////////////////////////////////////////////////////
#include <ESP8266WebServer.h>
#include <WiFiUdp.h>

ESP8266WebServer HttpServer(80);
WebSocketsServer WebSocket = WebSocketsServer(81);    // create a websocket server on port 81

String DebugString;
void SendConstants()
{
    String Json = "{";
    Json += "\"Title\":\""           + DeviceConfiguration.ClientName + "\",";
    Json += "\"SDKVersion\":\""      + String(ESP.getSdkVersion())    + "\",";
    Json += "\"BootVersion\":\""     + String(ESP.getBootVersion())   + "\",";
    Json += "\"FlashSize\":\""       + String(ESP.getFlashChipSize()) + "\",";
    Json += "\"CPUFreq\":\""         + String(ESP.getCpuFreqMHz())    + "\",";
    Json += "\"FirmwareVersion\":\"" + String(FirmwareVersion)        + "\",";
    Json += "\"ResetReason\":\""     + String(ESP.getResetInfo())     + "\",";
    Json += "\"MqttBroker\":\""      + String(MqttBrokerUrl)          + "\",";
    Json += "\"MqttDomain\":\""      + MqttDomainName                 + "\",";
    Json += "\"ProcessInterval\":\"" + String(DeviceConfiguration.ProcessLoopInterval_s) + "\"";    
    Json += '}';

    WebSocket.broadcastTXT(Json);

    SendDynamics();
    SendInfoTable();
}
void SendDynamics()
{
    String Json = "{";
    if (DebugString.length() > 0)
    {
        Json += "\"DebugString\":\"" + DebugString + "\",";
    }
    Json += "\"FreeHeap\":\""         + String(ESP.getFreeHeap())  + "\",";
    Json += "\"UpTimeMinutes\":\""    + String(GetUpTimeMinutes()) + "\",";
    Json += "\"CurrentTimestamp\":\"" + String(CurrentTime_s(0))   + "\",";
    Json += "\"Ssid\":\""             + String(WiFi.SSID())        + "\",";
    Json += "\"Rssi\":\""             + String(WiFi.RSSI())        + "\",";
    Json += "\"MqttBadConnects\":\""  + String(MqttBadConnects)    + "\"";
    
    Json += '}';    

    WebSocket.broadcastTXT(Json);    
}
void SendDynamic(String Key, String Value)
{
    WebSocket.broadcastTXT("{" + Key + ":" + Value + "}");    
}

void SendInfoTable()
{
    if (0 == InfoTableSize)
    {
        return;
    }
    
    String Msg = "{\"InfoTable\":\"";
    int i;
    for (i = 0; i < InfoTableSize-1; ++i)
    {
        String MinutesBack   = String((CurrentTime_s(0) - InfoTable[i].TS.toInt()) / 60);
        Msg += InfoTable[i].Name + "," + InfoTable[i].IP + "," +InfoTable[i].RSSI + "," +InfoTable[i].FW + "," +InfoTable[i].UT + "," +MinutesBack + ",";
    }
    String MinutesBack   = String((CurrentTime_s(0) - InfoTable[i].TS.toInt()) / 60);
    Msg += InfoTable[i].Name + "," + InfoTable[i].IP + "," +InfoTable[i].RSSI + "," +InfoTable[i].FW + "," +InfoTable[i].UT + "," +MinutesBack + "\"}";
    WebSocket.broadcastTXT(Msg);
    Serial.println(Msg);
}

void InitialPage()
{
    HttpServer.sendHeader("Connection", "close");
    HttpServer.sendHeader("Access-Control-Allow-Origin", "*");
    Serial.printf("PAGE LEN = %d\n", sizeof(SITE_index));
    HttpServer.send(200, "text/html", FPSTR(SITE_index));
}

void UpdateSent()
{
    if (HttpServer.uri() != "/update")
    {
        return;
    }
    HTTPUpload& upload = HttpServer.upload();
    if (upload.status == UPLOAD_FILE_START)
    {
        Serial.setDebugOutput(true);
        WiFiUDP::stopAll();
        Serial.printf("Update: %s\n", upload.filename.c_str());
        uint32_t maxSketchSpace = (ESP.getFreeSketchSpace() - 0x1000) & 0xFFFFF000;
        if (!Update.begin(maxSketchSpace))
        {   //start with max available size
            Update.printError(Serial);
        }
    }
    else if (upload.status == UPLOAD_FILE_WRITE)
    {
        if (Update.write(upload.buf, upload.currentSize) != upload.currentSize)
        {
            Update.printError(Serial);
        }
    }
    else if (upload.status == UPLOAD_FILE_END)
    {
        if (Update.end(true))
        {   //true to set the size to the current progress
            Serial.printf("Update Success: %u\nRebooting...\n", upload.totalSize);
        }
        else
        {
            Update.printError(Serial);
        }
        Serial.setDebugOutput(false);
    }
    yield();
}

void UpdateCheck()
{
    String PageData = Update.hasError() ? "FAIL" : "Update seems to have worked";
    HttpServer.sendHeader("Connection", "close");
    HttpServer.sendHeader("Access-Control-Allow-Origin", "*");
    HttpServer.send(200, "text/plain", PageData);
    ESP.restart();
}

void UpdateTempSet()
{
    Serial.println("Temp set...");
    if (HttpServer.args() > 0 )
    {
        for ( uint8_t i = 0; i < HttpServer.args(); i++ )
        {
            if (HttpServer.argName(i) == "CabinBasementTempSet")
            {
                Serial.println(String(HttpServer.arg(i)) + "=Cabin/Basement/Temp/Set");
                Publish(e_CabinBasementTempSet, HttpServer.arg(i));
            }
            if (HttpServer.argName(i) == "CabinGarageUpTempSet")
            {
                Serial.println(String(HttpServer.arg(i)) + "=Cabin/Garage/Up/Temp/Set");
                Publish(e_CabinGarageUpTempSet, HttpServer.arg(i));
            }
            if (HttpServer.argName(i) == "CabinBasementPropTempSet")
            {
                Serial.println(String(HttpServer.arg(i)) + "=Cabin/Basement/Prop/Temp/Set");
                Publish(e_CabinBasementPropTempSet, HttpServer.arg(i));
            }
            if (HttpServer.argName(i) == "CabinKitchenTempSet")
            {
                Serial.println(String(HttpServer.arg(i)) + "=Cabin/Kitchen/Temp/Set");
                Publish(e_CabinKitchenTempSet, HttpServer.arg(i));
            }
        }
    }

    InitialPage();
}

void myReboot()
{
    Serial.println("Rebooting...");
    delayMicroseconds(50 * 1000);
//    while (1);
    //ESP.restart();
}

void SendFavicon()
{
    // SEND THE ICON
    Serial.println("Find out how to send Favicon.ico?");
    HandlerNotFound();
}

void HandlerNotFound()
{
    Serial.println("\n\nNO handler found for this action:");
    String message = "URI: ";
    message += HttpServer.uri();
    message += "\nMethod: ";
    message += ( HttpServer.method() == HTTP_GET ) ? "GET" : "POST";
    message += "\nArguments: ";
    message += HttpServer.args();
    message += "\n";

    for ( uint8_t i = 0; i < HttpServer.args(); i++ ) {
        message += " " + HttpServer.argName ( i ) + ": " + HttpServer.arg ( i ) + "\n";
    }
    Serial.println(message);

    // Respond to the request.
    HttpServer.sendHeader("Connection", "close");
    HttpServer.sendHeader("Access-Control-Allow-Origin", "*");
    HttpServer.send(200, "text/plain", "No handler found for this action");
}

void WebSocketEvent(uint8_t num, WStype_t type, uint8_t * payload, size_t length) 
{
    Serial.printf("EVENT............................%d", num);
    switch(type) {
        case WStype_DISCONNECTED:
            Serial.printf("[%u] WS Disconnected!\n", num);
            break;
        case WStype_CONNECTED:
            {
                IPAddress ip = WebSocket.remoteIP(num);
                Serial.printf("[%u] WS Connected from %d.%d.%d.%d url: %s\n", num, ip[0], ip[1], ip[2], ip[3], payload);
                
                // send message to client
                SendConstants();
            }
            break;
        case WStype_TEXT:
            Serial.printf("[%u] WS get Text: %s\n", num, payload);
            // send message to client
            // WebSocket.sendTXT(num, "message here");

            // send data to all connected clients
            // WebSocket.broadcastTXT("message here");
            break;
        case WStype_BIN:
            Serial.printf("[%u] WS get binary length: %u\n", num, length);
            hexdump(payload, length);

            // send message to client
            // WebSocket.sendBIN(num, payload, length);
            break;
        case WStype_ERROR:
            Serial.printf("[%u] WS get ERROR: %s\n", num, payload);
            break;
        case WStype_FRAGMENT_TEXT_START:
            Serial.printf("[%u] WS get frag start Text: %s\n", num, payload);
            break;
        case WStype_FRAGMENT_BIN_START:
            Serial.printf("[%u] WS get frag bin start: %u\n", num, length);
            hexdump(payload, length);
            break;
        case WStype_FRAGMENT:
            Serial.printf("[%u] WS get Text: %s\n", num, payload);
            break;
        case WStype_FRAGMENT_FIN:
            Serial.printf("[%u] WS get Text: %s\n", num, payload);
            break;
    }
}

void WebServer_Setup()
{
    Serial.println("Starting web server");
    HttpServer.onNotFound(HandlerNotFound);
    HttpServer.on("/", HTTP_GET, InitialPage);
    HttpServer.on("/update", HTTP_POST, UpdateCheck);
    HttpServer.on("/temp_set", HTTP_POST, UpdateTempSet);
    HttpServer.on("/reboot", HTTP_POST, myReboot);
    HttpServer.on("/favicon.ico", HTTP_GET, SendFavicon);
    HttpServer.onFileUpload(UpdateSent);
    HttpServer.begin();

    WebSocket.begin();
    WebSocket.onEvent(WebSocketEvent);
    
    Serial.print("URL is http://");
    Serial.println(LastIPAddress);
}

void WebServer_Loop()
{
    HttpServer.handleClient();
    WebSocket.loop();
}

// Note this function take approx. 1 second.
String GetBestWifi()
{
    String BestSsid;
    int BestRssi = -999;
    int n = WiFi.scanNetworks();
    yield();
    for (int i = 0; i < n; ++i)
    {
        if (WiFi.RSSI(i) > BestRssi)
        {
            // Don't connect to anything NOT *fish*.
            if (WiFi.SSID(i).indexOf("fish") > -1)
            {
                BestRssi = WiFi.RSSI(i);
                BestSsid = WiFi.SSID(i);
            }
        }
        Serial.printf("Found %s at %d dbm\n", WiFi.SSID(i).c_str(), WiFi.RSSI(i));
        yield();
    }

    return BestSsid;
}

// Returns:
// 0 = fail
// 1 = reconnected
// 2 = stayed connected
int ConnectToBestWifi(bool AlwaysDisconnect)
{
    // Scan for best.
    String BestSsid = GetBestWifi();
    if (0 == BestSsid.length())// Sometimes scan doesn't find any...
    {
        return 0;
    }

    if (AlwaysDisconnect)
    {
        WiFi.disconnect();
    }
    else
    {
        // If connected to best, exit.
        if (WiFi.SSID().equals(BestSsid)) // Implies we've already connected even if we're not staying connected...
        {
            Serial.println("Already connected to best Wifi");
            return 2;
        }
        // Disconnect (even if we're not connected).
        WiFi.disconnect();
    }

    delay(100);// Not sure if this is needed. Seems to be in other's scripts.

    // Connect to best.
    WiFi.persistent(false);
    WiFi.mode(WIFI_OFF);// Supposed to fix some bug in the API related to the WiFi state machine going wonky.
    if (UseStaticIP)
    {
        WiFi.config(StaticIP, Gateway, Subnet, DNS1, DNS2);
    }
    WiFi.mode(WIFI_STA);
    WiFi.hostname(DeviceConfiguration.ClientName);
    WiFi.begin(BestSsid.c_str(), WiFiPassword);
    Serial.printf("\nAttempting connection to %s\n", BestSsid.c_str());

    // This allows up to 10 seconds for the connection to complete.
    for (int i = 0; i < 20; ++i)
    {
        Serial.print(".");
        if (WifiConnected())
        {
            Serial.println("");
            break;
        }
        delay(500);
    }
    if (!WifiConnected())
    {
        Serial.println("Wifi did not connect.");
        return 0;
    }

    return 1;
}

void Services_Setup()
{
    Ota_Setup();
    NTP_Setup();
    // DJF
    if (!MDNS.begin(DeviceConfiguration.ClientName.c_str())) {             // Start the mDNS responder for esp8266.local
       Serial.println("Error setting up MDNS responder!");
    }
    else
    {
        Serial.println("MDNS responder running");
    }
    WebServer_Setup();
    MQTT_Setup();
}

void Services_Loop()
{
    Ota_Loop();
    NTP_Loop();
    WebServer_Loop();
    MQTT_Loop();
}

WiFiEventStationModeGotIP myIP;

void WifiIPCB(const struct WiFiEventStationModeGotIP & event)
{
    (void)event;

    LastIPAddress = WiFi.localIP();

    Serial.println("");
    Serial.print("IP: ");
    Serial.println(LastIPAddress);

    // Configure things that need network access.
    Services_Setup();
}
void WifiConnectCB(const struct WiFiEventStationModeConnected & event)
{
    (void)event;
    Serial.println("");
    Serial.print(wifi_station_get_hostname());
    Serial.print(" successful connection to ");
    Serial.println(WiFi.SSID());
}
void WifiDisconnectedCB(const struct WiFiEventStationModeDisconnected & event)
{
    (void)event;
    Serial.println("Disconnection detected!");
}
void Network_Setup()
{
    static WiFiEventHandler e1, e2, e3;
    e1 = WiFi.onStationModeConnected   (&WifiConnectCB);
    e2 = WiFi.onStationModeGotIP       (&WifiIPCB);
    e3 = WiFi.onStationModeDisconnected(&WifiDisconnectedCB);

    if (0 == ConnectToBestWifi(true))
    {
        return;// Try again... in the loop?
        //ESP.reset();
    }
}

void Network_5MinLoop()
{
    Serial.println("Network 5 min loop");
    // Update published info.
    PublishDeviceInfo();

    // See if there's a better Wifi connection available.
    ConnectToBestWifi(false);
}

void Network_Loop()
{
    // Update the success time if we're still connected.
    //if (WifiConnected()) Sometimes doesn't work...
    if (WiFi.localIP().toString() != "0.0.0.0")// This doesn't seem to work either... always is 0.0.0.0
    {
        LastWifiSuccessTime_ms = millis();
    }
    else
    {
        // If we're without a connection for a minute, lock up to force a WDT.
        unsigned long WifiDisconnectTime_ms = millis() - LastWifiSuccessTime_ms;
        Serial.printf("Wifi status is %d for %lu seconds!\n", WiFi.status(), WifiDisconnectTime_ms / 1000);
        if (WifiDisconnectTime_ms > 30000)
        {
            LastWifiSuccessTime_ms = millis();
            //ESP.restart(); Seems to result in non-recoverable WDT.
            //ESP.reset(); Seems to result in non-recoverable WDT.
            ConnectToBestWifi(true);
        }
        else
        {
            delay(250);// Without this serial output above floods.
        }
    }

    Services_Loop();
}

void Network_Shutdown()
{
    // Clean up any pending messages.
    MQTT_Loop();
    MqttClient.disconnect();
    HttpServer.close();
    WebSocket.disconnect();
    WiFi.disconnect();
}
