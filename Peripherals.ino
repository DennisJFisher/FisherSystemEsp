
#include <OneWire.h>
#include <DallasTemperature.h>
#include <DHT.h>
#include <DHT_U.h>

extern String DebugString;

// from other:
// SCL, SDA = BMP
//
//
// D0 - servo
// D1 - SCL, Echo, PowerLED
// D2 - SDA, DS1, Trig
// D3 -
// D4 - DS1, DHT
// D5 - GUpHeat, GDRly, PRly, PropHeat, BHeat
// D6 - KHeat
// D7 - DHT
// D8 - Servo

// This:
// D0 -
// D1 - Echo
// D2 - Trig
// D3 - Relay1
// D4 - DS1, DHT, also... I believe this should be pulled high during a reboot?
// D5 - Relay2
// D6 - IndicationRed
// D7 - IndicationGrn

// Sensor pin definitions.
// Used for sleep     D0
#if (defined(GARAGE_UP) || defined(HOTTUB))  // FIX THIS!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
const int Echo_Pin   = D1;
const int Trig_Pin   = D4;
const int Relay1_Pin = D3;
const int DS1_Pin    = D2;// Shared with DHT
const int DHT_Pin    = D2;// Shared with DS1
const int Relay2_Pin = D5;
const int Red_Pin    = D6;
const int Grn_Pin    = D7;
#else
const int Echo_Pin   = D1;
const int Trig_Pin   = D2;
const int Relay1_Pin = D3;
const int DS1_Pin    = D4;// Shared with DHT
const int DHT_Pin    = D4;// Shared with DS1
const int Relay2_Pin = D5;
const int Red_Pin    = D6;
const int Grn_Pin    = D7;
#endif
// Measurement values.
float LastDHTTemp_C           = -99;
float LastDHTTemp_F           = -99;
float LastDHTHumidity         = -99;
float LastDS1Temp_C           = -99;
float LastDS1Temp_F           = -99;
float LastAdc_V               = -99;
int   MedianLastDistance_in   = -99;
bool  LastIndicationRed       = false;
bool  LastIndicationGrn       = false;
bool  LastRelay1              = false;
bool  LastRelay2              = false;

// Global sensor objects
OneWire DS1OneWire(DS1_Pin);
DallasTemperature DS1(&DS1OneWire);
DHT_Unified dht(DHT_Pin, DHT22, 1 /* count*/, 1/* temp sensor ID */, 1/* Humidity sensory ID */);

bool OutsideRange(float Actual, float Expected, float Tolerance)
{
    bool Status = false;

    if (Actual > Expected + Tolerance)
    {
        Status = true;
    }
    else if (Actual < Expected - Tolerance)
    {
        Status = true;
    }

    return Status;
}

