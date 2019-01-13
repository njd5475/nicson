/*
 * parse.c
 *
 *  Created on: Mar 30, 2018
 *      Author: nick
 */

#include "parse.h"

#include <math.h>
#include <limits.h>
#include <float.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "json.h"

//safety
#ifdef TRACK_ALLOCS
#define malloc(p) \
  malloc((printf("Allocating %.0f bytes, in %s at %d\n", (float)p, __FILE__, __LINE__) * 0)+ p)
#endif

#define NOT_IMPLEMENTED(p)   jsonSetParserError(p, 42, "parseArray: Not Implemented", __FILE__, __LINE__);
#define UNEXPECTED_TOKEN(p)  jsonSetParserError(p, 43, "Unexpected Token", __FILE__, __LINE__);
#define BAD_CHARACTER(p)     jsonSetParserError(p, 99, "Could not read first character", __FILE__, __LINE__);

void jsonSetParserError(Parser *p, unsigned int errNo, const char *msg,
    const char *file, int ln) {
  p->error = errNo;
  p->error_tok = p->cur;
  p->error_in_file = file;
  p->error_on_line = ln;
  if(p->error_message) {
    free(p->error_message);
  }
  p->error_message = strdup(msg);
}

void jsonPrintError(Parser *p) {
  fflush(stdout);
  if(p->error_tok) {
    char *token = (char*)malloc(sizeof(char)*p->error_tok->count+1);
    memset(token, 0, p->error_tok->count+1);
    jsonRead(token, p, p->error_tok->seek, p->error_tok->count);
    fprintf(stderr, "Parse error %s(%d): %s, for %s (%s), at ln %d, col %d\n",
        p->error_in_file, p->error_on_line, p->error_message,
        strTokType(p->error_tok), token, p->error_tok->line, p->error_tok->column);
    free(token);
  } else {
    fprintf(stderr, "Parse error: Unexpected end of file!\n");
  }
}

Tok *ffirst(Parser *p) {
  Tok *tok = (Tok*) malloc(sizeof(Tok));
  memset(tok, 0, sizeof(Tok));
  tok->seek = 0;
  tok->count = 0;
  tok->line = 1;
  tok->column = 0;
  p->cur = tok;
  return next(p);
}

Tok *next(Parser *p) {
  if(p->eof) {
    return 0;
  }
  p->cur->seek += p->cur->count; //advance the pointer
  p->cur->count = 0;
  char buf = '\0';
  jsonRead(&buf, p, p->cur->seek + p->cur->count, 1); //read one character
  if(buf == '\0') {
    p->eof = 1;
    return 0;
  }
  p->cur->type = tokType(buf);
  TokType nextType;
  do {
    p->cur->count++;
    p->cur->column++;
    jsonRead(&buf, p, p->cur->seek + p->cur->count, 1); //read one character
    nextType = tokType(buf);
  }while(p->cur->type == nextType && (p->cur->type == OTHER || p->cur->type == WHITESPACE || p->cur->type == NEWLINE));

  if(p->cur->type == NEWLINE) {
    p->cur->column = 0;
    p->cur->line++;
  }

  return p->cur;
}

char *getTerm(Parser *p) {
  Tok *t = NULL;
  t = p->cur;
  char *toRet = malloc(sizeof(char)*t->count+1);
  memset(toRet, 0, t->count+1);
  jsonRead(toRet, p, t->seek, t->count);
  return toRet;
}

TokType tokType(const char c) {
  if(c == '{') {
    return OPEN_BRACE;
  } else if(c == '}') {
    return CLOSE_BRACE;
  } else if(c == '"') {
    return DOUBLE_QUOTE;
  } else if(c == '\'') {
    return SINGLE_QUOTE;
  } else if(c == ':') {
    return COLON;
  } else if(c == '\\') {
    return BACK_SLASH;
  } else if(c == '[') {
    return OPEN_BRACKET;
  } else if(c == ']') {
    return CLOSE_BRACKET;
  } else if(c == ',') {
    return COMMA;
  } else if(c == ' ' || c == '\t') {
    return WHITESPACE;
  } else if(c == '\n' || c == '\r') {
    return NEWLINE;
  } else if(c >= '0' && c <= '9') {
    return DIGIT;
  } else if(c == '.') {
    return DOT;
  } else if(c == '+' || c == '-') {
    return PLUS_MINUS;
  }
  return OTHER;
}

