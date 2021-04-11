#ifdef ESP8266 
#include <ESP8266WiFi.h>
#else
#include "WiFi.h"
#endif
#include <PubSubClient.h>
#include "Config.h"

// This set allows numerical, string, and char * to be used as the value.
inline bool Publish(Topic_t Topic, String Value) 
{
    return MqttPublish(Topic, Value);
}
inline bool Publish(Topic_t Topic, int Value) 
{
    return Publish(Topic, String(Value));
}
inline bool Publish(Topic_t Topic, double Value) 
{
    // Need to publish with a resolution of 0.1
    Value = round(10 * Value) / 10;
//    int Num = (int)Value;
//    int Tenth = (int)((Value - Num)*10);
    return Publish(Topic, (String)Value);
}

const String MqttDomainName = "FisherSystem";
const String MqttBrokerUrl  = "broker.mqtt-dashboard.com";
//const String MqttBrokerUrl  = "iot.eclipse.org";
//const String MqttBrokerUrl  = "test.mosquitto.org";
const char   MQTTUsername[] = "Lemae";
const char   MQTTPassword[] = "salicia";
const int MqttBrokerPort = 1883;
const int TopicWidthPrint = 20;// Used below to pad topic strings.

WiFiClient MqttWifiClient;
PubSubClient MqttClient(MqttBrokerUrl.c_str(), MqttBrokerPort, ProcessReceivedTopic, MqttWifiClient);
int MqttBadConnects = 0;// Resets if a successful connection happens.
int MqttBadPubs = 0;

extern String DebugString;

////////////////////////////
// InfoTable rules
// If InfoTableName bogus, return.
// if Name in the table, store that row.
// If the payload is bogus, delete the name row and return.
// if row stored, replace that row.
// else add new row.
///////////////////////////
const int InfoTableMax = 20;
struct Info
{
    String Name;
    String IP;
    String RSSI;
    String FW;
    String UT;
    String TS;
} InfoTable[InfoTableMax];
int InfoTableSize = 0;
void InfoTableDeleteRow(int DeleteRow)
{
    if (DeleteRow < 0)
    {
        return;
    }

    for (int i = DeleteRow; i < InfoTableSize - 1; ++i)
    {
        InfoTable[i].Name = InfoTable[i + 1].Name;
        InfoTable[i].IP       = InfoTable[i + 1].IP;
        InfoTable[i].RSSI  = InfoTable[i + 1].RSSI;
        InfoTable[i].FW     = InfoTable[i + 1].FW;
        InfoTable[i].UT      = InfoTable[i + 1].UT;
        InfoTable[i].TS      = InfoTable[i + 1].TS;
    }

    --InfoTableSize;
}
void UpdateInfoTable(String DeviceInfoName, String PayloadStr)
{
    int NameRow = -1;
    
    // Make sure the table size is sane for now...
    if (InfoTableSize < 0)
    {
        DebugString = "InfoTableSize below 0 !!!!!!!!!!!!!!!!!!!!";
        Serial.println("InfoTableSize below 0 !!!!!!!!!!!!!!!!!!!!!");
        InfoTableSize = 0;
    }
    if (InfoTableSize >= InfoTableMax)
    {
        DebugString = "InfoTableSize above end !!!!!!!!!!!!!!!!!!!!!";
        Serial.println("InfoTableSize above end !!!!!!!!!!!!!!!!!!!!!");
        InfoTableSize = InfoTableMax - 1;
    }

    // Ignore bogus names.
    if (DeviceInfoName.length() < 8)
    {
        Serial.printf("  Ignored info for %s\n", DeviceInfoName.c_str());
        return;
    }

    // Store any row matching the name.
    for (int i = 0; i < InfoTableSize; ++i)
    {
        if (InfoTable[i].Name.equals(DeviceInfoName))
        {
            NameRow = i;
            break;
        }
    }

    // If the payload is empty, delete the name row and bail.
    if (0 == PayloadStr.length())
    {
        Serial.printf("Empty payload - deleting %s\n", DeviceInfoName.c_str());
        InfoTableDeleteRow(NameRow);
        return;
    }

    // Format of the payload:
    // <IP>:<RSSI>:<FW>:<UT>,<Device>,<TS>

    // Break out the info.
    int EndOfIPIndex     = PayloadStr.indexOf(':');
    int EndOfRSSIIndex   = PayloadStr.indexOf(':', EndOfIPIndex + 1);
    int EndOfFWIndex     = PayloadStr.indexOf(':', EndOfRSSIIndex + 1);
    int EndOfUTIndex     = PayloadStr.indexOf(',', EndOfFWIndex + 1);
    int EndOfDeviceIndex = PayloadStr.lastIndexOf(',');
    // Make sure we have a valid payload.
    // IP will be of the form x.x.x.x
    // RSSI will be of the form -xx or -xxx
    // FirmwareVersion will be of the form x.xx
    if (EndOfUTIndex >= EndOfDeviceIndex)
    {
        DebugString = "Bogus Value for info table payload";
        Serial.println("  Bogus Value for info table payload");
        InfoTableDeleteRow(NameRow);
        return;
    }

    String IP   = PayloadStr.substring(                   0, EndOfIPIndex);
    String RSSI = PayloadStr.substring(    EndOfIPIndex + 1, EndOfRSSIIndex);
    String FW   = PayloadStr.substring(  EndOfRSSIIndex + 1, EndOfFWIndex);
    String UT   = PayloadStr.substring(    EndOfFWIndex + 1, EndOfUTIndex);
    String Name = PayloadStr.substring(    EndOfUTIndex + 1, EndOfDeviceIndex);
    String TS   = PayloadStr.substring(EndOfDeviceIndex + 1);

    // If this is a new device, append it.
    if (NameRow < 0) // Not found
    {
        // Append a new row..
        NameRow = InfoTableSize;
        ++InfoTableSize;
    }

    InfoTable[NameRow].Name = Name;
    InfoTable[NameRow].IP   = IP;
    InfoTable[NameRow].RSSI = RSSI;
    InfoTable[NameRow].FW   = FW;
    InfoTable[NameRow].UT   = UT;
    InfoTable[NameRow].TS   = TS;

    SendInfoTable();
}

