#ifndef H_serial_writer_h
#define H_serial_writer_h
#include <Arduino.h>

#define SERIALWRITER_BUFFER_SIZE 100


class SerialWriter
{
    public:
    HardwareSerial* m_serial_ptr;
    char m_buffer[SERIALWRITER_BUFFER_SIZE];
    int m_size;
    int m_next;
    SerialWriter(HardwareSerial* serial);

    bool availableForWrite()
    {
        return (m_size == 0);
    }
    void run()
    {
        int n;
        if((m_size > 0) &&((n = m_serial_ptr->availableForWrite()) > 0)) {
            size_t howmany = m_serial_ptr->write(&m_buffer[m_next], n);
        }
    }
};


#endif