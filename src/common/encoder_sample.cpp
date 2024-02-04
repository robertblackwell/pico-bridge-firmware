#include "encoder_sample.h"
#include "trace.h"

void tojson_two_encoder_samples(StaticBuffers::Handle buffer_h, EncoderSample& left_sample, EncoderSample& right_sample)
{
	StaticBuffers::sb_sprintf(buffer_h, "[");
	tojson_one_encoder_sample(buffer_h, left_sample);
	StaticBuffers::sb_sprintf(buffer_h, ",");
	tojson_one_encoder_sample(buffer_h, right_sample);
	StaticBuffers::sb_sprintf(buffer_h, "]");
	FTRACE("buffer length : %d\n", strlen(buffer));
}
void tojson_one_encoder_sample(StaticBuffers::Handle buffer_h, EncoderSample& sample)
{
	size_t len;
	const char* fmt = "{'name': '%s', 'timestamp':%lu,'musecs_per_interrupt':%9.3f,'motor_rpm': %6.3f,'wheel_rpm': %.3f, 'speed_mm_sec': %f}" ;
	const char* fmt2 = "{'n': '%s','ts':%lu,'miq':%.3f,'mr':%9.3f,'wr':%6.3f,'sd':%.3f, 'ps':'%s'}" ;
	const char* fmt3 = "{'n': '%s','ts':%lu,'miq':%.3f,'mr':%9.3f,'wr':%6.3f,'sd':%.3f, 'ps':'F'}" ;

	if(sample.s_contains_data) {
		StaticBuffers::sb_sprintf(buffer_h, fmt2
			,sample.s_name 
			,sample.s_timestamp_musecs 
			// ,sample.s_pin_state, 
			,(double)sample.s_musecs_per_interrupt
			,(double)sample.s_motor_rpm
			,(double)sample.s_wheel_rpm 
			,(double)sample.s_speed_mm_per_second
			,sample.s_pin_state
		);
	} else {
		StaticBuffers::sb_sprintf(buffer_h, fmt3
			,"xx" 
			,1234567.9876//micros()
			,(double)0.00
			,(double)0.0
			,(double)0.0 
			,(double)0.0
			,"F"
		);
	}
}
