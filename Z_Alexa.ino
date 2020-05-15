#ifdef ALEXA

#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_ADXL345_U.h>

/* Assign a unique ID to this sensor at the same time */
Adafruit_ADXL345_Unified accel = Adafruit_ADXL345_Unified(12345);
double LastXAccel = 0.0;
double LastYAccel = 0.0;
double LastZAccel = 0.0;
int SprinklerValue = 0;

Topic_t TopicList[]
{
    e_CabinPowerEnabled,
    e_CabinGaragePowerEnabled,
    e_CabinBasementPropTempSet,
    e_CabinKitchenTempSet,
    e_CabinGarageUpTempSet,
    e_CabinBasementTempMeas,
    e_CabinKitchenTempMeas,
    e_CabinGarageUpTempMeas,
    e_Sprinkler,
};
const int NumTopics = sizeof(TopicList)/sizeof(Topic_t);
String CurrentPayloadValues[NumTopics];

void GetAccelerations()
{
  sensors_event_t event;
  accel.getEvent(&event);
  static int OldValue = 0;
  int DiceValue = 0;
//  sensor_t sensor;
//  accel.getSensor(&sensor);

  LastXAccel = event.acceleration.x;
  LastYAccel = event.acceleration.y;
  LastZAccel = event.acceleration.z;
  if (LastXAccel > +5) DiceValue = 1;
  if (LastXAccel < -5) DiceValue = 6;
  if (LastYAccel > +5) DiceValue = 2;
  if (LastYAccel < -5) DiceValue = 5;
  if (LastZAccel > +5) DiceValue = 3;
  if (LastZAccel < -5) DiceValue = 4;

  Serial.printf("Dice = %d\n", DiceValue);
  if (OldValue != DiceValue)
  {
    // Only publish if the time stamp has been found.
    if (CurrentTime_s(0) > 10000)
    {
      Serial.println("Publishing...");
      Publish(e_Sprinkler, DiceValue);
      OldValue = DiceValue;
    }
  }
}
// Function_* below are called by the setup and main loops.
void Function_Subscriptions()
{
    for (int i = 0; i < NumTopics; ++i)
    {
        CurrentPayloadValues[i] = "Unknown";
        Subscribe(TopicList[i]);
    }
}

void Function_Setup()
{
    DeviceConfiguration.ClientName            = "FS_Alexa";
    DeviceConfiguration.ProcessLoopInterval_s = 2;
    //DeviceConfiguration.SleepDelay_s        = 5;

    accel.begin();
}

void SendFunctionInfo()
{
    String Msg = "{\"FunctionTable\":\"";
    for (int i = 0; i < NumTopics; ++i)
    {
        Msg += SubtopicString[TopicList[i]] + ","   + String(CurrentPayloadValues[i]) + ",";
    }
//    String Acc = "Dice,0,";
//    Acc[5] += DiceValue;
//    Msg += Acc;
    // Remove last comma.
    Msg.remove(Msg.length()-1);
    Msg += "\"}";
    WebSocket.broadcastTXT(Msg);
    Serial.println(Msg);
}

// This will be run every 5 minutes.
void Function_5MinProcessLoop()
{
    SendFunctionInfo();
}

// This will be run only once.
void Function_RunOnceProcessLoop()
{
}

// This loop executes after all measurements have been taken.
void Function_ProcessLoop()
{
    GetAccelerations();
    SendFunctionInfo();
}

// Called whenever an MQTT topic is received.
void Function_ReceivedTopic(Topic_t Subtopic, String Value)
{
    // Go through the list of topics we care about.
    for (int i = 0; i < NumTopics; ++i)
    {
//        Serial.println("check " + String(Subtopic) + "=" + String(TopicList[i]));
        // If there's a match, save the value.
        if (Subtopic == TopicList[i])
        {
            CurrentPayloadValues[i] = Value;
        }
    }

    // Update the table with the new value.
    SendFunctionInfo();
}

// http://192.168.1.194/Alexa?Command=Get&Topic=%22Cabin/Garage/Up/Temp/Meas%22&Value=69
void AlexaParser()
{
    String Command;
    String TopicStr;
    String Value;
    bool Fault = false;
    
    String message = "Alexa stuffage\n\n";
    message += "URI: ";
    message += HttpServer.uri();
    message += "\nMethod: ";
    message += ( HttpServer.method() == HTTP_GET ) ? "GET" : "POST";
    message += "\nArguments: ";
    message += HttpServer.args();
    message += "\n";

    // Get Command, Subtopic, and Value (if exists)
    for ( uint8_t i = 0; i < HttpServer.args(); i++ )
    {
        if (HttpServer.argName(i).equals("Command"))
        {
            Command = HttpServer.arg(i);
        }
        if (HttpServer.argName(i).equals("Topic"))
        {
            TopicStr = HttpServer.arg(i);
            // Need to remove the parans.
            TopicStr.replace("\"", "");
        }
        if (HttpServer.argName(i).equals("Value"))
        {
            Value = HttpServer.arg(i);
        }
        
        message += " " + HttpServer.argName ( i ) + ": " + HttpServer.arg ( i ) + "\n";
    }

    // Validate the command.
    if (Command.length() != 3)
    {
        message += " Command argument is invalid (must be Get or Set). " + message + "\n";
        Fault = true;
    }
    else
    {
        // Validate the value and the topic (only care about the topic for a Set).
        if (Command.equals("Set"))
        {
            if (Value.length() < 2)
            {
                message += "Value argument is invalid for a Set command. " + message + "\n";
                Fault = true;
            }
            else
            {
                int val = Value.toInt();
                if ((val < 20) || (val > 81))
                {
                    message += "Value argument is invalid for a Set command. " + message + "\n";
                    Fault = true;
                }
            }
            if (TopicStr.length() < 5) // arbitrary
            {            
                message += "Topic argument is invalid. " + message + "\n";
                Fault = true;
            }
        }
    }

    // Alexa has a valid set or get request.
    if (!Fault)
    {
        // Assume we won't find the topic.
        Fault = true;
        for (int i = 0; i < NumTopics; ++i)
        {
            Topic_t Topic = TopicList[i];
            message += " sub=" + String(i) + "=" + SubtopicString[Topic] + "=" + TopicStr + "\n";
            Serial.printf("%d -%s- == -%s-\n", Topic, SubtopicString[Topic].c_str(), TopicStr.c_str());
            if (SubtopicString[Topic].equals(TopicStr))
            {
                Serial.println("Alexa " + Command + " of " + SubtopicString[Topic]);
                message += "Alexa " + Command + " of " + SubtopicString[Topic] + " 0=" + CurrentPayloadValues[0] + " 1=" + CurrentPayloadValues[1] + "\n";
                if (Command.equals("Get"))
                {
                    //message += CurrentPayloadValues[i];
                    message = CurrentPayloadValues[i];
                    Fault = false;
                }
                else
                {
                    //Publish(Topic, Value);
                }
                // All done so bail (only one topic).
                break;
            }
        }
    }

    HttpServer.send ( 404, "text/plain", message );

    Serial.println(message);
}

#endif
