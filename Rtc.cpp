#include "Rtc.hpp"
#include <ESP8266WiFi.h>
extern rtc::RtcMem RTC{};

namespace rtc
{
    /**
     * @brief  Calculates CRC from given data
     * 
     * @param data_ptr - pointer to data, from which CRC code will be calculated
     * @param length - size of data, from which CRC code will be calculated
     * @return uint32_t with calculated crc code
     */
    const uint32_t RtcMem::calcCRC32(const uint8_t *data_ptr, size_t length) 
    {
        uint32_t crc = 0xffffffff;
        while(length--) 
        {
            uint8_t c = *data_ptr++;

            for(uint32_t i = 0x80; i > 0; i >>= 1) 
            {
                bool bit = crc & 0x80000000;
                
                if(c & i)
                {
                    bit = !bit;
                }

                crc <<= 1;

                if(bit) 
                {
                    crc ^= 0x04c11db7;
                }
            }
        }

        return crc;
    }

    /**
     * @brief  Reads Data Saved in RTC Memory and overwrites current temporary rtc_data structure
     * @note  To check if data has been read succesfully - use RtcMem::isValid();
     */
    void RtcMem::readMem()
    {
        #ifdef DEBUG
            Serial.println("Reading RTC MEMORY");
        #endif //DEBUG

        is_rtc_valid = false;

        if(ESP.rtcUserMemoryRead(0, (uint32_t*) &rtc_data, sizeof(rtc_data))) 
        {
            uint32_t crc = calcCRC32(((uint8_t*) &rtc_data) + 4, sizeof( rtc_data ) - 4); //CRC for RTC MEMORY except 4 bytes with CRC itself
            if( crc == rtc_data.crc32 )
            {
                is_rtc_valid = true;
            }
        }
    }

    /**
     * @brief  Save rtc_data(Current settings) in RTC Memory
     * @return information if opeartion was done successfully
     */
    const bool RtcMem::saveToMem()
    {
        #ifdef DEBUG
            Serial.println("Saving to RTC MEMORY");
        #endif //DEBUG

        rtc_data.channel = WiFi.channel();
        memcpy(rtc_data.bssid, WiFi.BSSID(), 6); // Copy 6 bytes of BSSID (Mac Address of AP)

        rtc_data.crc32 = calcCRC32(((uint8_t*) &rtc_data) + 4, sizeof( rtc_data ) - 4); //CRC of data to save

        return ESP.rtcUserMemoryWrite(0, (uint32_t*) &rtc_data, sizeof(rtc_data));
    }

    /**
     * @brief  Save rtc_data(Current settings and data to be saved) in RTC Memory
     * @param data - data that will be set as user last data in RTC Memory
     * @return information if opeartion was done successfully
     */
    const bool RtcMem::saveToMemData(const RTC_mem_t &data)
    {
        rtc_data.last_data = data;
        return saveToMem();
    }

    /**
     * @brief  Performs operation of deepSleep after Error. Before that saves current settings to RTC Memory
     * @param error_code - Sets last error. If there's no Error - use RtcMem::deepSleep()
     * @param time_us - Sleep duration in microseconds. Default is 10 s - It's enough to wait for Watchdog to reset (if enabled).
     */
    void RtcMem::deepSleepErr(const ErrorCode &error_code, const uint64_t time_us)
    {
        #ifdef DEBUG
            Serial.print("Going to sleep: ");
            Serial.print(time_us);
            Serial.println("us");
        #endif //DEBUG

        rtc_data.last_error = error_code;
        saveToMem();
        WiFi.disconnect(true);
        WiFi.mode(WIFI_OFF);
        WiFi.forceSleepBegin();
        delay(1);
        // ESP.deepSleep( time_us, WAKE_RF_DISABLED );  //That option should be more power saving. Unfortunately it took too much time in NodeMCU to reconnect after use that
        
        ESP.deepSleep(time_us, WAKE_NO_RFCAL);
    }

    /**
     * @brief  Performs operation of deepSleep without any Error. Before that saves current settings to RTC Memory
     * @param time_us - Sleep duration in microseconds. Default is 10 s - It's enough to wait for Watchdog to reset (if enabled).
     */
    void RtcMem::deepSleep(const uint64_t time_us)
    {
        deepSleepErr(ErrorCode::NONE, time_us);
    }


    /**
     * @brief  Getter for RTC_Data_t struct
     * @return read-only structure with RTC Data
     */
    const RtcData_t RtcMem::getData() const
    {
        return rtc_data;
    }

    /**
     * @brief  Validation of RTC Data. This is used to make sure that Memory has been already and successfully read
     * @return True if read RTC Memory is valid.
     */
    const bool RtcMem::isValid() const
    {
        return is_rtc_valid;
    }

}//namespace rtc
