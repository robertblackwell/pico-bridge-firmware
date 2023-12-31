#ifndef h_log_h
#define h_log_h
// #include "Arduino.h"

#if 0
template<typename T>
void fun(T& arg)
{
    Serial.print(arg);
}
template <>
void fun<void*>(void* & arg )
{
    Serial.print((long)arg, HEX);
}
#endif

// void format_types(std::ostringstream& os);
template <typename T, typename... Types>
void log_print(const T& firstArg)
{
    #ifdef USE_FUN
        fun(firstArg);
    #else
        Serial.print(firstArg);
    #endif
}

template <typename T, typename... Types>
void log_print(const T& firstArg, const Types&... args)
{
    #ifdef USE_FUN
        fun(firstArg);
    #else
        Serial.print(firstArg);
    #endif
    log_print(args...);
}


#endif