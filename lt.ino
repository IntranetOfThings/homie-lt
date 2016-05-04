#include <Homie.h>
#include <OneWire.h>
#include <DallasTemperature.h>

#define FW_NAME "lt"
#define FW_VERSION "2.0.0"

/* Magic sequence for Autodetectable Binary Upload */
const char *__FLAGGED_FW_NAME = "\xbf\x84\xe4\x13\x54" FW_NAME "\x93\x44\x6b\xa7\x75";
const char *__FLAGGED_FW_VERSION = "\x6a\x3f\x3e\x0e\xe1" FW_VERSION "\xb0\x30\x48\xd4\x1a";
/* End of magic sequence for Autodetectable Binary Upload */


const int PIN_LED = D2;
const int PHOTOCELL = D6;

const int TEMP_INTERVAL = 10;			// seconds
unsigned long last_temp_sent = 0;

#define ONE_WIRE_BUS    D3	// DS18B20 pin
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature DS18B20(&oneWire);


HomieNode LEDNode("led", "led");
HomieNode lightNode("light", "cell");
HomieNode tempnode("temperature", "temperature");

unsigned long lastPhotocellReadingTime = 0L;
unsigned long PHOTOCELL_INTERVAL = 60;		// seconds


bool LEDOnHandler(String message)
{
	if (message == "true") {
		digitalWrite(PIN_LED, HIGH);
		Homie.setNodeProperty(LEDNode, "on", "true");
		Serial.println("LED is on");
	} else if (message == "false") {
		digitalWrite(PIN_LED, LOW);
		Homie.setNodeProperty(LEDNode, "on", "false");
		Serial.println("LED is off");
	} else {
		return false;
	}

	return true;
}

int last_light = -1;
int virgin_photo = true;

void loopHandler()
{
	if (millis() - last_temp_sent >= TEMP_INTERVAL * 1000UL
	    || last_temp_sent == 0)
	{

		float temp = 22.12;

		DS18B20.requestTemperatures();
		temp = DS18B20.getTempCByIndex(0);

		Serial.print("Temperature: ");
		Serial.print(temp);
		Serial.println(" °C");


		if (Homie.setNodeProperty(tempnode, "degrees", String(temp), true)) {
			    last_temp_sent = millis();
		}

		// Homie.setNodeProperty(tempnode, "freeheap", String(ESP.getFreeHeap(),DEC), false);
	}


	int light = digitalRead(PHOTOCELL);
	if (light != last_light) {
		if (virgin_photo || ((millis() - lastPhotocellReadingTime) >= (PHOTOCELL_INTERVAL * 1000))) {
			lastPhotocellReadingTime = millis();
			virgin_photo = false;
			last_light = light;
			Homie.setNodeProperty(lightNode, "cell", String(!light), false);
		}
	}

}

void setupHandler()
{
	Homie.setNodeProperty(tempnode, "unit", "c", true);
}

void setup()
{
	pinMode(PIN_LED, OUTPUT);
	pinMode(PHOTOCELL, INPUT);
	digitalWrite(PIN_LED, LOW);

	DS18B20.begin();

	Homie.setFirmware(FW_NAME, FW_VERSION);

	Homie.registerNode(LEDNode);
	LEDNode.subscribe("on", LEDOnHandler);

	Homie.registerNode(lightNode);
	Homie.registerNode(tempnode);

	Homie.setSetupFunction(setupHandler);
	Homie.setLoopFunction(loopHandler);

	Homie.setup();
}

void loop()
{
	Homie.loop();
}
