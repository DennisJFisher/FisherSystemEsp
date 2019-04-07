#ifdef GARAGE_DOOR

enum DoorState_t
{
    DoorStateUndefined = 0,
    DoorStateOpened    = 1,
    DoorStateClosed    = 2,
};

String PreviousDoorStateString[] = {"Undefined", "Opened", "Closed"};

// Code for this specific function here, called by one of the functions interfaces below.
int PublishedDistance_in       = -99;
int UnpublishedCount          = 0;
const int UnpublishedCountMax = 50;

DoorState_t CabinGarageDoorRequest = DoorStateUndefined;
DoorState_t PreviousDoorState      = DoorStateUndefined;
const int OpenDoorMax_in           = 36;
const int ClosedDoorMin_in         = 72;

void ProcessDistance()
{
    // Publish if there's a large delta, OR if there's a delta and it's been a while since publishing.
    bool PublishFlag = OutsideRange(MedianLastDistance_in, PublishedDistance_in, 48);

    // Bump the count since published if there's a delta. Otherwise clear the count.
    if (!PublishFlag)
    {
        ++UnpublishedCount;
    }
    else
    {
        UnpublishedCount = 0;
    }

    // If we've had a different count from the last published for some time.
    if (UnpublishedCount >= UnpublishedCountMax)
    {
        PublishFlag = true;
    }

    if (PublishFlag)
    {
        PublishedDistance_in = MedianLastDistance_in;
        Publish(DeviceConfiguration.Topics.Distance, PublishedDistance_in);
        UnpublishedCount = 0;
    }
}

void ProcessState()
{
    DoorState_t NewState = DoorStateUndefined;

    // Use last published distance to get some filtering.
    if (PublishedDistance_in < OpenDoorMax_in)
    {
        NewState = DoorStateOpened;
    }
    if (PublishedDistance_in > ClosedDoorMin_in)
    {
        NewState = DoorStateClosed;
    }

    if (NewState != PreviousDoorState)
    {
        Publish(e_CabinGarageDoorState, NewState);
        PreviousDoorState = NewState;
    }
}

// Function_* below are called by the setup and main loops.
void Function_Subscriptions()
{
    //  Subscribe(e_CabinGarageDoorCommand);
}

void Function_Setup()
{
    DeviceConfiguration.ClientName            = "FS_GarageDoor";
    DeviceConfiguration.ProcessLoopInterval_s = 10;
    DeviceConfiguration.Topics.Distance       = e_CabinGarageDoorDistance_in;
}

void SendFunctionInfo()
{
    String Msg = "{\"FunctionTable\":\"";
    Msg += "Median Distance (in),"    + String(MedianLastDistance_in)              + ",";
    Msg += "Last published distance," + String(PublishedDistance_in)                + ",";
    Msg += "Unpublished count,"       + String(UnpublishedCount)                   + ",";
    Msg += "Door state,"              + PreviousDoorStateString[PreviousDoorState] + "\"}";
    WebSocket.broadcastTXT(Msg);
    Serial.println(Msg);
}

// This will be run every 5 minutes.
void Function_5MinProcessLoop()
{
    Publish(DeviceConfiguration.Topics.Distance, PublishedDistance_in);
    Publish(e_CabinGarageDoorState, PreviousDoorState);
    
    // TODO fix this...
    SendFunctionInfo();
}

// This will be run only once.
void Function_RunOnceProcessLoop()
{
}

void Function_ProcessLoop()
{
    // See if we should publish.
    ProcessDistance();

    // Determine the current door state.
    ProcessState();

    // If the door state is not undefined, and the last request is not undefined, ensure the last request matches the door state.
    if ((DoorStateUndefined != PreviousDoorState) && (DoorStateUndefined != CabinGarageDoorRequest))
    {
        // If the door is in an incorrect state...
        if (PreviousDoorState != CabinGarageDoorRequest)
        {
            // Toggle the door switch.
            if (CabinGarageDoorRequest != PreviousDoorState)
            {
                // Toggle the relay
                SetRelay1(true);
                delay(250);
                SetRelay1(false);
                // Clear out the distance filter
                ResetDistanceFilter();
                // Yield for 20 seconds to give the door time to change
                unsigned long StartTime_ms = millis();
                while (StartTime_ms + 20000 < millis())
                {
                    yield();
                }
            }
        }

        // Set this to undefined now that we've determined it's correct... or toggled it to what should be correct.
        CabinGarageDoorRequest = DoorStateUndefined;
    }

    SendFunctionInfo();
}

// Called whenever an MQTT topic is received.
void Function_ReceivedTopic(Topic_t Subtopic, String Value)
{
    switch (Subtopic)
    {
        case e_CabinGarageDoorCommand:
            CabinGarageDoorRequest = (DoorState_t)Value.toInt();
            break;
        default:
            break;
    }

    Function_ProcessLoop();
}

#endif
