#include "mbed.h"
#include "BLE.h"
#include <string>
//#include "wire.h"
BLEDevice  ble;
DigitalOut led1(LED1);
//Serial pc(USBTX, USBRX);
I2C i2c_port(P0_10,P0_8);
Ticker ticker;



#ifdef BLE_Nano
#define SCL         P0_8
#define SDA         P0_10
#endif

const char headerSms = 'S';
const char headerTime = 'T';

//TwoWire Wire = TwoWire(NRF_TWI0);

const uint8_t addr = 0x44;

uint16_t customServiceUUID = 0xA000;
uint16_t readCharUUID       = 0xA001;
uint16_t writeCharUUID      = 0xA002;
uint16_t timeSyncUUID       = 0xA003;

static const uint16_t uuid16_list[] = {0xFFFF};

//Set up custom characteristics
static uint8_t readValue[100] = {0};
ReadOnlyArrayGattCharacteristic<uint8_t, sizeof(readValue)> readChar(readCharUUID, readValue);

static uint8_t writeValue[100] = {0};
WriteOnlyArrayGattCharacteristic<uint8_t, sizeof(writeValue)> writeChar(writeCharUUID, writeValue);

static uint8_t timeSyncValue[50] = {0};
WriteOnlyArrayGattCharacteristic<uint8_t, sizeof(timeSyncValue)> timeSyncChar(timeSyncUUID, timeSyncValue);


//Set up custom service
GattCharacteristic *characteristics[] = {&readChar, &writeChar, &timeSyncChar};
GattService customService(customServiceUUID, characteristics, sizeof(characteristics)/sizeof(GattCharacteristic *));


//#define DEBUG(STR) { pc.printf(STR); }
#define DEBUG(STR)

void disconnectionCallback(const Gap::DisconnectionCallbackParams_t *params)
{
    DEBUG("Disconnected!\n\r");
    DEBUG("Restarting the advertising process\n\r");
    ble.startAdvertising();
}

void periodicCallback(void)
{
    //led1 = !led1;
    DEBUG("ping\r\n");
}

char a='a';

void writeCharCallback(const GattWriteCallbackParams *params)
{
    //check to see what characteristic was written, by handle
    char *total;
    total = (char*)malloc(sizeof(char)*(params->len+1));
    strcat(total+1, (char*)params->data);
    total[params->len+1]= '\0';
    
    if(params->handle == writeChar.getValueHandle()) {
        if(params->len == 1)
            led1 = params->data[0];
        else {
            //pc.printf("\n\r Data received: length = %d, data = 0x",params->len);
            if(strncmp((char*)params->data, "ON",2) == 0) {
                //pc.printf("LED ON");
                led1=0;
            } else if(strncmp((char*)params->data, "OFF",3) == 0) {
                //pc.printf("LED OFF");
                led1=1;
            }
            //i2c_port.write(addr, (const char *)(params->data), (int)params->len);
            //i2c_port.write(addr, &a, 1);
            led1=!led1;
        }
        
        //Update the readChar with the value of writeChar
        //ble.updateCharacteristicValue(readChar.getValueHandle(), params->data, params->len);
        //i2c_port.write(addr, (char*)params->data, params->len);
        total[0] = headerSms;

        ble.updateCharacteristicValue(readChar.getValueHandle(), (const uint8_t *)total, strlen(total)+1);
        i2c_port.write(addr, total, strlen(total)+1);
    }
    else if(params->handle == timeSyncChar.getValueHandle()) {
        total[0] = headerTime;
        i2c_port.write(addr, total, strlen(total)+1);
        led1 = !led1;
    }
    //*/
    free(total);
}

int main(void)
{
    //pc.baud(9600);
    led1 = 1;
    ticker.attach(periodicCallback, 1);

    DEBUG("Initialising the nRF51822\n\r");
    ble.init();

    ble.onDisconnection(disconnectionCallback);

    ble.onDataWritten(writeCharCallback);

    //uart = new UARTService(ble);

    // setup advertising
    ble.accumulateAdvertisingPayload(GapAdvertisingData::BREDR_NOT_SUPPORTED | GapAdvertisingData::LE_GENERAL_DISCOVERABLE);
    ble.setAdvertisingType(GapAdvertisingParams::ADV_CONNECTABLE_UNDIRECTED);
    ble.accumulateAdvertisingPayload(GapAdvertisingData::SHORTENED_LOCAL_NAME,
                                     (const uint8_t *)"BLE NOTIF", sizeof("BLE NOTIF") - 1);
    ble.accumulateAdvertisingPayload(GapAdvertisingData::COMPLETE_LIST_16BIT_SERVICE_IDS, (uint8_t *)uuid16_list, sizeof(uuid16_list));
    //ble.setAdvertisingInterval(160); // 100ms; in multiples of 0.625ms.
    ble.setAdvertisingInterval(1600);

    ble.addService(customService);

    ble.startAdvertising();

    while (true) {
        ble.waitForEvent();
    }
}
