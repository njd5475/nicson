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
#define VAL_NULL          15

typedef struct JValue {
  unsigned char  value_type : 4;
  unsigned int   size;
  void*          value;
} JValue;

typedef struct JEntry {
  char   *name;
  JValue *value;
  int     hash;
  unsigned short   probes;
} JEntry;

typedef struct JObject {
  JEntry         **entries; // hashed arrangement by key.
  unsigned int   size;
  unsigned int   _arraySize;
  unsigned short _maxProbes;
} JObject;


extern JObject *stringCache;

// Building functions
JObject* jsonAddVal(JObject *obj, const char *name, struct JValue *value);
/* Convenience methods */
JObject* jsonAddObj(JObject *obj, const char *name, JObject *value);
JObject* jsonAddInt(JObject *obj, const char *name, const int value);
JObject* jsonAddUInt(JObject *obj, const char *name, const unsigned int value);
JObject* jsonAddStringDup(JObject *pbj, const char *name, const char *value);
JObject* jsonAddString(JObject *obj, const char *name, const char *value);
JObject* jsonDeleteKey(JObject *obj, const char *key);

JValue*  jsonBoolValue(const char value);

#define NO_DUP 0
#define DUP 1
JValue*  jsonStringValue(const char *value, short dup);
JValue*  jsonIntValue(const int value);
JValue*  jsonFloatValue(const float value);
JValue*  jsonDoubleValue(const double value);
JValue*  jsonUIntValue(const unsigned int value);
JValue*  jsonObjectValue(JObject *obj);
JValue*  jsonStringArrayValue(const char **strings);
JValue*  jsonIntArrayValue(int **vals);
JValue*  jsonFloatArrayValue(float **vals);
JValue*  jsonDoubleArrayValue(double **vals);
JValue*  jsonNullValue();

JValue*       jsonParse(const char *filename);
JValue*       jsonParseF(FILE *file);
const char*   jsonParserError();
void          jsonPrintError();
JObject*      jsonNewObject();
JValue*       jsonGet(const JObject *obj, const char *keys);
int           jsonInt(const JObject *obj, const char* keys);
unsigned int  jsonUInt(const JObject *obj, const char* keys);
float         jsonFloat(const JObject *obj, const char* keys);
double        jsonDouble(const JObject *obj, const char* keys);
char *        jsonString(const JObject *obj, const char* keys);
char          jsonBool(const JObject *obj, const char* keys);
JValue**      jsonArray(const JObject *obj, const char* keys);
char*         jsonBoolArray(const JObject *obj, const char* keys);
int**         jsonIntArray(const JObject *obj, const char* keys);
float**       jsonFoatArray(const JObject *obj, const char* keys);
double**      jsonDoubleArray(const JObject *obj, const char* keys);
char**        jsonStringArray(const JObject *obj, const char* keys);
JObject*      jsonObject(const JObject *obj, const char* keys);

void          jsonFree(JValue *val);

#endif
