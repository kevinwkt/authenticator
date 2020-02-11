#ifndef globals_h 
#define globals_h

#include <string>
#include <unordered_map>

#include "json.hpp"

// Basic errors specified in the documentation.
enum GenericErrors {
    kOk = 0,
    kAccountAlreadyInitialized,
    kAccountNotInitialized,
    kCardNotActive,
    kInsufficientLimit,
    kHighFrequencySmallInterval,
    kDoubledTransaction,
};

// Basic error messages corresponding to GenericErrors.
char const *kOkMessage = "Ok";
char const *kAccountAlreadyInitializedMessage = "account-already-initialized";
char const *kAccountNotInitializedMessage = "account-not-initialized";
char const *kCardNotActiveMessage = "card-not-active";
char const *kInsufficientLimitMessage = "insufficient-limit";
char const *kHighFrequencySmallIntervalMessage = "high-frequency-small-interval";
char const *kDoubledTransactionMessage = "doubled-transaction";
char const *kDefaultViolationMessage= "default-violation";

// Basic structure for user; Contains 2 main datastructures.
// 1. sequencialTransactions
//    A vector of json-transitions in a sequence as a vector.
// 2. specificTransactions
//    A hashmap where the (k, v) is (txnhash, vector of 'similar' transactions). 
struct User {
    // Class constructor to create accounts.
    User(nlohmann::json acc): account(acc) {};
    // JSON account obj with 2 fields in general, "active-card" and "available-limit".
    nlohmann::json account;
    // Vector of transaction jsons which are sequential.
    std::vector<nlohmann::json> sequencialTransactions;
    // Hashmap of transactions in sequencial order.
    // Hashes are calculated using std::hash and concatenation of the json in string form except for "time" field.
    std::unordered_map<std::string, std::vector<nlohmann::json> > specificTransactions; 
};

// Constants for doubled-transaction condition.
const unsigned long long kRepeatedWindow = 120;
const unsigned long long kMaxRepetition = 1;

// Constants for high-frequency-small-interval condition.
const unsigned long long kFrequencyWindow = 120;
const unsigned long long kMaxFrequency = 3;

#endif