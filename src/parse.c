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

void jsonSetParserError(Parser *p, unsigned int errNo, const char *msg, const char *file, int ln) {
  p->error = errNo;
  p->error_tok = p->cur;
  p->error_in_file = file;
  p->error_on_line = ln;
  if (p->error_message) {
    free(p->error_message);
  }
  p->error_message = strdup(msg);
}

void jsonPrintError(Parser *p) {
  fflush(stdout);
  fprintf(stderr, "Parse error %s(%d): %s, for %s, at ln %d, col %d\n", p->error_in_file, p->error_on_line,
      p->error_message, strTokType(p->error_tok), p->error_tok->line,
      p->error_tok->column);
}

Tok *first(const char *filename) {
  FILE *file = fopen(filename, "r");
  if (file == 0) {
    fprintf(stderr, "Could not open file %s\n", filename);
    exit(1);
  }
  return ffirst(file);
}

Tok *ffirst(FILE *file) {
  Tok *tok = (Tok*) malloc(sizeof(Tok));
  tok->file = file;
  tok->seek = 0;
  tok->count = 1;
  char ch;
  fread(&ch, 1, 1, tok->file);
  tok->type = tokType(ch);
  tok->previous = 0;
  tok->line = 1;
  tok->column = 0;
  return tok;
}

Tok *next(Tok *last) {
  if (last) {
    if (feof(last->file)) {
      return 0;
    }
    fseek(last->file, last->seek + last->count, SEEK_SET);
    fflush(last->file);
    char buf = '\0';
    fread(&buf, 1, 1, last->file);

    if (last->type == tokType(buf) && last->type == OTHER) {
      last->count++;
      return last;
    } else {
      Tok *next = (Tok*) malloc(sizeof(Tok));
      next->file = last->file;
      next->seek = last->seek + last->count;
      next->count = 1;
      if (buf == '\n') {
        next->line = last->line + 1;
        next->column = 0;
      } else {
        next->line = last->line;
        next->column = last->column + last->count;
      }
      next->type = tokType(buf);
      next->previous = last;
      return next;
    }
  }
  return 0;
}

TokType tokType(const char c) {
  if (c == '{') {
    return OPEN_BRACE;
  } else if (c == '}') {
    return CLOSE_BRACE;
  } else if (c == '"') {
    return DOUBLE_QUOTE;
  } else if (c == '\'') {
    return SINGLE_QUOTE;
  } else if (c == ':') {
    return COLON;
  } else if (c == '\\') {
    return BACK_SLASH;
  } else if (c == '[') {
    return OPEN_BRACKET;
  } else if (c == ']') {
    return CLOSE_BRACKET;
  } else if (c == ',') {
    return COMMA;
  } else if (c == ' ' || c == '\t' || c == '\n' || c == '\r') {
    return WHITESPACE;
  } else if (c >= '0' && c <= '9') {
    return DIGIT;
  } else if (c == '.') {
    return DOT;
  } else if (c == '+' || c == '-') {
    return PLUS_MINUS;
  }
  return OTHER;
}

const char *strTokType(Tok *tok) {
  if (tok) {
    if (tok->type == OPEN_BRACE) {
      return "Open Brace";
    } else if (tok->type == CLOSE_BRACE) {
      return "Close Brace";
    } else if (tok->type == DOUBLE_QUOTE) {
      return "Double Quote";
    } else if (tok->type == SINGLE_QUOTE) {
      return "Single Quote";
    } else if (tok->type == COLON) {
      return "Colon";
    } else if (tok->type == BACK_SLASH) {
      return "Back Slash";
    } else if (tok->type == OPEN_BRACKET) {
      return "Open Bracket";
    } else if (tok->type == CLOSE_BRACKET) {
      return "Close Bracket";
    } else if (tok->type == COMMA) {
      return "Comma";
    } else if (tok->type == WHITESPACE) {
      return "Whitespace";
    } else if (tok->type == DOT) {
      return "Dot";
    } else if (tok->type == DIGIT) {
      return "Digit";
    } else if (tok->type == PLUS_MINUS) {
      return "PlusOrMinus Symbol";
    } else {
      return "Other";
    }
  }
  return 0;
}

void printTok(Tok *tok) {
  printf("Tok type=%s, count=%d at ln %d, col %d\n", strTokType(tok),
      tok->count, tok->line, tok->column);
}

int isTerm(Parser *p, const char *cterm) {

  if (p->cur->type == OTHER) {

    Tok *start = p->cur;
    while(p->cur->type == OTHER) {
      consume(p);
    }
    jsonRewind(p, start);
    
    int cterm_len = strlen(cterm);
    if (cterm_len == p->cur->count) {
      char term[p->cur->count + 1];
      memset(term, 0, p->cur->count + 1);
      fseek(p->cur->file, p->cur->seek, SEEK_SET);
      fflush(p->cur->file);
      fread(term, sizeof(term[0]), p->cur->count, p->cur->file);

      if (strcmp(cterm, term) == 0) {
        consume(p);
        return 1;
      }

      jsonRewind(p, start);
    }
  }

  return 0;
}

