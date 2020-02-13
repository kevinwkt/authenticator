#define CATCH_CONFIG_MAIN

#include "catch.hpp"
#include "json.hpp"

#include "globals.h"
#include "utils.h"

#include <iostream>

TEST_CASE("ErrorToViolation") {
  SECTION("Make sure each enum has the corresponding correct message") {
    REQUIRE(ErrorToViolation(kOk) == "Ok");
    REQUIRE(ErrorToViolation(kAccountAlreadyInitialized) ==
            "account-already-initialized");
    REQUIRE(ErrorToViolation(kAccountNotInitialized) ==
            "account-not-initialized");
    REQUIRE(ErrorToViolation(kCardNotActive) == "card-not-active");
    REQUIRE(ErrorToViolation(kInsufficientLimit) == "insufficient-limit");
    REQUIRE(ErrorToViolation(kHighFrequencySmallInterval) ==
            "high-frequency-small-interval");
    REQUIRE(ErrorToViolation(kDoubledTransaction) == "doubled-transaction");
  }
}

TEST_CASE("ParseISO8601") {
  // Create input dates in 2 different ISO8601 forms.
  const std::string standardIso1 = "1997-08-14T01:24:21+00:00";
  const std::string standardIso2 = "2019-02-13T10:00:00.000Z";
  const std::string standardIso3 = "2016-07-30T09:27:06+00:00";
  const std::string standardIso4 = "2008-06-10T18:55:24.000Z";
  const std::string standardIso5 = "2020-01-13T01:02:13.000Z";

  // Create expected outputs in ull form.
  const unsigned long long standardUnix1 = 871521861;
  const unsigned long long standardUnix2 = 1550052000;
  const unsigned long long standardUnix3 = 1469870826;
  const unsigned long long standardUnix4 = 1213124124;

  SECTION("Parsing standard datetime from example") {
    REQUIRE((unsigned long long)ParseISO8601(standardIso1) == standardUnix1);
    REQUIRE((unsigned long long)ParseISO8601(standardIso2) == standardUnix2);
    REQUIRE((unsigned long long)ParseISO8601(standardIso3) == standardUnix3);
    REQUIRE((unsigned long long)ParseISO8601(standardIso4) == standardUnix4);
  }
}

TEST_CASE("IsTransactionFrequent") {
  // Create test vector with base transactions.
  std::vector<nlohmann::json> small_transactions{
      {
          {"merchant", "a"},
          {"amount", 10},
          {"time", "2008-06-10T18:55:24.000z"},
      },
  };

  std::vector<nlohmann::json> big_transactions{
      {
          {"merchant", "a"},
          {"amount", 10},
          {"time", "2008-06-10T18:55:24.000z"},
      },
      {
          {"merchant", "b"},
          {"amount", 20},
          {"time", "2008-06-10T18:55:26.000z"},
      },
      {
          {"merchant", "c"},
          {"amount", 20},
          {"time", "2008-06-10T18:55:26.000z"},
      },
  };

  SECTION("Checking for frequent transactions when transaction vector size is "
          "less than kMaxFrequency") {
    nlohmann::json random_incoming_transaction = {
        {"merchant", "c"},
        {"amount", 10},
        {"time", "2008-06-10T18:55:30.000Z"},
    };
    nlohmann::json window_edge_transaction = {
        {"merchant", "c"},
        {"amount", 10},
        {"time", "2008-06-10T18:55:31.000Z"},
    };

    // Incoming transaction is inside the 2 minute window but there are only 2
    // transactions. Must return false.
    REQUIRE(IsTransactionFrequent(small_transactions,
                                  random_incoming_transaction) == false);
    // Incoming transaction is on the edge of the 2 minute window but there are
    // only 2 transactions. Must return false.
    REQUIRE(IsTransactionFrequent(small_transactions,
                                  window_edge_transaction) == false);
  }

  SECTION("Checking for frequent transactions based on basic examples with "
          "default parameters") {
    nlohmann::json before_window_edge_transaction = {
        {"merchant", "c"},
        {"amount", 10},
        {"time", "2008-06-10T18:57:23.000Z"},
    };
    nlohmann::json window_edge_transaction = {
        {"merchant", "c"},
        {"amount", 10},
        {"time", "2008-06-10T18:57:24.000Z"},
    };
    nlohmann::json after_window_edge_transaction = {
        {"merchant", "c"},
        {"amount", 10},
        {"time", "2008-06-10T18:57:25.000Z"},
    };

    // Incoming transaction is the 4th inside the 2 minute window. Must return
    // true.
    REQUIRE(IsTransactionFrequent(big_transactions,
                                  before_window_edge_transaction) == true);
    // Incoming transaction is the 4th on the edge of the 2 minute window. Must
    // return false since it is 2 min inclusive.
    REQUIRE(IsTransactionFrequent(big_transactions, window_edge_transaction) ==
            false);
    // Incoming transaction is the 4th outside of the 2 minute window. Must
    // return false.
    REQUIRE(IsTransactionFrequent(big_transactions,
                                  after_window_edge_transaction) == false);
  }

  SECTION("Checking for frequent transactions based on basic examples with "
          "custom parameters") {
    nlohmann::json before_window_transaction = {
        {"merchant", "c"},
        {"amount", 10},
        {"time", "2008-06-10T18:55:25.000Z"},
    };
    nlohmann::json on_window_transaction = {
        {"merchant", "c"},
        {"amount", 10},
        {"time", "2008-06-10T18:55:34.000Z"},
    };
    nlohmann::json after_window_transaction = {
        {"merchant", "c"},
        {"amount", 10},
        {"time", "2008-06-10T18:55:35.000Z"},
    };

    // Create a frequency of 1 transaction every 10 seconds. (Rate-limit)
    const int kWindow = 10;
    const int kMaxFrequency = 1;

    // Incoming transaction is the 2nd inside the 10 second window where limit
    // is 1. Must return true.
    REQUIRE(IsTransactionFrequent(small_transactions, before_window_transaction,
                                  kWindow, kMaxFrequency) == true);
    // Incoming transaction is the 2nd on the 10 second window where limit is 1.
    // Must return false.
    REQUIRE(IsTransactionFrequent(small_transactions, on_window_transaction,
                                  kWindow, kMaxFrequency) == false);
    // Incoming transaction is the 2nd after the 10 second window where limit
    // is 1. Must return false.
    REQUIRE(IsTransactionFrequent(small_transactions, after_window_transaction,
                                  kWindow, kMaxFrequency) == false);
  }
}

