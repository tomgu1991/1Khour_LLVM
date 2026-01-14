#include "module.h"

// Module 06: String operations
std::string module_06_function(std::string s) {
    std::string result = s;
    
    // String transformation loop
    for (int i = 0; i < result.size(); i++) {
        if (result[i] >= 'a' && result[i] <= 'z') {
            result[i] = result[i] - 32;
        }
    }
    
    // String reversal
    std::reverse(result.begin(), result.end());
    
    // Append repeated character
    for (int i = 0; i < 50; i++) {
        result += "x";
    }
    
    // Character replacement
    for (int i = 0; i < result.size(); i++) {
        if (result[i] == 'X') {
            result[i] = 'Y';
        }
    }
    
    // String doubling
    if (result.size() < 200) {
        result += result;
    }
    
    // Character counting and modification
    int count = 0;
    for (char c : result) {
        if (c == 'Y') {
            count++;
        }
    }
    
    // Append count information
    result += std::to_string(count);
    
    // Character removal based on condition
    std::string filtered;
    for (char c : result) {
        if ((c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9')) {
            filtered += c;
        }
    }
    result = filtered;
    
    // Final string concatenation
    for (int i = 0; i < 5; i++) {
        result += "_";
        result += std::to_string(i);
    }
    
    return result;
}

// Helper: String uppercase conversion
static std::string helper_06_to_upper(std::string s) {
    for (auto& c : s) {
        if (c >= 'a' && c <= 'z') {
            c = c - 32;
        }
    }
    return s;
}

// Helper: String reverse
static std::string helper_06_reverse(std::string s) {
    std::reverse(s.begin(), s.end());
    return s;
}

// Helper: String repeat
static std::string helper_06_repeat(std::string s, int n) {
    std::string result;
    for (int i = 0; i < n; i++) {
        result += s;
    }
    return result;
}

// Helper: Count character occurrences
static int helper_06_count_char(const std::string& s, char c) {
    int count = 0;
    for (char ch : s) {
        if (ch == c) count++;
    }
    return count;
}
