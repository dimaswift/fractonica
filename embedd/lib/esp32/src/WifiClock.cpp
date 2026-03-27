//
// Created by Dmitry Popov on 27.03.2026.
//

#include "WifiClock.h"

int64_t Fractonica::WifiClock::now() {
    time_t now;
    time(&now);
    return now;
}

void Fractonica::WifiClock::begin() {
    Serial.println("Initializing SmartClock...");
    syncTime();
}

void Fractonica::WifiClock::update() {
    unsigned long currentMillis = millis();
    if (currentMillis - m_lastSyncMillis >= m_syncInterval) {
        syncTime();
    }
}

void Fractonica::WifiClock::syncTime() {
    Serial.println("Starting time sync process...");

    // 1. Turn on WiFi and connect
    WiFi.mode(WIFI_STA);
    WiFi.begin(m_ssid, m_password);

    Serial.print("Connecting to WiFi");
    int attempts = 0;
    while (WiFi.status() != WL_CONNECTED && attempts < 20) {
        delay(500);
        Serial.print(".");
        attempts++;
    }
    Serial.println();

    if (WiFi.status() == WL_CONNECTED) {
        Serial.println("WiFi connected. Requesting NTP time...");

        // 2. Request time from NTP servers
        // Unix time is strictly UTC, so we set timezone offsets to 0
        configTime(0, 0, "ru.pool.ntp.org", "time.nist.gov");

        // 3. Wait for the clock to set (Unix time > year 2000, roughly 1 billion)
        time_t now = time(nullptr);
        int ntpAttempts = 0;
        while (now < 1000000000 && ntpAttempts < 20) {
            delay(500);
            Serial.print(".");
            now = time(nullptr);
            ntpAttempts++;
        }

        if (now > 1000000000) {
            Serial.println("\nTime successfully synchronized!");
            m_lastSyncMillis = millis(); // Reset our interval timer
        } else {
            Serial.println("\nNTP sync failed.");
        }
    }

    WiFi.disconnect(true);
    WiFi.mode(WIFI_OFF);
}