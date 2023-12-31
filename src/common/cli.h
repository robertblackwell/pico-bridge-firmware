#ifndef H_cli_h
#define H_cli_h
// #include "Arduino.h"

#define CLI_BUFFER_MAX 100 // the maximum length of a command line
#define CLI_TOKENS_MAX 10  // the max number of tokens - max value of argc

class CommandBuffer;

/**
 * This is the equivalent of argc argc. Its a list of tokens
 * derived from an input cimmand line
 */
struct Argv
{
    char* token_at(int i);
    void  copyTo(Argv& other);
    void  dump(const char* msg);
    int   token_count;
    char* token_positions[CLI_TOKENS_MAX];
    int   token_lengths[CLI_TOKENS_MAX];
    /**
     * A copy of the original input line - is modified during the tokenization process
     * this class has its own copy to make it standalone
     */
    char  m_line_buffer[CLI_BUFFER_MAX];
};

/**
 * This class:
 *  -   asynchronously reads the input line,
 *  -   tokenizes the input command line
 *  -   and fills in a command structure that holds
 *      the arguments for any command
 */
class Cli
{
    public:
        Cli();
        void begin();
        void run();
        bool available();
        /**
         * This overload results in a copy of thee char* buffer and the token_positions array
         */
        void consume(Argv& argv);
        /**
         * This is the equivalent of returning a pointer except that:
         * -    the pointer cannot be null
         * -    the caller cannot accidentally free/dispose/delete() it
         */
        Argv& consume();
        int             m_state;
        char            m_buffer[CLI_BUFFER_MAX];
        int             m_next_pos;
        Argv            m_argv;
        bool  m_chars_available;
};

#include "commands.h"
#endif