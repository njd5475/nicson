#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "json.h"

JObject *jsonAddVal(JObject *obj, const char *name, JValue *value) {
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

JObject* jsonAddObj(JObject *obj, const char *name, JObject *value) {
  return jsonAddVal(obj, name, jsonObjectValue(value));
}

JObject* jsonAddInt(JObject *obj, const char *name, const int value) {
  return jsonAddVal(obj, name, jsonIntValue(value));
}

JObject* jsonAddUInt(JObject *obj, const char *name, const unsigned int value) {
  return jsonAddVal(obj, name, jsonUIntValue(value));
}

JObject *jsonAddString(JObject *obj, const char *name, const char *value) {
  return jsonAddVal(obj, name, jsonStringValue(value));
}

JValue* jsonGet(const JObject *obj, const char* key) {
  int keyhash = fnvstr(key);
  int index = keyhash % obj->_arraySize;

  while (obj->entries[index] != 0 && obj->entries[index]->hash != keyhash) {
    printf("We looked and did not find the key\n");
    // not found, keep probing
    index = (++keyhash) % obj->_arraySize;
  }

  if (obj->entries[index]) {
    return obj->entries[index]->value;
  }

  printf("Did not find key %s\n", key);
  return 0;
}

char *nextKey(const char *keys, int *last) {
  int start = *last;
  int begin = start;
  for (; keys[start] != '.' && keys[start]; ++start);
  
  int size = start - begin;
  if (size <= 0) {
    *last = -1;
    return 0;
  }
  char *newkey = malloc(size+1);
  memset(newkey, 0, size+1);
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
  const char *last = keys + len;
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

JValue* jsonUIntValue(const unsigned int value) {
  JValue *val = malloc(sizeof(JValue));
  val->value_type = VAL_INT;
  val->value = malloc(sizeof(value));
  memcpy(val->value, &value, sizeof(value));
  val->size = sizeof(value);
  return val;
}

JValue* jsonFloatValue(const float value) {
  JValue *val = malloc(sizeof(JValue));
  val->value_type = VAL_FLOAT;
  val->value = malloc(sizeof(value));
  memcpy(val->value, &value, sizeof(value));
  val->size = sizeof(value);
  return val;
}

JValue* jsonDoubleValue(const double value) {
  JValue *val = malloc(sizeof(JValue));
  val->value_type = VAL_DOUBLE;
  val->value = malloc(sizeof(value));
  memcpy(val->value, &value, sizeof(value));
  val->size = sizeof(value);
  return val;
}

JValue* jsonObjectValue(JObject *obj) {
  JValue *val = malloc(sizeof(JValue));
  val->value_type = VAL_OBJ;
  val->value = (void*)(obj);
  val->size = sizeof(obj);
  return val;
}

JValue* jsonStringArrayValue(const char **strings) {
  return 0;
}

JValue* jsonIntArrayValue(int **vals) {
  return 0;
}

JValue* jsonFloatArrayValue(float **vals) {
  return 0;
}

JValue* jsonDoubleArrayValue(double **vals) {
  return 0;
}

JObject *jsonNewObject() {
  JObject *obj = malloc(sizeof(JObject));
  obj->_arraySize = 100;
  obj->size = 0;
  obj->entries = malloc(sizeof(JEntry*) * obj->_arraySize);
  memset(obj->entries, 0, sizeof(JEntry*) * obj->_arraySize);
  return obj;
}

int jsonInt(const JObject *obj, const char* keys) {
  JValue *val = jsonGet(obj, keys);
  if(val && val->value_type == VAL_INT) {
    return * ((int*)val->value);
  }
  return -1;
}

unsigned int jsonUInt(const JObject *obj, const char* keys) {
  JValue *val = jsonGet(obj, keys);
  if(val && val->value_type == VAL_UINT) {
    return * ((unsigned int*)val->value);
  }
  return -1;
}

float jsonFloat(const JObject *obj, const char* keys) {
  JValue *val = jsonGet(obj, keys);
  if(val && val->value_type == VAL_FLOAT) {
    return * ((float*)val->value);
  }
  return -1.0f;
}

double jsonDouble(const JObject *obj, const char* keys) {
  JValue *val = jsonGet(obj, keys);
  if(val && val->value_type == VAL_INT) {
    return * ((double*)val->value);
  }
  return -1.0;
}

char* jsonString(const JObject *obj, const char* keys) {
  JValue *val = jsonGet(obj, keys);
  if(val && val->value_type == VAL_STRING) {
    return val->value;
  }
  return NULL;
}

char jsonBool(const JObject* obj, const char* keys) {
  return 0;
}

JValue** jsonArray(const JObject* obj, const char* keys) {
  return NULL;
}

int** jsonIntArray(const JObject* obj, const char* keys) {
  return NULL;
}

float** jsonFoatArray(const JObject* obj, const char* keys) {
  return NULL;
}

double** jsonDoubleArray(const JObject* obj, const char* keys) {
  return NULL;
}

char** jsonStringArray(const JObject* obj, const char* keys) {
  return NULL;
}

JObject* jsonObject(const JObject* obj, const char* keys) {
  if(obj == 0) {
    return 0;
  }

  int index = 0;
  char *key = nextKey(keys, &index);
  JValue *jval = jsonGet(obj, key);
  if(jval->value_type != VAL_OBJ) {
    return 0;
  }

  JObject *found = (JObject*)jval->value;
  while (index > -1 && found != 0) {
    key = nextKey(keys, &index);
    printf("Looking for key %s\n", key);
    jval = jsonGet(found, key);
    if (jval && jval->value_type == VAL_OBJ) {
      found = (JObject*) jval->value;
    } else {
      if(jval) {
        printf("Err: Key %s was the wrong type.\n", key);
      }else{
        printf("Err: Object did not contains a next object %s\n", key);
      }
      return 0;
    }
  }
  return found;
}

void jsonFree(JObject *obj) {
  
}
