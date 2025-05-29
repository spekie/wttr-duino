/*
 * Copyright (C) 2025 spekie
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#include <WiFi.h>
#include <HTTPClient.h>
#include <DHT.h>
#include <Adafruit_BMP280.h>

const char* ssid = "WIFI-SSID";
const char* password = "WIFI-PASSWORD";

const String apiKey = "API-KEY";
const String server = "http://api.thingspeak.com/update";

/* sensor section */
#define DHTPIN 4
#define DHTTYPE DHT22
DHT dht(DHTPIN, DHTTYPE);
Adafruit_BMP280 bmp;
const int mq135Pin = 34; // analog input IMPORTANT!!!

void setup() {
    Serial.begin(115200);
    delay(100);

    // code below starts the sensors
    dht.begin();
    if (!bmp.begin(0x76)) {
        Serial.println("BMP280 not found");
        while (1);
    }

    // internet connection
    WiFi.begin(ssid, password);
    Serial.print("Connecting to internet. . .");
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    Serial.println("\nConnected!");
}

void loop() {
    float temp = dht.readTemperature();
    float hum = dht.readHumidity();
    float pres = bmp.readPressure() / 100.0F;
    int airQuality = analogRead(mq135Pin);

    if (isnan(temp) || isnan(hum)) {
        Serial.println("Failed to read");
        return;
    }

    Serial.printf("T: %.2f°C, H: %.2f%%, P: %.2fhPa, AQ: %d\n", temp, hum, pres, airQuality);

    // api communication
    if (WiFi.status() == WL_CONNECTED) {
        HTTPClient http;
        String url = server + "?api_key=" + apiKey +
                     "&field1=" + String(temp) +
                     "&field2=" + String(hum) +
                     "&field3=" + String(pres) +
                     "&field4=" + String(airQuality);
        http.begin(url);
        int httpCode = http.GET();
        http.end();

        if (httpCode > 0) {
            Serial.println("Data sent to ThingSpeak.");
        } else {
            Serial.printf("HTTP Error: %s\n", http.errorToString(httpCode).c_str());
        }
    }

    delay(15000); // delay to not overload the API
}
