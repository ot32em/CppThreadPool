#include "catch.hpp"

TEST_CASE("Should be passed", "[first time]") {
    REQUIRE(1 == 1);
    REQUIRE(1 == 1);
}

TEST_CASE("Should be failed at final", "[first time]") {
    REQUIRE(1 == 1);
    REQUIRE(1 == 2);
}