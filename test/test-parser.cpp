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

TEST(JsonParserWorks, shouldParseNullvalue) {
  char *deleteMe = NULL;
  JValue *val = jsonParseF(inlineJson("{\"parsesNullValue\": null}", &deleteMe));
  ASSERT_NE(NULL, val);
  EXPECT_EQ(val->value_type, VAL_OBJ);
  
  JObject *obj = (JObject*)val->value;
  JValue *nullVal = jsonGet(obj, "parsesNullValue");
  ASSERT_NE(NULL, nullVal);
  EXPECT_EQ(nullVal->value_type, VAL_NULL);
  jsonFree(val);
  free(deleteMe);
}

TEST(JsonParserWorks, shouldParseBoolArray) {
  char *deleteMe = NULL;
  JValue *val = jsonParseF(inlineJson("{\"parsesBoolArray\": [\ntrue\n,\n false\n]\n}", &deleteMe));
  ASSERT_NE(NULL, val);
  EXPECT_EQ(val->value_type, VAL_OBJ);
  
  JObject *obj = (JObject*)val->value;
  char *bools = jsonBoolArray(obj, "parsesBoolArray");
  EXPECT_EQ(bools[0], 1);
  EXPECT_EQ(bools[1], 0);
  jsonFree(val);
  free(bools);
  free(deleteMe);
}

TEST(JsonParserWorks, shouldParseBoolArrayWithNewLines) {
  char *deleteMe = NULL;
  JValue *val = jsonParseF(inlineJson("{\"something_something\": true,\n\"items\": [\n{\n},\n false\n]\n}", &deleteMe));
  ASSERT_NE(NULL, val);
  EXPECT_EQ(val->value_type, VAL_OBJ);
  
  jsonFree(val);
  free(deleteMe);
}

TEST(JsonParserWorks, shouldParseBool) {
  char *deleteMe = NULL;
  FILE *file = inlineJson("{\"thisIsTrue\": true, \"thisIsFalse\": false}", &deleteMe);
  JValue *val = jsonParseF(file);
  ASSERT_NE(NULL, val);
  EXPECT_EQ(val->value_type, VAL_OBJ);
  jsonFree(val);
  free(deleteMe);
}

TEST(JsonParserWorks, shouldParseEmptyArray) {
  char *deleteMe = NULL;
  FILE *file = inlineJson("{\"thisIsTrue\": []}", &deleteMe);
  JValue *val = jsonParseF(file);
  ASSERT_NE(NULL, val);
  EXPECT_EQ(val->value_type, VAL_OBJ);
  jsonFree(val);
  free(deleteMe);
}

TEST(JsonParserWorks, shouldParseArrayAfterBool) {
  char *deleteMe = NULL;
  FILE *file = inlineJson("{\"thisIsTrue\": false, \"thisIsFalse\": []}", &deleteMe);
  JValue *val = jsonParseF(file);
  ASSERT_NE(NULL, val);
  EXPECT_EQ(val->value_type, VAL_OBJ);
  jsonFree(val);
  free(deleteMe);
}

TEST(JsonParserWorks, shouldParseTrueValue) {
  char *deleteMe = NULL;
  JValue *val = jsonParseF(inlineJson("{\"parsesTrues\": true}", &deleteMe));
  ASSERT_NE(NULL, val);
  EXPECT_EQ(val->value_type, VAL_OBJ);
  
  JObject *obj = (JObject*)val->value;
  EXPECT_EQ(jsonBool(obj, "parsesTrues"), 1);
  jsonFree(val);
  free(deleteMe);
}

TEST(JsonParserWorks, shouldParseFalseValue) {
  char *deleteMe = NULL;
  JValue *val = jsonParseF(inlineJson("{\"parsesFalses\": false}", &deleteMe));
  ASSERT_NE(NULL, val);
  EXPECT_EQ(val->value_type, VAL_OBJ);
  
  JObject *obj = (JObject*)val->value;
  EXPECT_EQ(jsonBool(obj, "parsesFalses"), 0);
  jsonFree(val);
  free(deleteMe);
}

TEST(JsonParserWorks, shouldEndAtEndOfStream) {
  char *deleteMe = NULL;
  FILE *file = inlineJson("{", &deleteMe);
  JValue *val = jsonParseF(file);
  ASSERT_NE(NULL, val);
  jsonFree(val);
  free(deleteMe);

  file = inlineJson("{\"message\":\"hello\"", &deleteMe);
  val = jsonParseF(file);
  ASSERT_NE(NULL, val);
  jsonFree(val);
  free(deleteMe);
}

TEST(JsonParserWorks, shouldParseObjectWithStringValues) {
  char *deleteMe = NULL;
  FILE *file = inlineJson("{\"message\":\"hello\"}", &deleteMe);
  JValue *val = jsonParseF(file);
  ASSERT_NE(NULL, val);
  EXPECT_EQ(val->value_type, VAL_OBJ);
  EXPECT_STREQ(jsonString((JObject*)val->value, "message"), "hello");
  jsonFree(val);
  free(deleteMe);
}

TEST(JsonParserWorks, shouldParseEmptyObject) {
  char *deleteMe = NULL;
  FILE *file = inlineJson("{}", &deleteMe);
  JValue *val = jsonParseF(file);
  ASSERT_NE(NULL, val);
  EXPECT_EQ(val->value_type, VAL_OBJ);
  jsonFree(val);
  free(deleteMe);
}

TEST(JsonParserWorks, shouldParseBools) {
  char *deleteMe = NULL;
  FILE *file = inlineJson("{\"thisIsTrue\": true, \"thisIsFalse\": false}", &deleteMe);
  JValue *val = jsonParseF(file);
  ASSERT_NE(NULL, val);
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

TEST (JsonParserWorks, shouldParseEscapedSequences) {
  char *deleteMe = NULL;
  FILE *file = inlineJson("{\"obj\":\"\\u0002\"}", &deleteMe);
  JValue *val = jsonParseF(file);
  ASSERT_TRUE(val != NULL);
  EXPECT_EQ(VAL_OBJ, val->value_type);
  EXPECT_STREQ("\\u0002", jsonString((JObject*)val->value,"obj"));
  jsonFree(val);
  free(deleteMe);
}

TEST (JsonParserWorks, shouldParseEscapedQuotes) {
  char *deleteMe = NULL;
  FILE *file = inlineJson("{\"obj\":\"\\\"\"}", &deleteMe);
  JValue *val = jsonParseF(file);
  ASSERT_TRUE(val != NULL);
  EXPECT_EQ(VAL_OBJ, val->value_type);
  EXPECT_STREQ("\\\"", jsonString((JObject*)val->value,"obj"));
  jsonFree(val);
  free(deleteMe);
}

