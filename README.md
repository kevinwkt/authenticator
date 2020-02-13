# Authenticator

## Requirements

Requirements for this project are the following:

* make
* docker
* clang++ or any c++ compiler supporting c++11 with stdlib (g++, clang++, apple-clang++)

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

Make sure you have clang++ for Makefile.

```
make main
./main
```

```
make test
./test
```

### Docker

```
docker build . -t authenticator:1.0.0
docker run --rm -it authenticator:1.0.0
```
## Description

**Complexity O(1) for both ```IsTransactionFrequent``` and ```IsTransactionSimilar```.**

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

```IsTransactionFrequent``` is a simple fuction with complexity of O(1) however we need to take into account some caveats. **Transactions have to be sequential and if a past transaction is to be added into the system, it would have the big O complexity of O(n) given that we are utilizing a std::vector.** The code can be seen in the following:
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
  return (incoming_time_unix - lower_limit_unix) < window;
}
```

These simple lines of code focus on finding the kMaxFrequency past transaction and checking whether or not the current transaction violates this constraint. Explanation for this can be seen in the following: 

```
Assuming time goes from 0 - 10 (s) and kMaxFrequency=3 kFequencyWindow=3(s)...
Given std::vector<int> txn = { 2 2 3 4 }, if incoming transaction has a timestamp of 5:
We would be checking for index txn.size()(4)-kFequencyWindow(3) meaning we would be comparing incoming 5 with txn[1] which would be the second '2' in txn.

This gives that incoming_time_unix(5) - txn[1](2) is 3 which is equal or greater than 3. This would mean IsTransactionFrequent would return ```false``` allowing the incoming_transaction to be accepted. This is due to the fact that transactions are always sequential and if the comparing element violates the restriction, the elements on its left are all invalid and the elements on the right represents any number could be within the window.

```


Lastly, ```IsTransactionSimilar``` focuses on mostly hashing a txn (disregarding the timestamp which is variable) and generating a uuid to separate transaction by it's similarity using a new datastructure ```specificTransactions```. Because of the hashmap property of lookup and insertion, the complexity is O(1). Once transactions are separated by their similarity we utilize ```IsTransactionFrequent``` in order to see whether or not it violates the defined business rules. **Caveats for this function would be the limits of hashing large json objects which can be circumvented by using or generating a uuid, and the use of hashing might result in some colliions in large systems which would need larger memory and better hash functions to avoid collisions.**

It is also important to note that because we use UNIX time for seconds calculations milliseconds can not be taken into account for this implementation and that the maximum over/underflow to take into account would be (2^63)-1 given our use of long long for operations.