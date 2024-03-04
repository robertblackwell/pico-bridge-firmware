#include <stdio.h>
#include <stdlib.h>

#undef FTRACE_ON
#undef FDEBUG_ON
#include "trace.h"
#include "argv.h"


bool tokenize_line(const char* line, Argv& argv)
{
    FDEBUG("tokenize_line entered [%s]\n", line);
    if(strlen(line) > ARGV_BUFFER_MAX) {
        printf("tokenize error line is too long");
        return false;
    }
    // ASSERT_MSG((strlen(line) < ARGV_BUFFER_MAX), "tokenize input line is too big")
    strcpy(&(argv.m_line_buffer[0]), line);
    strcpy(&(argv.m_unmodified_line_buffer[0]), line);

    FDEBUG("tokenize_line copy line [%s]\n", argv.m_line_buffer);
    FDEBUG("tokenize_line argv.m_line_buffer %p [%s]  line %p [%s]\n", argv.m_line_buffer, argv.m_line_buffer, line, line);
    argv.token_count = 0;
    char* buf = argv.m_line_buffer;
    char* tk = strtok(buf, " ");
    
    while(tk != NULL) {
        if(strcmp(tk, " ") != 0) {
            argv.token_positions[argv.token_count] = tk;
            argv.token_lengths[argv.token_count] = strlen(tk);
            argv.token_count++;
            if(argv.token_count >= ARGV_TOKENS_MAX) {
                printf("tokenize too many tokens");
                return false;
            }
            argv.token_positions[argv.token_count] = nullptr;
            argv.token_lengths[argv.token_count] = 0;

        }
        tk = strtok(NULL, " ");
    } 
    FDUMP_TOKENS((argv), "tokenize ");
    FDEBUG("tokenize_line argv.line.buffer %p [%s]  line.buffer %p [%s]\n", argv.m_line_buffer, argv.m_line_buffer, line, line);
    return true;
}
Argv::Argv()
{
    token_count = 0;
    m_line_buffer[0] = '\0';
    m_unmodified_line_buffer[0] = '\0';
}
Argv::Argv(const char* line) 
{
    token_count = 0;
    m_line_buffer[0] = '\0';
    m_unmodified_line_buffer[0] = '\0';
    tokenize_line(line, *this);
}
Argv::Argv(transport::buffer::Handle bh)
{
    token_count = 0;
    m_line_buffer[0] = '\0';
    m_unmodified_line_buffer[0] = '\0';
    tokenize_line(transport::buffer::sb_buffer_as_cstr(bh), *this);
}
bool Argv::tokenize(const char* line)
{
    return tokenize_line(line, *this);
}
void Argv::dump(const char* msg)
{
    print_fmt("Dump Argv msg:%s  tokens: %p token_count: %d\n", msg, (this), this->token_count);
    for(int i = 0; i < this->token_count; i++) {
        print_fmt("i: %d  token: %p[%s] len: %d\n", i, this->token_positions[i], this->token_positions[i], this->token_lengths[i]);
    }
}

void Argv::copyTo(Argv& other)
{
    FDUMP_TOKENS(*this, "Argv::copyTo entered")
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

const char* Argv::token_at(int i)
{
    ASSERT_PRINTF(((i >= 0) && (i < this->token_count)), "i: %d is out of range token_count: %d\n", i, this->token_count);
    return this->token_positions[i];
}

/*
Functions for parsing argument tokens
*/


bool isposnum(const char* p, int& v) {
    int tmp = 0;
    FTRACE("isposnum %s\n", p);
    const char* pp = p;
    while(*pp != (char)0) {
        if(!isdigit(*pp)) {
            FTRACE("isposnum %s %d\n", p, 0);
            return false;
        }
        tmp = tmp * 10 + (int)(*pp - '0');
        pp++;
    }
    FTRACE("isposnum %s %d\n", p, 1);
    v = tmp;
    return true;
}
bool isnegnum(const char* p, int& v) {
    int tmp;
    if(*p == '-') {
        if(isposnum(&p[1], tmp)) {
            v = -1 * tmp;
            return true;
        }
    }
    return false;
}

bool isdouble(const char* p, double& fv) {
    
    const char* q = p;
    if(*q == '\0') {
        return false;
    }
    if(*q == '-') {
        q++;
    }
    if(*q == '\0') {
        return false;
    }
    while(isdigit(*q)) {
        q++;
    }
    if(*q == '\0') {
        fv = atof(p);
        return true;
    } else if(*q == '.') {
        q++;
    } else {
        return  false;
    }
    while(isdigit(*q)) {
        q++;
    }
    if(*q != '\0') {
        return false;
    }
    fv = atof(p);
    return true;
}
bool argparse_posint(Argv& args, int at, int& return_value) 
{
    return isposnum(args.token_at(at), return_value);
}
bool argparse_negint(Argv& args, int at, int& return_value) 
{
    return isnegnum(args.token_at(at), return_value);
}
bool argparse_double(Argv& args, int at, double& dv) 
{
    return isdouble(args.token_at(at), dv);
}
