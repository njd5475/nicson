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

#define NOT_IMPLEMENTED(p)   jsonSetParserError(p, 42, "parseArray: Not Implemented");
#define UNEXPECTED_TOKEN(p)  jsonSetParserError(p, 43, "Unexpected Token");
#define BAD_CHARACTER(p)     jsonSetParserError(p, 99, "Could not read first character");

void jsonSetParserError(Parser *p, unsigned int errNo, const char *msg) {
  p->error = errNo;
  p->error_tok = p->cur;
  p->error_message = strdup(msg);
}

void jsonPrintError(Parser *p) {
  fflush(stdout);
  fprintf(stderr, "Parse error: %s, for %s, at ln %d, col %d\n",
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
  tok->_tok = 0;
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
    char buf;
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
    Tok *last;
    int cterm_len = strlen(cterm);
    int cur_len = 0;
    while (p->cur->type == OTHER && cur_len <= cterm_len) {
      last = p->cur;
      cur_len += p->cur->count;
      consume(p);
    }
    const char *term = getStrBetween(start, last);

    jsonRewind(p, start);

    if (strcmp(cterm, term) == 0) {
      return 1;
    }
  }

  return 0;
}

const char* jsonParseQuotedString(Parser* parser, char quote) {
  Tok *tok = parser->cur;
  Tok *cur = tok = next(tok);
  TokType quoteType = tokType(quote);
  while (cur->type != quoteType) {
    if (cur->type == BACK_SLASH) {
      cur = next(cur); // consume back slash and next character
      cur = next(cur);
    }
    cur = next(cur);
  }
  Tok *last = cur;
  cur = cur->previous; //before quote

  const char* str = getStrBetween(tok, cur);

  parser->cur = next(last); // advance the token

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

  if (p->error != 0) {
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
  } else {
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

JValue *jsonParsePair(Tok *startAt, JObject *obj) {
  return 0;
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

  jsonRewind(p, saved);

  val = jsonParseArray(p);

  if (val && !p->error) {
    return val;
  }

  jsonRewind(p, saved);

  val = jsonParseNumber(p);

  if (val && !p->error) {
    return val;
  }

  return 0;
}

JValue *jsonParseString(Parser *p) {
  Tok *cur = p->cur;
  if (cur->type == SINGLE_QUOTE) {
    return jsonStringValue(jsonParseQuotedString(p, '\''));
  } else if (cur->type == DOUBLE_QUOTE) {
    return jsonStringValue(jsonParseQuotedString(p, '"'));
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
      consume(p);
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

    //otherwise we have a double
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
    struct Entry {
      struct ArrayVal *next;
    } entry;
    JValue *val;
  } ArrayVal;

  if (p->cur->type != OPEN_BRACKET) {
    UNEXPECTED_TOKEN(p);
    return 0;
  }

  ArrayVal *curVal = malloc(sizeof(ArrayVal));
  ArrayVal *head = curVal;

  int count = 0;
  short singleValueType = -1;
  do {
    consume(p); //first time consume open bracket then commas
    consumeWhitespace(p);
    curVal->val = jsonParseValue(p);
    
    // could make this a ternary but I won't
    if(singleValueType == -1) {
      singleValueType = curVal->val->value_type;
    }
    if(singleValueType != curVal->val->value_type) {
      // as soon as it's not the same it's mixed
      // TODO: could be amended to promote integers to floats and doubles
      singleValueType = VAL_MIXED_ARRAY; 
    }
    singleValueType = curVal->val->value_type;
    
    if (!curVal || p->error) {
      return 0;
    }

    ArrayVal *nextVal = malloc(sizeof(ArrayVal));
    curVal->entry.next = nextVal;
    curVal = nextVal;
    ++count;
    consumeWhitespace(p);
  }while (p->cur->type == COMMA);

  if(p->cur->type != CLOSE_BRACKET) {
    UNEXPECTED_TOKEN(p);
    return 0;
  }
  consume(p);

  //Now we know how many we have lets allocate
  JValue *arrayVal = malloc(sizeof(JValue*));
  JValue** arry = arrayVal->value = malloc(sizeof(JValue*) * count);
  curVal = head;
  int index = 0;
  while (curVal != 0) {
    arry[index] = curVal->val;

    ArrayVal *deletable = curVal;
    curVal = curVal->entry.next;
    free(deletable);
    ++index;
  }
  arrayVal->value_type = singleValueType;

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
      jsonRewind(&p, first);
      free(first);
      return val;
    } else {
      jsonPrintError(&p);
    }
  }

  jsonRewind(&p, first);
  BAD_CHARACTER(&p)
  return 0;
}

char getCharAt(Tok *tok, int index) {
  char buf;
  int seekPos = tok->seek + index;
  if (seekPos > tok->seek + tok->count) {
    seekPos = tok->seek + tok->count;
  }
  fseek(tok->file, seekPos, SEEK_SET);
  fread(&buf, 1, 1, tok->file);
  return buf;
}

const char *getnStrBetween(Tok *start, Tok *end, int count) {
  if (end < start) {
    return 0;
  }

  int size = sizeof(char) * (count + 1);
  char *buf = (char*) malloc(size);
  memset(buf, 0, size);
  char *pos = buf + count;
  for (; end >= start && end; end = end->previous) {
    fseek(end->file, end->seek, SEEK_SET);
    pos -= end->count;
    fread(pos, end->count, 1, end->file);
  }

  return buf;
}

const char *getStrBetween(Tok *start, Tok *end) {
  if (end < start) {
    return 0;
  }
  int totalCount = 0;
  Tok *cur = end;
  for (; cur >= start && cur; cur = cur->previous) {
    totalCount += cur->count;
  }
  return getnStrBetween(start, end, totalCount);
}