void SetRelay1(bool State)
{
    if (State != LastRelay1)
    {
        Serial.printf("Toggling relay 1 %s\n", State ? "On" : "Off");
        LastRelay1 = State;
    }

    // ON (true) means applying a low to this pin.
    digitalWrite(Relay1_Pin, !State);
}
void SetRelay2(bool State)
{
    if (State != LastRelay2)
    {
        Serial.printf("Toggling relay 2 %s\n", State ? "On" : "Off");
        LastRelay2 = State;
    }

    // ON (true) means applying a low to this pin.
    digitalWrite(Relay2_Pin, !State);
}
void SetIndicatorRed(bool State)
{
    Serial.printf("Setting indicator red %s\n", State ? "On" : "Off");
    LastIndicationRed = State;
    // ON (true) means appling a low (sink) to this pin.
    digitalWrite(Red_Pin, !State);
}
void SetIndicatorGrn(bool State)
{
    Serial.printf("Setting indicator Green %s\n", State ? "On" : "Off");
    LastIndicationGrn = State;
    // ON (true) means appling a low (sink) to this pin.
    digitalWrite(Grn_Pin, !State);
}
void GetDHT()
{
    if (e_None != DeviceConfiguration.Topics.DHTTemp)
    {
        Serial.println("Getting DHT");
        sensor_t DHTTempSensor;
        sensor_t DHTHumiditySensor;
        sensors_event_t DHTTempEvent;
        sensors_event_t DHTHumidityEvent;

        dht.begin();
        // Get temp sensor details
        dht.temperature().getSensor(&DHTTempSensor);
        dht.temperature().getEvent(&DHTTempEvent);
        LastDHTTemp_C = DHTTempEvent.temperature;
        if (isnan(LastDHTTemp_C))
        {
            Serial.println("DHT returned nan!");
            LastDHTHumidity = LastDHTTemp_F = LastDHTTemp_C;
        }
        else
        {
            LastDHTTemp_F = DHTTempEvent.temperature * 9 / 5 + 32;
            // Get humidity sensor details.
            dht.humidity().getSensor(&DHTHumiditySensor);
            dht.humidity().getEvent(&DHTHumidityEvent);
            LastDHTHumidity = DHTHumidityEvent.relative_humidity;
        }
        Serial.println(String("DHT Temp = ") + LastDHTTemp_F + 'F');
        Serial.println(String("DHT Humidity = ") + LastDHTHumidity + '%');
    }
}
void GetDS1()
{
    if (e_None != DeviceConfiguration.Topics.DS1)
    {
        Serial.println("Getting DS1");
        DS1.begin();
        DS1.requestTemperatures();
        LastDS1Temp_C = DS1.getTempCByIndex(0);
        LastDS1Temp_F = LastDS1Temp_C * 9.0 / 5 + 32;
        if (isnan(LastDS1Temp_C) || (LastDS1Temp_F < -100))
        {
            // One more time...
            Serial.println("DS1 value is bogus!");
        }
        else
        {
            Serial.println(String("DS1 Temp = ") + LastDS1Temp_F + 'F');
        }
    }
}
void GetADC()
{
    if (e_None != DeviceConfiguration.Topics.ADC)
    {
        Serial.println("Getting ADC");
        // Units are supposed to be 1/1024 V, except I'm seeing 1/300
        unsigned int Reading = system_adc_read();
        LastAdc_V = Reading / 300.0;
        Serial.println("ADC = " + String(Reading) + ", Volt = " + LastAdc_V);
    }
}
long microsecondsToInches(float Temp_C, long Time_us)
{
    if ((Temp_C < -40) || (Temp_C > 40) || (isnan(Temp_C)))
    {
        Serial.println("TEMP IS NOT SET FOR DISTANCE!!!!!!!!!!!");
        Temp_C = 24; // Assume room temp
    }
    //return Time_us * (331.3+0.606*Temp_C) / 50800;

    // speed of sound is effected by temp and humidity. Assume 50% humidity.
    // us for half the round trip * s/us * m/s * in/m
    //return (Time_us / 2) * 1/1000000 * (331.4 + 0.606*Temp_C + (0.0124 * 50)) * 39.3701;
    double x = (Time_us * (331.4 + 0.606 * Temp_C + 0.62) * 0.000019685050); // Last val is 39.3701/1000000/2
    return x / 2;// HACK for some reason the measurement seems 2x.
}
const int MaxPulseEventSize = 20;
long PulseEvent[MaxPulseEventSize] = {0};
int PulseEvents = 0;
void ResetDistanceFilter()
{
    PulseEvents = 0;
}
long GetDistance(float Temp_C)
{
    MedianLastDistance_in = 0;
    if (e_None != DeviceConfiguration.Topics.Distance)
    {
        // Fill the array with raw pulse measurements.
        for (int i = 0; i < MaxPulseEventSize; ++i)
        {
            // The PING is triggered by a HIGH pulse of 2 or more microseconds.
            // Give a short LOW pulse beforehand to ensure a clean HIGH pulse:
            digitalWrite(Trig_Pin, LOW);
            delayMicroseconds(2);
            digitalWrite(Trig_Pin, HIGH);
            delayMicroseconds(5);
            digitalWrite(Trig_Pin, LOW);
    
            // a HIGH pulse whose duration is the time (in microseconds) from the sending
            // of the ping to the reception of its echo off of an object.
            long PulseWidth_us = pulseIn(Echo_Pin, HIGH);

            PulseEvent[PulseEvents] = PulseWidth_us;
            ++PulseEvents;
        }

        // bubble Sort the events in place.
        int numLength = PulseEvents; 
        // TODO this could be shortened by 1 for each iteration.
        for(int i = 1; i <= numLength; ++i)
        {
            bool Flag = 0;
            for (int j=0; j < (numLength -1); j++)
            {
                if (PulseEvent[j+1] > PulseEvent[j])      // ascending order simply changes to <
                { 
                    int Temp = PulseEvent[j];             // swap elements
                    PulseEvent[j] = PulseEvent[j+1];
                    PulseEvent[j+1] = Temp;
                    Flag = 1;               // indicates that a swap occurred.
                }
            }

            if (!Flag)
            {
                break;
            }
        }

        MedianLastDistance_in = microsecondsToInches(Temp_C, PulseEvent[MaxPulseEventSize/2]);
    }

    return MedianLastDistance_in;
}

void Peripherals_Setup()
{
    // Distance pins.
    pinMode(Echo_Pin, INPUT);
    digitalWrite(Echo_Pin, LOW);// High active
    pinMode(Trig_Pin, OUTPUT);
    digitalWrite(Trig_Pin, LOW);// High active

    // OFF (false) means applying a high to the relay pins.
    pinMode(Relay1_Pin, OUTPUT);
    SetRelay1(false);
    pinMode(Relay2_Pin, OUTPUT);
    SetRelay2(false);

    // Start with LEDs off.
    // OFF (false) means applying a high to LEDs
    pinMode(Red_Pin, OUTPUT);
    SetIndicatorRed(false);
    pinMode(Grn_Pin, OUTPUT);
    SetIndicatorGrn(false);
}
void Peripherals_Loop()
{
    // If we are using a DS1, wait, otherwise the DHT gets angry due to the shared pin.
    if (e_None != DeviceConfiguration.Topics.DS1)
    {
        delayMicroseconds(500 * 1000);
    }
    GetDHT();
    GetADC();
    // This temp will be used for the next distance measurement.
    GetDistance(LastDHTTemp_C);

    // If we are using a DHT, wait, otherwise the DS1 gets angry due to the shared pin.
    if (e_None != DeviceConfiguration.Topics.DHTTemp)
    {
        delayMicroseconds(500 * 1000);
    }

    GetDS1();
}
