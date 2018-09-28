
#ifndef _PARSE_H_
#define _PARSE_H_


#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "json.h"

#define OTHER         1
#define OPEN_BRACE    2
#define CLOSE_BRACE   3
#define DOUBLE_QUOTE  4
#define COLON         5
#define OPEN_BRACKET  6
#define CLOSE_BRACKET 7
#define SINGLE_QUOTE  8
#define BACK_SLASH    9
#define COMMA        10
#define WHITESPACE   11
#define DIGIT        12
#define DOT          13
#define EXPONENT     14
#define PLUS_MINUS   15

typedef unsigned char TokType;
typedef struct Tok {
  int   seek;
  unsigned char  count;
  unsigned short line;
  unsigned short column;
  TokType type : 4;
  struct Tok *previous;
} Tok;

#ifndef TOK_BUF_SIZE
#define TOK_BUF_SIZE 8192
#endif

typedef struct Parser {
    FILE *file;
    Tok *cur;
    Tok *first;
    unsigned int error;
    Tok *error_tok;
    char* error_message;
    const char* error_in_file;
    int error_on_line;
    int buf_seek;
    char buf[TOK_BUF_SIZE];
    char eof : 1;
} Parser;

TokType     tokType(const char c);
Tok*        ffirst(Parser *p);
Tok*        next(Tok *last, Parser *p);
const char *strTokType(Tok *tok);
void        printTok(Tok *tok);
int         isTerm(Parser *p, const char *term);

// forward declarations
char        getCharAt(Parser *p, int index);
const char* getStrBetween(Parser *p, Tok *start, Tok *end);
const char* getnStrBetween(Parser *p, Tok *start, Tok *end, int count);

void        jsonRewind(Parser *p, int saved);
void        jsonExpectPairSeparator(Parser *p);
void        jsonRead(char *buf, Parser *p, int seek, int count);

void*       expectPairSeparator(Tok *start);

void        jsonSetParserError(Parser *p, unsigned int, const char* err, const char *file, int ln);

JValue*     jsonParseArray(Parser *p);
JValue*     jsonParseObject(Parser *p);
void        jsonParseMembers(Parser *p, JObject *obj);
const char* jsonParseQuotedString(Parser* parser, char quote);
JValue*     jsonParseString(Parser *p);
JValue*     jsonParseBool(Parser *p);
JValue*     jsonParseNumber(Parser *p);
JValue*     jsonParseValue(Parser *startAt);

void        jsonPrintParserInfo();
void        consumeWhitespace(Parser *p);
void        consume(Parser *p);

#endif