const char* jsonParseQuotedString(Parser* p, char quote) {
  Tok *start = p->cur;
  TokType quoteType = tokType(quote);
  consume(p);
  while (p->cur->type != quoteType) {
    if (p->cur->type == BACK_SLASH) {
      consume(p);
      consume(p);
    }
    consume(p);
  }
  const char* str = getStrBetween(start, p->cur /* quote char */);

  consume(p);
  return str;
}

JValue *jsonParseObject(Parser *p) {
  if (p->cur->type == OPEN_BRACE) {
    consume(p);
  } else {
    UNEXPECTED_TOKEN(p)
    return 0;
  }
  JObject *obj = jsonNewObject();
  char haveComma = 0;
  do {
    if (haveComma) {
      consume(p);
      haveComma = 0;
    }
    consumeWhitespace(p);
    if (p->cur->type == CLOSE_BRACE) {
      break;
    }
    jsonParseMembers(p, obj);
    if (p->error) {
      break;
    }
    consumeWhitespace(p);
    if (p->cur->type == COMMA) {
      haveComma = 1;
    }
  } while (p->cur->type == COMMA);

  if (p->error) {
    jsonFree(jsonObjectValue(obj));
    return 0;
  }

  consume(p);

  return jsonObjectValue(obj);
}

void jsonParseMembers(Parser *p, JObject *obj) {
  const char* key = NULL;
  if (p->cur->type == SINGLE_QUOTE) {
    key = jsonParseQuotedString(p, '\'');
  } else if (p->cur->type == DOUBLE_QUOTE) {
    key = jsonParseQuotedString(p, '"');
  }
  jsonExpectPairSeparator(p);
  if (!p->error) {
    JValue *val = jsonParseValue(p);
    if (!p->error) {
      jsonAddVal(obj, key, val);
    }
    free((void*)key);
  } else {
    free((void*)key);
    jsonPrintError(p);
  }
}

void jsonExpectPairSeparator(Parser *p) {
  consumeWhitespace(p);
  if (p->cur->type == COLON) {
    p->cur = next(p->cur);
  } else {
    Tok *tok = p->cur;
    fprintf(stderr, "Unexpected token: %s at ln %d cl %d\n", strTokType(tok),
        tok->line, tok->column);
    UNEXPECTED_TOKEN(p)
  }
  consumeWhitespace(p);
}

JValue *jsonParseValue(Parser *p) {
  Tok *saved = p->cur;
  JValue *val = jsonParseString(p);
  if (val && p->error == 0) {
    return val;
  }
  jsonRewind(p, saved);

  val = jsonParseObject(p);

  if (val && !p->error) {
    return val;
  }
  
  if(val) {
    jsonFree(val);
  }

  jsonRewind(p, saved);

  val = jsonParseArray(p);

  if (val && !p->error) {
    return val;
  }

  if (val) {
    jsonFree(val);
  }
  
  jsonRewind(p, saved);

  val = jsonParseNumber(p);

  if (val && !p->error) {
    return val;
  }
  
  if(val) {
    jsonFree(val);
  }
  
  jsonRewind(p, saved);
  
  val = jsonParseBool(p);
  
  if(val && !p->error) {
    return val;
  }
  
  if(val) {
    jsonFree(val);
  }

  jsonRewind(p, saved);
  
  if(isTerm(p, "null")) {
    return jsonNullValue();
  }else{
    UNEXPECTED_TOKEN(p);
  }
  
  return 0;
}

JValue *jsonParseString(Parser *p) {
  Tok *cur = p->cur;
  const char* str = NULL;
  if (cur->type == SINGLE_QUOTE) {
    str = jsonParseQuotedString(p, '\'');
  } else if (cur->type == DOUBLE_QUOTE) {
    str = jsonParseQuotedString(p, '"');
  }
  
  if(str) {
    JValue *val = jsonStringValue(str);
    free((void*)str);
    return val;
  }
  return 0;
}

JValue *jsonParseBool(Parser *p) {
  consumeWhitespace(p);
  if(isTerm(p, "true")) {
    return jsonBoolValue(1);
  }else if(isTerm(p, "false")){
    return jsonBoolValue(0);
  }else{
    UNEXPECTED_TOKEN(p)
  }
  return 0;
}

