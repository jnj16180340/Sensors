/***************************************************************************
  This is a library for the BME280 humidity, temperature & pressure sensor

  Designed specifically to work with the Adafruit BME280 Breakout
  ----> http://www.adafruit.com/products/2650

  These sensors use I2C or SPI to communicate, 2 or 4 pins are required
  to interface. The device's I2C address is either 0x76 or 0x77.

  Adafruit invests time and resources providing this open source code,
  please support Adafruit andopen-source hardware by purchasing products
  from Adafruit!

  Written by Limor Fried & Kevin Townsend for Adafruit Industries.
  BSD license, all text above must be included in any redistribution
  See the LICENSE file for details.
 ***************************************************************************/

#include <Wire.h>
#include <SPI.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>

// NB: Using DiyMalls custom forkl of this library - Necessary?
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#define OLED_RESET 4 // maybe not necessary?
Adafruit_SSD1306 display(OLED_RESET);

#define SEALEVELPRESSURE_HPA (1013.25)

#define BME280_ADDRESS 0x76

Adafruit_BME280 bme; // I2C

#define UPDATE_PERIOD 1000 // ms

#define SERIAL Serial1 // SeriaL = USB/FTDI; Serial1 = Xbee

// NB: Need v5 becuase v6 is broken on this board/libraries
#include <ArduinoJson.h>

#include <CircularBuffer.h>
// NB: *WEIRD* MEMORY OVERFLOWS WHEN THIS IS TOO LARGE!!!!!
CircularBuffer<uint8_t,64> humidityHistory; 

#include <Math.h>

void setup() {
    SERIAL.begin(9600);
    while(!SERIAL);    // time to get serial running
    SERIAL.println(F("BME280 test"));

    unsigned status;
    // default settings
    status = bme.begin(BME280_ADDRESS);  
    if (!status) {
        SERIAL.println("Could not find a valid BME280 sensor, check wiring, address, sensor ID!");
        while (1) delay(10);
    }
    
    display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
    display.clearDisplay(); display.display();
    display.setTextSize(1);
    display.setTextColor(WHITE);
    display.setCursor(0,20);

    display.println("Smoke weed every day");
    display.display();
    delay(200);
    display.clearDisplay(); display.display();

    SERIAL.println("Initialized and ready");
}


void loop() {
    float currentPressure; // 100*hPa
    float currentTemperature; // C
    float currentHumidity; // % RH
    currentTemperature = bme.readTemperature();
    currentPressure = bme.readPressure() / 100.0F;
    //SERIAL.print(bme.readAltitude(SEALEVELPRESSURE_HPA));
    currentHumidity = bme.readHumidity();

    humidityHistory.unshift((uint8_t)currentHumidity);
    
    printValues(currentPressure, currentTemperature, currentHumidity);
    displayValues(currentPressure, currentTemperature, currentHumidity);
    delay(UPDATE_PERIOD);
}


void printValues(float currentPressure, float currentTemperature, float currentHumidity) {
    // Should read + serialize measurements as a struct - see ArduinoJson examples
    StaticJsonBuffer<200> outputBuffer;
    JsonObject& output = outputBuffer.createObject();
    JsonObject& measurementSection = output.createNestedObject("data");
    // TODO - should contain units, etc. Use a standard schema for sensor data.
    measurementSection["temperature"] = currentTemperature;
    measurementSection["pressure"] = currentPressure;
    measurementSection["humidity"] = currentHumidity;

    //serializeJson(output, SERIAL);
    output.printTo(SERIAL);
    
    //SERIAL.print("P, T, RH (hPa, C, %RH) = ");
    //SERIAL.print(currentPressure);SERIAL.print(", ");
    //SERIAL.print(currentTemperature);SERIAL.print(", ");
    //SERIAL.print(currentHumidity);SERIAL.print(", ");
    //SERIAL.println();
}

void displayValues(float currentPressure, float currentTemperature, float currentHumidity){
    //display.clearDisplay();
    // Clear just the text area -- How to get it automatically?
    display.fillRect(0,0,128,5*5,0);
    display.setCursor(0,0);
    display.print("T (C):   "); display.println(currentTemperature);
    display.print("P (hPa): "); display.println(currentPressure);
    display.print("RH (%):  "); display.println(currentHumidity);

    // "Graph" section
    display.fillRect(0,24,128,64-24,0);
    display.drawRect(0,24,128,64-24,1);

    using index_t = decltype(humidityHistory)::index_t;
    for (index_t i = 0; i < humidityHistory.size(); i++) {
      //avg += humidityHistory[i] / humidityHistory.size();
      // Should draw lines between points here!!

      //map(value, fromLow, fromHigh, toLow, toHigh)
      int scaledHumidity = map(humidityHistory[i], 0, 100, 64, 24);
      display.drawPixel(128-i,scaledHumidity,1);
    }
    
    
    display.display();
}
