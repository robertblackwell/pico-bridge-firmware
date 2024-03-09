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
            transport::buffer::sb_json_add(buffer_h, " \"ss\":%ld", sample_ptr->s_saved_interrupt_count);
            transport::buffer::sb_json_add(buffer_h, ", \"t0\":%llu", sample_ptr->s_starttime_us);
            transport::buffer::sb_json_add(buffer_h, ", \"t1\":%llu", sample_ptr->s_endtime_us);
            transport::buffer::sb_json_add(buffer_h, ", \"et\":%llu", sample_ptr->s_elapsed_usecs);
            transport::buffer::sb_json_add(buffer_h, ", \"ws\":%9.3f", sample_ptr->s_speed_mm_per_second);
            transport::buffer::sb_json_add(buffer_h, ", \"mr\":%9.3f", sample_ptr->s_motor_rpm);
            transport::buffer::sb_json_add(buffer_h, ", \"ps\":%d", (uint8_t)sample_ptr->s_pin_state[0]);
        transport::buffer::sb_json_add(buffer_h, "}");

	} else {
        transport::buffer::sb_json_add(buffer_h, "{");
            // transport::buffer::sb_json_add(buffer_h, "\"n\":%p", (void*)sample_ptr);
            transport::buffer::sb_json_add(buffer_h, " \"ss\":%ld", 0);
            transport::buffer::sb_json_add(buffer_h, ", \"t0\":%ld", 123456789);
            transport::buffer::sb_json_add(buffer_h, ", \"t1\":%ld", 123456789);
            transport::buffer::sb_json_add(buffer_h, ", \"et\":%ld", 123456789);
            transport::buffer::sb_json_add(buffer_h, ", \"ws\":%9.3f", 0.0);
            transport::buffer::sb_json_add(buffer_h, ", \"mr\":%9.3f", 0.0);
            transport::buffer::sb_json_add(buffer_h, ", \"ps\":%d", (uint8_t)'F');
        transport::buffer::sb_json_add(buffer_h, "}");
	}
	// print_fmt("tojson_one_encoder_sample after and returning if\n");
}
