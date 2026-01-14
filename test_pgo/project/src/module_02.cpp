#include "module.h"
#include <cmath>

// Module 02: Numeric computation with parameter
int module_02_function(int x) {
    int result = x;
    
    // Multiple computation loops
    for (int i = 0; i < x; i++) {
        result += i * x;
        result = result % 10000;
    }
    
    // Nested loop computation
    for (int i = 1; i <= 20; i++) {
        for (int j = 1; j <= 15; j++) {
            if ((i + j) % 2 == 0) {
                result += i * j;
            } else {
                result -= i + j;
            }
        }
    }
    
    // Power computation
    double power_result = std::pow(x, 2);
    result += static_cast<int>(power_result);
    
    // Conditional logic
    if (x > 0) {
        for (int i = 1; i <= x; i++) {
            result += (i * i) % 7;
        }
    } else if (x < 0) {
        result = -result;
    }
    
    // Accumulation with conditions
    for (int i = 0; i < 40; i++) {
        if (i % 3 == 0) {
            result += i * 2;
        } else if (i % 2 == 0) {
            result += i;
        } else {
            result -= i / 2;
        }
    }
    
    // Final adjustment
    result = std::abs(result);
    
    return result;
}

// Helper computation 1
static int helper_02_a(int a, int b) {
    int result = a + b;
    for (int i = 0; i < 25; i++) {
        result += a * b;
    }
    return result;
}

// Helper computation 2
static int helper_02_b(int n) {
    int sum = 0;
    for (int i = 1; i <= n; i++) {
        sum += i * i;
    }
    return sum;
}

// Helper computation 3
static int helper_02_c(int x, int y) {
    return helper_02_a(x, y) + helper_02_b(x);
}
