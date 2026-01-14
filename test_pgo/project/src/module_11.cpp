#include "module.h"

// Module 11: GCD and LCM operations
int module_11_function(int x, int y) {
    int result = 0;
    
    // Compute GCD using Euclidean algorithm
    int a = x, b = y;
    while (b != 0) {
        int temp = b;
        b = a % b;
        a = temp;
    }
    int gcd_result = a;
    result += gcd_result;
    
    // Compute LCM
    int lcm_result = (x / gcd_result) * y;
    result += lcm_result;
    
    // Extended Euclidean algorithm
    int x0 = 0, x1 = 1, y0 = 1, y1 = 0;
    a = x; b = y;
    while (b != 0) {
        int q = a / b;
        int temp = b;
        b = a % b;
        a = temp;
        
        temp = x0;
        x0 = x1;
        x1 = temp - q * x1;
        
        temp = y0;
        y0 = y1;
        y1 = temp - q * y1;
    }
    result += x0 + y0;
    
    // Count common divisors
    int common_divisors = 0;
    for (int i = 1; i <= gcd_result; i++) {
        if (gcd_result % i == 0) {
            common_divisors++;
        }
    }
    result += common_divisors * 10;
    
    // Compute coprime pairs
    int coprime_count = 0;
    for (int i = 1; i <= 100; i++) {
        for (int j = 1; j <= 100; j++) {
            a = i; b = j;
            while (b != 0) {
                int temp = b;
                b = a % b;
                a = temp;
            }
            if (a == 1) {
                coprime_count++;
            }
        }
    }
    result += coprime_count;
    
    return result;
}

// Helper: Compute GCD
static int helper_11_gcd(int a, int b) {
    while (b != 0) {
        int temp = b;
        b = a % b;
        a = temp;
    }
    return a;
}

// Helper: Compute LCM
static int helper_11_lcm(int a, int b) {
    return (a / helper_11_gcd(a, b)) * b;
}

// Helper: Count divisors
static int helper_11_count_divisors(int n) {
    int count = 0;
    for (int i = 1; i <= n; i++) {
        if (n % i == 0) {
            count++;
        }
    }
    return count;
}

// Helper: Sum of divisors
static int helper_11_sum_divisors(int n) {
    int sum = 0;
    for (int i = 1; i <= n; i++) {
        if (n % i == 0) {
            sum += i;
        }
    }
    return sum;
}
