#include "reporter.h"
#include <stdio.h>
#include "transport/buffers.h"
#include "transport/transmit_buffer_pool.h"
#include "transport/transport.h"
#include "trace.h"
#include <robot.h>
int sprintf_sample(char* buffer, EncoderSample* sample_ptr);
void report_both_samples(EncoderSample* left_sample_ptr, EncoderSample* right_sample_ptr);

Reporter::Reporter(){}
Reporter::Reporter(MotionControl& mc)
{
    m_encoder_left_ptr = mc.m_left_encoder_ptr;
    m_encoder_right_ptr = mc.m_right_encoder_ptr;
    m_number_required = 0;
    m_count = 0;
}
Reporter::Reporter(Encoder* encoder_left_ptr, Encoder* encoder_right_ptr)
{
	begin(encoder_left_ptr, encoder_right_ptr);
}
Reporter::Reporter(Encoder& encoder_left, Encoder& encoder_right)
{
	begin(&encoder_left, &encoder_right);
}

void Reporter::begin(Encoder* left, Encoder* right)
{
	m_encoder_left_ptr = left;
	m_encoder_right_ptr = right;
	m_number_required = 0;
	m_count = 0;
}
void Reporter::run()
{

	uint64_t now = to_ms_since_boot(get_absolute_time());
	if(

		(now >= m_interval_ms + m_last_report_time_since_boot_ms) && 
		(
			(m_count == 0) ||
			(m_count > 0) && ((m_count < m_number_required) || (m_number_required < 0))
		)
	) {
		FTRACE("reporter::run m_number_required: %d m_count: %d\n", m_number_required, m_count);
		m_last_report_time_since_boot_ms = now;
        transport::buffer::Handle buffer_h = transport::buffer::tx_pool::allocate();
        Encoder::unsafe_collect_two_encoder_samples(*m_encoder_left_ptr, (m_encoder_left_ptr->m_sample), *m_encoder_right_ptr, m_encoder_right_ptr->m_sample);
        tojson_two_encoder_samples(buffer_h, &m_encoder_left_ptr->m_sample, &m_encoder_right_ptr->m_sample);
        transport::send_json_response(&buffer_h);
		m_count++;
		}
}
void report_both_samples(EncoderSample* left_sample_ptr, EncoderSample* right_sample_ptr)
{
	char buffer[512];
	int len = 0;
	#if 1
	len += sprintf(buffer+len, "[");
	#else
	len += sprintf(buffer+len, "/*AA*/\n[");
	#endif
	// len += sprintf(buffer+len, "{'left':");
	len += sprintf_sample(buffer+len, left_sample_ptr);
	len += sprintf(buffer+len, ",");
	len += sprintf_sample(buffer+len, right_sample_ptr);
	#if 1
	len += sprintf(buffer+len, "]");
	#else
	len += sprintf(buffer+len, "]\n/*BB*/");
	#endif 
	printf("buffer length : %d\n", strlen(buffer));
	// transport_send(buffer, len);
}
#if 0
int sprintf_sample(char* buffer, EncoderSample* sample_ptr)
{
	int len;
	const char* fmt = "{'name': '%s', 'timestamp':%lu,'musecs_per_interrupt':%9.3f,'motor_rpm': %6.3f,'wheel_rpm': %.3f, 'speed_mm_sec': %f}" ;
	const char* fmt2 = "{'n': '%s','ts':%lu,'miq':%.3f,'mr':%9.3f,'wr':%6.3f,'sd':%.3f, 'ps':'%s'}" ;
	const char* fmt3 = "{'n': '%s','ts':%lu,'miq':%.3f,'mr':%9.3f,'wr':%6.3f,'sd':%.3f, 'ps':'F'}" ;

	if(sample_ptr->s_contains_data) {
		len = sprintf(buffer, fmt2
			,sample_ptr->s_name 
			,sample_ptr->s_timestamp_musecs 
			// ,sample.s_pin_state, 
			,(double)sample_ptr->s_musecs_per_interrupt
			,(double)sample_ptr->s_motor_rpm
			,(double)sample_ptr->s_wheel_rpm 
			,(double)sample_ptr->s_speed_mm_per_second
			,sample_ptr->s_pin_state
		);
	} else {
		len = sprintf(buffer, fmt3
			,"xx" 
			,micros()
			,(double)0.00
			,(double)0.0
			,(double)0.0 
			,(double)0.0
			,"F"
		);
	}
	return len;
	// printf("sample: \n%s", buffer);
	// elapsed = millis() - bms;
	// Serial.print("Elapsed time for this report ");Serial.println(elapsed);
}
#endif
/**
 * Requests the reporter object to produce 'number' consecutive
 * sample reports
*/
void Reporter::request(uint interval_ms, uint number)
{
    m_interval_ms = interval_ms;
	m_number_required = number;
	m_count = 0;
}
void Reporter::request(uint interval_ms)
{
    m_interval_ms = interval_ms;
    m_number_required = 0;
    m_count = 0;
}