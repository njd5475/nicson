#include "gtest/gtest.h"

extern "C" {
  #include "json.h"
  #include "parse.h"
};

FILE *inlineJson(const char *jstr) {
    char *buf = strdup(jstr);
    return fmemopen(buf, (sizeof(char)*strlen(buf)+1), "r");
}

TEST(JsonParserWorks, shouldTokenizeIntoGroups) {
  Tok *head = first("./src/test1.json");
  Tok *cur = head;
  Tok *last = 0;
  while(cur) {
    cur = next(cur);
    if(cur) {
      last = cur;
    }
  }
  
  //now delete
  fclose(last->file);
  while(last != NULL) {
    Tok *toDel = cur;
    last = last->previous;
    free(toDel);
  }
}

TEST(JsonParserWorks, shouldEndAtEndOfStream) {
  FILE *file = inlineJson("{");
  JValue *val = jsonParseF(file);
  EXPECT_TRUE(val == NULL);

  file = inlineJson("{\"message\":\"hello\"");
  val = jsonParseF(file);
  EXPECT_TRUE(val != NULL);
}

TEST(JsonParserWorks, shouldParseObjectWithStringValues) {
  FILE *file = inlineJson("{\"message\":\"hello\"}");
  JValue *val = jsonParseF(file);
  EXPECT_TRUE(val != NULL);
  EXPECT_EQ(val->value_type, VAL_OBJ);
  EXPECT_STREQ(jsonString((JObject*)val->value, "message"), "hello");
}

TEST(JsonParserWorks, shouldParseEmptyObject) {
  FILE *file = inlineJson("{}");
  JValue *val = jsonParseF(file);
  EXPECT_TRUE(val != NULL);
  EXPECT_EQ(val->value_type, VAL_OBJ);
}

TEST (JsonParserWorks, shouldParseIntegerValues) {
  FILE* file = inlineJson("{\"val\":200860}");
  JValue *val = jsonParseF(file);
  ASSERT_TRUE(val != NULL);
  JObject* obj = (JObject*)val->value;
  JValue* vals = jsonGet(obj, "val");
  ASSERT_TRUE(vals != NULL);
  EXPECT_EQ(VAL_INT, vals->value_type);
  EXPECT_EQ(200860, jsonInt(obj, "val"));
}

TEST (JsonParserWorks, shouldParseFloatValues) {
  FILE* file = inlineJson("{\"val\":0.123}");
  JValue *val = jsonParseF(file);
  ASSERT_TRUE(val != NULL);
  EXPECT_EQ(VAL_OBJ, val->value_type);
  EXPECT_EQ(VAL_FLOAT, jsonGet((JObject*)val->value, "val")->value_type);
  EXPECT_EQ(0.123f, jsonFloat((JObject*)val->value, "val"));
}

TEST (JsonParserWorks, shouldParseFloatWithScientificNotation) {
  FILE* file = inlineJson("{\"val\":0.123E9}");
  JValue *val = jsonParseF(file);
  ASSERT_TRUE(val != NULL);
  EXPECT_EQ(VAL_OBJ, val->value_type);
  EXPECT_EQ(VAL_FLOAT, jsonGet((JObject*)val->value, "val")->value_type);
  EXPECT_EQ(0.123e9f, jsonFloat((JObject*)val->value, "val"));
}

TEST (JsonParserWorks, shouldParseArrayOfIntegers) {
  FILE* file = inlineJson("[1,2,3,4,5]");
  JValue *val = jsonParseF(file);
  ASSERT_TRUE(val != NULL);
  EXPECT_EQ(VAL_INT_ARRAY, val->value_type);
  free(val->value);
  free(val);
}

TEST (JsonParserWorks, shouldParseArrayOfFloats) {
  FILE *file = inlineJson("[1.1, 2.123, 3.345, 4.54, 5.43]");
  JValue *val = jsonParseF(file);
  ASSERT_TRUE(val != NULL);
  EXPECT_EQ(VAL_FLOAT_ARRAY, val->value_type);
  free(val->value);
  free(val);
}

