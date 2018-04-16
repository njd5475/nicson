#ifndef JSON_H
#define JSON_H

#include <stdio.h>
#include "fnv.h"

#define VAL_STRING        1
#define VAL_INT           2
#define VAL_UINT          3
#define VAL_FLOAT         4
#define VAL_DOUBLE        5
#define VAL_BOOL          6
#define VAL_OBJ           7
#define VAL_STRING_ARRAY  8
#define VAL_INT_ARRAY     9
#define VAL_FLOAT_ARRAY   10
#define VAL_DOUBLE_ARRAY  11
#define VAL_BOOL_ARRAY    12
#define VAL_OBJ_ARRAY     13
#define VAL_MIXED_ARRAY   14 /* JValue array */

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
  unsigned int _maxProbes;
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
JObject* jsonAddVal(JObject *obj, const char *name, struct JValue *value);
/* Convenience methods */
JObject* jsonAddObj(JObject *obj, const char *name, JObject *value);
JObject* jsonAddInt(JObject *obj, const char *name, const int value);
JObject* jsonAddUInt(JObject *obj, const char *name, const unsigned int value);
JObject* jsonAddString(JObject *pbj, const char *name, const char *value);

JValue* jsonGet(const JObject *obj, const char *key);
JValue* jsonStringValue(const char *name);
JValue* jsonIntValue(const int value);
JValue* jsonFloatValue(const float value);
JValue* jsonDoubleValue(const double value);
JValue* jsonUIntValue(const unsigned int value);
JValue* jsonObjectValue(JObject *obj);
JValue* jsonStringArrayValue(const char **strings);
JValue* jsonIntArrayValue(int **vals);
JValue* jsonFloatArrayValue(float **vals);
JValue* jsonDoubleArrayValue(double **vals);

JValue*       jsonParse(const char *filename);
JValue*       jsonParseF(FILE *file);
const char*   jsonParserError();
void          jsonPrintError();
JObject*      jsonNewObject();
int           jsonInt(const JObject *obj, const char* keys);
unsigned int  jsonUInt(const JObject *obj, const char* keys);
float         jsonFloat(const JObject *obj, const char* keys);
double        jsonDouble(const JObject *obj, const char* keys);
char *        jsonString(const JObject *obj, const char* keys);
char          jsonBool(const JObject *obj, const char* keys);
JValue**      jsonArray(const JObject *obj, const char* keys);
int**         jsonIntArray(const JObject *obj, const char* keys);
float**       jsonFoatArray(const JObject *obj, const char* keys);
double**      jsonDoubleArray(const JObject *obj, const char* keys);
char**        jsonStringArray(const JObject *obj, const char* keys);
JObject*      jsonObject(const JObject *obj, const char* keys);

void          jsonFree(JValue *val);

#endif
