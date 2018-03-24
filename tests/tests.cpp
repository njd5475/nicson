#include "gtest/gtest.h"

#include "json.h"

TEST (SquareRootTest, PositiveNos) {
    JObject *obj = jsonNewObject();
    EXPECT_TRUE(obj);
}
