#include <Adafruit_ADXL345_U.h>

#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>
#include "WifiSettings.h"

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 32 // OLED display height, in pixels

// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
#define OLED_RESET     -1
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

Adafruit_ADXL345_Unified accel = Adafruit_ADXL345_Unified();

const char* ssid = STASSID;
const char* password = STAPSK;

bool startup = true;

void log(String line) {
    Serial.println(line);
    if (startup) {
        display.setCursor(0, 0);
        display.println(line);
    }
}


void setup() {
    Serial.begin(9600);

    if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { // Address 0x3C for 128x32
        Serial.println(F("SSD1306 allocation failed"));
        for(;;); // Don't proceed, loop forever
    }
    display.setTextSize(1);      // Normal 1:1 pixel scale
    display.setTextColor(WHITE); // Draw white text
    display.setCursor(0, 0);     // Start at top-left corner
    display.cp437(true);         // Use full 256 char 'Code Page 437' font

    // Clear the buffer
    display.clearDisplay();


    Serial.println("Booting");
    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid, password);
    while (WiFi.waitForConnectResult() != WL_CONNECTED) {
        Serial.println("Connection Failed! Rebooting...");
        delay(5000);
        ESP.restart();
    }

    // Hostname defaults to esp8266-[ChipID]
    ArduinoOTA.setHostname("meettrein");

    ArduinoOTA.onStart([]() {
        String type;
        if (ArduinoOTA.getCommand() == U_FLASH) {
            type = "sketch";
        } else { // U_FS
            type = "filesystem";
        }

        // NOTE: if updating FS this would be the place to unmount FS using FS.end()
        Serial.println("Start updating " + type);
    });
    ArduinoOTA.onEnd([]() {
        Serial.println("\nEnd");
    });
    ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
        Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
    });
    ArduinoOTA.onError([](ota_error_t error) {
        Serial.printf("Error[%u]: ", error);
        if (error == OTA_AUTH_ERROR) {
            Serial.println("Auth Failed");
        } else if (error == OTA_BEGIN_ERROR) {
            Serial.println("Begin Failed");
        } else if (error == OTA_CONNECT_ERROR) {
            Serial.println("Connect Failed");
        } else if (error == OTA_RECEIVE_ERROR) {
            Serial.println("Receive Failed");
        } else if (error == OTA_END_ERROR) {
            Serial.println("End Failed");
        }
    });
    ArduinoOTA.begin();
    Serial.println("Ready");
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());

    display.println(F("Welcome! My IP address is: "));
    display.println(WiFi.localIP());
    display.display();

    /* Initialise the sensor */
    if(!accel.begin())
    {
        /* There was a problem detecting the ADXL345 ... check your connections */
        Serial.println(F("Ooops, no ADXL345 detected ... Check your wiring!"));
        while(1);
    }

    /* Set the range to whatever is appropriate for your project */
    accel.setRange(ADXL345_RANGE_2_G);
    delay(3000);
    startup = false;
}

void loop() {
    ArduinoOTA.handle();

    /* Get a new sensor event */
    sensors_event_t event;
    accel.getEvent(&event);

    display.clearDisplay();
    display.setCursor(0, 0);
    display.print(F("X: "));
    display.println(event.acceleration.x);
    display.print(F("Y: "));
    display.println(event.acceleration.y);
    display.print(F("Z: "));
    display.println(event.acceleration.z);
    display.display();      // Show initial text
    delay(500);
}
