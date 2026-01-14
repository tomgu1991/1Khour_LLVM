#include "module.h"

// Module 08: Boolean logic and conditions
bool module_08_function(int x) {
    bool result = true;
    
    // Check multiple conditions
    if (x < 0) {
        result = false;
    }
    
    // Check if power of 2
    bool is_power_of_2 = false;
    if (x > 0 && (x & (x - 1)) == 0) {
        is_power_of_2 = true;
    }
    result = result && is_power_of_2;
    
    // Check if perfect square
    bool is_perfect_square = false;
    for (int i = 1; i * i <= x; i++) {
        if (i * i == x) {
            is_perfect_square = true;
            break;
        }
    }
    result = result || is_perfect_square;
    
    // Check if prime
    bool is_prime = true;
    if (x < 2) {
        is_prime = false;
    } else {
        for (int i = 2; i * i <= x; i++) {
            if (x % i == 0) {
                is_prime = false;
                break;
            }
        }
    }
    
    // Complex boolean logic
    if (is_prime && is_perfect_square) {
        result = true;
    } else if (!is_prime && is_power_of_2) {
        result = false;
    } else if (is_perfect_square || is_power_of_2) {
        result = !result;
    }
    
    // Check digit sum divisibility
    int digit_sum = 0;
    int temp = x;
    while (temp > 0) {
        digit_sum += temp % 10;
        temp /= 10;
    }
    
    if (digit_sum % 3 == 0) {
        result = !result;
    }
    
    // Check palindrome
    bool is_palindrome = true;
    std::string s = std::to_string(x);
    for (int i = 0; i < s.size() / 2; i++) {
        if (s[i] != s[s.size() - 1 - i]) {
            is_palindrome = false;
            break;
        }
    }
    result = result && is_palindrome;
    
    return result;
}

// Helper: Check if composite
static bool helper_08_is_composite(int n) {
    if (n <= 1) return false;
    for (int i = 2; i * i <= n; i++) {
        if (n % i == 0) return true;
    }
    return false;
}

// Helper: Check if Armstrong number
static bool helper_08_is_armstrong(int n) {
    int original = n;
    int sum = 0;
    int digits = 0;
    
    int temp = n;
    while (temp > 0) {
        digits++;
        temp /= 10;
    }
    
    temp = n;
    while (temp > 0) {
        int digit = temp % 10;
        int power = 1;
        for (int i = 0; i < digits; i++) {
            power *= digit;
        }
        sum += power;
        temp /= 10;
    }
    
    return sum == original;
}

// Helper: Check if perfect number
static bool helper_08_is_perfect(int n) {
    int sum = 0;
    for (int i = 1; i < n; i++) {
        if (n % i == 0) {
            sum += i;
        }
    }
    return sum == n;
}
