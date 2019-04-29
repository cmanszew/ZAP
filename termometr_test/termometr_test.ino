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
BLEPeripheral blePeripheral;   // BLE Peripheral Device (the board you're programming)
BLEService TempService("1809"); // Temperature Service - it doesn't matter what you call it as long as you have the correct UUID

// BLE temperature Measurement Characteristic"
BLECharacteristic TempChar("2A1C", BLERead | BLEIndicate, 2);

long previousMillis = 0;  // last time the temperature was checked, in ms
/* ============================================== </BLE stuff> ============================================== */

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

    /* Set a local name for the BLE device
     This name will appear in advertising packets
     and can be used by remote devices to identify this BLE device
     The name can be changed but maybe be truncated based on space left in advertisement packet */
    blePeripheral.setLocalName("TempSketch");
    blePeripheral.setAdvertisedServiceUuid(TempService.uuid());  // add the service UUID
    blePeripheral.addAttribute(TempService);   // Add the BLE temperatureservice
    blePeripheral.addAttribute(TempChar); // add the RSC Measurement characteristic

    /* Now activate the BLE device.  It will start continuously transmitting BLE
     advertising packets and will be visible to remote BLE central devices
     until it receives a new connection */
    blePeripheral.begin();
    Serial.println("Bluetooth device active, waiting for connections...");
/* ============================================== </BLE stuff> ============================================== */
}

void loop(void)
{
    // listen for BLE peripherals to connect:
    BLECentral central = blePeripheral.central();

    // if a central is connected to peripheral:
    if (central) {
        Serial.print("Connected to central: ");
        // print the central's MAC address:
        Serial.println(central.address());
        // turn on the LED to indicate the connection:
        digitalWrite(13, HIGH);

        // check the temperature every 200ms
        // as long as the central is still connected:
        while (central.connected()) {
            long currentMillis = millis();
            // if 200ms have passed, check the temperaturemeasurement:
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
    Serial.print(tempC);
    Serial.println();

    const unsigned char temperatureCharArray[2] = { 63, (char)tempC};
    TempChar.setValue(temperatureCharArray, 2);  // and update the heart rate measurement characteristic
    delay(1000);
}