TEST (JsonParserWorks, shouldParseArrayOfDoubles) {
  FILE *file = inlineJson("[1.3E23, 2.4E24]");
  JValue *val = jsonParseF(file);
  ASSERT_TRUE(val != NULL);
  EXPECT_EQ(VAL_DOUBLE_ARRAY, val->value_type);
  free(val->value);
  free(val);
}

TEST (JsonParserWorks, shouldParseNestedJsonObjects) {
  FILE *file = inlineJson("{\"obj\":{\"obj\":{\"status\":\"I am nested\"}}}");
  JValue *val = jsonParseF(file);
  ASSERT_TRUE(val != NULL);
  EXPECT_EQ(VAL_OBJ, val->value_type);
  EXPECT_STREQ("I am nested", jsonString((JObject*)val->value,"obj.obj.status"));
}


TEST(JsonObjectManipulation, shouldHaveAValidJValue) {
  JValue *val = jsonStringValue("Success");
  EXPECT_TRUE(val != NULL);
  EXPECT_EQ(val->value_type, VAL_STRING);
  EXPECT_EQ(val->size, 8);
  free(val->value);
  free(val);
}

TEST(JsonObjectManipulation, shouldBuildAnewObjectAndAddAKey) {
    JObject *obj = jsonNewObject();
    EXPECT_TRUE(obj != NULL);

    jsonAddString(obj, "status", "Success");
    EXPECT_EQ(obj->size, 1);
    jsonFree(obj);
}

TEST(JsonObjectManipulation, shouldGetKeyValue) {
  JObject *obj = jsonNewObject();
  EXPECT_TRUE(obj != NULL);

  jsonAddString(obj, "status", "Success");
  JValue *val = jsonGet(obj, "status");
  EXPECT_TRUE(val != NULL);
  EXPECT_EQ(val->value_type, VAL_STRING);
  EXPECT_EQ(val->size, 8);
  EXPECT_STREQ((const char*)val->value, "Success");
}

TEST(JsonObjectManipulation, shouldGetKeyValueOutOfKeys) {
  JObject *obj = jsonNewObject();
  EXPECT_TRUE(obj != 0);

  jsonAddString(obj, "status2", "Not Successful");
  jsonAddString(obj, "status", "Success");
  JValue *val = jsonGet(obj, "status");
  EXPECT_TRUE(val != NULL);
  EXPECT_EQ(val->value_type, VAL_STRING);
  EXPECT_EQ(val->size, 8);
  EXPECT_STREQ((const char*)val->value, "Success");
}

TEST(JsonObjectManipulation, shouldGetStringOutOfObjectAsStored) {
  JObject *obj = jsonNewObject();
  EXPECT_EQ(obj->size, 0);

  jsonAddString(obj, "status", "Success");
  const char* out = jsonString(obj, "status");
  EXPECT_STREQ(out, "Success");
}

TEST(JsonObjectManipulation, shouldGetObjectValueOutOfNestedKeys) {
  JObject *obj = jsonNewObject();
  EXPECT_EQ(obj->size, 0);

  JObject *nestedOne =jsonNewObject();
  EXPECT_EQ(nestedOne->size, 0);

  JObject *nestedTwo = jsonNewObject();
  EXPECT_EQ(nestedTwo->size, 0);

  JObject *nestedThree = jsonNewObject();

  jsonAddObj(nestedTwo, "nestedObject3", nestedThree);
  jsonAddObj(nestedOne, "nestedObject2", nestedTwo);
  jsonAddObj(obj, "nestedObject1", nestedOne);

  JObject* found = jsonObject(obj, "nestedObject1.nestedObject2.nestedObject3");
  EXPECT_EQ(found, nestedThree);
}

