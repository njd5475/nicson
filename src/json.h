#ifndef JSON_H
#define JSON_H

#include "fnv.h"

#define VAL_STRING        1
#define VAL_INT           2
#define VAL_FLOAT         3
#define VAL_DOUBLE        4
#define VAL_BOOL          5
#define VAL_OBJ           6
#define VAL_STR_ARRAY     7
#define VAL_INT_ARRAY     8
#define VAL_FLOAT_ARRAY   9
#define VAL_DOUBLE_ARRAY 10
#define VAL_BOOL_ARRAY   11
#define VAL_OBJ_ARRAY    12
#define VAL_MIXED_ARRAY  13 /* JValue array */

typedef struct JValue {
  short value_type;
  int   size;
  void  *value;
} JValue;

typedef struct JEntry {
  char   *name;
  JValue *value;
  int     hash;
  short   probes;
} JEntry;

typedef struct JObject {
  JEntry       **entries; // hashed arrangement by key.
  unsigned int size;
  unsigned int _arraySize;
} JObject;

/**
 * We want to access JSON data as simply as we can.
 *
 * int val = jsonInt(jsonObject, "key.key.key");
 * float val = jsonFloat(jsonObject, "key.key.key");
 * double val = jsonDouble(jsonObject, "key.key.key");
 * char *val = jsonString(jsonObject, "key.key.key");
 * bool val = jsonBool(jsonObject, "key.key.key");
 * int objects[] = jsonArray(jsonObject, "key.key.key");
 * int vals[] = jsonIntArray(jsonObject, "key.key.key");
 * float vals[] = jsonFoatArray(jsonObject, "key.key.key");
 * double vals[] = jsonDoubleArray(jsonObject, "key.key.key");
 * char **vals = jsonStringArray(jsonObject, "key.key.key");
 * jsonObject = jsonObject(jsonObject, "key.key.key");
 */

// Building functions
JObject* jsonAddKey(JObject *obj, const char *name, struct JValue *value);
JValue* jsonGet(const JObject *obj, const char *key);
JValue* jsonStringValue(const char *name);
JValue* jsonIntValue(const int value);
JValue* jsonUIntValue(const unsigned int value);
JValue* jsonObjectValue(JObject *obj);
JValue* jsonStringArrayValue(const char **strings);

JObject*      jsonParse(const char *filename);
JObject*      jsonNewObject();
int           jsonInt(const JObject *obj, const char* keys);
unsigned int  jsonUInt(const JObject *obj, const char* keys);
float         jsonFloat(const JObject *obj, const char* keys);
double        jsonDouble(const JObject *obj, const char* keys);
char *        jsonString(const JObject *obj, const char* keys);
char          jsonBool(const JObject *obj, const char* keys);
int*          jsonArray(const JObject *obj, const char* keys);
int*          jsonIntArray(const JObject *obj, const char* keys);
float*        jsonFoatArray(const JObject *obj, const char* keys);
double*       jsonDoubleArray(const JObject *obj, const char* keys);
char **       jsonStringArray(const JObject *obj, const char* keys);
JObject*      jsonObject(const JObject *obj, const char* keys);

#endif
