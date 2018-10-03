#include "gtest/gtest.h"

extern "C" {
  #include "../src/json.h"
  #include "../src/parse.h"
};

#define NO_DUP 0
TEST(JsonObjectManipulation, shouldHaveAValidJValue) {
  JValue *val = jsonStringValue("Success", DUP);
  EXPECT_TRUE(val != NULL);
  EXPECT_EQ(val->value_type, VAL_STRING);
  EXPECT_EQ(val->size, 8);
  jsonFree(val);
}

TEST(JsonObjectManipulation, shouldBuildAnewObjectAndAddAKey) {
    JObject *obj = jsonNewObject();
    EXPECT_TRUE(obj != NULL);

    jsonAddString(obj, "status", strdup("Success"));
    EXPECT_EQ(obj->size, 1);
    jsonFree(jsonObjectValue(obj));
}

TEST(JsonObjectManipulation, shouldGetKeyValue) {
  JObject *obj = jsonNewObject();
  EXPECT_TRUE(obj != NULL);

  jsonAddString(obj, "status", strdup("Success"));
  JValue *val = jsonGet(obj, "status");
  EXPECT_TRUE(val != NULL);
  EXPECT_EQ(val->value_type, VAL_STRING);
  EXPECT_EQ(val->size, 8);
  EXPECT_STREQ((const char*)val->value, "Success");
  jsonFree(jsonObjectValue(obj));
}

TEST(JsonObjectManipulation, shouldGetKeyValueOutOfKeys) {
  JObject *obj = jsonNewObject();
  EXPECT_TRUE(obj != 0);

  jsonAddString(obj, "status2", strdup("Not Successful"));
  jsonAddString(obj, "status", strdup("Success"));
  JValue *val = jsonGet(obj, "status");
  EXPECT_TRUE(val != NULL);
  EXPECT_EQ(val->value_type, VAL_STRING);
  EXPECT_EQ(val->size, 8);
  EXPECT_STREQ((const char*)val->value, "Success");
  jsonFree(jsonObjectValue(obj));
}

TEST(JsonObjectManipulation, shouldGetStringOutOfObjectAsStored) {
  JObject *obj = jsonNewObject();
  EXPECT_EQ(obj->size, 0);

  jsonAddString(obj, "status", strdup("Success"));
  jsonAddString(obj, "hello", strdup("it's hello"));
  jsonAddString(obj, "message", strdup("it's message"));
  const char* out = jsonString(obj, "status");
  EXPECT_STREQ(out, "Success");
  EXPECT_STREQ(jsonString(obj, "hello"), "it's hello");
  EXPECT_STREQ(jsonString(obj, "message"), "it's message");
  jsonFree(jsonObjectValue(obj));
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
  jsonFree(jsonObjectValue(obj));
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
  jsonFree(jsonObjectValue(expandable));
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


