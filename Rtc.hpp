#ifndef RTC_HPP
#define RTC_HPP

#include <stdint.h>
#include "Device_Settings.h"

#ifndef size_t
    typedef unsigned int size_t;
#endif //size_t

namespace rtc
{
    enum class ErrorCode : uint8_t;
    struct RtcData_t;

    class RtcMem;
}//namespace rtc


struct rtc::RtcData_t
{
    uint32_t crc32;                         // 4 bytes
    uint8_t channel;                        // 1 byte
    uint8_t bssid[6];                       // 6 bytes
    ErrorCode last_error;                    // 1 byte
    RTC_mem_t last_data;                    // 8 bytes
};  //total 20 bytes
//RTC Memory for user data is 512 bytes long

class rtc::RtcMem final
{
public:
    static const uint32_t calcCRC32(const uint8_t *data_ptr, size_t length);

    void readMem();
    const bool saveToMem();
    const bool saveToMemData(const RTC_mem_t &data = 0);

    void deepSleepErr(const ErrorCode &error_code, const uint64_t time_us = 10000000UL);
    void deepSleep(const uint64_t time_us = 10000000UL);

    const RtcData_t getData() const;
    const bool isValid() const;

private:
    bool is_rtc_valid{false};
    RtcData_t rtc_data{};

};

enum class rtc::ErrorCode: uint8_t
{
    NONE,
    NO_WIFI,
    NO_ID,
    NO_MQTT,
    MQTT_SUB_FAIL,
    MQTT_PUB_FAIL,
    UNKNOWN
};


extern rtc::RtcMem RTC;

#endif // RTC_HPP
