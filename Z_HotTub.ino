#ifdef HOTTUB

// Code for this specific function here, called by one of the functions interfaces below.
float PreviousTemp_F = -99;

// Function_* below are called by the setup and main loops.
void Function_Subscriptions()
{
}

void Function_Setup()
{
    DeviceConfiguration.ClientName            = "FS_Hottub";
    DeviceConfiguration.ProcessLoopInterval_s = 10;
    DeviceConfiguration.Topics.DS1            = e_HottubTempMeas;
}

void SendFunctionInfo()
{
    String Msg = "{\"FunctionTable\":\"";
    Msg += "Hot tub temp (F)," + String(LastDS1Temp_F) + "\"}";
    WebSocket.broadcastTXT(Msg);
    Serial.println(Msg);
}

// This will be run every 5 minutes.
void Function_5MinProcessLoop()
{
    PreviousTemp_F = LastDS1Temp_F;
    Publish(DeviceConfiguration.Topics.DS1, PreviousTemp_F);
    SendFunctionInfo();
}

// This will be run only once.
void Function_RunOnceProcessLoop()
{
    PreviousTemp_F = LastDS1Temp_F;
    Publish(DeviceConfiguration.Topics.DS1, PreviousTemp_F);
}

void Function_ProcessLoop()
{
    // Only publish on an update.
    if (OutsideRange(LastDS1Temp_F, PreviousTemp_F, 1))
    {
        PreviousTemp_F = LastDS1Temp_F;
        Publish(DeviceConfiguration.Topics.DS1, PreviousTemp_F);
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
}

#endif
