#ifndef utils_h
#define utils_h

#include "globals.h"
#include "json.hpp"

#include <algorithm>
#include <ctime>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <stdio.h>
#include <string>
#include <vector>

// Converts error enum into corresponding violation message.
std::string ErrorToViolation(GenericErrors error) {
  switch (error) {
  case kOk:
    return kOkMessage;
  case kAccountAlreadyInitialized:
    return kAccountAlreadyInitializedMessage;
  case kAccountNotInitialized:
    return kAccountNotInitializedMessage;
  case kCardNotActive:
    return kCardNotActiveMessage;
  case kInsufficientLimit:
    return kInsufficientLimitMessage;
  case kHighFrequencySmallInterval:
    return kHighFrequencySmallIntervalMessage;
  case kDoubledTransaction:
    return kDoubledTransactionMessage;
  default:
    return kDefaultViolationMessage;
  }
};

// Parses ISO8601 from string format and returns a time_t.
// Assumes date is always in the same format like "2019-02-13T10:00:00.000Z".
// Caveat: based on *nix time. Must support this format.
std::time_t ParseISO8601(const std::string &dateStr) {
  int y, M, d, h, m;
  float s;
  sscanf(dateStr.c_str(), "%d-%d-%dT%d:%d:%fZ", &y, &M, &d, &h, &m, &s);
  tm time;
  time.tm_year = y - 1900;
  time.tm_mon = M - 1;
  time.tm_mday = d;
  time.tm_hour = h;
  time.tm_min = m;
  time.tm_sec = (int)s;
  return timegm(&time);
}

// Checks if an expected unix time is present in a vector from left_idx to
// right_idx. Basically a binary_search. O(logn).
bool IsTimestampPresent(const std::vector<nlohmann::json> &txn,
                        const int left_idx, const int right_idx,
                        const long long target) {
  if (right_idx >= left_idx && left_idx >= 0 && right_idx < txn.size()) {
    int mid_idx = left_idx + (right_idx - left_idx) / 2;
    const long long mid_unix_time = ParseISO8601(txn[mid_idx]["time"]);

    if (mid_unix_time == target)
      return true;

    if (mid_unix_time > target)
      return IsTimestampPresent(txn, left_idx, mid_idx - 1, target);

    return IsTimestampPresent(txn, mid_idx + 1, right_idx, target);
  }
  return false;
}

// Firstly verifies if window is filled by calculating lower_limit_idx. Then
// verifies that given the last kMaxFrequency, kFrequencyWindow has not passed
// (meaning it violates business rules).
// Edge case happens when there are multiple transactions with the same
// timelines. For this we verify that none of the transactions on the left side
// has the same time. If found, returns false.
bool IsTransactionFrequent(const std::vector<nlohmann::json> &txn,
                           const nlohmann::json incoming_transaction,
                           const int window = kFrequencyWindow,
                           const int allowed = kMaxFrequency) {
  const int lower_limit_idx = txn.size() - allowed;

  // If not enough transactions incoming transaction will always be acceptable.
  if (lower_limit_idx < 0)
    return false;

  const long long incoming_time_unix =
      ParseISO8601(incoming_transaction["time"]);
  const long long lower_limit_unix = ParseISO8601(txn[lower_limit_idx]["time"]);
  return ((incoming_time_unix - lower_limit_unix) < window) &&
         (!IsTimestampPresent(txn, 0, lower_limit_idx - 1, lower_limit_unix));
}

// Hashes incoming transaction to make a uuid and verifies using
// IsTransactionFrequent function and the datastructure specificTransactions
// whether or not incoming transaction violates business rules.
bool IsTransactionSimilar(
    const std::unordered_map<std::string, std::vector<nlohmann::json>>
        map_by_txn,
    const nlohmann::json incoming_transaction,
    const int window = kRepeatedWindow, const int allowed = kMaxRepetition) {
  // Check if hash of incoming_transaction is present in txn.
  std::stringstream ss;
  for (const auto &el : incoming_transaction.items()) {
    if (el.key() != "time") {
      ss << el.key() << ":" << el.value() << ":";
    }
  }
  if (map_by_txn.count(ss.str())) {
    return IsTransactionFrequent(map_by_txn.at(ss.str()), incoming_transaction,
                                 window, allowed);
  }
  return false;
}

#endif