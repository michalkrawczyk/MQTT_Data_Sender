#ifndef DEV_SETTINGS_H
#define DEV_SETTINGS_H

#ifdef UINT_RTC     //RTC_MEMORY_TYPE
    typedef uint64_t RTC_mem_t;
#elif INT_RTC
    typedef long long int RTC_mem_t;
#else
    typedef double RTC_mem_t;
#endif //RTC_MEMORY_TYPE


#endif //DEV_SETTINGS