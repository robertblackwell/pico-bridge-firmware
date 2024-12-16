#ifndef H_task_h
#define H_task_h
// #include <Arduino.h>
/**
 * This class provides a rudimentary fixed interval scheduler.
*/
#include <pico_gpio_irq_dispatcher.h>
#include <dri0002.h>
class Taskable {
    public:
    virtual void run() = 0;
};

/**
 * Wrap a Taskable instance or a void(*callable)() and call it every interval_ms milli secs
 * If interval_ms is zero cal the Taskable/caallable every time the Task,run method is called
*/
struct Task {
    long m_interval_ms;
    long m_previous_time_ms;
    bool m_active;
    Taskable* m_taskable;
    void(*m_callable)();

    Task(long interval_ms, Taskable* taskable):m_taskable(taskable) {
        m_interval_ms = interval_ms;
        m_previous_time_ms = millis();
        m_callable = nullptr;
        m_active = true;
    }
    Task(long interval_ms, void(*callable)())
    {
        m_interval_ms = interval_ms;
        m_previous_time_ms = millis();
        m_callable = callable;
        m_taskable = nullptr;
        m_active = true;
    }
    void run()
    {   if(!m_active) {
            return;
        }
        if(m_interval_ms > 0) {
            long ct = millis();
            if((ct - m_previous_time_ms) < m_interval_ms) {
                return;
            } else {
                m_previous_time_ms = ct;
            }
        }
        if(m_taskable != nullptr) {
            m_taskable->run();
        } else if(m_callable != nullptr) {
            m_callable();
        }
    }
    void operator()()
    {
        run();
    }
    void update_interval(int interval_ms )
    {
        m_interval_ms = interval_ms;
    }
    void start()
    {
        m_active = true;
    }
    void suspend()
    {
        m_active = false;
    }
};

#endif