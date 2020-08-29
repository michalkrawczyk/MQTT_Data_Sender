#include "Rtc.hpp"
#include <ESP8266WiFi.h>

namespace rtc
{
    extern RtcMem rtc_memory{};

    /**
     * @brief  Calculates CRC from given data
     * 
     * @param data_ptr - pointer to data, from which CRC code will be calculated
     * @param length - size of data, from which CRC code will be calculated
     * @return uint32_t with calculated crc code
     */
    const uint32_t RtcMem::calculateCRC32(const uint8_t *data_ptr, size_t length) 
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
     */
    void RtcMem::readMem()
    {
        #ifdef DEBUG
            Serial.println("Reading RTC MEMORY");
        #endif //DEBUG

        is_rtc_valid = false;

        if(ESP.rtcUserMemoryRead(0, (uint32_t*) &rtc_data, sizeof(rtc_data))) 
        {
            uint32_t crc = calculateCRC32(((uint8_t*) &rtc_data) + 4, sizeof( rtc_data ) - 4); //CRC for RTC MEMORY except 4 bytes with CRC itself
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

        rtc_data.crc32 = calculateCRC32(((uint8_t*) &rtc_data) + 4, sizeof( rtc_data ) - 4); //CRC of data to save

        return ESP.rtcUserMemoryWrite(0, (uint32_t*) &rtc_data, sizeof(rtc_data));
    }

    /**
     * @brief  Save rtc_data(Current settings and data to be saved) in RTC Memory
     * @param data - data that will be set as user last data in RTC Memory
     * @return information if opeartion was done successfully
     */
    const bool RtcMem::saveToMemWithData(const rtc_mem_t &data)
    {
        rtc_data.last_data = data;
        return saveToMem();
    }

    /**
     * @brief  Performs operation of deepSleep. Before that saves current settings to RTC Memory
     * @param error_code - 
     * @return information if opeartion was done successfully
     */
    void RtcMem::goDeepSleep(const RtcErrorCode &error_code, const uint64_t time_us)
    {
        #ifdef DEBUG
            Serial.print("Going to sleep: ");
            Serial.print(time_us);
            Serial.println("us");
        #endif //DEBUG

        rtc_data.last_error = error_code;
        saveToMem();
        WiFi.forceSleepBegin();
        /*
        WiFi.disconnect( true );
        WiFi.forceSleepBegin();
        delay(1);
        ESP.deepSleep( time_us, WAKE_RF_DISABLED ); 
        */ //That option should be more power saving. Unfortunately it took too much time in NodeMCU to reconnect after use that
        // ESP.deepSleep(time_us);
        ESP.deepSleep(time_us, RF_NO_CAL);
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