const char *strTokType(Tok *tok) {
  if(tok) {
    if(tok->type == OPEN_BRACE) {
      return "Open Brace";
    } else if(tok->type == CLOSE_BRACE) {
      return "Close Brace";
    } else if(tok->type == DOUBLE_QUOTE) {
      return "Double Quote";
    } else if(tok->type == SINGLE_QUOTE) {
      return "Single Quote";
    } else if(tok->type == COLON) {
      return "Colon";
    } else if(tok->type == BACK_SLASH) {
      return "Back Slash";
    } else if(tok->type == OPEN_BRACKET) {
      return "Open Bracket";
    } else if(tok->type == CLOSE_BRACKET) {
      return "Close Bracket";
    } else if(tok->type == COMMA) {
      return "Comma";
    } else if(tok->type == WHITESPACE) {
      return "Whitespace";
    } else if(tok->type == NEWLINE) {
      return "Newline";
    } else if(tok->type == DOT) {
      return "Dot";
    } else if(tok->type == DIGIT) {
      return "Digit";
    } else if(tok->type == PLUS_MINUS) {
      return "PlusOrMinus Symbol";
    } else {
      return "Other";
    }
  }
  return 0;
}

int sizeOfType(const short type) {
  if(type == VAL_OBJ) {
    return sizeof(JObject);
  }else if(type == VAL_INT) {
    return sizeof(int);
  }else if(type == VAL_UINT) {
    return sizeof(unsigned int);
  }else if(type == VAL_FLOAT) {
    return sizeof(float);
  }else if(type == VAL_DOUBLE) {
    return sizeof(double);
  }else if(type == VAL_BOOL) {
    return sizeof(char);
  }else if(type == VAL_BOOL_ARRAY ||
      type == VAL_INT_ARRAY ||
      type == VAL_DOUBLE_ARRAY ||
      type == VAL_FLOAT_ARRAY ||
      type == VAL_STRING_ARRAY ||
      type == VAL_OBJ_ARRAY) {
    return sizeof(JArray);
  }else{
    return 0;
  }
}

void printTok(Tok *tok) {
  printf("Tok type=%s, count=%d at ln %d, col %d\n", strTokType(tok),
      tok->count, tok->line, tok->column);
}

int isTerm(Parser *p, const char *cterm) {
  if(!p->cur) {
    return 0;
  }

  if(p->cur->type == OTHER) {

    int startPos = p->cur->seek;
    while(p->cur && p->cur->type == OTHER) {
      consume(p);
    }
    if(!p->cur) {
      return 0;
    }
    jsonRewind(p, startPos);

    int cterm_len = strlen(cterm);
    if(cterm_len == p->cur->count) {
      char term[p->cur->count + 1];
      memset(term, 0, p->cur->count + 1);
      jsonRead(term, p, p->cur->seek, p->cur->count);

      if(strcmp(cterm, term) == 0) {
        consume(p);
        return 1;
      }

      jsonRewind(p, startPos);
    }
  }

  return 0;
}

int jsonParseQuotedString(Parser* p, char quote) {
  TokType quoteType = tokType(quote);
  consume(p);
  int start = p->cur->seek;
  while(p->cur->type != quoteType) {
    if(p->cur->type == BACK_SLASH) {
      consume(p);
      consume(p);
    }
    if(p->cur->type != quoteType) {
      consume(p);
    }else{
      break;
    }
  }
  int size = (p->cur->seek-start);
  consume(p);
  return size;
}

