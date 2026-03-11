//
// Created by Dmitry Popov on 10.03.2026.
//

#ifndef EMBEDD_NTPCLOCK_H
#define EMBEDD_NTPCLOCK_H
#include <IUnixClock.h>

#if defined(ESP32)
#include <WiFi.h>
#elif defined(ESP8266)
#include <ESP8266WiFi.h>
#else
#error "This class requires an ESP32 or ESP8266!"
#endif

#include <time.h>

namespace Fractonica {
    class WifiClock : public Fractonica::IUnixClock {
    public:
        WifiClock(const char *ssid, const char *password, unsigned long syncIntervalSeconds = 3600)
            : m_ssid(ssid), m_password(password), m_syncInterval(syncIntervalSeconds * 1000) {
        }

        void begin() {
            Serial.println("Initializing SmartClock...");
            syncTime();
        }

        void update() {
            unsigned long currentMillis = millis();
            if (currentMillis - m_lastSyncMillis >= m_syncInterval) {
                syncTime();
            }
        }

    private:
        const char *m_ssid;
        const char *m_password;
        unsigned long m_syncInterval;
        unsigned long m_lastSyncMillis = 0;

        void syncTime() {
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

    public:
        int64_t now() override {
            time_t now;
            time(&now);
            return now;
        }
    };
}

#endif
