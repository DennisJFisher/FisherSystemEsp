#ifdef KITCHEN

// Code for this specific function here, called by one of the functions interfaces below.
float PreviousTemp_F   = -99;
float PreviousHumidity = -99;

// Function_* below are called by the setup and main loops.
void Function_Subscriptions()
{
}

void Function_Setup()
{
    DeviceConfiguration.ClientName            = "FS_Kitchen";
    DeviceConfiguration.ProcessLoopInterval_s = 10;
    DeviceConfiguration.Topics.DHTTemp        = e_CabinKitchenTempMeas;
    DeviceConfiguration.Topics.DHTHumidity    = e_CabinKitchenHumidityMeas;
}

void SendFunctionInfo()
{
    String Msg = "{\"FunctionTable\":\"";
    Msg += "Kitchen temp meas (F),"    + String(LastDHTTemp_F) + ",";
    Msg += "Kitchen humidity meas (%)," + String(LastDHTHumidity) + "\"}";
    WebSocket.broadcastTXT(Msg);
    Serial.println(Msg);
}

// This will be run only once.
void Function_RunOnceProcessLoop()
{
    if (!isnan(LastDHTTemp_F))
    {
        Publish(DeviceConfiguration.Topics.DHTTemp,     LastDHTTemp_F);
        PreviousTemp_F = LastDHTTemp_F;
        Publish(DeviceConfiguration.Topics.DHTHumidity, LastDHTHumidity);
        PreviousHumidity = LastDHTHumidity;
    }
}

// This will be run every 5 minutes.
void Function_5MinProcessLoop()
{
    if (!isnan(LastDHTTemp_F))
    {
        PreviousTemp_F = LastDHTTemp_F;
        Publish(DeviceConfiguration.Topics.DHTTemp, PreviousTemp_F);
        PreviousHumidity = LastDHTHumidity;
        Publish(DeviceConfiguration.Topics.DHTHumidity, PreviousHumidity);
    }
    SendFunctionInfo();
}

void Function_ProcessLoop()
{
    if (!isnan(LastDHTTemp_F))
    {
        // Only publish on an update.
        if (OutsideRange(LastDHTTemp_F, PreviousTemp_F, 1))
        {
            PreviousTemp_F = LastDHTTemp_F;
            Publish(DeviceConfiguration.Topics.DHTTemp, PreviousTemp_F);
        }
        // Only publish on an update.
        if (OutsideRange(LastDHTHumidity, PreviousHumidity, 1))
        {
            PreviousHumidity = LastDHTHumidity;
            Publish(DeviceConfiguration.Topics.DHTHumidity, PreviousHumidity);
        }
    }

    SendFunctionInfo();
}

// Called whenever an MQTT topic is received.
void Function_ReceivedTopic(Topic_t Subtopic, String Value)
{
    switch (Subtopic)
    {
        default:
            (void)Value;
            break;
    }

    Function_ProcessLoop();
}

#endif