JObject *jsonParseObject(Parser *p) {

  if(p->cur->type == OPEN_BRACE) {
    consume(p);
  } else {
    UNEXPECTED_TOKEN(p)
    return 0;
  }
  JObject *obj = jsonNewObject();
  char haveComma = 0;
  do {
    if(haveComma) {
      consume(p);
      haveComma = 0;
    }
    consumeWhitespace(p);
    if(p->error || p->eof) {
      break;
    }
    if(p->cur->type == CLOSE_BRACE) {
      break;
    }
    jsonParseMembers(p, obj);
    if(p->error || p->eof) {
      break;
    }
    consumeWhitespace(p);
    if(p->cur->type == COMMA) {
      haveComma = 1;
    }
  } while(p->cur->type == COMMA);

  if(p->error) {
    jsonFree((JItemValue) { obj }, VAL_OBJ);
    return 0;
  }

  consume(p);

  return obj;
}

void jsonParseMembers(Parser *p, JObject *obj) {
  int start = p->cur->seek+1;
  int size = -1;
  if(p->cur->type == SINGLE_QUOTE) {
    size = jsonParseQuotedString(p, '\'');
  } else if(p->cur->type == DOUBLE_QUOTE) {
    size = jsonParseQuotedString(p, '"');
  }
  jsonExpectPairSeparator(p);
  if(!p->error) {
    short type = 0;
    JItemValue val = jsonParseValue(p, &type);
    if(!p->error) {
      char buf[size+1];
      memset(buf, '\0', size+1);
      jsonRead(buf, p, start, size);
      jsonAddVal(obj, buf, val, type);
    }
  } else {
    jsonPrintError(p);
  }

}

void jsonExpectPairSeparator(Parser *p) {
  consumeWhitespace(p);
  if(p->cur->type == COLON) {
    consume(p);
  } else {
    Tok *tok = p->cur;
    fprintf(stderr, "Unexpected token: %s at ln %d cl %d\n", strTokType(tok),
        tok->line, tok->column);
    UNEXPECTED_TOKEN(p)
  }
  consumeWhitespace(p);
}

JItemValue jsonParseValue(Parser *p, short *type) {
  int saved = p->cur->seek;

  JItemValue val = { 0 };
  if(p->cur->type == SINGLE_QUOTE || p->cur->type == DOUBLE_QUOTE) {
    val.string_val = jsonParseString(p);
    *type = VAL_STRING;
    if(val.string_val && p->error == 0) {
      return val;
    }
    jsonRewind(p, saved);
  } else if(p->cur->type == OPEN_BRACE) {
    val.object_val = jsonParseObject(p);
    *type = VAL_OBJ;

    if(val.object_val && !p->error) {
      return val;
    }

    if(val.object_val) {
      jsonFree(val, VAL_OBJ);
    }

    jsonRewind(p, saved);
  }else if(p->cur->type == OPEN_BRACKET) {
    val.array_val = jsonParseArray(p, type);

    if(val.array_val && !p->error) {
      return val;
    }

    if(val.array_val) {
      jsonFree(val, *type);
    }
    jsonRewind(p, saved);
  }else if(p->cur->type == PLUS_MINUS || p->cur->type == DIGIT || p->cur->type == DOT) {
    val = jsonParseNumber(p, type);

    if((val.ptr_val || type != 0) && !p->error) {
      return val;
    }

    if(val.ptr_val) {
      jsonFree(val, *type);
    }

    jsonRewind(p, saved);
  }else if(p->cur->type == OTHER) {
    val.char_val = jsonParseBool(p, type);

    if((val.ptr_val || *type == VAL_BOOL) && !p->error) {
      return val;
    }

    if(val.ptr_val) {
      jsonFree(val, *type);
    }

    jsonRewind(p, saved);

    if(isTerm(p, "null")) {
      *type = VAL_NULL;
      return (JItemValue) { 0 };
    } else {
      UNEXPECTED_TOKEN(p);
    }
  }

  return (JItemValue) { 0 };
}

