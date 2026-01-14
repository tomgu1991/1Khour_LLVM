#include "module.h"
#include <algorithm>
#include <numeric>

// Module 05: Vector operations
std::vector<int> module_05_function(int n) {
    std::vector<int> result;
    
    // Fill vector with computed values
    for (int i = 0; i < n; i++) {
        int val = i * i;
        for (int j = 0; j < 10; j++) {
            val += j * i;
        }
        result.push_back(val);
    }
    
    // Sort and rearrange
    std::sort(result.begin(), result.end());
    
    // Reverse first half
    if (result.size() > 0) {
        std::reverse(result.begin(), result.begin() + result.size() / 2);
    }
    
    // Apply transformations
    for (auto& val : result) {
        val = val * 2 + 1;
    }
    
    // Accumulate and modify
    int sum = std::accumulate(result.begin(), result.end(), 0);
    for (auto& val : result) {
        val = (val * sum) % 10000;
    }
    
    // Additional filtering loop
    std::vector<int> temp;
    for (int val : result) {
        if (val % 2 == 0) {
            temp.push_back(val);
        }
    }
    result = temp;
    
    // Final adjustment loop
    for (int i = 0; i < result.size(); i++) {
        result[i] += i * 3;
    }
    
    return result;
}

// Helper: Generate Fibonacci sequence
static std::vector<int> helper_05_fibonacci(int n) {
    std::vector<int> fib;
    int a = 0, b = 1;
    for (int i = 0; i < n; i++) {
        fib.push_back(a);
        int temp = a + b;
        a = b;
        b = temp;
    }
    return fib;
}

// Helper: Prime number generation
static std::vector<int> helper_05_primes(int limit) {
    std::vector<int> primes;
    for (int i = 2; i <= limit; i++) {
        bool is_prime = true;
        for (int j = 2; j * j <= i; j++) {
            if (i % j == 0) {
                is_prime = false;
                break;
            }
        }
        if (is_prime) {
            primes.push_back(i);
        }
    }
    return primes;
}

// Helper: Vector statistics
static int helper_05_vector_sum(const std::vector<int>& v) {
    return std::accumulate(v.begin(), v.end(), 0);
}
