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



    private:
        const char *m_ssid;
        const char *m_password;
        unsigned long m_syncInterval;
        unsigned long m_lastSyncMillis = 0;

        void syncTime();


    public:
        int64_t now() override;

        void begin();

        void update();

    };
}

#endif