JValue *jsonParseNumber(Parser *p) {
  double value = 0;
  char signValue = '+';
  if (p->cur->type == PLUS_MINUS) {
    signValue = getCharAt(p->cur, 0);
    consume(p);
  }
  if (p->cur->type == DIGIT) {
    value = getCharAt(p->cur, 0) - '0';
    value *= (signValue == '-') ? -1 : 1;
    consume(p);

    if (p->cur->type == DIGIT) {
      do {
        for (int i = 0; i < p->cur->count; ++i) {
          value *= 10;
          value += (getCharAt(p->cur, i) - '0');
        }
        consume(p);
      } while (p->cur->type == DIGIT);
    }

    if (p->cur->type == DOT) {
      consume(p);
      double divisor = 10;
      if (p->cur->type == DIGIT) {
        value = (getCharAt(p->cur, 0) - '0') / divisor;
        consume(p);
        if (p->cur->type == DIGIT) {
          do {
            for (int i = 0; i < p->cur->count; ++i) {
              divisor *= 10;
              value += (getCharAt(p->cur, i) - '0') / divisor;
            }
            consume(p);
          } while (p->cur->type == DIGIT);
        }
      } else {
        UNEXPECTED_TOKEN(p);
        return 0;
      }
    }

    if (isTerm(p, "E") || isTerm(p, "e")) {
      char sign = '+';
      if (p->cur->type == PLUS_MINUS) {
        sign = getCharAt(p->cur, 0);
        consume(p);
      }

      if (p->cur->type == DIGIT) {
        double power = (getCharAt(p->cur, 0) - '0');
        power *= (sign == '-' ? -1 : 1);
        consume(p);

        if (p->cur->type == DIGIT) {
          do {
            for (int i = 0; i < p->cur->count; ++i) {
              power *= 10;
              power += getCharAt(p->cur, i);
            }
            consume(p);
          } while (p->cur->type == DIGIT);
        }

        value *= pow(10, power);
      } else {
        UNEXPECTED_TOKEN(p);
        return 0;
      }
    }

//determine if we have a integer, float, or double
    if (value <= FLT_MAX) {
      //we have a float, check for decimals
      if (value <= INT_MAX && !(value - floor(value) > 0)) {
        //we have an integer
        return jsonIntValue((int) value);
      }

      return jsonFloatValue((float) value);
    }

    // otherwise we have a double
    return jsonDoubleValue(value);
  }

  UNEXPECTED_TOKEN(p);
  return 0;
}

void jsonRewind(Parser *p, Tok *saved) {
  while (p->cur && p->cur != saved) {
    Tok *deletable = p->cur;
    p->cur = p->cur->previous;
    free(deletable);
  }

  if (p->cur == NULL) {
    p->error = 45;
    return;
  }

  p->error = 0;
  p->error_tok = 0;
}

JValue *jsonParseArray(Parser *p) {
  typedef struct ArrayVal {
    JValue *val;
    struct ArrayVal *next;
  } ArrayVal;

#define ARRAY_TYPE(t) \
   (t == VAL_INT ? VAL_INT_ARRAY : \
     (t == VAL_FLOAT ? VAL_FLOAT_ARRAY : \
       (t == VAL_DOUBLE ? VAL_DOUBLE_ARRAY : \
         (t == VAL_STRING ? VAL_STRING_ARRAY : \
             (t == VAL_BOOL ? VAL_BOOL_ARRAY : VAL_MIXED_ARRAY)))))

  if (p->cur->type != OPEN_BRACKET) {
    UNEXPECTED_TOKEN(p);
    return 0;
  }

  ArrayVal *curVal = malloc(sizeof(ArrayVal));
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
    curVal->val = jsonParseValue(p);

    if (!curVal || p->error) {
      //need to cleanup ArrayVal's
      puts("We need to cleanup ArrayVals");
      return 0;
    }

    // could make this a ternary but I won't
    if (singleValueType == -1) {
      singleValueType = ARRAY_TYPE(curVal->val->value_type);
    }
    if (singleValueType != ARRAY_TYPE(curVal->val->value_type)) {
      // as soon as it's not the same it's mixed
      singleValueType = VAL_MIXED_ARRAY;
    }
    singleValueType = ARRAY_TYPE(curVal->val->value_type);

    nextVal = malloc(sizeof(ArrayVal));
    memset(nextVal, 0, sizeof(ArrayVal));
    nextVal->val = 0;
    curVal->next = nextVal;
    curVal = nextVal;
    ++count;
    consumeWhitespace(p);
  } while (p->cur->type == COMMA);

  if (p->cur->type != CLOSE_BRACKET) {
    UNEXPECTED_TOKEN(p);
    return 0;
  }
  consume(p);
  
