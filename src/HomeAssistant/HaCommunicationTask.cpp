#include  "HaCommunicationTask.h"

#define MODEM_RX 13  // ESP32 RX -> TX modemu
#define MODEM_TX 14  // ESP32 TX -> RX modemu
#define HARDWARE_MODEM_NUMBER 2  


HaCommunicationTask::HaCommunicationTask() : ActiveTask("HaCommunicationTask", 8192, 3, 1),
    haQueue(10), 
    // construct timer to send SystemMessagePacket using TimerToSystemMessage converter
    timer(1, 1000, SystemTimerT<SystemMessagePacket, TimerToSystemMessage>::Mode::OneShot, ActiveQueueRef<SystemMessagePacket>(haQueue.nativeHandle()), TimerToSystemMessage()),
    hardwareModem (HARDWARE_MODEM_NUMBER), 
    tinyGsmModem(hardwareModem), 
    tinyGsmClient(tinyGsmModem),
    mqttClient(tinyGsmClient)
{
}

void HaCommunicationTask::setup() {
   
}

void HaCommunicationTask::loop()
{
    SystemMessagePacket msg;
    if (haQueue.receive(msg))
    {
        switch (msg.type)
        {
        case SystemDataType::Timer:
            // Restart the timer for another second 
            /* code */
            break;
        case SystemDataType::Measurements:
            // Handle measurement data
            /* code */
            break;
        
        default:
            break;
        }/* code */
    }
    
}

// bool HaCommunicationTask::initModem() {

//     hardwareModem.begin(57600, SERIAL_8N1, MODEM_RX, MODEM_TX);
//     DBG("Initializing modem...");
//     if (!tinyGsmModem.restart()) {
//         DBG("Modem not responding");
//         return false;
//     }
//     DBG("Modem is responding");
//     delay(1500);

//     if (!tinyGsmModem.waitForNetwork()) {       //this is blocking call and will wait until network is connected
//         DBG("Network not connected");
//         return false;
//     }
//     Serial.println("Network failed");
//     while (true) delay(1000);
//   }

//     // Additional modem initialization code can go here

//     return true;
// }




