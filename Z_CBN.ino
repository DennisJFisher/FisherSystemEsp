#ifdef CABIN_BASEMENT_NORTH

// Code for this specific function here, called by one of the functions interfaces below.
bool PreviousHeatOn              = false;
float CabinBasementTempMeas_F    = 47;// Fairly safe start values.
float CabinBasementPropTempSet_F = 47;// Keeps the basement from drifting down...

void UpdateHeat(bool HeatOn)
{
    // Set the heat relay to the requested state.
    PreviousHeatOn = HeatOn;
    Serial.printf("Heat published to %s\n", PreviousHeatOn ? "On" : "Off");
    SetRelay2(PreviousHeatOn);
    SetIndicatorGrn(PreviousHeatOn);   // Light it up if heat is requested.
    Publish(e_CabinBasementPropHeatOn, HeatOn);
}

// Function_* below are called by the setup and main loops.
void Function_Subscriptions()
{
    Subscribe(e_CabinBasementTempMeas);
    Subscribe(e_CabinBasementPropTempSet);
}

void Function_Setup()
{
    DeviceConfiguration.ClientName            = "FS_CabinBasementNorth";
    DeviceConfiguration.ProcessLoopInterval_s = 10;
}

// Needs to return data in rows... i.e.
// <li>Last RSSI: " + String(LastRSSI_dbm) + " dbm" + "</li>
String Function_Info()
{
    String PageData;
    PageData += String("<table  class=\"snazzy\"><tr><th><u>Sensor</u></th><th><u>Value</u></th></tr>");
    PageData += String("<tr><td>Basement temp propane set (F)</td><td>") + CabinBasementPropTempSet_F               + " F</td></tr>";
    PageData += String("<tr><td>Basement temp meas (F)</td><td>")        + CabinBasementTempMeas_F                  + " F</td></tr>";
    PageData += String("<tr><td>Propane heat </td><td>")                 + String(PreviousHeatOn ? "ON" : "OFF")    + "</td></tr>";
    PageData += String("<tr><td>Indication green </td><td>")             + String(LastIndicationGrn ? "ON" : "OFF") + "</td></tr>";
    PageData += String("</table>");

    return PageData;
}

void SendFunctionInfo()
{
    String Msg = "{\"FunctionTable\":\"";
    Msg += "Basement temp propane set (F)," + String(CabinBasementPropTempSet_F) + ",";
    Msg += "Basement temp meas (F),"        + String(CabinBasementTempMeas_F) + ",";
    Msg += "Propane heat,"                  + String(PreviousHeatOn ? "ON" : "OFF") + ",";
    Msg += "Green LED,"                     + String(LastIndicationGrn ? "ON" : "OFF") + "\"}";
    WebSocket.broadcastTXT(Msg);
    Serial.println(Msg);
}

// This will be run only once.
void Function_RunOnceProcessLoop()
{
    // Force to off.
    Serial.printf("Heat initialized to Off\n");
    UpdateHeat(false);
}

// This will be run every 5 minutes.
void Function_5MinProcessLoop()
{
    // Force a publish.
    UpdateHeat(PreviousHeatOn);
}

// This loop executes after all measurements have been taken.
void Function_ProcessLoop()
{
    // See if the heat should be on.
    bool HeatOn = CabinBasementPropTempSet_F > CabinBasementTempMeas_F;

    // Look for a change.
    if (HeatOn != PreviousHeatOn)
    {
        Serial.printf("Heat switched to %s\n", HeatOn ? "On" : "Off");
        UpdateHeat(HeatOn);
    }

    SendFunctionInfo();
}

// Called whenever an MQTT topic is received.
void Function_ReceivedTopic(Topic_t Subtopic, String Value)
{
    switch (Subtopic)
    {
        case e_CabinBasementTempMeas:
            CabinBasementTempMeas_F = Value.toFloat();
            break;
        case e_CabinBasementPropTempSet:
            CabinBasementPropTempSet_F = Value.toFloat();
            break;
        default:
            break;
    }}

#endif
