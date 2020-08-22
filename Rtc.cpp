#include "Rtc.hpp"
#include <ESP8266WiFi.h>

namespace rtc
{
    extern bool is_rtc_valid(false);
    extern RtcData_t rtc_data = {0};

    /**
     * @brief  Calculates CRC from given data
     * 
     * @param data_ptr - pointer to data, from which CRC code will be calculated
     * @param length - size of data, from which CRC code will be calculated
     * @return uint32_t with calculated crc code
     */
    const uint32_t calculateCRC32(const uint8_t *data_ptr, size_t length) 
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
    void readMem()
    {
        #ifdef DEBUG
            Serial.println("Reading RTC MEMORY");
        #endif //DEBUG

        bool is_rtc_valid = false;

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
    const bool saveToMem()
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
    const bool saveToMemWithData(const uint32_t &data)
    {
        rtc_data.last_data = data;
        return saveToMem();
    }

    /**
     * @brief  Performs operation of deepSleep. Before that saves current settings to RTC Memory
     * @param error_code - 
     * @return information if opeartion was done successfully
     */
    void goDeepSleep(const RtcErrorCode &error_code, const uint64_t time_us)
    {
        #ifdef DEBUG
            Serial.print("Going to sleep: ");
            Serial.print(time_us);
            Serial.println("us");
        #endif //DEBUG

        rtc_data.last_error = static_cast<uint8_t>(error_code);
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
     * @brief  Get last saved data as uint32_t
     * @note For now it is just unnecessary getter
     * 
     * @return RTC last data as uin32t_t
     */
    const uint32_t getLastDataAsUint32()
    {
        return rtc_data.last_data;
    }

    /**
     * @brief  Get last saved data in RTC Memory as float
     * 
     * @param  decimal_number - how many numbers will be moved to decimal part
     * @param  is_positive - is positive or negative number
     * @return RTC last data as Float
     */
    const float getLastDataAsFloat(uint8_t decimal_number, bool is_positive)
    {
        float result = static_cast<float>(rtc_data.last_data);

        while(decimal_number--)
        {
            result /= 10;
        }

        return is_positive ? result : -result;
    }

    /**
     * @brief  Get last saved data in RTC Memory as double
     * 
     * @param  decimal_number - how many numbers will be moved to decimal part
     * @param  is_positive - is positive or negative number
     * @return RTC last data as Double
     */
    const double getLastDataAsDouble(uint16_t decimal_number, bool is_positive)
    {
        double result = static_cast<double>(rtc_data.last_data);

        while(decimal_number--)
        {
            result /= 10;
        }

        return is_positive ? result : -result;
    }

    /**
     * @brief  Converts float to uint32_t
     * @note  This function will not work properly if float multiplied by 10^decimal_number will exceed uint32_t
     *  For now has been left - propably whole function won't be needed
     * @note because of uint32_t in rtc data, data will lose sign 
     * 
     * @param  data - data that will be converted
     * @param  decimal_number - number of decimal places that will be saved
     * @return rounded data as uint32_t
     */
    const uint32_t FloatToU32(float data, uint8_t decimal_number)
    {
        if(data < 0.0)
        {
            data *= -1;
        }

        while(decimal_number--)
        {
            data *= 10;
        }

        return static_cast<uint32_t>(data);
    }

    /**
     * @brief  Converts Double to uint32_t
     * @note  This function will not work properly if double multiplied by 10^decimal_number will exceed uint32_t
     *  For now has been left - propably whole function won't be needed
     * @note because of uint32_t in rtc data, data will lose sign (Will be fixed in future)
     * 
     * @param  data - data that will be converted
     * @param  decimal_number - number of decimal places that will be saved
     * @return rounded data as uint32_t
     */
    const uint32_t DoubleToU32(double data, uint16_t decimal_number)
    {
        if(data < 0.0)
        {
            data *= -1;
        }

        while(decimal_number--)
        {
            data *= 10;
        }

        return static_cast<uint32_t>(data);
    }

    /**
     * @brief  Merge two uint16_t to uint32_t
     * 
     * @param  data1 - data that will be converted
     * @param  data2 - second data to convert
     * @return rounded data as uint32_t
     */
    const uint32_t U16x2ToU32(uint16_t data1, uint16_t data2)
    {
        uint32_t result(data1 << 16);
        
        return result |= data2;
    }
}//namespace rtc
