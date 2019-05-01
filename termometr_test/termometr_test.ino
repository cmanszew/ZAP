#include <CurieBLE.h>
#include <OneWire.h>
#include <DallasTemperature.h>

/* ========================================== <Thermometer stuff>  ========================================== */
// Data wire is plugged into port 2 on the Arduino
#define ONE_WIRE_BUS          2
#define TEMPERATURE_PRECISION 9
#define DEVICE_NUMBER         0

// Setup a oneWire instance to communicate with any OneWire devices (not just Maxim/Dallas temperature ICs)
OneWire oneWire(ONE_WIRE_BUS);

// Pass our oneWire reference to Dallas Temperature.
DallasTemperature sensors(&oneWire);
DeviceAddress deviceAddress;
/* ========================================== </Thermometer stuff> ========================================== */


/* ============================================== <BLE stuff>  ============================================== */
#define charArraySize 5
BLEPeripheral blePeripheral;
BLEService healthThermometerService("1809");
BLECharacteristic healthThermometerChar("2A1C", BLENotify, charArraySize);
unsigned char temperatureCharArray[charArraySize] = { 0 };
/* ============================================== </BLE stuff> ============================================== */

long previousMillis = 0;  // last time the temperature was checked, in ms

void setup(void)
{
    Serial.begin(9600);

/* ========================================== <Thermometer stuff>  ========================================== */
    pinMode(ONE_WIRE_BUS, INPUT_PULLUP); //TO JEST ŻYCIE!!!
    sensors.begin();
    sensors.getAddress(deviceAddress, DEVICE_NUMBER);
/* ========================================== </Thermometer stuff> ========================================== */

/* ============================================== <BLE stuff>  ============================================== */
    pinMode(13, OUTPUT);   // initialize the LED on pin 13 to indicate when a central is connected

    blePeripheral.setLocalName("BLEThermometer");
    blePeripheral.setAdvertisedServiceUuid(healthThermometerService.uuid());
    blePeripheral.addAttribute(healthThermometerService);
    blePeripheral.addAttribute(healthThermometerChar);

    blePeripheral.begin();
    Serial.println("Bluetooth device active, waiting for connections...");
/* ============================================== </BLE stuff> ============================================== */
}

void loop(void)
{
    BLECentral central = blePeripheral.central();

    if (central) {
        Serial.print("Connected to central: ");
        Serial.println(central.address());
        // turn on the LED to indicate the connection:
        digitalWrite(13, HIGH);

        // check the temperature every 200ms
        // as long as the central is still connected:
        while (central.connected()) {
            long currentMillis = millis();
            if (currentMillis - previousMillis >= 200) {
                previousMillis = currentMillis;
                updateTemperature();
            }
        }
        // when the central disconnects, turn off the LED:
        digitalWrite(13, LOW);
        Serial.print("Disconnected from central: ");
        Serial.println(central.address());
    }
}

void updateTemperature(void)
{
    sensors.requestTemperatures();
    float tempC = sensors.getTempC(deviceAddress);
    Serial.print("Temp C: ");
    Serial.println(tempC);

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
