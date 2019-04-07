#ifdef HOTTUB

// Code for this specific function here, called by one of the functions interfaces below.
float PreviousPublishedTemp_F = -99;
float LimitLowerTemp_F = 97;
float LimitUpperTemp_F = 106;
bool WarningSent = false;

// Function_* below are called by the setup and main loops.
void Function_Subscriptions()
{
  Subscribe(e_HotTubLimitLower);
  Subscribe(e_HotTubLimitUpper);
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
    Msg += "Hot tub temp (F),"    + String(LastDS1Temp_F)    + "\"}";
    Msg += "Hot tub temp LL (F)," + String(LimitLowerTemp_F) + "\"}";
    Msg += "Hot tub temp UL (F)," + String(LimitUpperTemp_F) + "\"}";
    WebSocket.broadcastTXT(Msg);
    Serial.println(Msg);
}

// This will be run every 5 minutes.
void Function_5MinProcessLoop()
{
    PreviousPublishedTemp_F = LastDS1Temp_F;
    Publish(DeviceConfiguration.Topics.DS1, PreviousPublishedTemp_F);
}

// This will be run only once.
void Function_RunOnceProcessLoop()
{
    PreviousPublishedTemp_F = LastDS1Temp_F;
    Publish(DeviceConfiguration.Topics.DS1, PreviousPublishedTemp_F);
}

void Function_ProcessLoop()
{
    // Check for out of range
    if ((LastDS1Temp_F < LimitLowerTemp_F) || (LastDS1Temp_F > LimitUpperTemp_F))
    {
        // How to send email?
        WarningSent = true;
    }
    else
    {
        // Clear the flag.
        WarningSent = false;
    }
    
    // Only publish on an update.
    if (OutsideRange(LastDS1Temp_F, PreviousPublishedTemp_F, 1))
    {
        PreviousPublishedTemp_F = LastDS1Temp_F;
        Publish(DeviceConfiguration.Topics.DS1, PreviousPublishedTemp_F);
    }

    SendFunctionInfo();
}

// Called whenever an MQTT topic is received.
void Function_ReceivedTopic(Topic_t Subtopic, String Value)
{
    switch (Subtopic)
    {
        case e_HotTubLimitLower:
            LimitLowerTemp_F = Value.toFloat();
            break;
        case e_HotTubLimitUpper:
            LimitUpperTemp_F = Value.toFloat();
            break;
        default:
            (void)Value;
            break;
    }
}

#endif