void PublishDeviceInfo()
{
    // Format of the payload:
    // <IP>:<RSSI>:<FW>:UTM,<Device>,<TS>
    String Value = WiFi.localIP().toString() + ':' + WiFi.RSSI() + ':' + FirmwareVersion + ':' + String(GetUpTimeMinutes());

    Publish(e_DeviceInfo, Value);
}

bool FirstTSUpdate = true;
unsigned int CurrentTime_s(unsigned int UpdateTime_s)
{
    static unsigned int LastUpdate_ms = 0;
    static unsigned int LastTimestamp_s = 0;
    unsigned int RetVal;
    if (UpdateTime_s > 0)
    {
        RetVal = LastTimestamp_s = UpdateTime_s;
        LastUpdate_ms = millis();

        if (FirstTSUpdate)
        {
            // Recursively called by PublishDeviceInfo so clear this flag first.
            FirstTSUpdate = false;
            // Publish the config info if this is the first timestamp update.
            PublishDeviceInfo();
        }
    }
    else
    {
        RetVal = LastTimestamp_s + (millis() - LastUpdate_ms) / 1000;
    }

    return RetVal;
}
// Helper calls used so the loop can be called for each subscription.
inline String BuildFullTopic(Topic_t Subtopic)
{
    return MqttDomainName + '/' + SubtopicString[Subtopic] + "/#";
}
void Subscribe(Topic_t Subtopic)
{
    String FullTopic = BuildFullTopic(Subtopic);
    Serial.println("Subscribing to " + FullTopic);
    // boolean subscribe (topic, [qos]);
    MqttClient.subscribe(FullTopic.c_str(), 1);
    // Need this so we don't run out of buffers when a number of subscriptions are requested.
    MqttClient.loop();
}

