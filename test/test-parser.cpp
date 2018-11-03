#include "gtest/gtest.h"

extern "C" {
  #include "../src/json.h"
  #include "../src/parse.h"
};

#define NULL_JVAL (JItemValue){0}

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
  short type = 0;
  JItemValue val = jsonParseF(inlineJson("{\"parsesNullValue\": null}", &deleteMe), &type);

  ASSERT_NE((void*)NULL, val.object_val);
  EXPECT_EQ(type, VAL_OBJ);
  
  JObject *obj = val.object_val;
  JItemValue nullVal = jsonGet(obj, "parsesNullValue", &type);
  ASSERT_EQ((void*)NULL, nullVal.ptr_val);
  EXPECT_EQ(type, VAL_NULL);
  jsonFree(val, VAL_OBJ);
  free(deleteMe);
}

TEST(JsonParserWorks, shouldParseBoolArray) {
  char *deleteMe = NULL;
  short type = 0;
  JItemValue val = jsonParseF(inlineJson("{\"parsesBoolArray\": [\ntrue\n,\n false\n]\n}", &deleteMe), &type);
  ASSERT_NE(NULL_JVAL.object_val, val.object_val);
  EXPECT_EQ(type, VAL_OBJ);
  
  JObject *obj = val.object_val;
  char *bools = jsonBoolArray(obj, "parsesBoolArray");
  EXPECT_EQ(bools[0], 1);
  EXPECT_EQ(bools[1], 0);
  jsonFree(val, type);
  free(bools);
  free(deleteMe);
}

TEST(JsonParserWorks, shouldParseBoolArrayWithNewLines) {
  char *deleteMe = NULL;
  short type = 0;
  JItemValue val = jsonParseF(inlineJson("{\"something_something\": true,\n\"items\": [\n{\n},\n false\n]\n}", &deleteMe), &type);
  ASSERT_NE(NULL_JVAL.object_val, val.object_val);
  EXPECT_EQ(type, VAL_OBJ);
  
  jsonFree(val, type);
  free(deleteMe);
}

TEST(JsonParserWorks, shouldParseBool) {
  char *deleteMe = NULL;
  FILE *file = inlineJson("{\"thisIsTrue\": true, \"thisIsFalse\": false}", &deleteMe);
  short type = 0;
  JItemValue val = jsonParseF(file, &type);
  ASSERT_NE(NULL_JVAL.object_val, val.object_val);
  EXPECT_EQ(type, VAL_OBJ);
  jsonFree(val, type);
  free(deleteMe);
}

TEST(JsonParserWorks, shouldParseEmptyArray) {
  char *deleteMe = NULL;
  FILE *file = inlineJson("{\"thisIsTrue\": []}", &deleteMe);
  short type = 0;
  JItemValue val = jsonParseF(file, &type);
  ASSERT_NE(NULL_JVAL.object_val, val.object_val);
  EXPECT_EQ(type, VAL_OBJ);
  jsonFree(val, type);
  free(deleteMe);
}

TEST(JsonParserWorks, shouldParseArrayAfterBool) {
  char *deleteMe = NULL;
  short type = 0;
  FILE *file = inlineJson("{\"thisIsTrue\": false, \"thisIsFalse\": []}", &deleteMe);
  JItemValue val = jsonParseF(file, &type);
  ASSERT_NE(NULL_JVAL.object_val, val.object_val);
  EXPECT_EQ(type, VAL_OBJ);
  jsonFree(val, type);
  free(deleteMe);
}

TEST(JsonParserWorks, shouldParseTrueValue) {
  char *deleteMe = NULL;
  short type = 0;
  JItemValue val = jsonParseF(inlineJson("{\"parsesTrues\": true}", &deleteMe), &type);
  ASSERT_NE(NULL_JVAL.object_val, val.object_val);
  EXPECT_EQ(type, VAL_OBJ);
  
  JObject *obj = val.object_val;
  EXPECT_EQ(jsonBool(obj, "parsesTrues"), 1);
  jsonFree(val, type);
  free(deleteMe);
}

