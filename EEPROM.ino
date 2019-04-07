const long EEPROM_Key = 0x55AA;
typedef enum
{
    e_Addr_Key           = 0,  // 2 bytes
    e_Addr_Iteration     = 2,  // 2 bytes
    e_Addr_Temp          = 4,  // 1 byte
    e_Addr_Distance      = 5,  // 1 byte
    e_Addr_TS            = 6,  // 4 bytes
    e_Addr_Accumulated_s = 10, // 4 bytes
    e_Addr_End           = 14,
} EEPROMAddress;
#define EEPROMWrite1(addr, val) EEPROMWrite(addr, val, 1)
#define EEPROMWrite2(addr, val) EEPROMWrite(addr, val, 2)
#define EEPROMWrite4(addr, val) EEPROMWrite(addr, val, 4)
void EEPROMWrite(uint8_t Addr, uint32_t Value, uint8_t Size)
{
    Serial.printf("EEPROM write %d bytes at %d = %X\n", Size, Addr, Value);
    switch(Size)
    {
        case 4:
            EEPROM.write(Addr++, Value >> 24);
            EEPROM.write(Addr++, Value >> 16);
        case 2:
            EEPROM.write(Addr++, Value >> 8);
        case 1:
            EEPROM.write(Addr++, Value >> 0);
            break;
    }
    
    EEPROM.commit();
}
#define EEPROMRead1(addr) EEPROMRead(addr, 1)
#define EEPROMRead2(addr) EEPROMRead(addr, 2)
#define EEPROMRead4(addr) EEPROMRead(addr, 4)
uint32_t EEPROMRead(uint16_t Addr, uint8_t Size)
{
    uint32_t Val = 0;
    switch(Size)
    {
        // Intentionally fall through for each.
        case 4:
            Val += EEPROM.read(Addr++) << 24;
            Val += EEPROM.read(Addr++) << 16;
        case 2:
            Val += EEPROM.read(Addr++) << 8;
        case 1:
            Val += EEPROM.read(Addr) << 0;
            break;
    }

    Serial.printf("EEPROM read %d bytes at %d = %08X\n", Size, Addr-Size, Val);

    return Val;
}

long StartTime_s = 0;
void EEPROM_Setup()
{
    EEPROM.begin(256);
    // See if we need to initialize...
    if (EEPROM_Key != EEPROMRead2(e_Addr_Key))
    {
        EEPROMWrite2(e_Addr_Key, EEPROM_Key);
        for (int i = 2; i < e_Addr_End; ++i)
        {
            EEPROMWrite1(i, 00);
        }
    }
}

int EEPROM_BumpIterations()
{
    int Val = EEPROMRead2(e_Addr_Iteration) + 1;
    EEPROMWrite2(e_Addr_Iteration, Val);

    Serial.printf("Iteration count = %d\n", Val);
    return Val;
}

void EEPROM_AccumulateRunTime()
{
    long EndTime_s = millis() / 1000.0 + 0.5;
    long AccumulatedTime_s = EndTime_s - StartTime_s;
    StartTime_s = EndTime_s;
    long PrevTime_s = EEPROMRead4(e_Addr_Accumulated_s);
    EEPROMWrite4(e_Addr_Accumulated_s, PrevTime_s + AccumulatedTime_s);        
}
// ACCOUNT FOR RTC + sleep?