// Helper call used so the loop can be called for each subscription/sunsubscription..
void Unsubscribe(Topic_t Subtopic)
{
    String FullTopic = BuildFullTopic(Subtopic);
    Serial.println("Unsubscribing from " + FullTopic);
    MqttClient.unsubscribe(FullTopic.c_str());
}

// Set up common and device specific subscriptions.
void MqttSubscriptions()
{
    // All need these
    Subscribe(e_CurrentTimestamp);
    Subscribe(e_Reboot);
#ifndef RIVER
// Unsubscribe isn't fast enough :(
    Subscribe(e_DeviceInfo);
#endif

    // Specific function subscriptions.
    Function_Subscriptions();
}

bool MqttConnect()
{
    if (MQTT_CONNECTED == MqttClient.state())
    {
        return true;
    }

    // Either the server is down or the WiFi object f'ed up again... Try this?
    if (30 == MqttBadConnects)
    {
        MqttBadConnects = 0;
        ConnectToBestWifi(true);
    }

    Serial.println("MQTT state=" + String(MqttClient.state()));
    String WillTopic = DeviceConfiguration.ClientName + "/Death";

    // boolean connect (clientID, username, password, willTopic, willQoS, willRetain, willMessage);
    Serial.println("Connecting to MQTT:");
    Serial.println("  Broker ->    " + MqttBrokerUrl);
    Serial.println("  Port ->      " + String(MqttBrokerPort));
    Serial.println("  User name -> " + String(MQTTUsername));
    Serial.println("  User PW ->   " + String(MQTTPassword));
    Serial.println("  Client ID -> " + DeviceConfiguration.ClientName);
    if (MqttClient.connect(DeviceConfiguration.ClientName.c_str(), MQTTUsername, MQTTPassword, WillTopic.c_str(), 2, 1, DeviceConfiguration.ClientName.c_str()))
    {

// Weird bug in WiFi client or MQTT... connect returns true yet state is -4 (MQTT_CONNECTION_TIMEOUT).
        if (-4 == MqttClient.state())
        {
            ++MqttBadConnects;
            return false;
        }
        Serial.println("MQTT Connection successful");
        Serial.println("MQTT state=" + String(MqttClient.state()));
        MqttSubscriptions();
        MqttBadConnects = 0;// Reset this.
        return true;
    }
    else
    {
        int State = MqttClient.state();
        Serial.println("MQTT Connection failed (state=" + String(State) + ")... try again in 1 sec...");
        ++MqttBadConnects;
        delay(1000);
    }

    return false;
}

bool MqttPublish(Topic_t Subtopic, String Value)
{
    bool Status = false;// Assume we'll fail.
    if (MQTT_CONNECTED != MqttClient.state())
    {
        return Status;
    }

    // Build the full topic.
    String FullTopic = MqttDomainName + "/" + SubtopicString[Subtopic];

    // For generic topics, append the client name to the topic.
    // It NEEDS to be in there to make it unique.
    switch (Subtopic)
    {
        case e_Birth:
        case e_DeviceInfo:
            FullTopic += '/' + DeviceConfiguration.ClientName;
            break;
        default:
            break;
    }

    String DisplayTopicString(FullTopic);
    for (int i = DisplayTopicString.length(); i < TopicWidthPrint; ++i)
    {
        DisplayTopicString += ' ';
    }

#ifdef DISABLE_PUBLISHING
    (void)Value;
#else
    String FullValue = Value + + "," + DeviceConfiguration.ClientName + "," + String(CurrentTime_s(0));
    Serial.printf("Publishing %s -> %s\n", FullTopic.c_str(), FullValue.c_str());
    Status = MqttClient.publish(FullTopic.c_str(), FullValue.c_str(), true);
    if (!Status)
    {
        ++MqttBadPubs;
        Serial.print(" --FAILED(");
        Serial.print(MqttClient.state());
        Serial.println(")--");
    }
#endif

    // Also do yield() so some of the repeated requests don't cause issues.
    yield();

    return Status;
}