TEST(JsonParserWorks, shouldParseFalseValue) {
  char *deleteMe = NULL;
  short type = 0;
  JItemValue val = jsonParseF(inlineJson("{\"parsesFalses\": false}", &deleteMe), &type);
  ASSERT_NE(NULL_JVAL.object_val, val.object_val);
  EXPECT_EQ(type, VAL_OBJ);
  
  JObject *obj = val.object_val;
  EXPECT_EQ(jsonBool(obj, "parsesFalses"), 0);
  jsonFree(val, type);
  free(deleteMe);
}

TEST(JsonParserWorks, shouldEndAtEndOfStream) {
  char *deleteMe = NULL;
  short type = 0;
  FILE *file = inlineJson("{", &deleteMe);
  JItemValue val = jsonParseF(file, &type);
  ASSERT_NE(NULL_JVAL.object_val, val.object_val);
  jsonFree(val, type);
  free(deleteMe);

  file = inlineJson("{\"message\":\"hello\"", &deleteMe);
  short vtype = 0;
  val = jsonParseF(file, &vtype);
  ASSERT_NE(NULL_JVAL.object_val, val.object_val);
  jsonFree(val, type);
  free(deleteMe);
}

TEST(JsonParserWorks, shouldParseObjectWithStringValues) {
  char *deleteMe = NULL;
  FILE *file = inlineJson("{\"message\":\"hello\", \"status\": \"started\"}", &deleteMe);
  short type = 0;
  JItemValue val = jsonParseF(file, &type);
  ASSERT_NE(NULL_JVAL.object_val, val.object_val);
  EXPECT_EQ(type, VAL_OBJ);
  const char* result = jsonString(val.object_val, "message");
  EXPECT_STREQ(result, "hello");
  jsonFree(val, type);
  free(deleteMe);
}

TEST(JsonParserWorks, shouldParseEmptyObject) {
  char *deleteMe = NULL;
  FILE *file = inlineJson("{}", &deleteMe);
  short type = 0;
  JItemValue val = jsonParseF(file, &type);
  ASSERT_NE(NULL_JVAL.object_val, val.object_val);
  EXPECT_EQ(type, VAL_OBJ);
  jsonFree(val, type);
  free(deleteMe);
}

TEST(JsonParserWorks, shouldParseBools) {
  char *deleteMe = NULL;
  FILE *file = inlineJson("{\"thisIsTrue\": true, \"thisIsFalse\": false}", &deleteMe);
  short type = 0;
  JItemValue val = jsonParseF(file, &type);
  ASSERT_NE(NULL_JVAL.object_val, val.object_val);
  EXPECT_EQ(type, VAL_OBJ);
  jsonFree(val, type);
  free(deleteMe);
}

TEST (JsonParserWorks, shouldParseIntegerValues) {
  char *deleteMe = NULL;
  FILE* file = inlineJson("{\"val\":200860}", &deleteMe);
  short type = 0;
  JItemValue val = jsonParseF(file, &type);
  ASSERT_TRUE(val.object_val != NULL);
  JObject* obj = val.object_val;
  JItemValue valVal = jsonGet(obj, "val", &type);
  EXPECT_EQ(VAL_INT, type);
  EXPECT_EQ(200860, valVal.int_val);
  jsonFree(val, VAL_OBJ);
  free(deleteMe);
}

TEST (JsonParserWorks, shouldParseFloatValues) {
  char *deleteMe = NULL;
  FILE* file = inlineJson("{\"val\":0.123}", &deleteMe);
  short type = 0;
  JItemValue val = jsonParseF(file, &type);
  ASSERT_TRUE(val.object_val != NULL);
  EXPECT_EQ(VAL_OBJ, type);
  short ftype = 0;
  JItemValue valVal = jsonGet(val.object_val, "val", &ftype);
  EXPECT_EQ(VAL_FLOAT, ftype);
  EXPECT_EQ(0.123f, jsonFloat(val.object_val, "val"));
  jsonFree(val, type);
  free(deleteMe);
}