char* jsonParseString(Parser *p) {
  Tok *cur = p->cur;
  int start = p->cur->seek+1;
  int size = -1;
  if(cur->type == SINGLE_QUOTE) {
    size = jsonParseQuotedString(p, '\'');
  } else if(cur->type == DOUBLE_QUOTE) {
    size = jsonParseQuotedString(p, '"');
  }

  if(size != -1) {
    char buf[size+1];
    memset(buf, '\0', size+1);
    jsonRead(buf, p, start, size);
    return getOrCacheString(buf);
  }
  return 0;
}

char true = 1;
char false = 0;

char jsonParseBool(Parser *p, short *type) {
  consumeWhitespace(p);
  if(isTerm(p, "true")) {
    *type = VAL_BOOL;
    return true;
  } else if(isTerm(p, "false")) {
    *type = VAL_BOOL;
    return false;
  } else {
    *type = 0;
    UNEXPECTED_TOKEN(p)
  }
  return 0;
}

JItemValue jsonParseNumber(Parser *p, short *type) {
  double value = 0.0L;
  int signValue = 1;
  if(p->cur->type == PLUS_MINUS) {
    signValue = -1;
    consume(p);
  }
  if(p->cur->type == DIGIT) {
    value = signValue * (getCharAt(p, 0) - '0');
    consume(p);

    if(p->cur->type == DIGIT) {
      do {
        for(int i = 0; i < p->cur->count; ++i) {
          value *= 10.0d;
          value = value + signValue * (getCharAt(p, i) - '0');
        }
        consume(p);
      } while(p->cur->type == DIGIT);
    }

    if(p->cur->type == DOT) {
      consume(p);
      double divisor = 10.0d;
      if(p->cur->type == DIGIT) {
        value += ((double)signValue * (double)(getCharAt(p, 0) - '0')) / divisor;
        consume(p);
        if(p->cur->type == DIGIT) {
          do {
            for(int i = 0; i < p->cur->count; ++i) {
              divisor *= 10;
              value = value + ((signValue * (getCharAt(p, i) - '0')) / divisor);
            }
            consume(p);
          } while(p->cur->type == DIGIT);
        }
      } else {
        UNEXPECTED_TOKEN(p);
        return (JItemValue){ 0 };
      }
    }

    if(isTerm(p, "E") || isTerm(p, "e")) {
      int sign = 1;
      if(p->cur->type == PLUS_MINUS) {
        sign = -1;
        consume(p);
      }

      if(p->cur->type == DIGIT) {
        double power = (double)(getCharAt(p, 0) - '0');
        power *= (sign == '-' ? -1.0d : 1.0d);
        consume(p);

        if(p->cur->type == DIGIT) {
          do {
            for(int i = 0; i < p->cur->count; ++i) {
              power *= 10.0d;
              power += sign * (double)(getCharAt(p, i) - '0');
            }
            consume(p);
          } while(p->cur->type == DIGIT);
        }

        value *= pow(10.0d, power);
      } else {
        UNEXPECTED_TOKEN(p);
        return (JItemValue) { 0 };
      }
    }

    //determine if we have a integer, float, or double
    JItemValue retVal = { 0 };
    double absValue = signValue * value;
    if(absValue <= FLT_MAX) {
      //we have a float, check for decimals
      if(absValue <= INT_MAX && !(absValue - floor(absValue) > 0)) {
        //we have an integer
        *type = VAL_INT;
        retVal.int_val = (int)value;
      }else{
        *type = VAL_FLOAT;
        retVal.float_val = (float)value;
      }
    }else{

      // otherwise we have a double
      *type = VAL_DOUBLE;
      retVal.double_val = value;
    }

    return retVal;
  }

  UNEXPECTED_TOKEN(p);
  return (JItemValue) { 0 };
}

void jsonRewind(Parser *p, int saved) {
  if(p->cur == NULL) {
    p->error = 45;
    return;
  }

  if(saved == p->cur->seek) {
    p->error = 0;
    p->error_tok = 0;
    return;
  }

  p->cur->seek = saved;
  if(p->cur->type == NEWLINE) {
    p->cur->column = 0;
    p->cur->line--;
  }else{
    p->cur->column -= p->cur->count;
  }
  p->cur->count = 0;
  p->cur = next(p);

  if(p->cur == NULL) {
    p->error = 45;
    return;
  }

  p->error = 0;
  p->error_tok = 0;
}

