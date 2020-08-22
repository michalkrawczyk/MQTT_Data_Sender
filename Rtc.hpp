#ifndef RTC
#define RTC

#include <stdint.h>

#ifndef size_t
    typedef unsigned int size_t;
#endif //size_t

struct RtcData_t
{
    uint32_t crc32;             // 4 bytes
    uint8_t channel;            // 1 byte
    uint8_t bssid[6];           // 6 bytes
    uint8_t last_error;         // 1 byte
    uint32_t last_data;         // 4 bytes
};  //total 16 bytes
//TODO: Think of storing also sign of last data((with number of decimal)), type, maybe even allow to store double
//TODO: Check size of RTC User Memory

namespace rtc
{
    extern bool is_rtc_valid;
    extern RtcData_t rtc_data;

    enum class RtcErrorCode : uint8_t;

    const uint32_t calculateCRC32(const uint8_t *data_ptr, size_t length);
    void readMem();
    const bool saveToMem();
    const bool saveToMemWithData(const uint32_t &data = 0);
    void goDeepSleep(const RtcErrorCode &error_code, const uint64_t time_us = 1000000UL);
    
    const uint32_t getLastDataAsUint32();
    const float getLastDataAsFloat(uint8_t decimal_number = 2, bool is_positive = true);
    const double getLastDataAsDouble(uint16_t decimal_number = 2, bool is_positive = true);

    const uint32_t FloatToU32(float data, uint8_t decimal_number = 2);
    const uint32_t DoubleToU32(double data, uint16_t decimal_number = 2);
    const uint32_t U16x2ToU32(uint16_t data1, uint16_t data2);
}

enum class rtc::RtcErrorCode: uint8_t
{
    NONE,
    NO_WIFI,
    NO_ID,
    NO_MQTT,
    MQTT_SUB_FAIL,
    MQTT_PUB_FAIL,
    UNKNOWN
};




#endif // RTC_
