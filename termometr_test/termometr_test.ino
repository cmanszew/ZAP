#include <OneWire.h>
#include <DallasTemperature.h>

// Data wire is plugged into port 2 on the Arduino
#define ONE_WIRE_BUS          2
#define TEMPERATURE_PRECISION 9
#define DEVICE_NUMBER         0

// Setup a oneWire instance to communicate with any OneWire devices (not just Maxim/Dallas temperature ICs)
OneWire oneWire(ONE_WIRE_BUS);

// Pass our oneWire reference to Dallas Temperature.
DallasTemperature sensors(&oneWire);
DeviceAddress deviceAddress;

void setup(void)
{
    Serial.begin(9600);
    pinMode(ONE_WIRE_BUS, INPUT_PULLUP); //TO JEST ŻYCIE!!!
    sensors.begin();
    sensors.getAddress(deviceAddress, DEVICE_NUMBER);
}

void loop(void)
{
    sensors.requestTemperatures();
    float tempC = sensors.getTempC(deviceAddress);
    Serial.print("Temp C: ");
    Serial.print(tempC);
    Serial.println();
    delay(1000);
}