//Now we know how many we have lets allocate
  JValue *arrayVal = malloc(sizeof(JValue));

  if(count == 0) {
    arrayVal->size = 0;
    arrayVal->value = 0;
    arrayVal->value_type = VAL_MIXED_ARRAY;
    return arrayVal;
  }
  
  JValue** arry = malloc(sizeof(*arry) * count);
  arrayVal->size = sizeof(*arry) * count;
  arrayVal->value = arry;
  curVal = 0;
  curVal = head;
  int index = 0;
  ArrayVal *deletable = curVal;
  while (curVal != 0) {
    if (curVal->val) {
      arry[index] = curVal->val;
    }

    deletable = curVal;
    curVal = curVal->next;
    free(deletable);
    ++index;
  }
  arrayVal->value_type = singleValueType;

  if (arrayVal->value_type == VAL_INT_ARRAY) {
    //convert to array of ints
    int *integers = malloc(sizeof(*integers) * count);
    for (int i = 0; i < count; ++i) {
      int *num = (int*) arry[i]->value;
      integers[i] = *num;
      jsonFree(arry[i]);
    }
    arrayVal->value = integers;
  } else if (arrayVal->value_type == VAL_FLOAT_ARRAY) {
    //convert to array of floats
    float *floats = malloc(sizeof(*floats) * count);
    for (int i = 0; i < count; ++i) {
      float *num = (float*) arry[i]->value;
      floats[i] = *num;
      jsonFree(arry[i]);
    }
    arrayVal->value = floats;
  } else if (arrayVal->value_type == VAL_DOUBLE_ARRAY) {
    //convert to array of doubles
    float *doubles = malloc(sizeof(*doubles) * count);
    for (int i = 0; i < count; ++i) {
      double *num = (double*) arry[i]->value;
      doubles[i] = *num;
      jsonFree(arry[i]);
    }
    arrayVal->value = doubles;
  } else if (arrayVal->value_type == VAL_STRING_ARRAY) {
    char **strings = malloc(sizeof(*strings) * count);
    for (int i = 0; i < count; ++i) {
      char *string = (char*) arry[i]->value;
      strings[i] = strdup(string);
      jsonFree(arry[i]);
    }
    arrayVal->value = strings;
  } else if (arrayVal->value_type == VAL_BOOL_ARRAY) {
    char *bools = malloc(sizeof(bools[0]) * count);
    for (int i = 0; i < count; ++i) {
      bools[i] = ((char*)arry[i]->value)[0];
      jsonFree(arry[i]);
    }
    arrayVal->value = bools;
  }
  
  if(arrayVal->value_type != VAL_MIXED_ARRAY) {
    free(arry);
  }

  return arrayVal;
}

void consumeWhitespace(Parser *p) {
  while (p->cur->type == WHITESPACE) {
    p->cur = next(p->cur);
  }
}

void consume(Parser *p) {
  p->cur = next(p->cur);
}

JValue *jsonParse(const char *filename) {
  FILE *file = fopen(filename, "r");
  return jsonParseF(file);
}

JValue *jsonParseF(FILE *file) {
  Parser p;
  p.error = 0;
  p.error_message = strdup("Unknown Error");
  Tok *first = p.cur = ffirst(file);
  if (p.cur) {
    consumeWhitespace(&p);
    JValue *val;
    if (p.cur->type == OPEN_BRACKET) {
      val = jsonParseArray(&p);
    } else if (p.cur->type == OPEN_BRACE) {
      val = jsonParseObject(&p);
    }

    if (val && !p.error) {
      free(p.error_message);
      jsonRewind(&p, first);
      free(first);
      fclose(file);
      return val;
    } else {
      jsonPrintError(&p);
    }
  }

  jsonRewind(&p, first);
  free(first);
  fclose(file);
  BAD_CHARACTER(&p)
  jsonPrintError(&p);
  free(p.error_message);
  return 0;
}

char getCharAt(Tok *tok, int index) {
  if(!tok) {
    return -1;
  }
  char buf;
  int seekPos = tok->seek + index;
  if (seekPos > tok->seek + tok->count) {
    seekPos = tok->seek + tok->count;
  }
  fseek(tok->file, seekPos, SEEK_SET);
  fflush(tok->file);
  fread(&buf, 1, 1, tok->file);
  return buf;
}

const char *getnStrBetween(Tok *start, Tok *end, int count) {
  if (end == start) {
    return 0;
  }

  int size = sizeof(char) * (count + 1);
  char *buf = (char*) malloc(size);
  memset(buf, 0, size);
  char *pos = buf + count;
  end = end->previous;
  while (start != end) {
    fseek(end->file, end->seek, SEEK_SET);
    pos -= end->count;
    fread(pos, end->count, 1, end->file);
    end = end->previous;
  }

  return buf;
}

const char *getStrBetween(Tok *start, Tok *end) {
  if (start == end) {
    return 0;
  }

  int totalCount = 0;
  Tok *cur = end;
  cur = cur->previous;
  while (start != cur) {
    totalCount += cur->count;
    cur = cur->previous;
  }

  return getnStrBetween(start, end, totalCount);
}
