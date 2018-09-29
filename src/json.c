#include "json.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define DEFAULT_HASH_SIZE   5
#define DEFAULT_INC_AMOUNT  10

JObject *stringCache = NULL;

int jsonGetEntryIndex(const JObject *obj, const char* keys);

char *nextKey(const char *keys, int *last) {
  if (!keys || !last) {
    return 0;
  }

  int start = *last;
  int begin = start;
  for (; keys[start] != '.' && keys[start]; ++start) {
    ;
  }

  int size = start - begin;
  if (size <= 0) {
    *last = -1;
    return 0;
  }

  char *newkey = malloc(size + 1);
  memset(newkey, 0, size + 1);

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

JEntry* insertInto(JEntry *entry, JObject *obj) {
  int index = (entry->hash + (entry->probes - 1)) % obj->_arraySize;

  while (obj->entries[index] != 0) {
    // steal the slot if the probed entrys' probes is less than mine
    if (obj->entries[index]->probes < entry->probes) {
      JEntry *tmp = obj->entries[index];
      obj->entries[index] = entry;
      entry = tmp; //swap
    }

    ++entry->probes;
    // adjust hash key and keep going
    index = (entry->hash + (entry->probes - 1)) % obj->_arraySize;

    if (entry->probes > obj->_maxProbes) {
      obj->_maxProbes = entry->probes;
    }
  }
  obj->entries[index] = entry;
  ++obj->size;

  return 0;
}

JObject* jsonDeleteKey(JObject *obj, const char *key) {
  int index = jsonGetEntryIndex(obj, key);
  if(index > -1) {
    JEntry *toDel = obj->entries[index];
    obj->entries[index] = 0;
    free(toDel->name);
    jsonFree(toDel->value);
    free(toDel);
    return obj;
  }
  obj->size--;
  return 0;
}

JObject *jsonAddVal(JObject *obj, const char *name, JValue *value) {
  if (!obj) {
    obj = jsonNewObject();
  }

  JEntry *entry = malloc(sizeof(JEntry));
  entry->name = name;
  entry->value = value;
  entry->probes = 1;
  entry->hash = fnvstr(entry->name);
  entry = insertInto(entry, obj);

  if (entry || obj->size == obj->_arraySize) {
    //now determine to expand the entry array
    JEntry **oldEntries = obj->entries;
    int oldCount = obj->_arraySize;
    obj->_arraySize += DEFAULT_INC_AMOUNT;
    obj->size = 0;
    obj->entries = malloc(sizeof(JEntry*) * obj->_arraySize);
    memset(obj->entries, 0, sizeof(JEntry*) * obj->_arraySize);
    for (int i = 0; i < oldCount; ++i) {
      if (oldEntries[i]) {
        oldEntries[i]->probes = 1;
        if (insertInto(oldEntries[i], obj) != 0) {
          printf("Our new size did not spread the keys enough\n");
        }
      }
    }
    if (entry) {
      entry->probes = 0;
      insertInto(entry, obj);
    }
    free(oldEntries);
  }

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
  return jsonAddVal(obj, name, jsonStringValue(value, NO_DUP));
}

JObject *jsonAddStringDup(JObject *obj, const char *name, const char *value) {
  return jsonAddVal(obj, strdup(name), jsonStringValue(value, DUP));
}

JValue* _jsonGetObjVal(const JObject *obj, const char* keys) {
  int keyhash = fnvstr(keys);
  int index = keyhash % obj->_arraySize;

  int probes = -1;
  do {
    // not found, keep probing
    ++probes;
    index = (keyhash + probes) % obj->_arraySize;
    if (obj->entries[index] != 0 && obj->entries[index]->hash == keyhash) {
      return obj->entries[index]->value;
    }
  } while (probes < obj->_maxProbes);

  return 0;
}

int jsonGetEntryIndex(const JObject *obj, const char* keys) {
  int keyhash = fnvstr(keys);
  int index = keyhash % obj->_arraySize;

  int probes = -1;
  do {
    // not found, keep probing
    ++probes;
    index = (keyhash + probes) % obj->_arraySize;
    if (obj->entries[index] != 0 && obj->entries[index]->hash == keyhash) {
      return index;
    }
  } while (probes < obj->_maxProbes);

  return -1;
}

JValue* jsonGet(const JObject *obj, const char* keys) {
  if (obj == 0 || keys == NULL) {
    return 0;
  }

  JValue *jval = NULL;
  const JObject *found = obj;
  char *key = NULL;
  int index = 0;
  do {
    key = nextKey(keys, &index);
    jval = _jsonGetObjVal(found, key);
    if (jval && jval->value_type == VAL_OBJ) {
      found = (JObject*) jval->value;
    } else if (key && index > -1) {
      if (jval) {
        printf("Err: Key %s was the wrong type.\n", keys);
      } else {
        printf("Err: Object did not contain a next object %s\n", keys);
      }
      if (key) {
        free(key);
      }
      return 0;
    }

    if (key != NULL) {
      free(key);
      key = NULL;
    }
  } while (index > -1);

  return jval;
}

char *last(const char *keys) {
  if (!keys) {
    return 0;
  }

  int len = strlen(keys);
  const char *lastDot = keys + len;
  const char* end = lastDot;
  while (*lastDot != '.' && lastDot > keys) {
    --lastDot;
  }
  if (keys == lastDot) {
    return strdup(keys);
  }

  ++lastDot;

  if ((end - lastDot) <= 0) {
    return 0;
  }

  char *newkeys = malloc((end - lastDot) + 1);
  memset(newkeys, 0, (end - lastDot) + 1);
  strncpy(newkeys, lastDot, end - lastDot);
  return newkeys;
}

char *allButLast(const char *keys) {
  if (!keys) {
    return 0;
  }

  int len = strlen(keys);
  const char *last = keys + len;
  while (*last != '.' && last > keys) {
    --last;
  }

  if (keys == last) {
    return 0;
  }

  char *newkeys = malloc((last - keys) + 1);
  memset(newkeys, 0, (last - keys) + 1);
  strncpy(newkeys, keys, last - keys);
  return newkeys;
}

JValue* jsonBoolValue(const char value) {
  JValue *val = malloc(sizeof(JValue));
  val->value_type = VAL_BOOL;
  val->value = malloc(sizeof(char));
  memcpy(val->value, &value, sizeof(char));
  val->size = sizeof(char);
  return val;
}

JValue* jsonStringValue(const char *value, short dup) {
  JValue *val = malloc(sizeof(JValue));
  val->value_type = VAL_STRING;
  if(dup) {
    val->value = strdup(value);
  }else{
    val->value = value;
  }
  val->size = strlen(val->value) + 1;
  return val;
}

JValue* jsonIntValue(const int value) {
  JValue *val = malloc(sizeof(JValue));
  val->value_type = VAL_INT;
  memcpy(&val->value, &value, sizeof(int));
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
  val->value = (void*) (obj);
  val->size = sizeof(obj);
  return val;
}

JValue null = {VAL_NULL,0,0};

JValue* jsonNullValue() {
  return &null;
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

JValue *jsonMixedArrayValue(JValue **values) {

  return 0;
}

JObject *jsonNewObject() {
  JObject *obj = malloc(sizeof(JObject));
  obj->_arraySize = DEFAULT_HASH_SIZE;
  obj->_maxProbes = obj->_arraySize / 5;
  obj->size = 0;
  obj->entries = malloc(sizeof(JEntry*) * obj->_arraySize);
  memset(obj->entries, 0, sizeof(JEntry*) * obj->_arraySize);
  return obj;
}

int jsonInt(const JObject *obj, const char* keys) {
  JValue *val = jsonGet(obj, keys);
  if (val && val->value_type == VAL_INT) {
    return (int) val->value;
  }
  return -1;
}

unsigned int jsonUInt(const JObject *obj, const char* keys) {
  JValue *val = jsonGet(obj, keys);
  if (val && val->value_type == VAL_UINT) {
    return *((unsigned int*) val->value);
  }
  return -1;
}

float jsonFloat(const JObject *obj, const char* keys) {
  JValue *val = jsonGet(obj, keys);
  if (val && val->value_type == VAL_FLOAT) {
    return *((float*) val->value);
  }
  return -1.0f;
}

double jsonDouble(const JObject *obj, const char* keys) {
  JValue *val = jsonGet(obj, keys);
  if (val && val->value_type == VAL_INT) {
    return *((double*) val->value);
  }
  return -1.0;
}

char* jsonString(const JObject *obj, const char* keys) {
  if (keys == NULL || obj == NULL) {
    return NULL;
  }
  char *firstKeys = allButLast(keys);
  const JObject *parentObj = obj;

  if (firstKeys != NULL) {
    parentObj = jsonObject(obj, firstKeys);
    free(firstKeys);
  }

  if (parentObj != NULL) {
    char *lastKey = last(keys);
    if (lastKey != NULL) {
      JValue *val = jsonGet(parentObj, lastKey);
      free(lastKey);
      if (val && val->value_type == VAL_STRING) {
        return val->value;
      }
    }
  }

  return NULL;
}

char jsonBool(const JObject* obj, const char* keys) {
  if (keys == NULL) {
    return -1;
  }
  char *firstKeys = allButLast(keys);
  const JObject *parentObj = obj;

  if (firstKeys != NULL) {
    parentObj = jsonObject(obj, firstKeys);
    free(firstKeys);
  }

  if (parentObj != NULL) {
    char *lastKey = last(keys);
    if (lastKey != NULL) {
      JValue *val = jsonGet(parentObj, lastKey);
      free(lastKey);
      if (val && val->value_type == VAL_BOOL) {
        return *(char*) val->value;
      }
    }
  }

  return -1;
}

char* jsonBoolArray(const JObject *obj, const char* keys) {
  JValue *val = jsonGet(obj, keys);
  if (val && val->value_type == VAL_BOOL_ARRAY) {
    if (val->size > 0 && val->value) {
      char *ret = malloc(sizeof(ret[0]) * val->size);
      memcpy(ret, val->value, val->size);
      return ret;
    }
  }
  return NULL;
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
  if (obj == 0 || !keys) {
    return 0;
  }

  int index = 0;
  char *key = nextKey(keys, &index);
  JValue *jval = jsonGet(obj, key);
  if (!jval || jval->value_type != VAL_OBJ) {
    free(key);
    return 0;
  }

  JObject *found = (JObject*) jval->value;
  while (index > -1 && found != 0) {
    free(key);
    key = nextKey(keys, &index);
    printf("Looking for key %s\n", key);
    jval = jsonGet(found, key);
    if (jval && jval->value_type == VAL_OBJ) {
      found = (JObject*) jval->value;
    } else if (key) {
      if (jval) {
        printf("Err: Key %s was the wrong type.\n", key);
      } else {
        printf("Err: Object did not contains a next object %s\n", key);
      }
      return 0;
    }
  }
  free(key);
  return found;
}

void jsonFree(JValue *val) {
  if (!val) {
    return;
  }

  short vtype = val->value_type;
  if (vtype == VAL_OBJ) {
    JObject *obj = (JObject*) val->value;
    JEntry *toDel = NULL;
    JValue *valToDel = NULL;
    for (int i = 0; i < obj->_arraySize; ++i) {
      if (obj->entries[i] != NULL) {
        toDel = obj->entries[i];

        valToDel = toDel->value;

        jsonFree((JValue*) valToDel);

        free(toDel->name);
        free(toDel);
      }
    }
    free(obj->entries);
  } else if (vtype == VAL_MIXED_ARRAY || vtype == VAL_OBJ_ARRAY) {
    JValue **values = val->value;
    int count = val->size / sizeof(JValue*);
    for (int i = 0; i < count; ++i) {
      jsonFree(values[i]);
    }
  } else if (vtype == VAL_STRING_ARRAY) {
    char **strings = val->value;
    int count = val->size / sizeof(*strings);
    for (int i = 0; i < count; ++i) {
      free(strings[i]);
    }
  }

  if(vtype != VAL_INT && vtype != VAL_NULL) {
    free(val->value);
  }
  if(vtype != VAL_NULL) {
    free(val);
  }
}
