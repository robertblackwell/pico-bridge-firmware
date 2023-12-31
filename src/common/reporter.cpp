#include <stdio.h>
#include <transport.h>
#include <utils.h>
#include "reporter.h"

int sprintf_sample(char* buffer, EncoderSample* sample_ptr);
void report_both_samples(EncoderSample* left_sample_ptr, EncoderSample* right_sample_ptr);

Reporter::Reporter(){}
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
	if((m_count < m_number_required) || (m_number_required < 0)) {
		FTRACE("reporter::run m_number_required: %d m_count: %d\n", m_number_required, m_count);
		bool got_some = false;
		EncoderSample left_sample;
		EncoderSample right_sample;
		m_encoder_left_ptr->run();
		m_encoder_right_ptr->run();
		if(m_encoder_left_ptr->available()) {
			FTRACE("Reporter.run - got a left sample\n"," ")
			m_encoder_left_ptr->consume(left_sample);
			got_some = true;
		}
		if(m_encoder_right_ptr->available()) {
			FTRACE("Reporter.run - got a right sample\n", " ")
			m_encoder_right_ptr->consume(right_sample);
			got_some = true;
		}
		report_both_samples(&left_sample, &right_sample);
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
	transport_send(buffer, len);
}
int sprintf_sample(char* buffer, EncoderSample* sample_ptr)
{
	int len;
	const char* fmt = "{'name': '%s', 'timestamp':%lu,'musecs_per_interrupt':%9.3f,'motor_rpm': %6.3f,'wheel_rpm': %.3f, 'speed_mm_sec': %f}" ;
	const char* fmt2 = "{'n': '%s','ts':%lu,'miq':%.3f,'mr':%9.3f,'wr':%6.3f,'sd':%.3f}" ;
	const char* fmt3 = "{'n': '%s','ts':%lu,'miq':%.3f,'mr':%9.3f,'wr':%6.3f,'sd':%.3f}" ;

	if(sample_ptr->s_contains_data) {
		len = sprintf(buffer, fmt2
			,sample_ptr->s_name 
			,sample_ptr->s_timestamp_musecs 
			// ,sample.s_pin_state, 
			,(double)sample_ptr->s_musecs_per_interrupt
			,(double)sample_ptr->s_motor_rpm
			,(double)sample_ptr->s_wheel_rpm 
			,(double)sample_ptr->s_speed_mm_per_second
		);
	} else {
		len = sprintf(buffer, fmt2
			,"" 
			,micros()
			,(double)0.00
			,(double)0.0
			,(double)0.0 
			,(double)0.0
		);
	}
	return len;
	// printf("sample: \n%s", buffer);
	// elapsed = millis() - bms;
	// Serial.print("Elapsed time for this report ");Serial.println(elapsed);
}
/**
 * Requests the reporter object to produce 'number' consecutive
 * sample reports
*/
void Reporter::request(int number)
{
	m_number_required = number;
	m_count = 0;
}