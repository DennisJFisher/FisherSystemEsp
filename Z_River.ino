#ifdef RIVER

//
// Read old values of temp and distance.
// Process...
//   Measure temp, humidity, and distance.
//   If they're valid...
//     if temp +2/-2 of prev temp, don't publish.
//     if distance +5/-5 of prev distance, don't publish.
//     if last publish was < 60 min, don't publish.
//     Sleep...
//
//Use NTP? Or TS?

// Code for this specific function here, called by one of the functions interfaces below.
const int SleepTime_s     = 10 * 60; // 10 minutes
float OldTempMeas_f      = -99;
uint8_t OldDistance_in    = -99;
uint32_t OldPublishedTS_s = 0;
float Batt_v              = -99;
const float BattSF        = 4.05/3.3;

bool PublishState()
{
    bool Flag = false;

    Serial.printf("Pub? %f %f\n", LastDHTTemp_F, OldTempMeas_f);
    if (OutsideRange(LastDHTTemp_F, OldTempMeas_f, 2))
    {
        Flag = true;
        Serial.printf("PUBLISH - Temp outside range %f %f\n", LastDHTTemp_F, OldTempMeas_f);
    }
    Serial.printf("Pub? %d %d\n", MedianLastDistance_in, OldDistance_in);
    if ((MedianLastDistance_in > 0) && OutsideRange(MedianLastDistance_in, OldDistance_in, 5))
    {
        Flag = true;
        Serial.printf("PUBLISH - Distance outside range %d %d\n", MedianLastDistance_in, OldDistance_in);
    }
    Serial.printf("Pub? %d %d\n", CurrentTime_s(0), OldPublishedTS_s);
    if ((CurrentTime_s(0) > OldPublishedTS_s) && (CurrentTime_s(0) - OldPublishedTS_s > 3600))
    {
        Flag = true;
        Serial.printf("PUBLISH - TS outside range %u %u\n", CurrentTime_s(0), OldPublishedTS_s);
    }

    return Flag;
}

// Function_* below are called by the setup and main loops.

void Function_Subscriptions()
{
//    Unsubscribe(e_DeviceInfo);
}

void Function_Setup()
{
    //store the previous readings and only send on an update?
    
    DeviceConfiguration.ClientName            = "FS_River";
    DeviceConfiguration.ProcessLoopInterval_s = 3;
    DeviceConfiguration.Topics.Distance       = e_CabinRiverDistanceMeas;
    DeviceConfiguration.Topics.DHTTemp        = e_CabinRiverTempMeas;
    DeviceConfiguration.Topics.DHTHumidity    = e_CabinRiverHumidityMeas;
    DeviceConfiguration.Topics.ADC            = e_RiverBattery;
}

void SendFunctionInfo()
{
    String Msg = "{\"FunctionTable\":\"";
    Msg += "Last river temp (F),"      + String(LastDHTTemp_F)         + ",";
    Msg += "Last river humidity (%),"  + String(LastDHTHumidity)       + ",";
    Msg += "Last river battery (V),"   + String(Batt_v)                + ",";
    Msg += "Median distance (in),"     + String(MedianLastDistance_in) + "\"}";
    WebSocket.broadcastTXT(Msg);
    Serial.println(Msg);
}

// This will be run every 5 minutes.
void Function_5MinProcessLoop()
{
}

// This will be run only once.
void Function_RunOnceProcessLoop()
{
    EEPROM_BumpIterations();
    // Get the old measurements from the EEPROM.
    OldTempMeas_f    = EEPROMRead1(e_Addr_Temp);
    OldDistance_in   = EEPROMRead1(e_Addr_Distance);
    OldPublishedTS_s = EEPROMRead4(e_Addr_TS);
}

void Function_ProcessLoop()
{
//    Serial.printf("NTP Time %d %d\n", CurrentTime_s(0), NTP_GetEpochTime());
    Batt_v = BattSF * LastAdc_V * 10; // Get an extra digit into it.
//    if (CurrentTime_s(0) > 10000)
    {
        bool Success = true;// Assume the publishing succeeds OR we don't have to publish.
        if (PublishState())
        {
            Network_Setup();// ONLY RIVER is set up to connect if necessary.
            MQTT_Loop();
            while (CurrentTime_s(0) < 10000)// Wait until 

            Success |= Publish(DeviceConfiguration.Topics.DHTTemp,     LastDHTTemp_F);
            Success |= Publish(DeviceConfiguration.Topics.DHTHumidity, LastDHTHumidity);
            Success |= Publish(DeviceConfiguration.Topics.ADC,         Batt_v);
            // Sometimes we don't get a median before we get here.
            if (MedianLastDistance_in > 0)
            {
                Success |= Publish(DeviceConfiguration.Topics.Distance, MedianLastDistance_in);
            }
            else
            {
                Success = false;
            }
    
            // Store the published values.
            if (Success)
            {
                EEPROMWrite1(e_Addr_Temp,     LastDHTTemp_F);
                EEPROMWrite1(e_Addr_Distance, MedianLastDistance_in);
                EEPROMWrite4(e_Addr_TS,       CurrentTime_s(0));
                // Allow the publishing to complete (not sure why this is needed... but without it they don't get out before the close).
                delay(3000);
            }
        }
        else
        {
            Serial.println("NOT PUBLISHING");
        }
        
        SendFunctionInfo();

        if (Success)
        {
            Network_Shutdown();
            Sleep_s(SleepTime_s);
        }
    }
    Serial.printf("Time too early %d\n", CurrentTime_s(0));
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

