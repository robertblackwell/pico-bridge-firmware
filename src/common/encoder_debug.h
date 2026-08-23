#ifndef H_encoder_debug_h
#define H_encoder_debug_h
#include "encoder_v2.h"
#include "encoder_sample.h"

#ifdef ENCODER_TRACE_ON
void trace_encoder(const char* tag, const Encoder& encoder);
void trace_sample(const char* tag, const EncoderSample& sample);

#define PRINT_ENCODER(tag, encoder) trace_encoder(tag, encoder);
#define PRINT_SAMPLE(tag, sample) trace_sample(tag, sample);
#define PRINT_BEGIN  printf("Begin========================================================================================================\n");
#define PRINT_END    printf("End========================================================================================================\n");

#else
#define PRINT_ENCODER(tag, encoder)
#define PRINT_SAMPLE(tag, sample)
#define PRINT_BEGIN
#define PRINT_END
#endif

#endif