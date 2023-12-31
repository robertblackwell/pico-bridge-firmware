#ifdef ARDUINO
// #include <Arduino.h>
#else
#include <stdio.h>
#include "pico/stdlib.h"
#include <pico/error.h>
#endif
#undef FTRACE_ON
#include "utils.h"
#include "cli.h"

#define CLI_STATE_START 1
#define CLI_STATE_READING_LINE 2
#define CLI_STATE_ARGS_AVAILABLE 3
#define CLI_STATE_LINE_AVAILABLE 4

#define CLI_STATE_NEED_ARGS 4

static bool get_char_if_available(int* char_received) {
    int ch = getchar_timeout_us(0);
    if(ch == PICO_ERROR_TIMEOUT) {
        return false;
    }
    *char_received = ch;
    return true;
}
char static tolower(char c) {
    return ((c >= 'A') && (c <= 'Z')) ? (char)(c - 'A' + 'a'): c;
}


void tokenize_line(char* line, Argv& argv);

Cli::Cli(): m_argv(Argv())
{
    m_state = CLI_STATE_START;
    m_next_pos = 0;
    m_chars_available = false;
}
void Cli::begin()
{
}
void Cli::run()
{
    if(m_state == CLI_STATE_ARGS_AVAILABLE)
        return;
    // printf("cli::run test ch available\n");
    int chin;
    char ch;
    while(get_char_if_available(&chin)) {
        m_state = CLI_STATE_READING_LINE;
        ch = tolower(chin);
        if((ch == '\n')|| (ch == '\r')) {
            m_buffer[m_next_pos] = (char)0;
            m_state = CLI_STATE_LINE_AVAILABLE;
            tokenize_line(m_buffer, m_argv);
            m_next_pos = 0;
            m_buffer[0] = (char)0;
            m_state = CLI_STATE_ARGS_AVAILABLE;
            // could call a function here to execute the command
            // or schedule a call back with the command as an argument
            return;
        } else {
            FTRACE("cli::run ch: %c\n", ch);
            m_buffer[m_next_pos] = ch;
            m_next_pos++;
            ASSERT_MSG((m_next_pos < CLI_BUFFER_MAX), "input line too long in cli::run")
            m_buffer[m_next_pos] = (char)0;
        }
    }
    return;
}
bool Cli::available()
{
    return (m_state == CLI_STATE_ARGS_AVAILABLE);
}
void Cli::consume(Argv& argv)
{
    FTRACE("Cli::consume entered\n","");
    FDUMP_TOKENS((this->m_argv), "In Cli::consume this->m_argv");
    m_state = CLI_STATE_START;
    m_next_pos = 0;
    m_buffer[0] = '\0';
    m_argv.copyTo(argv);
    FTRACE("Cli::consume exit\n","");
}
Argv& Cli::consume()
{
    FTRACE("Cli::consume entered\n","");
    FDUMP_TOKENS((this->m_argv), "In Cli::consume this->m_argv");
    m_state = CLI_STATE_START;
    m_next_pos = 0;
    m_buffer[0] = '\0';
    return m_argv;
    FTRACE("Cli::consume exit\n","");
}

void tokenize_line(char* line, Argv& argv)
{
    FDEBUG("tokenize_line entered [%s\n", line.buffer);
    ASSERT_MSG((strlen(line) < CLI_BUFFER_MAX), "tokenize input line is too big")
    strcpy(argv.m_line_buffer, line);

    FDEBUG("tokenize_line copy line [%s]\n", this->line.buffer);
    FDEBUG("tokenize_line argv.line.buffer %p [%s]  line.buffer %p [%s]\n", this->line.buffer, this->line.buffer, line.buffer, line.buffer);
    argv.token_count = 0;
    char* buf = argv.m_line_buffer;
    char* tk = strtok(buf, " ");
    
    while(tk != NULL) {
        if(strcmp(tk, " ") != 0) {
            argv.token_positions[argv.token_count] = tk;
            argv.token_lengths[argv.token_count] = strlen(tk);
            argv.token_count++;
        }
        tk = strtok(NULL, " ");
    } 
    FDUMP_TOKENS((*this), "tokenize ");
    FDEBUG("tokenize_line argv.line.buffer %p [%s]  line.buffer %p [%s]\n", this->line.buffer, this->line.buffer, line.buffer, line.buffer);
}
void Argv::dump(const char* msg)
{
    printf("Dump Argv msg:%s  tokens: %p token_count: %d\n", msg, (this), this->token_count);
    for(int i = 0; i < this->token_count; i++) {
        printf("i: %d  token: %p[%s] len: %d\n", i, this->token_positions[i], this->token_positions[i], this->token_lengths[i]);
    }
}

void Argv::copyTo(Argv& other)
{
    FDUMP_TOKENS(this, "Argv::copyTo entered")
    other.token_count = this->token_count;
    char* src_origin = &(this->m_line_buffer[0]);
    char* dest_origin = &(other.m_line_buffer[0]);

    // this->line.copyTo(other.line);

    for(int i = 0; i < this->token_count; i++) {
        FDEBUG("source count: %d target count %d\n", this->token_count, other.token_count);
        char* p_src = this->token_positions[i];
        char* p_dest = dest_origin + (p_src - src_origin);
        other.token_positions[i] = p_dest;
        other.token_lengths[i] = this->token_lengths[i];
        strcpy(other.token_positions[i], this->token_positions[i]);
        FDEBUG("source count: %d target count %d\n", this->token_count, other.token_count);
        FDEBUG("Argv::copyTo src %p[%s] dest %p[%s]\n", p_src, p_dest, this->token_positions[i], other.token_positions[i]);

        ASSERT_PRINTF((strcmp(p_src, p_dest) == 0), "Argv::copyTo strcmp failed %p[%s] %p[%s]\n", p_src, p_dest, this->token_positions[i], other.token_positions[i]);
        ASSERT_PRINTF((strncmp(p_src, p_dest, this->token_lengths[i]) == 0), "Argv::copyTo strncmp failed %p[%s] %p[%s]\n", p_src, p_dest, this->token_positions[i], other.token_positions[i]);
    }
    FDEBUG("Argv::copyTo src count %d target count : %d \n", this->token_count, other.token_count);
    FDUMP_TOKENS(other, "Argv::copyTo other at exit");
    FDUMP_TOKENS(*this, "Argv::copyTo this at exit");
}

char* Argv::token_at(int i)
{
    ASSERT_PRINTF(((i >= 0) && (i < this->token_count)), "i: %d is out of range token_count: %d\n", i, this->token_count);
    return this->token_positions[i];
}

