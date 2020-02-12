# Authenticator

## Requirements

Requirements for this project are the following:

* cmake
* docker
* c++ compiler supporting c++11 with stdlib (g++, clang++, apple-clang++)

## Usage

### Basic Usage

For simplified usage without the use of either ```cmake``` or ```docker```, use a c++ compiler of choice (clang++, apple-clang++, g++, etc) to run the following:

```
// For main binary.
clang++ -std=c++11 -o main src/main.cc -I include -I lib
// Run main binary.
./main
```

```
// For unit test binary.
clang++ -std=c++11 -o test include/utils_test.cc -I include -I lib
// Run unit test binary.
./test
```

### Makefile


### Docker

## Description

In this section I will justify/explain the decisions taken in terms of data-structures/algorithms/design-patterns in this project. Given the requirements in the pdf there were 2 main entities: "account" and "transaction".

Both representations of "account" and "transaction" are saved as json objects given the defined schemas of the input jsons and the extendability of json objects. Despite the requirement pdf specified that only a single user could be active (for now...) the following structure was used to represent the necessary data-structures in a vector so more users can be integrated in the future.

```
struct User {
    // Constructor, ignore for now...
    User(nlohmann::json acc): account(acc) {};

    nlohmann::json account;
    std::vector<nlohmann::json> sequentialTransactions;
    std::unordered_map<std::string, std::vector<nlohmann::json> > specificTransactions; 
};
```

As mentioned before "account" and "transactionn" are represented as json objects within the User class. However, the datastructures "sequentialTransactions" and "specificTransactions" were utilized due to the business rules defined in the pdf. 

"sequentialTransactions" is just a vector of sequential (by time) transactions for a single user which is used mostly for the business rule ```high-frequency-small-interval```. This violation is managed by utilizing the function ```IsTransactionFrequent``` explained in the following.

```IsTransactionFrequent``` is a simple fuction which has the **caveat that transactions has to be sequential and that if a past transaction is to be added into the system, it would have the big O complexity of O(n) given that we are utilizing a std::vector.** The code can be seen in the following:
```
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
```

These simple lines of code focus on finding the kMaxFrequency past transaction and checking whether or not the current transaction violates this constraint. However, there is an edge case where this algorithm does not take into account which is the following:

```
Assuming time goes from 0 - 10 (s) and kMaxFrequency=3 kFequencyWindow=4(s)...
Given std::vector<int> txn = { 1 1 1 2 3 }, if incoming transaction has a timestamp of 4:
We would be checking for index txn.size()(5)-allowed(3) meaning we would be comparing incoming 4 with txn[2] which would be the third '1' in txn.

Given that incoming_time_unix(4) - lower_limit_unix(1) is 3 which is smaller than 4.
HOWEVER if we look closely, there are 2 other '1's on the left of txn meaning that there were actually more transactions within the same time window.

Because of this the second condition we had to make sure was to see if any element on the left side from the lower_limit_idx had the same timestamp. To do this we implemented IsTimestampPresent where if present, we return false.
```

```IsTimestampPresent``` is just a simple implementation of a binary search which utilizes reference variables instead to save space however uses the recursion stack which shouldn't be a problem given -O2 optimizations. This allows the big O notation of ```IsTransactionFrequent``` to be O(logn).

Lastly, ```IsTransactionSimilar``` focuses on mostly hashing a txn (disregarding the timestamp which is variable) and generating a uuid to separate transaction by it's similarity using a new datastructure ```specificTransactions```. Once transactions are separated by their similarity we utilize ```IsTransactionFrequent``` in order to see whether or not it violates the defined business rules.