#include "Particle.h"

//#include "BleLogging.h"
SerialLogHandler serialLogHandler(LOG_LEVEL_TRACE);

const unsigned long LOG_INTERVAL = 1000; // milliseconds
unsigned long lastLog = 0;
size_t counter = 0;

// This example does not require the cloud so you can run it in manual mode or
// normal cloud-connected mode
// SYSTEM_MODE(MANUAL);

// These UUIDs were defined by Nordic Semiconductor and are now the defacto standard for
// UART-like services over BLE. Many apps support the UUIDs now, like the Adafruit Bluefruit app.
const BleUuid serviceUuid("6E400001-B5A3-F393-E0A9-E50E24DCCA9E");
const BleUuid rxUuid("6E400002-B5A3-F393-E0A9-E50E24DCCA9E");
const BleUuid txUuid("6E400003-B5A3-F393-E0A9-E50E24DCCA9E");

const size_t UART_TX_BUF_SIZE = 20;
const size_t SCAN_RESULT_COUNT = 20;

BleScanResult scanResults[SCAN_RESULT_COUNT];

BleCharacteristic peerTxCharacteristic;
BleCharacteristic peerRxCharacteristic;
BlePeerDevice peer;


uint8_t txBuf[UART_TX_BUF_SIZE];
size_t txLen = 0;

// start of ads
BleCharacteristic txCharacteristic("tx", BleCharacteristicProperty::NOTIFY, txUuid, serviceUuid);
BleCharacteristic rxCharacteristic("rx", BleCharacteristicProperty::WRITE_WO_RSP, rxUuid, serviceUuid, onDataReceived, NULL);
// end

const unsigned long SCAN_PERIOD_MS = 2000;
unsigned long lastScan = 0;


void onDataReceived(const uint8_t* data, size_t len, const BlePeerDevice& peer, void* context) 
{
    char str[len+1];
    memcpy(str, data, len);
    str[len] = '\0';
    if ((String)str == (String)"HELLO")
    {
      Particle.publish("SPECIAL MESSAGE", "SPECIAL MESSAGE RECEIVED");
    }
    else
    {
    Particle.publish("tupperware_upload",  str);
    }
}

// setup() runs once, when the device is first turned on.
void setup() 
{
  // Put initialization like pinMode and begin functions here.

  Particle.publish("Target", "Tupperware_13,Tupperware_14,Tupperware_15");
  Serial.begin();
  BLE.on();
  peerTxCharacteristic.onDataReceived(onDataReceived, &peerTxCharacteristic);


// start of ad
  BLE.addCharacteristic(txCharacteristic);
  BLE.addCharacteristic(rxCharacteristic);

  BleAdvertisingData data;
  data.appendLocalName("New_Name");
  data.appendServiceUUID(serviceUuid);

  Particle.publish("advertise",data.deviceName());
  BLE.advertise(&data);
  
// end  
  
//   bleLogHandler.setup();
}

// loop() runs over and over again, as quickly as it can execute.
  int incomingByte = 0;
void loop() 
{

    
    // The core of your code likely lives here

    // bleLogHandler.loop();

    if (millis() - lastLog >= LOG_INTERVAL) 
    {
        lastLog = millis();

        // This is just so the demo prints a message every second so the log updates frequently
        //Log.info("counter=%u", counter++);
    }
  //Particle.publish("tupperware_upload", "Hello2");
    char string[32];
    if (BLE.connected()) {
        while (Serial.available() && txLen < UART_TX_BUF_SIZE) 
        {
            delay(3);                                                                                                                                                                                                                                                                  
            
            int availableBytes = Serial.available();
            for(int i=0; i<availableBytes; i++)
            {
             string[i] = Serial.read();
             string[i+1] = '\0'; // Append a null
            }

            txBuf[txLen++] = Serial.read();
            Serial.write(txBuf[txLen - 1]);
            Particle.publish("Serial", "Serial Data Received ");
            Particle.publish("Serial", string);
        }
        
        

        if (txLen > 0) 
        {
            // Transmit the data to the BLE peripheral
            peerRxCharacteristic.setValue(txBuf, txLen);
            txLen = 0;
        }
    }
    
    
        if (millis() - lastScan >= SCAN_PERIOD_MS) 
        {
            // Time to scan
            lastScan = millis();

            size_t count = BLE.scan(scanResults, SCAN_RESULT_COUNT);
            if (count > 0) 
            {
                for (uint8_t ii = 0; ii < count; ii++) 
                {
                    // Our serial peripheral only supports one service, so we only look for one here.
                    // In some cases, you may want to get all of the service UUIDs and scan the list
                    // looking to see if the serviceUuid is anywhere in the list.
                    Log.info("rssi=%d address=%02X:%02X:%02X:%02X:%02X:%02X \n", scanResults[ii].rssi, scanResults[ii].address[0], scanResults[ii].address[1], scanResults[ii].address[2], scanResults[ii].address[3], scanResults[ii].address[4], scanResults[ii].address[5]);
                    
                    

                    BleUuid foundServiceUuid;
                    size_t svcCount = scanResults[ii].advertisingData.serviceUUID(&foundServiceUuid, 1);
                    String peripheralName = scanResults[ii].advertisingData.deviceName();

                    if(peripheralName.length() > 0)
                    {
                        Log.info("Advertising name: %s \n", peripheralName.c_str());
                    }

                    Serial.printf("svcCount = %d \n", svcCount);
                    Serial.printf("foundServiceUuid = %X \n", foundServiceUuid);

                    //replace peripheralName != NULL with actual value
                    if(peripheralName == "Tupperware_13" || peripheralName == "Tupperware_14" || peripheralName == "Tupperware_15" || peripheralName == "FreshSurety_UART") //if (svcCount > 0 && foundServiceUuid == serviceUuid) 
                    {   
                        
                        peer = BLE.connect(scanResults[ii].address);
                        if (peer.connected()) 
                        {
                          
                            if(peripheralName == "Tupperware_13")
                            {
                              Particle.publish("connect", "Tupperware_13");
                            }
                            if(peripheralName == "Tupperware_14")
                            {
                              Particle.publish("connect", "Tupperware_14");
                            }
                            if(peripheralName == "Tupperware_15")
                            { 
                              Particle.publish("connect", "Tupperware_15");
                            }
                            if (peripheralName == "FreshSurety_UART")
                            {
                              Particle.publish("connect", "FreshSurety_UART");
                            }
                            Particle.publish("connect", peripheralName);
                            
                            peer.getCharacteristicByUUID(peerTxCharacteristic, txUuid);
                            peer.getCharacteristicByUUID(peerRxCharacteristic, rxUuid);
                        }
                        break;
                    }
                }
            }
        }

    
}