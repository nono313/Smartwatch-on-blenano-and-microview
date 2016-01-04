#include "mbed.h"
#include "BLE.h"
#include <string>

/** Global variables **/
BLEDevice  ble;     // BLE peripheral
DigitalOut led1(LED1);  // Debug LED

/* I2C configuration */
I2C i2c_port(P0_10,P0_8);   // I2C Port, connected as master

const uint8_t addr = 0x44;  // Address of the slave we want to communicate with (in this case, the MicroView)

/* Headers used for transfering messages to the MicroView */
const char headerSms = 'S';
const char headerTime = 'T';

/*** Bluetooth Low Energy configuration ***/

/* UUIDs used for GATT services and characteristics (BLE) */
uint16_t customServiceUUID = 0xA000;
uint16_t readCharUUID       = 0xA001;
uint16_t writeCharUUID      = 0xA002;
uint16_t timeSyncUUID       = 0xA003;
static const uint16_t uuid16_list[] = {0xFFFF};

/* GATT Characteristics */
/* readValue allows a device to read the last message written to the writeValue characteristic */
static uint8_t readValue[100] = {0};
ReadOnlyArrayGattCharacteristic<uint8_t, sizeof(readValue)> readChar(readCharUUID, readValue); 
/* writeValue is used to transmit a message to print on the MicroView's screen */
static uint8_t writeValue[100] = {0};
WriteOnlyArrayGattCharacteristic<uint8_t, sizeof(writeValue)> writeChar(writeCharUUID, writeValue);
/* timeSyncValue is a characteristic used for time synchronisation */
static uint8_t timeSyncValue[50] = {0};
WriteOnlyArrayGattCharacteristic<uint8_t, sizeof(timeSyncValue)> timeSyncChar(timeSyncUUID, timeSyncValue);

/* Set up custom service */
GattCharacteristic *characteristics[] = {&readChar, &writeChar, &timeSyncChar};
GattService customService(customServiceUUID, characteristics, sizeof(characteristics)/sizeof(GattCharacteristic *));


//#define DEBUG(STR) { pc.printf(STR); }
#define DEBUG(STR)

/**
 * Disconnection callback
 * This function is called when the devices is disconnected from a GAP central device (e.g : smartphone)
 */
void disconnectionCallback(const Gap::DisconnectionCallbackParams_t *params)
{
    DEBUG("Disconnected!\n\r");
    DEBUG("Restarting the advertising process\n\r");
    ble.startAdvertising(); // Start advertising its presence
}

/**
 * Write callback
 * Called when a Gatt characteristic is written to by a central device
 */
void writeCharCallback(const GattWriteCallbackParams *params)
{
    /* Prepare to add header to transmit message to the MicroView */
    char *total;
    total = (char*)malloc(sizeof(char)*(params->len+1));
    strcat(total+1, (char*)params->data);
    total[params->len+1]= '\0';
    
    // Check to see what characteristic was written, by handle    
    /* If it's a standard message (comming from a notification) */
    if(params->handle == writeChar.getValueHandle()) {
        /* If only one byte is sent, we change the LED's state with its value*/
        if(params->len == 1)
            led1 = params->data[0];
        else {
            //pc.printf("\n\r Data received: length = %d, data = 0x",params->len);
            /* If the data received is equal to "ON" or "OFF", we modify the LED's state accordingly */
            if(strncmp((char*)params->data, "ON",2) == 0) {
                //pc.printf("LED ON");
                led1=1;
            } else if(strncmp((char*)params->data, "OFF",3) == 0) {
                //pc.printf("LED OFF");
                led1=0;
            }
            else {
                /* For each message, we toggle the LED's state */
                led1=!led1;
            }
        }
                
         /* Add the header specific to standard messages (not time sync) */
        total[0] = headerSms;
        /* Update the readChar characteristic with the value of writeChar */
        //ble.updateCharacteristicValue(readChar.getValueHandle(), params->data, params->len);
        ble.updateCharacteristicValue(readChar.getValueHandle(), (const uint8_t *)total, strlen(total)+1);
        /* Send the whole string, including the header, to the Microview */
        i2c_port.write(addr, total, strlen(total)+1);
    }
    /* If the message has been sent to the timeSync Gatt characteristic */
    else if(params->handle == timeSyncChar.getValueHandle()) {
        /* Set the corresponding header byte */
        total[0] = headerTime;
        /* Send the whole string, including the header, to the MicroView */
        i2c_port.write(addr, total, strlen(total)+1);
        led1 = !led1;
    }
    free(total);
}

int main(void)
{
    led1 = 1;

    DEBUG("Initialising the nRF51822\n\r");
    ble.init(); // Initialise the BLE radio

    /* Register callbacks */
    ble.onDisconnection(disconnectionCallback);
    ble.onDataWritten(writeCharCallback);

    //uart = new UARTService(ble);

    /* Setup Advertising */
    ble.accumulateAdvertisingPayload(GapAdvertisingData::BREDR_NOT_SUPPORTED | GapAdvertisingData::LE_GENERAL_DISCOVERABLE);
    ble.setAdvertisingType(GapAdvertisingParams::ADV_CONNECTABLE_UNDIRECTED);
    ble.accumulateAdvertisingPayload(GapAdvertisingData::SHORTENED_LOCAL_NAME,
                                     (const uint8_t *)"BLE NOTIF", sizeof("BLE NOTIF") - 1);
    ble.accumulateAdvertisingPayload(GapAdvertisingData::COMPLETE_LIST_16BIT_SERVICE_IDS, (uint8_t *)uuid16_list, sizeof(uuid16_list));
    ble.setAdvertisingInterval(1600); // 1000ms; in multiples of 0.625ms.

    ble.addService(customService);

    ble.startAdvertising();

    while (true) {
        ble.waitForEvent();
    }
}