TEST_CASE("IsTransactionSimilar") {
  // Create test vector with base transactions.
  std::vector<nlohmann::json> no_transactions{};

  std::vector<nlohmann::json> small_transactions{
      {
          {"merchant", "a"},
          {"amount", 20},
          {"time", "2008-06-10T18:55:26.000z"},
      },
  };

  std::vector<nlohmann::json> big_transactions{
      {
          {"merchant", "a"},
          {"amount", 10},
          {"time", "2008-06-10T18:55:24.000z"},
      },
      {
          {"merchant", "c"},
          {"amount", 20},
          {"time", "2008-06-10T18:55:26.000z"},
      },
      {
          {"merchant", "c"},
          {"amount", 20},
          {"time", "2008-06-10T18:55:277000z"},
      },
  };

  SECTION("Checking for repeated transactions when transaction vector size is "
          "less than kMaxRepetition") {
    nlohmann::json random_incoming_transaction = {
        {"merchant", "c"},
        {"amount", 10},
        {"time", "2008-06-10T18:55:30.000Z"},
    };

    std::unordered_map<std::string, std::vector<nlohmann::json>> map_txn;

    // Incoming transaction handles when kMaxRepetition (1) is greater than
    // transactions.size().
    REQUIRE(IsTransactionSimilar(map_txn, random_incoming_transaction) ==
            false);
  }

  SECTION("Checking for repeated transactions using business rules and common "
          "examples") {
    // Create map from the transactions within small_transactions.
    std::unordered_map<std::string, std::vector<nlohmann::json>> map_txn;
    std::stringstream ss;
    for (int i = 0; i < small_transactions.size(); ++i) {
      for (const auto &el : small_transactions[i].items()) {
        if (el.key() != "time") {
          ss << el.key() << ":" << el.value() << ":";
        }
      }
      map_txn[ss.str()].push_back(small_transactions[i]);
      ss.str(std::string());
    }

    nlohmann::json before_window_transaction = {
        {"merchant", "a"},
        {"amount", 20},
        {"time", "2008-06-10T18:57:25.000Z"},
    };

    nlohmann::json on_window_transaction = {
        {"merchant", "a"},
        {"amount", 20},
        {"time", "2008-06-10T18:57:26.000Z"},
    };

    nlohmann::json after_window_transaction = {
        {"merchant", "a"},
        {"amount", 20},
        {"time", "2008-06-10T18:57:27.000Z"},
    };

    // Incoming transaction is inside the 2 minute window and previous
    // transaction is similar. Must return true.
    REQUIRE(IsTransactionSimilar(map_txn, before_window_transaction) == true);
    // Incoming transaction is on the edge of the 2 minute window and previous
    // transaction is similar. Must return false (120s is valid; not similar).
    REQUIRE(IsTransactionSimilar(map_txn, on_window_transaction) == false);
    // Incoming transaction is after the 2 minute window and previous
    // transaction is similar. Must return false.
    REQUIRE(IsTransactionSimilar(map_txn, after_window_transaction) == false);
  }

  SECTION("Checking for repeated transactions using custom business rules and "
          "common examples") {
    // Create map from the transactions within big_transactions.
    std::unordered_map<std::string, std::vector<nlohmann::json>> map_txn;
    std::stringstream ss;
    for (int i = 0; i < big_transactions.size(); ++i) {
      for (const auto &el : big_transactions[i].items()) {
        if (el.key() != "time") {
          ss << el.key() << ":" << el.value() << ":";
        }
      }
      map_txn[ss.str()].push_back(big_transactions[i]);
      ss.str(std::string());
    }

    nlohmann::json before_window_transaction = {
        {"merchant", "c"},
        {"amount", 20},
        {"time", "2008-06-10T18:55:27.000Z"},
    };

    nlohmann::json on_window_transaction = {
        {"merchant", "c"},
        {"amount", 20},
        {"time", "2008-06-10T18:55:36.000Z"},
    };

    nlohmann::json after_window_transaction = {
        {"merchant", "c"},
        {"amount", 20},
        {"time", "2008-06-10T18:55:37.000Z"},
    };

    // Create a frequency of 2 similar transaction every 10 seconds.
    // (Rate-limit)
    const int kWindow = 10;
    const int kMaxRepetition = 2;

    // Incoming transaction is inside the 10 seconds window but there are only 2
    // transactions. Must return true.
    REQUIRE(IsTransactionSimilar(map_txn, before_window_transaction, kWindow,
                                 kMaxRepetition) == true);
    // Incoming transaction is on the edge of the 10 seconds window but there
    // are only 2 transactions. Must return false.
    REQUIRE(IsTransactionSimilar(map_txn, on_window_transaction, kWindow,
                                 kMaxRepetition) == false);
    // Incoming transaction is after the 10 seconds window but there are only 2
    // transactions. Must return false.
    REQUIRE(IsTransactionSimilar(map_txn, after_window_transaction, kWindow,
                                 kMaxRepetition) == false);
  }
}
