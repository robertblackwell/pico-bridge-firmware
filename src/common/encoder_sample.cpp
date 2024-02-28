#include "encoder_sample.h"
#include "trace.h"

void tojson_two_encoder_samples(transport::buffer::Handle buffer_h, EncoderSample* left_sample_ptr, EncoderSample* right_sample_ptr)
{
	transport::buffer::sb_sprintf(buffer_h, "[");
	tojson_one_encoder_sample(buffer_h, left_sample_ptr);
	transport::buffer::sb_sprintf(buffer_h, ",");
	tojson_one_encoder_sample(buffer_h, right_sample_ptr);
	transport::buffer::sb_sprintf(buffer_h, "]");
	FTRACE("buffer length : %d\n", strlen(buffer));
	// print_fmt("Leaving tojson_two 6. %s\n", transport::buffer::sb_buffer_as_cstr(buffer_h));

}
void tojson_one_encoder_sample(transport::buffer::Handle buffer_h, EncoderSample* sample_ptr)
{
	size_t len;
	const char* fmt = "{'name': '%s', 'timestamp':%lu,'musecs_per_interrupt':%9.3f,'motor_rpm': %6.3f,'wheel_rpm': %.3f, 'speed_mm_sec': %f}" ;
	const char* fmt2 = "{'n': '%s','ts':%lu,'miq':%.3f,'mr':%9.3f,'wr':%6.3f,'sd':%.3f, 'ps':'%s'}" ;
	const char* fmt3 = "{'n': '%s','ts':%lu,'miq':%.3f,'mr':%9.3f,'wr':%6.3f,'sd':%.3f, 'ps':'F'}" ;
	if(sample_ptr->s_contains_data) {
        transport::buffer::sb_json_add(buffer_h, "{");
            // transport::buffer::sb_json_add(buffer_h, "\"n\":%p", (void*)sample_ptr);
            transport::buffer::sb_json_add(buffer_h, " \"ss\":%ld", sample_ptr->s_sample_sum);
            transport::buffer::sb_json_add(buffer_h, ", \"ts\":%ld", sample_ptr->s_timestamp_musecs);
            transport::buffer::sb_json_add(buffer_h, ", \"mr\":%9.3f", sample_ptr->s_motor_rpm);
            transport::buffer::sb_json_add(buffer_h, ", \"ps\":%d", (uint8_t)sample_ptr->s_pin_state[0]);
        transport::buffer::sb_json_add(buffer_h, "}");

		// tbuf_len += sprintf(&(tbuf[tbuf_len]),       "}");
		// print_fmt("5. %s\n", transport::buffer::sb_buffer_as_cstr(buffer_h));

		// transport::buffer::sb_sprintf(buffer_h, fmt2
		// 	,sample.s_name 
		// 	,sample.s_timestamp_musecs 
		// 	,(double)sample.s_musecs_per_interrupt
		// 	,(double)sample.s_motor_rpm
		// 	,(double)sample.s_wheel_rpm 
		// 	,(double)sample.s_speed_mm_per_second
		// 	,sample.s_pin_state
		// );
	} else {
		print_fmt("tojson_one_encoder_sample NOT contains_data\n");
		transport::buffer::sb_sprintf(buffer_h, fmt3
			,"xx" 
			,1234567.9876//micros()
			,(double)0.00
			,(double)0.0
			,(double)0.0 
			,(double)0.0
			,"F"
		);
	}
	// print_fmt("tojson_one_encoder_sample after and returning if\n");
}
