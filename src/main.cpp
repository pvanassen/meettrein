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

#define MODE_STARTUP 0
#define MODE_VOLTAGE 1
#define MODE_TILT 2
#define MODE_SWITCH_AFTER_MS 5000
#define MODE_DELAY 450

// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
#define OLED_RESET     -1
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

Adafruit_ADXL345_Unified accel = Adafruit_ADXL345_Unified();

const char* ssid = STASSID;
const char* password = STAPSK;

int analogInput = A0;
float R1 = 986000.00; // Vul hier de exacte waarde van R1 in
float R2 = 99000.00;  // Vul hier de exacte waarde van R2 in
float V1 = 3.3;       // Vul hier de voedingsspanging in (3.3 works for me)

int modeVisibleMs = 0;
int mode = MODE_STARTUP;

void setup() {
    Serial.begin(9600);

    if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { // Address 0x3C for 128x32
        Serial.println(F("Cannot find screen!"));
        delay(5000);
        ESP.restart();
    }
    display.setTextSize(1);
    display.setTextColor(WHITE);
    display.setCursor(0, 0);
    display.cp437(true);

    display.clearDisplay();

    pinMode(analogInput, INPUT); //assigning the input port

    display.println("Booting");
    display.display();

    if(!accel.begin())
    {
        display.println(F("Cannot find tilt sensor!"));
        display.display();
        delay(5000);
        ESP.restart();
    }
    display.println("Tilt sensor detected");
    display.display();

    /* Set the range to whatever is appropriate for your project */
    accel.setRange(ADXL345_RANGE_2_G);

    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid, password);
    while (WiFi.waitForConnectResult() != WL_CONNECTED) {
        display.println(F("Connection Failed! Rebooting..."));
        display.display();
        delay(5000);
        ESP.restart();
    }
    display.println("Got WiFi connection");
    display.display();

    // Hostname defaults to esp8266-[ChipID]
    ArduinoOTA.setHostname("meettrein");

    ArduinoOTA.onStart([]() {
        String type;
        if (ArduinoOTA.getCommand() == U_FLASH) {
            type = "sketch";
        } else {
            type = "filesystem";
        }
        display.clearDisplay();
        display.setCursor(0, 0);
        display.println("Start firmware update");
        display.display();
    });
    ArduinoOTA.onEnd([]() {
        display.clearDisplay();
        display.setCursor(0, 0);
        display.println("Firmware update done");
        display.display();
    });
    ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
        display.clearDisplay();
        display.setCursor(0, 0);
        display.println("Updating firmware");
        display.printf("Progress: %u%%\r", (progress / (total / 100)));
        display.display();
    });
    ArduinoOTA.onError([](ota_error_t error) {
        display.clearDisplay();
        display.printf("Error[%u]: ", error);
        if (error == OTA_AUTH_ERROR) {
            display.println("Auth Failed");
        } else if (error == OTA_BEGIN_ERROR) {
            display.println("Begin Failed");
        } else if (error == OTA_CONNECT_ERROR) {
            display.println("Connect Failed");
        } else if (error == OTA_RECEIVE_ERROR) {
            display.println("Receive Failed");
        } else if (error == OTA_END_ERROR) {
            display.println("End Failed");
        }
        display.display();
        delay(10000);
    });
    ArduinoOTA.begin();
    Serial.println(F("Ready"));
    Serial.print(F("IP address: "));
    Serial.println(WiFi.localIP());

    display.clearDisplay();
    display.setCursor(0, 0);
    display.println(F("Welcome! IP address: "));
    display.println(WiFi.localIP());
    display.display();

    delay(3000);
    mode = MODE_VOLTAGE;
}

void loop() {
    ArduinoOTA.handle();
    if (mode == MODE_TILT) {
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
        display.display();
        delay(MODE_DELAY);
        modeVisibleMs += MODE_DELAY;
        if (modeVisibleMs > MODE_SWITCH_AFTER_MS) {
            mode = MODE_VOLTAGE;
            modeVisibleMs = 0;
        }
    }

    if (mode == MODE_VOLTAGE) {
        int delayMs = 5;
        int loops = MODE_DELAY / delayMs;
        float Vout;
        float Vin = 0.00;
        for (int i=0;i!=loops;i++) {
            Vout = (analogRead(analogInput) * V1) / 1024.00;
            Vin += Vout / (R2/(R1+R2));
            delay(delayMs);
        }
        display.clearDisplay();
        display.setCursor(0, 0);

        display.print(F("Voltage: "));
        display.println(Vin / loops);
        display.display();      // Show initial text
        modeVisibleMs += MODE_DELAY;
        if (modeVisibleMs > MODE_SWITCH_AFTER_MS) {
            mode = MODE_TILT;
            modeVisibleMs = 0;
        }
    }
}
