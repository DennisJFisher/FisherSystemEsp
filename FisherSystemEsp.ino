#include "Config.h"
#include "Ticker.h"
#include <EEPROM.h>

/******************* NEW TODO **************
 *  
 *  Get all this into GIT
 *  Change all measurements to 0.1 resolution
 *  WDT everything
 *  
 */
/********** IDEAS ****************
 *  
 *  Reboot ack should null payload?
 *  
    After x many disconnects... or MQTT bad connects? Reboot?
    After timestamp is so old... reboot?

    Clear persistant stuff in setup.

    For forced reset... drive D16 (OR BETTER D0) low???????????????

    Wifi should auto reconnect... don't manually do it. REMOVE reboot to test

    On network disconnect, stop services?

    Use ESP.rtcUserMemoryWrite instead of EEPROM?

    Use built in (NTP-DZ-DST) time functions.

    If OTA.... don't sleep or process?

    Add a field to device for publishing so it's common?

    F macro...
    Serial.print(F("Write something on the Serial Monitor that is stored in FLASH"));
*/
//#define DISABLE_PUBLISHING

//#define  TEST
//#define KITCHEN
//#define GARAGE_UP
//#define GARAGE_DOOR
//#define CABIN_BASEMENT_NORTH
//#define RIVER
//#define HOTTUB

const char FirmwareVersion[] = "0.50A";

// These 3 objects allow the device's process loop to periodically run.
Ticker TickerProcessLoop;
bool Function_ProcessLoopFlag = false;
void RunProcessLoopCB()
{
    Function_ProcessLoopFlag = true;
}

// These 3 objects allow the device's 5 minute process loop to periodically run.
Ticker Ticker5MinuteProcessLoop;
bool Function_5MinuteProcessLoopFlag = false;
void Run5MinuteProcessLoopCB()
{
    Function_5MinuteProcessLoopFlag = true;
}

// Needed for light sleep and adc
extern "C"
{
#include "user_interface.h"
}

void Sleep_s(uint32_t Seconds)
{
    // No sleep requested.
    if (0 == Seconds)
    {
        return;
    }
    
    Serial.print(F("Deepsleep for "));
    Serial.print(Seconds);
    Serial.println("s");
    
    Network_Shutdown();
    
    // Value is in microseconds.
    ESP.deepSleep(Seconds * 1000000, WAKE_RF_DEFAULT);
    delay(1000);// IF THIS IS NOT HERE, the above sleep will cause an exception!!!!!!!!
    //ESP.deepSleep(sleep_time);//12mA without UART
    //ESP.deepSleep(sleep_time,WAKE_RF_DEFAULT); //31mA without UART
    //ESP.deepSleep(sleep_time,WAKE_RFCAL);//12.4 mA without UART
    //ESP.deepSleep(sleep_time,WAKE_NO_RFCAL);//10-10.3mA working wifi after reset without UART
    //ESP.deepSleep(sleep_time,WAKE_RF_DISABLED);
}

// Track the time over longer than the ~72 minutes the SDK offers.
// Each count here is worth 0x100000000 uS = ~4295S = 71.58 minutes
unsigned int RollOvers = 0;
unsigned int LastSystemTime = 0;
void SystemTimeTrack()
{
    unsigned int CurrentSystemTime = system_get_time();
    // If it rolled over, bump the counter.
    if (CurrentSystemTime < LastSystemTime)
    {
        ++RollOvers;
    }
    LastSystemTime = CurrentSystemTime;
}
// Note this can be off by 72 minutes, every 72 minutes for whatever the loop time is.
unsigned int GetUpTimeMinutes()
{
    //  uS / 60,000,000 us/min + roll * 71.58 min/roll
    return system_get_time() / 60000000.0 + RollOvers * 71.58;
}

void setup()
{
    Serial.begin(115200);
    //  Serial.setDebugOutput(true);// Allow wifi debug output

    system_update_cpu_freq(160);

    //wifi_set_sleep_type(NONE_SLEEP_T);// TODO might not be needed?
    WiFi.setSleepMode(WIFI_NONE_SLEEP);// TODO might not be needed?

    EEPROM_Setup();

    Peripherals_Setup();

    Function_Setup();

#ifndef RIVER
    Network_Setup();
#endif

    // Start up the processing loops for the device.
    TickerProcessLoop.attach(DeviceConfiguration.ProcessLoopInterval_s, RunProcessLoopCB);
    Ticker5MinuteProcessLoop.attach(300, Run5MinuteProcessLoopCB);
}

// Allows the device to initialize things based on the first sensor readings.
bool RunOnceProcessLoop = true;
void loop()
{
    SystemTimeTrack();

#ifndef RIVER
    Network_Loop();
#endif

    // See if the ticker timed out to run the device's process loop.
    if (Function_ProcessLoopFlag)
    {
        Function_ProcessLoopFlag = false;

        // Get all the sensor readings.
        Peripherals_Loop();
        
        //    Serial.println("  Executing Function_ProcessLoop");
        if (RunOnceProcessLoop)
        {
            Function_RunOnceProcessLoop();
            RunOnceProcessLoop = false;
        }
        Function_ProcessLoop();

        Serial.printf("Free heap: %d\n", ESP.getFreeHeap() );

        if (ESP.getFreeHeap() < 15000)
        {
            Serial.println("DID THIS GET A POWER RESET after programming by serial?");
            ESP.restart();// Needs a POWER reset to work...
        }
    }

    // See if the ticker timed out to run the device's 5 minute loop.
    if (Function_5MinuteProcessLoopFlag)
    {
        Serial.println("******* 5 minute *******");
        Function_5MinuteProcessLoopFlag = false;
        Function_5MinProcessLoop();
        Network_5MinLoop();
        SendDynamics();
    }
}