JArray* jsonParseArray(Parser *p, short *type) {
  typedef struct ArrayVal {
    JItemValue       val;
    int              type : 4;
    struct ArrayVal* next;
  } ArrayVal;

#define ARRAY_TYPE(t) \
   (t == VAL_INT ? VAL_INT_ARRAY : \
     (t == VAL_FLOAT ? VAL_FLOAT_ARRAY : \
       (t == VAL_DOUBLE ? VAL_DOUBLE_ARRAY : \
         (t == VAL_STRING ? VAL_STRING_ARRAY : \
             (t == VAL_BOOL ? VAL_BOOL_ARRAY : VAL_MIXED_ARRAY)))))

  if(p->cur->type != OPEN_BRACKET) {
    UNEXPECTED_TOKEN(p);
    return 0;
  }

  ArrayVal *curVal = malloc(sizeof(ArrayVal));
  memset(curVal, 0, sizeof(ArrayVal));
  ArrayVal *head = 0;
  head = curVal;

  int count = 0;
  short singleValueType = -1;
  ArrayVal *nextVal = 0;
  do {
    consume(p); //first time consume open bracket then commas
    consumeWhitespace(p);
    if(p->cur->type == CLOSE_BRACKET) {
      break;
    }
    short valType = 0;
    curVal->val = jsonParseValue(p, &valType);
    curVal->type = valType;

    if(curVal == NULL || p->error) {
      while(head != 0) {
        ArrayVal *deletable = head;
        head = head->next;
        free(deletable);
      }
      return 0;
    }

    // could make this a ternary but I won't
    if(singleValueType == -1) {
      singleValueType = ARRAY_TYPE(curVal->type);
    }
    if(singleValueType != ARRAY_TYPE(curVal->type)) {
      // as soon as it's not the same it's mixed
      singleValueType = VAL_MIXED_ARRAY;
    }

    nextVal = malloc(sizeof(ArrayVal));
    memset(nextVal, 0, sizeof(ArrayVal));
    curVal->next = nextVal;
    curVal = nextVal;
    ++count;
    consumeWhitespace(p);
  } while(p->cur->type == COMMA);

  if(p->cur->type != CLOSE_BRACKET) {
    if(curVal) {
      free(curVal);
    }
    UNEXPECTED_TOKEN(p);
    return 0;
  }
  consume(p);

  //Now we know how many we have lets allocate
  JArray *arrayVal = malloc(sizeof(JArray));
  memset(arrayVal, 0, sizeof(JArray));

  if(count == 0) {
    if(curVal) {
      free(curVal);
    }
    arrayVal->count = 0;
    arrayVal->_internal.vItems = 0;
    *type = VAL_MIXED_ARRAY;
    return arrayVal;
  }

  *type = singleValueType;
  if(singleValueType == VAL_MIXED_ARRAY) {
    arrayVal->type = singleValueType;
    arrayVal->count = count;
    arrayVal->_internal.vItems = (JArrayItem*)malloc(sizeof(JArrayItem)*count);

    curVal = head;
    int countDown = count;
    JArrayItem *item = 0;
    while(curVal && countDown > 0) {
      item = &arrayVal->_internal.vItems[countDown-1];
      item->type = curVal->type;
      item->value = curVal->val;
      ArrayVal *toDel = curVal;
      curVal = curVal->next;
      free(toDel);
      countDown--;
    }

  }else if(singleValueType != VAL_MIXED_ARRAY) {
    JItemValue *itemArray = malloc(sizeof(JItemValue) * count);
    curVal = head;
    int countDown = count;
    while(curVal && countDown > 0) {
      itemArray[count-countDown] = curVal->val;
      ArrayVal *toDel = curVal;
      curVal = curVal->next;
      free(toDel);
      countDown--;
    }
    arrayVal->_internal.items = itemArray;
    arrayVal->type = *type;
    arrayVal->count = count;
  }


  return arrayVal;
}

