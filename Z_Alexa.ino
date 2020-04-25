#ifdef ALEXA

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
};
const int NumTopics = sizeof(TopicList)/sizeof(Topic_t);
String CurrentValues[NumTopics];

// Function_* below are called by the setup and main loops.
void Function_Subscriptions()
{
    for (int i = 0; i < NumTopics; ++i)
    {
        CurrentValues[i] = "Unknown";
        Subscribe(TopicList[i]);
    }
}

void Function_Setup()
{
    DeviceConfiguration.ClientName            = "FS_Alexa";
    DeviceConfiguration.ProcessLoopInterval_s = 60;
    //DeviceConfiguration.SleepDelay_s        = 5;
}

void SendFunctionInfo()
{
    String Msg = "{\"FunctionTable\":\"";
    for (int i = 0; i < NumTopics; ++i)
    {
        Msg += SubtopicString[TopicList[i]] + ","   + String(CurrentValues[i]) + ",";
    }
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
}

// Called whenever an MQTT topic is received.
void Function_ReceivedTopic(Topic_t Subtopic, String Value)
{
    // Go through the list of topics we care about.
    for (int i = 0; i < NumTopics; ++i)
    {
        Serial.println("check " + String(Subtopic) + "=" + String(TopicList[i]));
        // If there's a match, save the value.
        if (Subtopic == TopicList[i])
        {
            CurrentValues[i] = Value;
        }
    }
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

    // Alexa has requested getting a value.
    if (!Fault)
    {
        // Assume we won't find the topic.
        Fault = true;
        for (int i = 0; i < NumTopics; ++i)
        {
            Topic_t Topic = TopicList[i];
            message += " sub=" + String(i) + "=" + SubtopicString[Topic] + "=" + TopicStr + "\n";
            if (SubtopicString[Topic].equals(TopicStr))
            {
                Serial.println("Alexa " + Command + " of " + SubtopicString[Topic]);
                message += "Alexa " + Command + " of " + SubtopicString[Topic] + " 0=" + CurrentValues[0] + " 1=" + CurrentValues[1] + "\n";
                if (Command.equals("Get"))
                {
                    //message += CurrentValues[i];
                    message = CurrentValues[i];
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