void ProcessReceivedTopic(char* Topic, byte* Payload, unsigned int Length)
{
    // Parse topic... Domain/x

    // Get a ptr to the first '/' (after the FisherSystem)
    char *ptr = strchr(Topic, '/') + 1;
    if (0 == ptr)
    {
        Serial.println("");
        Serial.print("Bogus topic! -> ");
        Serial.println(Topic);
        return;
    }

    String TopicString(ptr);

    // Create the value from the payload.
    char PayloadChr[Length + 1];
    memcpy(&PayloadChr[0], Payload, Length);
    PayloadChr[Length] = 0;

    String PayloadStr(PayloadChr);
    Serial.println(String("Rec  ") + TopicString + "->" + PayloadStr);

    // These topics have the specific device name appended to make them unique for publishing.
    // Strip it off here. The device name will be in the value if needed.
    String DeviceInfoName;
    if (TopicString.startsWith("DeviceInfo") || TopicString.startsWith("Birth"))
    {
        // Strip off the device name.
        int Index = TopicString.indexOf('/');
        if (Index > 0)
        {
            DeviceInfoName = TopicString.substring(Index + 1);
            TopicString = TopicString.substring(0, Index);
        }
    }

    // Slow but... get the descriptor by matching the subtopic string.
    Topic_t Subtopic = e_None;
    for (int i = 1; i < e_MaxSubtopics; ++i)
    {
        if (SubtopicString[i].equals(TopicString))
        {
            Subtopic = (Topic_t)i;
        }
    }

    // Terminate at any ',' since that's not the part we care about.
    String Value = PayloadStr;
    int Index = PayloadStr.indexOf(',');
    if (Index > 0)
    {
        Value = Value.substring(0, Index);
    }

    // Process all common subtopics.
    // Specific subtopics should be handled by the function code.
    switch (Subtopic)
    {
        case e_None:
            Serial.println(" (Subtype was e_None!!!)");
            break;
        case e_CurrentTimestamp:
            CurrentTime_s(Value.toInt());
            break;
        case e_DeviceInfo:
            UpdateInfoTable(DeviceInfoName, PayloadStr);
            break;
        case e_Sprinkler:
            Serial.println("Sprinkler:" + Value);
            break;
        case e_Reboot:
            Serial.println("Found reboot for " + Value);
            // If the value is this client name, then reboot immediately
            if (DeviceConfiguration.ClientName.equals(Value))
            {
                // Republish this with an empty value to keep from getting into a reboot loop.
                Publish(e_Reboot, "Acknowledge");
                delay(1000); // Give it time to publish.
                myReboot();
            }
            break;
        // These are handled by the devices that care about them.
        case e_CabinGarageDoorCommand:
        case e_CabinBasementTempMeas:
        case e_CabinBasementPropTempSet:
        case e_CabinKitchenTempSet:
        case e_CabinGarageUpTempMeas:
        case e_CabinGarageUpTempSet:
        case e_CabinGarageDownTempMeas:
        case e_CabinKitchenTempMeas:
        case e_CabinPowerEnabled:
        case e_CabinGaragePowerEnabled:
            break;
        default:
            Serial.println("Unhandled topic:" + SubtopicString[Subtopic]);
            Serial.println("          Value:" + String(Value));
            break;
    }

    Function_ReceivedTopic(Subtopic, Value);
}

void MQTT_Setup()
{
    Serial.println("Starting MQTT client");
    InfoTableSize = 0;
    // Actually connect in MQTT_Loop();
}

IPAddress LastPublishedIPAddress;
void MQTT_Loop()
{
    if (WifiConnected())
    {
        // Make sure we're connected to the broker.
        if (MqttConnect())
        {
            // Process MQTT activities.
            MqttClient.loop();

            // If the current IP hasn't been published, do so.
            IPAddress NewIPAddress = WiFi.localIP();

            // Save and publish it if it's new.
            if (NewIPAddress != LastPublishedIPAddress)
            {
                LastPublishedIPAddress = NewIPAddress;
                PublishDeviceInfo();
            }
        }
    }
}
