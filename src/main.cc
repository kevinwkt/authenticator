#include <iostream>
#include <memory>
#include <string>
#include <unordered_map>

#include "json.hpp"

#include "globals.h"
#include "utils.h"

std::vector<User> users;
std::stringstream ss;

void HandleAccountCreation(nlohmann::json user_json) {
  nlohmann::json output;
  output["violations"] = std::vector<std::string>();

  // Verify if current account is active.
  if (users.empty()) {
    users.push_back(User(user_json));
  } else {
    output["violations"].push_back(
        ErrorToViolation(kAccountAlreadyInitialized));
  }

  output["account"] = user_json;

  std::cout << output.dump() << "\n";
}

void HandleTransactionCreation(nlohmann::json transaction_json) {
  // std::cout << "Handling transaction creation\n";
  nlohmann::json output;
  output["violations"] = std::vector<std::string>();

  // If no registered user.
  if (users.size() == 0) {
    output["violations"].push_back(ErrorToViolation(kAccountNotInitialized));
  } else {
    // Set the current user to the only registered user for now and set output.
    User &current_user = users[0];

    // If user card is not active.
    if (!current_user.account["active-card"]) {
      output["violations"].push_back(ErrorToViolation(kCardNotActive));
    }

    // If not enough funds in account.
    if (current_user.account["available-limit"] < transaction_json["amount"]) {
      output["violations"].push_back(ErrorToViolation(kInsufficientLimit));
    }

    // If too frequent.
    if (IsTransactionFrequent(current_user.sequentialTransactions,
                              transaction_json)) {
      output["violations"].push_back(
          ErrorToViolation(kHighFrequencySmallInterval));
    }

    // If duplicate.
    if (IsTransactionSimilar(current_user.specificTransactions,
                             transaction_json)) {
      output["violations"].push_back(ErrorToViolation(kDoubledTransaction));
    }

    // Accept transaction.
    if (output["violations"].empty()) {
      // Add transactions to user profile.
      current_user.sequentialTransactions.push_back(transaction_json);

      // Get hash of transaction and add to transaction-specifit data structure.
      for (auto &el : transaction_json.items()) {
        if (el.key() != "time") {
          ss << el.key() << ":" << el.value() << ":";
        }
      }
      current_user.specificTransactions[ss.str()].push_back(transaction_json);

      // Reset the stream and clear the error state.
      std::stringstream().swap(ss);

      // Update account balance to user.
      current_user.account["available-limit"] =
          (unsigned long long)current_user.account["available-limit"] -
          (unsigned long long)transaction_json["amount"];
    }

    // Set account details as output json.
    output["account"] = current_user.account;
  }
  std::cout << output.dump() << "\n";
}

int main() {
  // Placeholder string and json obj.
  std::string s;
  nlohmann::json incoming_json;

  while (std::getline(std::cin, s)) {
    incoming_json = nlohmann::json::parse(s);
    if (incoming_json.contains("account")) {
      HandleAccountCreation(incoming_json["account"]);
    } else {
      HandleTransactionCreation(incoming_json["transaction"]);
    }
  }
}