TEST (JsonParserWorks, shouldParseFloatWithScientificNotation) {
  char *deleteMe = NULL;
  FILE* file = inlineJson("{\"val\":0.123E20}", &deleteMe);
  short type = 0;
  JItemValue val = jsonParseF(file, &type);
  ASSERT_TRUE(val.object_val != NULL);
  EXPECT_EQ(VAL_OBJ, type);
  short ftype = 0;
  JItemValue valVal = jsonGet(val.object_val, "val", &ftype);
  EXPECT_EQ(VAL_FLOAT, ftype);
  EXPECT_EQ(0.123e20f, jsonFloat(val.object_val, "val"));
  jsonFree(val, type);
  free(deleteMe);
}

TEST (JsonParserWorks, shouldParseArrayOfIntegers) {
  char *deleteMe = NULL;
  FILE* file = inlineJson("[1,2,3,4,5]", &deleteMe);
  short type = 0;
  JItemValue val = jsonParseF(file, &type);
  ASSERT_TRUE(val.object_val != NULL);
  EXPECT_EQ(VAL_INT_ARRAY, type);
  jsonFree(val, type);
  free(deleteMe);
}

TEST (JsonParserWorks, shouldParseArrayOfFloats) {
  char *deleteMe = NULL;
  FILE *file = inlineJson("[-4.54E37, 1.1, 0.123E7, 3.345, 5.43, +8.9, -0.0004, +0.03]", &deleteMe);
  short type = 0;
  JItemValue val = jsonParseF(file, &type);
  ASSERT_TRUE(val.object_val != NULL);
  EXPECT_EQ(VAL_FLOAT_ARRAY, type);
  jsonFree(val, type);
  free(deleteMe);
}

TEST (JsonParserWorks, shouldParseArrayOfDoubles) {
  char *deleteMe = NULL;
  FILE *file = inlineJson("[1.3E39, 2.4E124]", &deleteMe);
  short type = 0;
  JItemValue val = jsonParseF(file, &type);
  ASSERT_TRUE(val.object_val != NULL);
  EXPECT_EQ(VAL_DOUBLE_ARRAY, type);
  jsonFree(val, type);
  free(deleteMe);
}

TEST (JsonParserWorks, shouldParseNestedJsonObjects) {
  char *deleteMe = NULL;
  FILE *file = inlineJson("{\"obj\":{\"obj\":{\"status\":\"I am nested\"}}}", &deleteMe);
  short type = 0;
  JItemValue val = jsonParseF(file, &type);
  ASSERT_TRUE(val.object_val != NULL);
  EXPECT_EQ(VAL_OBJ, type);
  EXPECT_STREQ("I am nested", jsonString(val.object_val,"obj.obj.status"));
  jsonFree(val, type);
  free(deleteMe);
}

TEST (JsonParserWorks, shouldParseEscapedSequences) {
  char *deleteMe = NULL;
  FILE *file = inlineJson("{\"obj7\":\"\\u0002\"}", &deleteMe);
  short type = 0;
  JItemValue val = jsonParseF(file, &type);
  ASSERT_TRUE(val.object_val != NULL);
  EXPECT_EQ(VAL_OBJ, type);
  EXPECT_STREQ("\\u0002", jsonString(val.object_val,"obj7"));
  jsonFree(val, type);
  free(deleteMe);
}

TEST (JsonParserWorks, shouldParseEscapedQuotes) {
  char *deleteMe = NULL;
  FILE *file = inlineJson("{\"obj\":\"\\\"\"}", &deleteMe);
  short type = 0;
  JItemValue val = jsonParseF(file, &type);
  ASSERT_TRUE(val.object_val != NULL);
  EXPECT_EQ(VAL_OBJ, type);
  EXPECT_STREQ("\\\"", jsonString(val.object_val,"obj"));
  jsonFree(val, type);
  free(deleteMe);
}

TEST(JsonParserWorks, shouldFreeMixedArraysInsideNestedObjects) {
  char *deleteMe = NULL;
  FILE *file = inlineJson("{\"obj3\":{\"key\":[{\"hello67\":\"whatsup\"},{\"hello8\":\"whatsup\"},true,false,null,0]}}", &deleteMe);
  short type = 0;
  JItemValue val = jsonParseF(file, &type);
  ASSERT_TRUE(val.object_val != NULL);
  EXPECT_EQ(VAL_OBJ, type);
  jsonFree(val, type);
  free(deleteMe);
}

