/*
    Cat Tracker 2013.
    This is an Arduino sketch to control a GPS module + an SD card module.
    It logs location data to the configured logfile in bursts every N
    seconds.
 */

#include <Adafruit_GPS.h>
#include <SoftwareSerial.h>
#include <FlexiTimer2.h>

#define TXPIN PIN_D0
#define RXPIN PIN_D1

// Baud rate of Arduino serial terminal. Must be fast.
#define TERMBAUD    115200
// Baud rate of the GPS module.
#define GPSBAUD  4800

TinyGPS gps;
SoftwareSerial uart_gps(RXPIN, TXPIN);

// SD card setup.
SoftwareSerial logger(PIN_D2, PIN_D3);

#define BURST_COUNT 60
#define SLEEP_DURATION_MS 15 * 1000

//------------------------------------------------------------------------------

void setup()
{
    Serial.begin(TERMBAUD);
    uart_gps.begin(GPSBAUD);

    Serial.println("Cat Tracker online.");

    pinMode(PIN_D0, OUTPUT);
    // pinMode(PIN_D2, OUTPUT);

    startLogging();
    // Log one burst, then run the timer to control the next bursts.
    logGPSData();

	FlexiTimer2::set(SLEEP_DURATION_MS, logGPSData); // 500ms period
	FlexiTimer2::start();
}

void loop()
{
	readSerialCommand();
}

//------------------------------------------------------------------------------

void logGPSData()
{
	int counter = BURST_COUNT;

    while (uart_gps.available() && counter)
    {
		int c = uart_gps.read();
		if (gps.encode(c))
		{
			addLogEntry(gps);
			counter--;
		}
    }
}

void addLogEntry(TinyGPS &gps)
{
    // log entries are formatted as JSON: { date:"", lat:"", lon:"" }
    // date format: ddmmyy hhmmsscc

    float latitude, longitude;
    gps.f_get_position(&latitude, &longitude);

    unsigned long date, time, age;
    gps.get_datetime(&date, &time, &age);

    Serial.print("{\"date\":\"");
    Serial.print(date, DEC);
    Serial.print(" ");
    Serial.print(time, DEC);

    Serial.print("\",\"lat\":\"");
    Serial.print(latitude, 10);

    Serial.print("\",\"lon\":\"");
    Serial.print(longitude, 10);

    Serial.println("\"}");
}


void reset()
{
    // no-op for the moment
}

void startLogging()
{
	Serial.println("Now tracking cat.");
    //logger.begin(4800);
    //logger.println("Hello, world?");
	//logger.println("------ start");
}

void dumpLog()
{
    Serial.println("dumpLog() unimplemented");
}

void readSerialCommand()
{
    // Serial.readBytesUntil(character, buffer, length)
    if (Serial.available())
    {
        char command = Serial.read();

        switch (command)
        {
            case 'r':
                reset();
                break;

            case 'd':
                dumpLog();
                break;

            case 'g':
                startLogging();
                break;

            case 's':
                // status
                // TODO
                break;
        }
    }
}
