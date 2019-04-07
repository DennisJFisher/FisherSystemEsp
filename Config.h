#ifndef CONFIG_H
#define CONFIG_H

// For some f'ed reason this needs to be included NOT on the Network page.
#include "WebSocketsServer.h"

enum Topic_t
{
    e_None,
    e_Birth,
    e_DeviceInfo,
    e_DebugInfo,
    e_CabinBasementPropTempSet,
    e_CabinBasementPropHeatOn,
    e_CabinBasementTempMeas,
    e_CabinBasementTempSet,
    e_CabinKitchenHeatOn,
    e_CabinKitchenTempMeas,
    e_CabinKitchenTempSet,
    e_CabinKitchenHumidityMeas,
    e_CabinGarageDoorCommand,
    e_CabinGarageDoorState,
    e_CabinGarageDoorDistance_in,
    e_CabinGarageDownTempMeas,
    e_CabinGaragePowerEnabled,
    e_CabinGarageUpTempMeas,
    e_CabinGarageUpTempSet,
    e_CabinGarageUpHeatOn,
    e_CabinPowerEnabled,
    e_CabinRiverTempMeas,
    e_CabinRiverHumidityMeas,
    e_CabinRiverDistanceMeas,
    e_HottubTempMeas,
    e_PressureMeas,
    e_CurrentTimestamp,
    e_Reboot,
    e_CycledRouter,
    e_Sprinkler,
    e_RiverBattery,
    e_TestDHTTempMeas,
    e_TestDHTHumidityMeas,
    e_TestDS1TempMeas,
    e_TestADCMeas,
    e_TestDistanceMeas,
    e_MaxSubtopics,
};

const String SubtopicString[e_MaxSubtopics] =
{
    "None",
    "Birth",
    "DeviceInfo",
    "DebugInfo",
    "Cabin/Basement/Prop/Temp/Set",
    "Cabin/Basement/Prop/Heat/On",
    "Cabin/Basement/Temp/Meas",
    "Cabin/Basement/Temp/Set",
    "Cabin/Kitchen/Heat/On",
    "Cabin/Kitchen/Temp/Meas",
    "Cabin/Kitchen/Temp/Set",
    "Cabin/Kitchen/Humidity/Meas",
    "Cabin/Garage/Door/Command",
    "Cabin/Garage/Door/State",
    "Cabin/Garage/Door/Distance",
    "Cabin/Garage/Down/Temp/Meas",
    "Cabin/Garage/Power/Enabled",
    "Cabin/Garage/Up/Temp/Meas",
    "Cabin/Garage/Up/Temp/Set",
    "Cabin/Garage/Up/Heat/On",
    "Cabin/Power/Enabled",
    "Cabin/River/Temp/Meas",
    "Cabin/River/Humidity/Meas",
    "Cabin/River/Distance/Meas",
    "House/Hottub/Temp/Meas",
    "Pressure/Meas",
    "CurrentTimestamp",
    "Reboot",
    "Cabin/CycledRouter",
    "Sprinkler",
    "Cabin/River/Battery",
    "Test/DHT/Temp/Meas",
    "Test/DHT/Humidity/Meas",
    "Test/DS1/Temp/Meas",
    "Test/ADC/Meas",
    "Test/Distance/Meas",
};

struct TopicsStruct_t
{
    Topic_t DHTTemp;
    Topic_t DHTHumidity;
    Topic_t DS1;
    Topic_t ADC;
    Topic_t Distance;
};

struct DeviceConfiguration_t
{
    String ClientName;
    TopicsStruct_t Topics;

    // How often to process information.
    unsigned int ProcessLoopInterval_s;

    DeviceConfiguration_t()
    {
        this->ClientName            = "FS_Unknown";
        this->ProcessLoopInterval_s = 60;
        this->Topics.DHTTemp        = e_None;
        this->Topics.DHTHumidity    = e_None;
        this->Topics.DS1            = e_None;
        this->Topics.ADC            = e_None;
        this->Topics.Distance       = e_None;
    }
} DeviceConfiguration;

#endif // CONFIG_H
