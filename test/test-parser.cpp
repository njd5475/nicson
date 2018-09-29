#include "gtest/gtest.h"

extern "C" {
  #include "../src/json.h"
  #include "../src/parse.h"
};

#define NULL_JVAL (JValue*)(0)

FILE *inlineJson(const char *jstr, char **deleteThis) {
    char *buf = strdup(jstr);
    *deleteThis = buf;
    return fmemopen(buf, (sizeof(char)*strlen(buf)+1), "r");
}

TEST(JsonParserWorks, shouldGetNextToken) {
  char *deleteMe = NULL;
  Parser p;
  memset(&p, 0, sizeof(Parser));
  p.cur = 0;
  p.file = inlineJson("{\"parsesNullValue\": null}", &deleteMe);
  p.buf_seek = -1;
  p.error_message = strdup("Unknown Error");
  p.cur = ffirst(&p);

  ASSERT_EQ(p.cur->type, OPEN_BRACE);
  next(&p);
  ASSERT_EQ(p.cur->type, DOUBLE_QUOTE);
  next(&p);
  ASSERT_EQ(p.cur->count, 15);
  char *term = NULL;
  term = getTerm(&p);
  ASSERT_STREQ(term, "parsesNullValue");

  free(p.error_message);
  free(p.cur);
  free(term);
  free(deleteMe);
  fclose(p.file);
}

TEST(JsonParserWorks, shouldGetPreviousToken) {
  char *deleteMe = NULL;
  Parser p;
  memset(&p, 0, sizeof(p));
  p.file = inlineJson("parsesNullValue\": null}", &deleteMe);
  p.buf_seek = -1;
  p.error_message = strdup("Unknown Error");
  p.cur = ffirst(&p);
  int start = p.cur->seek;
  ASSERT_EQ(p.cur->count,15);
  char *term = NULL;
  term = getTerm(&p);
  ASSERT_STREQ(term, "parsesNullValue");
  free(term);
  next(&p);
  ASSERT_EQ(p.cur->type, DOUBLE_QUOTE);
  jsonRewind(&p, start);
  ASSERT_EQ(p.cur->count, 15);
  term = getTerm(&p);
  ASSERT_STREQ(term, "parsesNullValue");

  free(p.error_message);
  free(p.cur);
  free(term);
  free(deleteMe);
  fclose(p.file);
}

TEST(JsonParserWorks, shouldParseNullvalue) {
  char *deleteMe = NULL;
  JValue *val = jsonParseF(inlineJson("{\"parsesNullValue\": null}", &deleteMe));

  ASSERT_NE(NULL_JVAL, val);
  EXPECT_EQ(val->value_type, VAL_OBJ);
  
  JObject *obj = (JObject*)val->value;
  JValue *nullVal = jsonGet(obj, "parsesNullValue");
  ASSERT_NE(NULL_JVAL, nullVal);
  EXPECT_EQ(nullVal->value_type, VAL_NULL);
  jsonFree(val);
  free(deleteMe);
}

TEST(JsonParserWorks, shouldParseBoolArray) {
  char *deleteMe = NULL;
  JValue *val = jsonParseF(inlineJson("{\"parsesBoolArray\": [\ntrue\n,\n false\n]\n}", &deleteMe));
  ASSERT_NE(NULL_JVAL, val);
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
  ASSERT_NE(NULL_JVAL, val);
  EXPECT_EQ(val->value_type, VAL_OBJ);
  
  jsonFree(val);
  free(deleteMe);
}

TEST(JsonParserWorks, shouldParseBool) {
  char *deleteMe = NULL;
  FILE *file = inlineJson("{\"thisIsTrue\": true, \"thisIsFalse\": false}", &deleteMe);
  JValue *val = jsonParseF(file);
  ASSERT_NE(NULL_JVAL, val);
  EXPECT_EQ(val->value_type, VAL_OBJ);
  jsonFree(val);
  free(deleteMe);
}

