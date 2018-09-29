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

    jsonAddStringDup(obj, "status", "Success");
    EXPECT_EQ(obj->size, 1);
    jsonFree(jsonObjectValue(obj));
}

TEST(JsonObjectManipulation, shouldGetKeyValue) {
  JObject *obj = jsonNewObject();
  EXPECT_TRUE(obj != NULL);

  jsonAddStringDup(obj, "status", "Success");
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

  jsonAddStringDup(obj, "status2", "Not Successful");
  jsonAddStringDup(obj, "status", "Success");
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

  jsonAddStringDup(obj, "status", "Success");
  const char* out = jsonString(obj, "status");
  EXPECT_STREQ(out, "Success");
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

  jsonAddObj(nestedTwo, strdup("nestedObject3"), nestedThree);
  jsonAddObj(nestedOne, strdup("nestedObject2"), nestedTwo);
  jsonAddObj(obj, strdup("nestedObject1"), nestedOne);

  JObject* found = jsonObject(obj, "nestedObject1.nestedObject2.nestedObject3");
  EXPECT_EQ(found, nestedThree);
  jsonFree(jsonObjectValue(obj));
}

TEST(JsonObjectManipulation, shouldExpandObjectIfMaxProbesReached) {
  char buf[80];
  
  JObject *expandable = jsonNewObject();
  for(int i = 0; i < 100; ++i) {
    sprintf(buf, "Key %d", i);
    jsonAddInt(expandable, strdup(buf), i);
  }
  EXPECT_EQ(expandable->size, 100);
  
  for(int i = 0; i < 100; ++i) {
    sprintf(buf, "Key %d", i);
    EXPECT_EQ(jsonInt(expandable, strdup(buf)), i);
  }
  jsonFree(jsonObjectValue(expandable));
}