void consumeWhitespace(Parser *p) {
  if(!p->cur) {
    return;
  }

  while(p->cur->type == WHITESPACE || p->cur->type == NEWLINE) {
    p->cur = next(p);
  }
}

void consume(Parser *p) {
  p->cur = next(p);
}

JItemValue jsonParse(const char *filename, short *type) {
  FILE *file = fopen(filename, "r");
  return jsonParseF(file, type);
}

JItemValue jsonParseF(FILE *file, short *type) {
  if(!file) {
    return (JItemValue) { 0 };
  }
  Parser p;
  memset(&p, 0, sizeof(p));
  p.file = file;
  p.buf_seek = -1;
  p.error_message = strdup("Unknown Error");
  Tok *first = p.first = p.cur = ffirst(&p);
  if(p.cur) {
    consumeWhitespace(&p);
    void *val = NULL;
    if(p.cur->type == OPEN_BRACE) {
      *type = VAL_OBJ;
      val = jsonParseObject(&p);
    } else if(p.cur->type == OPEN_BRACKET) {
      val = jsonParseArray(&p, type);
    }

    if(val && !p.error) {
      free(p.error_message);
      free(first);
      fclose(file);
      return (JItemValue) { val };
    } else {
      jsonPrintError(&p);
    }
  }


  BAD_CHARACTER(&p)
  jsonPrintError(&p);
  free(p.error_message);
  free(first);
  fclose(file);
  return (JItemValue) { 0 };
}

char getCharAt(Parser *p, int index) {
  Tok *tok = p->cur;
  if(!tok) {
    return -1;
  }
  char buf;
  int seekPos = tok->seek + index;
  if(seekPos > tok->seek + tok->count) {
    seekPos = tok->seek + tok->count;
  }
  jsonRead(&buf, p, seekPos, 1);
  return buf;
}

void jsonPrintParserInfo() {
  printf("Parser struct size %d\n", (unsigned int) sizeof(Parser));
  printf("Token struct size  %d\n", (unsigned int) sizeof(Tok));
  printf("Array struct size  %d\n", (unsigned int) sizeof(JArray));
  printf("Entry struct size  %d\n", (unsigned int) sizeof(JEntry));
  printf("Object struct size %d\n", (unsigned int) sizeof(JObject));
}

void jsonRead(char *buf, Parser *p, int seek, int count) {
  if(seek + count > p->buf_seek + TOK_BUF_SIZE || p->buf_seek < 0) {
    if(p->buf_seek < 0) {
      p->buf_seek = 0;
    }
    p->buf_seek = seek;
    memset(p->buf, 0, TOK_BUF_SIZE * sizeof(buf[0]));
    fseek(p->file, p->buf_seek, SEEK_SET);
    fread(p->buf, sizeof(buf[0]), TOK_BUF_SIZE * sizeof(buf[0]), p->file);
  } else if(seek + count < p->buf_seek && p->buf_seek > 0) {
    p->buf_seek = (seek / TOK_BUF_SIZE) * TOK_BUF_SIZE;
    memset(p->buf, 0, TOK_BUF_SIZE * sizeof(buf[0]));
    fseek(p->file, p->buf_seek, SEEK_SET);
    fread(p->buf, sizeof(buf[0]), TOK_BUF_SIZE * sizeof(buf[0]), p->file);
  } else if(seek < p->buf_seek && seek + count >= p->buf_seek
      && p->buf_seek > 0) {
    p->buf_seek = seek - TOK_BUF_SIZE / 2;
    memset(p->buf, 0, TOK_BUF_SIZE * sizeof(buf[0]));
    fseek(p->file, p->buf_seek, SEEK_SET);
    fread(p->buf, sizeof(buf[0]), TOK_BUF_SIZE * sizeof(buf[0]), p->file);
  }
  char *pbuf = p->buf;
  memcpy(buf, pbuf + (seek - p->buf_seek), count);
}
