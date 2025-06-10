/*
 * Copyright (c) 2025 spekie
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
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
