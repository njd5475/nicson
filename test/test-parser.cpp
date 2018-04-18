#include "gtest/gtest.h"

extern "C" {
  #include "json.h"
  #include "parse.h"
};

FILE *inlineJson(const char *jstr, char **deleteThis) {
    char *buf = strdup(jstr);
    *deleteThis = buf;
    return fmemopen(buf, (sizeof(char)*strlen(buf)+1), "r");
}

TEST(JsonParserWorks, shouldEndAtEndOfStream) {
  char *deleteMe = NULL;
  FILE *file = inlineJson("{", &deleteMe);
  JValue *val = jsonParseF(file);
  EXPECT_TRUE(val == NULL);
  jsonFree(val);
  free(deleteMe);

  file = inlineJson("{\"message\":\"hello\"", &deleteMe);
  val = jsonParseF(file);
  EXPECT_TRUE(val != NULL);
  jsonFree(val);
  free(deleteMe);
}

TEST(JsonParserWorks, shouldParseObjectWithStringValues) {
  char *deleteMe = NULL;
  FILE *file = inlineJson("{\"message\":\"hello\"}", &deleteMe);
  JValue *val = jsonParseF(file);
  EXPECT_TRUE(val != NULL);
  EXPECT_EQ(val->value_type, VAL_OBJ);
  EXPECT_STREQ(jsonString((JObject*)val->value, "message"), "hello");
  jsonFree(val);
  free(deleteMe);
}

TEST(JsonParserWorks, shouldParseEmptyObject) {
  char *deleteMe = NULL;
  FILE *file = inlineJson("{}", &deleteMe);
  JValue *val = jsonParseF(file);
  EXPECT_TRUE(val != NULL);
  EXPECT_EQ(val->value_type, VAL_OBJ);
  jsonFree(val);
  free(deleteMe);
}

TEST (JsonParserWorks, shouldParseIntegerValues) {
  char *deleteMe = NULL;
  FILE* file = inlineJson("{\"val\":200860}", &deleteMe);
  JValue *val = jsonParseF(file);
  ASSERT_TRUE(val != NULL);
  JObject* obj = (JObject*)val->value;
  JValue* vals = jsonGet(obj, "val");
  ASSERT_TRUE(vals != NULL);
  EXPECT_EQ(VAL_INT, vals->value_type);
  EXPECT_EQ(200860, jsonInt(obj, "val"));
  jsonFree(val);
  free(deleteMe);
}

TEST (JsonParserWorks, shouldParseFloatValues) {
  char *deleteMe = NULL;
  FILE* file = inlineJson("{\"val\":0.123}", &deleteMe);
  JValue *val = jsonParseF(file);
  ASSERT_TRUE(val != NULL);
  EXPECT_EQ(VAL_OBJ, val->value_type);
  EXPECT_EQ(VAL_FLOAT, jsonGet((JObject*)val->value, "val")->value_type);
  EXPECT_EQ(0.123f, jsonFloat((JObject*)val->value, "val"));
  jsonFree(val);
  free(deleteMe);
}

TEST (JsonParserWorks, shouldParseFloatWithScientificNotation) {
  char *deleteMe = NULL;
  FILE* file = inlineJson("{\"val\":0.123E9}", &deleteMe);
  JValue *val = jsonParseF(file);
  ASSERT_TRUE(val != NULL);
  EXPECT_EQ(VAL_OBJ, val->value_type);
  EXPECT_EQ(VAL_FLOAT, jsonGet((JObject*)val->value, "val")->value_type);
  EXPECT_EQ(0.123e9f, jsonFloat((JObject*)val->value, "val"));
  jsonFree(val);
  free(deleteMe);
}

TEST (JsonParserWorks, shouldParseArrayOfIntegers) {
  char *deleteMe = NULL;
  FILE* file = inlineJson("[1,2,3,4,5]", &deleteMe);
  JValue *val = jsonParseF(file);
  ASSERT_TRUE(val != NULL);
  EXPECT_EQ(VAL_INT_ARRAY, val->value_type);
  jsonFree(val);
  free(deleteMe);
}

TEST (JsonParserWorks, shouldParseArrayOfFloats) {
  char *deleteMe = NULL;
  FILE *file = inlineJson("[1.1, 2.123, 3.345, 4.54, 5.43]", &deleteMe);
  JValue *val = jsonParseF(file);
  ASSERT_TRUE(val != NULL);
  EXPECT_EQ(VAL_FLOAT_ARRAY, val->value_type);
  jsonFree(val);
  free(deleteMe);
}

TEST (JsonParserWorks, shouldParseArrayOfDoubles) {
  char *deleteMe = NULL;
  FILE *file = inlineJson("[1.3E23, 2.4E24]", &deleteMe);
  JValue *val = jsonParseF(file);
  ASSERT_TRUE(val != NULL);
  EXPECT_EQ(VAL_DOUBLE_ARRAY, val->value_type);
  jsonFree(val);
  free(deleteMe);
}

TEST (JsonParserWorks, shouldParseNestedJsonObjects) {
  char *deleteMe = NULL;
  FILE *file = inlineJson("{\"obj\":{\"obj\":{\"status\":\"I am nested\"}}}", &deleteMe);
  JValue *val = jsonParseF(file);
  ASSERT_TRUE(val != NULL);
  EXPECT_EQ(VAL_OBJ, val->value_type);
  EXPECT_STREQ("I am nested", jsonString((JObject*)val->value,"obj.obj.status"));
  jsonFree(val);
  free(deleteMe);
}
