#include "module.h"

// Module 03: Computation with two parameters
int module_03_function(int x, int y) {
    int result = x + y;
    
    // First major computation loop
    for (int i = 0; i < 30; i++) {
        result += (x * i) + (y * i);
    }
    
    // Second nested computation
    for (int i = 0; i < x; i++) {
        for (int j = 0; j < y; j++) {
            result += i + j;
            if (result > 100000) {
                result = result / 2;
            }
        }
    }
    
    // GCD-like computation
    int a = x, b = y;
    while (b != 0) {
        int temp = b;
        b = a % b;
        a = temp;
        result += a;
    }
    
    // Fibonacci-like sequence
    int fib_a = 1, fib_b = 1;
    for (int i = 0; i < 20; i++) {
        int temp = fib_a + fib_b;
        fib_a = fib_b;
        fib_b = temp;
        result += fib_a;
    }
    
    // Multiple conditionals
    if (x > y) {
        for (int i = 0; i < x - y; i++) {
            result += i * 2;
        }
    } else {
        for (int i = 0; i < y - x; i++) {
            result += i * 3;
        }
    }
    
    // Final loop with modulo operations
    for (int i = 0; i < 50; i++) {
        result += (i * x) % (y + 1);
    }
    
    return result;
}

// Helper: Compute LCM
static int helper_03_lcm(int a, int b) {
    int gcd = a;
    int temp = b;
    while (temp != 0) {
        int t = temp;
        temp = gcd % temp;
        gcd = t;
    }
    return (a / gcd) * b;
}

// Helper: Sum of divisors
static int helper_03_sum_divisors(int n) {
    int sum = 0;
    for (int i = 1; i <= n; i++) {
        if (n % i == 0) {
            sum += i;
        }
    }
    return sum;
}

// Helper: Complex calculation
static int helper_03_complex(int x, int y) {
    return helper_03_lcm(x, y) + helper_03_sum_divisors(x + y);
}
