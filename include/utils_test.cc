#define CATCH_CONFIG_MAIN

#include "catch.hpp"
#include "json.hpp"

#include "globals.h"
#include "utils.h"

#include <iostream>

TEST_CASE("ErrorToViolation") {
    SECTION("Make sure each enum has the corresponding correct message") {
        REQUIRE(ErrorToViolation(kOk) == "Ok" );
        REQUIRE(ErrorToViolation(kAccountAlreadyInitialized) == "account-already-initialized");
        REQUIRE(ErrorToViolation(kAccountNotInitialized) == "account-not-initialized");
        REQUIRE(ErrorToViolation(kCardNotActive) == "card-not-active");
        REQUIRE(ErrorToViolation(kInsufficientLimit) == "insufficient-limit");
        REQUIRE(ErrorToViolation(kHighFrequencySmallInterval) == "high-frequency-small-interval");
        REQUIRE(ErrorToViolation(kDoubledTransaction) == "doubled-transaction");
    }
}

TEST_CASE("ParseISO8601") {
    const std::string standardIso1 = "1997-08-14T01:24:21+00:00";
    const std::string standardIso2 = "2019-02-13T11:00:00.000Z";
    const std::string standardIso3 = "2016-07-30T09:27:06+00:00";

    const unsigned long long standardUnix1 = 871521861;
    const unsigned long long standardUnix2 = 1550052000;
    const unsigned long long standardUnix3 = 1469870826;

    SECTION("Parsing standard datetime from example") {
        REQUIRE((unsigned long long)ParseISO8601(standardIso1) == standardUnix1);
        REQUIRE((unsigned long long)ParseISO8601(standardIso2) == standardUnix2);
        REQUIRE((unsigned long long)ParseISO8601(standardIso3) == standardUnix3);
    }
}

TEST_CASE("IsTransactionFrequent") {
    // Create test vector with transactions.
    std::vector<nlohmann::json> transactions {};

    SECTION("Checking for frequent transactions based on basic examples") {
        nlohmann::json incoming_transaction;
        REQUIRE(IsTransactionFrequent(transactions, incoming_transaction) == false);
    }
}

// TEST_CASE("IsTransactionSimilar") {

// }
