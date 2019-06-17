#include <CurieBLE.h>
#include <OneWire.h>
#include <DallasTemperature.h>

#define ONE_WIRE_BUS          2
#define TEMPERATURE_PRECISION 9
#define DEVICE_NUMBER         0

OneWire oneWire(ONE_WIRE_BUS);

DallasTemperature sensors(&oneWire);
DeviceAddress deviceAddress;

#define charArraySize 5
BLEPeripheral blePeripheral;
BLEService healthThermometerService("1809");
BLECharacteristic healthThermometerChar("2A1C", BLENotify, charArraySize);
unsigned char temperatureCharArray[charArraySize] = { 0 };

void setup(void)
{
    pinMode(ONE_WIRE_BUS, INPUT_PULLUP);
    sensors.begin();
    sensors.getAddress(deviceAddress, DEVICE_NUMBER);

    blePeripheral.setLocalName("BLEThermometer");
    blePeripheral.setAdvertisedServiceUuid(healthThermometerService.uuid());
    blePeripheral.addAttribute(healthThermometerService);
    blePeripheral.addAttribute(healthThermometerChar);

    blePeripheral.begin();
}

void loop(void)
{
    BLECentral central = blePeripheral.central();

    if (central) {
        while (central.connected()) {
            updateTemperature();
            delay(1000);
        }
    }
}

void updateTemperature(void)
{
    sensors.requestTemperatures();
    float tempC = sensors.getTempC(deviceAddress);

    unsigned int IEEE11073tempC = floatTempToIEEE11073(tempC);
    memcpy(temperatureCharArray + 1, &IEEE11073tempC, charArraySize - 1);
    healthThermometerChar.setValue(temperatureCharArray, charArraySize);
}

unsigned int floatTempToIEEE11073(float val)
{
    unsigned int mantissa = val * 100;
    unsigned int exponent = 0xFE000000;

    mantissa = (((mantissa >> 8) & 0x00800000) | mantissa) & 0x00ffffff;
    return exponent | mantissa;
}
