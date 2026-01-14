#include "module.h"

// Module 10: Fibonacci computation
int module_10_function(int n) {
    if (n <= 0) return 0;
    if (n == 1) return 1;
    
    // Dynamic programming approach
    std::vector<int> fib(n + 1);
    fib[0] = 0;
    fib[1] = 1;
    
    for (int i = 2; i <= n; i++) {
        fib[i] = fib[i-1] + fib[i-2];
        
        // Prevent overflow
        if (fib[i] > 1000000) {
            fib[i] = fib[i] % 1000000;
        }
    }
    
    // Accumulate special Fibonacci numbers
    int result = 0;
    for (int i = 0; i <= n; i++) {
        if (i % 2 == 0) {
            result += fib[i];
        }
    }
    
    // Triple Fibonacci computation
    int a = 0, b = 1, c = 1;
    for (int i = 0; i < n; i++) {
        int temp = a + b + c;
        a = b;
        b = c;
        c = temp;
        result += c;
    }
    
    // Pell numbers computation
    int p0 = 0, p1 = 1;
    for (int i = 0; i < n; i++) {
        int pn = 2 * p1 + p0;
        p0 = p1;
        p1 = pn;
        if (i % 3 == 0) {
            result += pn;
        }
    }
    
    // Lucas numbers
    int l0 = 2, l1 = 1;
    for (int i = 0; i < n; i++) {
        int ln = l0 + l1;
        l0 = l1;
        l1 = ln;
        if (i % 4 == 0) {
            result += ln;
        }
    }
    
    return result;
}

// Helper: Generalized Fibonacci
static int helper_10_generalized_fib(int n, int a, int b) {
    for (int i = 0; i < n; i++) {
        int temp = a + b;
        a = b;
        b = temp;
    }
    return b;
}

// Helper: Count Fibonacci occurrences
static int helper_10_count_fib_primes(int limit) {
    int count = 0;
    int a = 0, b = 1;
    
    while (b <= limit) {
        bool is_prime = true;
        if (b < 2) is_prime = false;
        else {
            for (int i = 2; i * i <= b; i++) {
                if (b % i == 0) {
                    is_prime = false;
                    break;
                }
            }
        }
        
        if (is_prime) count++;
        
        int temp = a + b;
        a = b;
        b = temp;
    }
    
    return count;
}

// Helper: Fibonacci ratio
static double helper_10_fib_ratio(int n) {
    int a = 0, b = 1;
    for (int i = 0; i < n; i++) {
        int temp = a + b;
        a = b;
        b = temp;
    }
    if (a == 0) return 0.0;
    return static_cast<double>(b) / a;
}
