#include "gtest/gtest.h"

extern "C" {
  #include "../src/json.h"
  #include "../src/parse.h"
};

#define NO_DUP 0

TEST(JsonObjectManipulation, shouldBuildAnewObjectAndAddAKey) {
    JObject *obj = jsonNewObject();
    EXPECT_TRUE(obj != NULL);

    jsonAddString(obj, "status", "Success");
    EXPECT_EQ(obj->size, 1);
    JItemValue val;
    val.object_val = obj;
    jsonFree(val, VAL_OBJ);
}

TEST(JsonObjectManipulation, shouldGetKeyValue) {
  JObject *obj = jsonNewObject();
  EXPECT_TRUE(obj != NULL);

  jsonAddString(obj, "status", "Success");
  short type = 0;
  JItemValue val = jsonGet(obj, "status", &type);
  EXPECT_TRUE(val.string_val != NULL);
  EXPECT_EQ(type, VAL_STRING);
  EXPECT_STREQ(val.string_val, "Success");
  jsonFree(val, type);
}

TEST(JsonObjectManipulation, shouldGetKeyValueOutOfKeys) {
  JObject *obj = jsonNewObject();
  EXPECT_TRUE(obj != 0);

  jsonAddString(obj, "status2", "Not Successful");
  jsonAddString(obj, "status", "Success");
  short type;
  JItemValue val = jsonGet(obj, "status", &type);
  EXPECT_TRUE(val.string_val != NULL);
  EXPECT_EQ(type, VAL_STRING);
  EXPECT_STREQ(val.string_val, "Success");
  jsonFree(val, type);
}

TEST(JsonObjectManipulation, shouldGetStringOutOfObjectAsStored) {
  JObject *obj = jsonNewObject();
  EXPECT_EQ(obj->size, 0);

  jsonAddString(obj, "status", "Success");
  jsonAddString(obj, "hello", "it's hello");
  jsonAddString(obj, "message", "it's message");
  const char* out = jsonString(obj, "status");
  EXPECT_STREQ(out, "Success");
  EXPECT_STREQ(jsonString(obj, "hello"), "it's hello");
  EXPECT_STREQ(jsonString(obj, "message"), "it's message");
  JItemValue val = { obj };
  jsonFree(val, VAL_OBJ);
}

TEST(JsonObjectManipulation, shouldGetObjectValueOutOfNestedKeys) {
  JObject *obj = jsonNewObject();
  EXPECT_EQ(obj->size, 0);

  JObject *nestedOne = jsonNewObject();
  EXPECT_EQ(nestedOne->size, 0);

  JObject *nestedTwo = jsonNewObject();
  EXPECT_EQ(nestedTwo->size, 0);

  JObject *nestedThree = jsonNewObject();

  jsonAddObj(nestedTwo, "nestedObject3", nestedThree);
  jsonAddObj(nestedOne, "nestedObject2", nestedTwo);
  jsonAddObj(obj, "nestedObject1", nestedOne);

  JObject* found = jsonObject(obj, "nestedObject1.nestedObject2.nestedObject3");
  EXPECT_EQ(found, nestedThree);
  jsonFree( (JItemValue) { obj }, VAL_OBJ);
}

TEST(JsonObjectManipulation, shouldExpandObjectIfMaxProbesReached) {
  char buf[80];
  
  JObject *expandable = jsonNewObject();
  for(int i = 0; i < 100; ++i) {
    sprintf(buf, "Key %d", i);
    jsonAddInt(expandable, buf, i);
  }
  EXPECT_EQ(expandable->size, 100);
  
  for(int i = 0; i < 100; ++i) {
    sprintf(buf, "Key %d", i);
    EXPECT_EQ(jsonInt(expandable, buf), i);
  }
  jsonFree( (JItemValue) { expandable }, VAL_OBJ);
}

TEST(JsonObjectManipulation, shouldGetStringsFromCache) {
  JObject* store = jsonNewObject();

  jsonAddString(store, "status", "started");
  jsonAddString(store, "second_status", "started");

  const char* firstStatus = jsonString(store, "status");
  const char* secondStatus = jsonString(store, "second_status");
  EXPECT_EQ(firstStatus, secondStatus);
}

TEST(JsonObjectManipulation, shouldGetStringsFromCacheNoKeyInterpretation) {
  JObject* store = jsonNewObject();

  jsonAddString(store, "status", "started.later");
  jsonAddString(store, "second_status", "started.later");

  const char* firstStatus = jsonString(store, "status");
  const char* secondStatus = jsonString(store, "second_status");
  EXPECT_EQ(firstStatus, secondStatus);
}


