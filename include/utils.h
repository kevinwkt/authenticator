#ifndef utils_h
#define utils_h

#include "globals.h"
#include "json.hpp"

#include <ctime>
#include <iostream>
#include <stdio.h>
#include <stdexcept>
#include <string>
#include <sstream>
#include <vector>

// Function that receives a Generic Error and converts it into violation message.
std::string ErrorToViolation(GenericErrors error) {
    std::string violation;
    switch(error) {
        case kOk:
            violation = kOkMessage;
            break;
        case kAccountAlreadyInitialized:
            violation = kAccountAlreadyInitializedMessage;
            break;
        case kAccountNotInitialized:
            violation = kAccountNotInitializedMessage;
            break;
        case kCardNotActive:
            violation = kCardNotActiveMessage;
            break;
        case kInsufficientLimit:
            violation = kInsufficientLimitMessage;
            break;
        case kHighFrequencySmallInterval:
            violation = kHighFrequencySmallIntervalMessage;
            break;
        case kDoubledTransaction:
            violation = kDoubledTransactionMessage;
            break;
        default:
            violation = kDefaultViolationMessage;
            break;
    }
    return violation;
};

// Parses ISO8601 from string format and returns a time_t.
// Assumes date is always in the same format like "2019-02-13T10:00:00.000Z".
std::time_t ParseISO8601(const std::string& dateStr) {
    int y,M,d,h,m;
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
// std::time_t ParseISO8601(const std::string& dateStr) {
//     std::tm time= {};
//     std::istringstream ss(dateStr);
//     ss >> std::get_time(&time, "%Y-%m-%dT%H:%M:%S");
//     if (ss.fail()) {
//         throw std::runtime_error{"failed to parse time string"};
//     }
    
//     std::cout << std::put_time(&time, "%c") << '\n';
//     return timegm(&time);
// }

// Function that receives a sorted vector of the same transaction type and verifies if within a specified 'time' and
// allowed number of transactions, a transaction can be processed.
bool IsTransactionFrequent(const std::vector<nlohmann::json> &txn, nlohmann::json incoming_transaction, const int window = kFrequencyWindow, const int allowed = kMaxFrequency) {
    // if((int)(txn.size()-allowed) < 0) {
    //     std::cout << "Returning false\n";
    // } else {
    //     std::cout << "comparing " << (int)txn.size() << " with " << allowed << " wierd " << (int)(txn.size()-allowed) << "\n";
    //     std::cout << "else\n";
    //     std::cout << (long long)ParseISO8601(txn[txn.size()-allowed]["time"]) << "\n";
    //     std::cout << (long long)ParseISO8601(incoming_transaction["time"]) << "\n";
    //     if(
    //         (long long)ParseISO8601(incoming_transaction["time"]) - (long long)ParseISO8601(txn[txn.size()-allowed]["time"]) > window)
    //         std::cout << "Returning truee\n";
    //     else
    //         std::cout << "Returning falsee\n";
    // }
    return ((int)(txn.size()-allowed) < 0)? false :
        (long long)ParseISO8601(incoming_transaction["time"]) - ((long long)ParseISO8601(txn[txn.size()-allowed]["time"]) > window);
}

// 
bool IsTransactionSimilar(const std::unordered_map<std::string, std::vector<nlohmann::json> > map_by_txn, nlohmann::json incoming_transaction) {
    // Check if hash of incomin_transaction is present in txn.
    std::stringstream ss;
    for (auto& el : incoming_transaction.items()) {
        if(el.key() != "time") {
            ss << el.key() << ":" << el.value() << ":";
        }
    }
    if(map_by_txn.count(ss.str())) {
        return IsTransactionFrequent(map_by_txn.at(ss.str()), incoming_transaction, kRepeatedWindow, kMaxRepetition);
    } 
    return false; 
}

#endif