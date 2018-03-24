#include "json.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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

typedef unsigned short TokType;
typedef struct Tok {
  FILE *file;
  struct Tok* _tok;
  int seek;
  int count;
  TokType type;
  struct Tok *previous;
} Tok;

TokType tokType(const char c);
Tok *next(Tok *last);

Tok *first(const char *filename) {
  Tok *tok = malloc(sizeof(Tok));
  tok->_tok = 0;
  tok->file = fopen(filename, "r");
  tok->seek = 0;
  tok->count = 1;
  char ch;
  fread(&ch, 1, 1, tok->file);
  tok->type = tokType(ch);
  tok->previous = 0;
  return tok;
}

Tok *next(Tok *last) {
  if (last) {
    if (feof(last->file)) {
      return 0;
    }
    fseek(last->file, last->seek + last->count, 0);
    char buf;
    fread(&buf, 1, 1, last->file);

    if (last->type == tokType(buf) && last->type == OTHER) {
      last->count++;
      return last;
    } else {
      Tok *next = malloc(sizeof(Tok));
      next->file = last->file;
      next->seek = last->seek + last->count;
      next->count = 1;
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
    } else {
      return "Other";
    }
  }
  return 0;
}

struct StackItem {
  struct StackItem *next;
  JObject *obj;
};
typedef struct StackItem StackItem;

StackItem **_object(StackItem **stack, JObject *obj, Tok *startAt) {
  return 0;
}

unsigned int error = 0;
_Bool parserError() {
  return !error;
}

JObject *jsonParse(const char *filename) {
  Tok *cur = first(filename);

  JObject *obj = jsonNewObject();
  StackItem **stack = malloc(sizeof(StackItem));
  while (!parserError()) {
    stack = _object(stack, obj, cur);
  }

  return obj;
}

JObject *jsonAddKey(JObject *obj, const char *name, JValue *value) {
  if (!obj) {
    obj = jsonNewObject();
  }

  JEntry *entry = malloc(sizeof(JEntry));
  entry->name = strdup(name);
  entry->value = value;
  entry->probes = 1;
  entry->hash = fnvstr(entry->name);
  int index = (entry->hash + (entry->probes - 1)) % obj->_arraySize;

  while (obj->entries[index] != 0) {
    printf("Hash Collision...probing\n");

    // steal the slot if the probed entrys' probes is less than mine
    if (obj->entries[index]->probes < entry->probes) {
      JEntry *tmp = obj->entries[index];
      obj->entries[index] = entry;
      entry = tmp; //swap
    }

    // adjust hash key and keep going
    ++entry->probes;
    index = (entry->hash + (entry->probes - 1)) % obj->_arraySize;

    //TODO: handle resizing the hashtable
  }
  obj->entries[index] = entry;
  ++obj->size;
  return obj;
}

JValue* jsonGet(const JObject *obj, const char* key) {
  int keyhash = fnvstr(key);
  int index = keyhash % obj->_arraySize;

  while (obj->entries[index] != 0 && obj->entries[index]->hash != keyhash) {
    printf("We looked and did not find the key\n");
    // not found, keep probing
    index = (++keyhash) % obj->_arraySize;
  }

  if(obj->entries[index]) {
    return obj->entries[index]->value;
  }

  printf("Did not find key %s\n", key);
  return 0;
}

JValue* jsonStringValue(const char *name) {
  JValue *val = malloc(sizeof(JValue));
  val->value_type = VAL_STRING;
  val->value = strdup(name);
  val->size = strlen(val->value) + 1;
  return val;
}

JValue* jsonIntValue(const int value) {
  JValue *val = malloc(sizeof(JValue));
  val->value_type = VAL_INT;
  int *ival = malloc(sizeof(value));
  memcpy(ival, &value, sizeof(value));
  val->value = ival;
  val->size = sizeof(value);
  return val;
}

char *nextKey(const char *keys, int *last) {
  int start = *last;
  int begin = start;
  for (; keys[start] != '.' && keys[start] != '\0'; ++start)
    ;
  int size = start - begin;
  if (size <= 0) {
    *last = -1;
    return 0;
  }
  char *newkey = malloc(size);
  int another = begin;
  for (; begin <= start; ++begin) {
    newkey[start - begin] = keys[another + (start - begin)];
  }
  newkey[size] = '\0';
  *last = start + 1;
  if (keys[start] == '\0') {
    *last = -1;
  }
  return newkey;
}

char *allButLast(const char *keys) {
  int len = strlen(keys);
  char *last = keys + len;
  while (*last != '.' && last >= keys) {
    --last;
  }
  if (keys == last) {
    return 0;
  }

  char *newkeys = malloc(last - keys);
  strncpy(newkeys, keys, last - keys);
  return newkeys;
}

JValue* jsonUIntValue(const unsigned int value) {
  return 0;
}

JValue* jsonObjectValue(JObject *obj) {
  JValue *val = malloc(sizeof(JValue));
  val->value_type = VAL_OBJ;
  val->value = obj->_arraySize * sizeof(JEntry*);
  return val;
}

JValue* jsonStringArrayValue(const char **strings) {
  return 0;
}

JObject *jsonNewObject() {
  JObject *obj = malloc(sizeof(JObject));
  obj->_arraySize = 100;
  obj->size = 0;
  obj->entries = malloc(sizeof(JEntry*) * obj->_arraySize);
  return obj;
}

int jsonInt(const JObject *obj, const char* keys) {
  return -1;
}

unsigned int jsonUInt(const JObject *obj, const char* keys) {
  return -1;
}

float jsonFloat(const JObject *obj, const char* keys) {
  return -1.0f;
}

double jsonDouble(const JObject *obj, const char* keys) {
  return -1.0f;
}

char* jsonString(const JObject *obj, const char* keys) {

  return NULL;
}

char jsonBool(const JObject* obj, const char* keys) {
  return 0;
}

int* jsonArray(const JObject* obj, const char* keys) {
  return NULL;
}

int* jsonIntArray(const JObject* obj, const char* keys) {
  return NULL;
}

float* jsonFoatArray(const JObject* obj, const char* keys) {
  return NULL;
}

double* jsonDoubleArray(const JObject* obj, const char* keys) {
  return NULL;
}

char** jsonStringArray(const JObject* obj, const char* keys) {
  return NULL;
}

JObject* jsonObject(const JObject* obj, const char* keys) {
  int index = 0;
  char *key = nextKey(keys, &index);
  while (index > -1 && obj != 0) {
    printf("Looking for key %s\n", key);
    JValue *jval = jsonGet(obj, key);
    if (jval && jval->value_type == VAL_OBJ) {
      obj = (JObject*) jval->value;
    } else {
      printf("Err: Object did not contains a next object %s\n", key);
      return 0;
    }
    char *key = nextKey(keys, &index);
  }
  return obj;
}
