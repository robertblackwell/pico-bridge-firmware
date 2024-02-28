#ifndef H_cli_argv_h
#define H_cli_argv_h
#include "transport/buffers.h"
#define ARGV_BUFFER_MAX 256
#define ARGV_TOKENS_MAX 10  // the max number of tokens - max value of argc

/**
 * This is the equivalent of argc argc. Its a list of tokens
 * derived from an input cimmand line
 */
struct Argv
{
    Argv();
    Argv(const char* line);
    Argv(transport::buffer::Handle bh);
    void  copyTo(Argv& other);
    void tokenize(const char* line);

    const char* token_at(int i);

    void  dump(const char* msg);

    int   token_count;
    char* token_positions[ARGV_TOKENS_MAX];
    int   token_lengths[ARGV_TOKENS_MAX];
    /**
     * A copy of the original input line - is modified during the tokenization process
     * this class has its own copy to make it standalone
     */
    char  m_line_buffer[ARGV_BUFFER_MAX];
    /**
     * An unmodified copy of the original line
    */
    char  m_unmodified_line_buffer[ARGV_BUFFER_MAX];

};
bool argparse_posint(Argv& args, int at, int& return_value); 
bool argparse_negint(Argv& args, int at, int& return_value);
bool argparse_double(Argv& args, int at, double& dv); 

#endif