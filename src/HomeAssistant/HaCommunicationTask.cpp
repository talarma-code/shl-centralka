#include  "HaCommunicationTask.h"

#define MODEM_RX 13  // ESP32 RX -> TX modemu
#define MODEM_TX 14  // ESP32 TX -> RX modemu
#define HARDWARE_MODEM_NUMBER 2  


HaCommunicationTask::HaCommunicationTask() : ActiveTask("HaCommunicationTask", 8192, 3, 1),
    dataRecorderQueue(10), 
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




