#ifdef ARDUINO
#include <Arduino.h>
#endif

#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <stdint.h>
#include "utils.h"

#define PRINT_FMT_BUFLEN 512
static char str_buf[PRINT_FMT_BUFLEN];

void print_fmt(const char* fmt, ...) {
    va_list(args);
    va_start(args, fmt);
    int len = vsnprintf(str_buf, PRINT_FMT_BUFLEN, fmt, args);
    ASSERT_MSG((len < PRINT_FMT_BUFLEN), "print_fmt buffer overflow\n");
//    if(len >= BUFLEN) {
//        while(1) {
//            Serial.println("print_fmt buffer overflow");
//            delay(5000);
//        }
//    }
#ifdef ARDUINO
    for(int i = 0; i < len; i++) {
        while(Serial.availableForWrite() <= 0) {delayMicroseconds(100);}
        Serial.print(str_buf[i]);
    }
//    Serial.print(str_buf, len);
#else
    printf("%.*s", len, str_buf);
#endif
    va_end(args);
}
void mshed_dump_hex(const char* m, uint8_t* buf, uint16_t len ) {

    print_fmt("%s addr: %p length : %d  0x", m, buf, len);
    for(uint16_t i = 0; i < len; i++) {
        print_fmt("%2.2X", (uint8_t)buf[i]);
    }
    print_fmt("\n");
}

void mdb_print_uint16_as_hex(const char* msg, uint16_t v) {
    uint8_t  lb = mshed_lowByte(v);
    uint8_t  hb = mshed_highByte(v);
    print_fmt("%s as uint16_t 0x%4.4X lowbyte 0x%2.2X highbyte 0x%2.2X \n", msg, v, lb, hb);
}
void mdb_print_uint32_as_hex(const char* msg, uint32_t v) {
    print_fmt("%s as uint32_t 0x%8.8X  \n", msg, v);
    mshed_dump_hex("as memory ", (uint8_t*)&v, 4);
}

char* bits2string(uint8_t bits) {
    static char buf[20];
    char* p = buf;
    strcpy(p, "0b");
    int count = 0;
    uint8_t  mask = 0b10000000;
    for (mask = 0b1000000; mask > 0; mask >>= 1) {
        // auto x = ((bits & mask) == mask) ? "1" : "0";
        if(count % 4 == 0) {
            strcat(buf, " ");
        }
        strcat(buf, ((bits & mask) != 0) ? "1" : "0");
        count++;
    }
    return buf;
}
char* bits2string(uint16_t bits) {
    static char buf[40];
    char* p = buf;
    strcpy(p, "0b");
    int count = 0;
    uint16_t  mask = (unsigned short)0x8000; //0b10000000000000000;
    for (; mask > 0; mask >>= 1) {
        // auto x = ((bits & mask) == mask) ? "1" : "0";
        if(count % 4 == 0) {
            strcat(buf, " ");
        }
        strcat(buf, ((bits & mask) != 0) ? "1" : "0");
        count++;
    }
    return buf;
}
// printFloat prints out the float 'value' rounded to 'places' places after the decimal point

// void printFloat(float value, int places) {
//   // this is used to cast digits
//     int digit;
//     float tens = 0.1;
//     int tenscount = 0;
//     int i;
//     float tempfloat = value;

//     // make sure we round properly. this could use pow from <math.h>, but doesn't seem worth the import
//     // if this rounding step isn't here, the value  54.321 prints as 54.3209

//     // calculate rounding term d:   0.5/pow(10,places)  
//     float d = 0.5;
//     if (value < 0)
//         d *= -1.0;
//         // divide by ten for each decimal place
//     for (i = 0; i < places; i++)
//         d/= 10.0;    
//     // this small addition, combined with truncation will round our values properly
//     tempfloat +=  d;

//     // first get value tens to be the large power of ten less than value
//     // tenscount isn't necessary but it would be useful if you wanted to know after this how many chars the number will take

//     if (value < 0)
//         tempfloat *= -1.0;
//     while ((tens * 10.0) <= tempfloat) {
//         tens *= 10.0;
//         tenscount += 1;
//     }


//     // write out the negative if needed
//     if (value < 0)
//         Serial.print('-');

//     if (tenscount == 0)
//         Serial.print(0, DEC);

//     for (i=0; i< tenscount; i++) {
//         digit = (int) (tempfloat/tens);
//         Serial.print(digit, DEC);
//         tempfloat = tempfloat - ((float)digit * tens);
//         tens /= 10.0;
//     }

//     // if no places after decimal, stop now and return
//     if (places <= 0)
//         return;

//     // otherwise, write the point and continue on
//     Serial.print('.');  

//     // now write out each decimal place by shifting digits one by one into the ones place and writing the truncated value
//     for (i = 0; i < places; i++) {
//         tempfloat *= 10.0;
//         digit = (int) tempfloat;
//         Serial.print(digit,DEC);  
//         // once written, subtract off that digit
//         tempfloat = tempfloat - (float) digit;
//     }
// }
char* floa2str(float f)
{
    static char buffer[40];
    // char* p = buffer;
    int nbefore = (long)(f);
    // int after2 = ((long)(f * 100.0) % 100);
    sprintf(buffer, "%d", nbefore);
    return buffer;
}
