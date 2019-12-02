#ifdef TEST

// Code for this specific function here, called by one of the functions interfaces below.
bool OldState = false;
float PublishedDistance_in = 0;
float PublishedLastAdc_V = 0;

// Function_* below are called by the setup and main loops.
void Function_Subscriptions()
{
//        SendEmail = false;

    Subscribe(e_CabinKitchenTempMeas);
    Subscribe(e_CabinKitchenTempSet);
}

void Function_Setup()
{
    DeviceConfiguration.ClientName            = "FS_TestPlatform";
    DeviceConfiguration.ProcessLoopInterval_s = 60;
    //DeviceConfiguration.SleepDelay_s        = 5;
    DeviceConfiguration.Topics.DHTTemp        = e_TestDHTTempMeas;
    DeviceConfiguration.Topics.DHTHumidity    = e_TestDHTHumidityMeas;
    DeviceConfiguration.Topics.DS1            = e_TestDS1TempMeas;
    DeviceConfiguration.Topics.ADC            = e_TestADCMeas;
    DeviceConfiguration.Topics.Distance       = e_TestDistanceMeas;
}

void SendFunctionInfo()
{
    String Msg = "{\"FunctionTable\":\"";
    Msg += "DHT Temp (F),"           + String(LastDHTTemp_F) + ",";
    Msg += "DHT Humidity (%),"       + String(LastDHTHumidity) + ",";
    Msg += "DS1 Temp(F),"            + String(LastDS1Temp_F) + ",";
    Msg += "Median Distance (in),"   + String(MedianLastDistance_in) + ",";
    Msg += "ADC (V),"                + String(LastAdc_V) + ",";
    Msg += "LED Red,"                + String(LastIndicationRed ? "ON" : "OFF") + ",";
    Msg += "LED Green,"              + String(LastIndicationGrn ? "ON" : "OFF") + ",";
    Msg += "Relay 1,"                + String(LastRelay2 ? "ON" : "OFF") + ",";
    Msg += "Relay 2,"                + String(LastRelay2 ? "ON" : "OFF") + "\"}";
    WebSocket.broadcastTXT(Msg);
    Serial.println(Msg);
}

// This will be run every 5 minutes.
void Function_5MinProcessLoop()
{
    Publish(DeviceConfiguration.Topics.Distance, MedianLastDistance_in);
    PublishedDistance_in = MedianLastDistance_in;
    Publish(DeviceConfiguration.Topics.ADC, LastAdc_V);
    PublishedLastAdc_V = LastAdc_V;
    SendFunctionInfo();
}

// This will be run only once.
void Function_RunOnceProcessLoop()
{
}

// This loop executes after all measurements have been taken.
void Function_ProcessLoop()
{
    if (OutsideRange(MedianLastDistance_in, PublishedDistance_in, 4))
    {
        Publish(DeviceConfiguration.Topics.Distance, MedianLastDistance_in);
        PublishedDistance_in = MedianLastDistance_in;
    }

    if (OutsideRange(LastAdc_V, PublishedLastAdc_V, 0.1))
    {
        Publish(DeviceConfiguration.Topics.ADC, LastAdc_V);
        PublishedLastAdc_V = LastAdc_V;
    }
    SetIndicatorRed(OldState);
    SetIndicatorGrn(!OldState);
    SetRelay1(OldState);
    SetRelay2(!OldState);
    OldState = !OldState;
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