TEST(JsonParserWorks, shouldParseEmptyArray) {
  char *deleteMe = NULL;
  FILE *file = inlineJson("{\"thisIsTrue\": []}", &deleteMe);
  JValue *val = jsonParseF(file);
  ASSERT_NE(NULL_JVAL, val);
  EXPECT_EQ(val->value_type, VAL_OBJ);
  jsonFree(val);
  free(deleteMe);
}

TEST(JsonParserWorks, shouldParseArrayAfterBool) {
  char *deleteMe = NULL;
  FILE *file = inlineJson("{\"thisIsTrue\": false, \"thisIsFalse\": []}", &deleteMe);
  JValue *val = jsonParseF(file);
  ASSERT_NE(NULL_JVAL, val);
  EXPECT_EQ(val->value_type, VAL_OBJ);
  jsonFree(val);
  free(deleteMe);
}

TEST(JsonParserWorks, shouldParseTrueValue) {
  char *deleteMe = NULL;
  JValue *val = jsonParseF(inlineJson("{\"parsesTrues\": true}", &deleteMe));
  ASSERT_NE(NULL_JVAL, val);
  EXPECT_EQ(val->value_type, VAL_OBJ);
  
  JObject *obj = (JObject*)val->value;
  EXPECT_EQ(jsonBool(obj, "parsesTrues"), 1);
  jsonFree(val);
  free(deleteMe);
}

TEST(JsonParserWorks, shouldParseFalseValue) {
  char *deleteMe = NULL;
  JValue *val = jsonParseF(inlineJson("{\"parsesFalses\": false}", &deleteMe));
  ASSERT_NE(NULL_JVAL, val);
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
  ASSERT_NE(NULL_JVAL, val);
  jsonFree(val);
  free(deleteMe);

  file = inlineJson("{\"message\":\"hello\"", &deleteMe);
  val = jsonParseF(file);
  ASSERT_NE(NULL_JVAL, val);
  jsonFree(val);
  free(deleteMe);
}

TEST(JsonParserWorks, shouldParseObjectWithStringValues) {
  char *deleteMe = NULL;
  FILE *file = inlineJson("{\"message\":\"hello\", \"status\": \"started\"}", &deleteMe);
  JValue *val = jsonParseF(file);
  ASSERT_NE(NULL_JVAL, val);
  EXPECT_EQ(val->value_type, VAL_OBJ);
  const char* result = jsonString((JObject*)val->value, "message");
  EXPECT_STREQ(result, "hello");
  jsonFree(val);
  free(deleteMe);
}

TEST(JsonParserWorks, shouldParseEmptyObject) {
  char *deleteMe = NULL;
  FILE *file = inlineJson("{}", &deleteMe);
  JValue *val = jsonParseF(file);
  ASSERT_NE(NULL_JVAL, val);
  EXPECT_EQ(val->value_type, VAL_OBJ);
  jsonFree(val);
  free(deleteMe);
}

TEST(JsonParserWorks, shouldParseBools) {
  char *deleteMe = NULL;
  FILE *file = inlineJson("{\"thisIsTrue\": true, \"thisIsFalse\": false}", &deleteMe);
  JValue *val = jsonParseF(file);
  ASSERT_NE(NULL_JVAL, val);
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
  FILE *file = inlineJson("{\"obj7\":\"\\u0002\"}", &deleteMe);
  JValue *val = jsonParseF(file);
  ASSERT_TRUE(val != NULL);
  EXPECT_EQ(VAL_OBJ, val->value_type);
  EXPECT_STREQ("\\u0002", jsonString((JObject*)val->value,"obj7"));
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


TEST(JsonParserWorks, shouldFreeMixedArraysInsideNestedObjects) {
  char *deleteMe = NULL;
  FILE *file = inlineJson("{\"obj3\":{\"key\":[{\"hello67\":\"whatsup\"},{\"hello8\":\"whatsup\"},true,false,null,21345,1,1,1,1,0]}}", &deleteMe);
  JValue *val = jsonParseF(file);
  ASSERT_TRUE(val != NULL);
  EXPECT_EQ(VAL_OBJ, val->value_type);
  jsonFree(val);
  free(deleteMe);
}

