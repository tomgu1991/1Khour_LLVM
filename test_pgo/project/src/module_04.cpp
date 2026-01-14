#include "module.h"
#include <cmath>

// Module 04: Floating point operations
double module_04_function(double x) {
    double result = x;
    
    // Trigonometric computations
    for (int i = 0; i < 50; i++) {
        result += std::sin(x + i) * std::cos(x - i);
    }
    
    // Logarithmic and exponential operations
    if (x > 0.0) {
        for (int i = 1; i <= 20; i++) {
            result += std::log(x + i) * std::exp(-i * 0.1);
        }
    }
    
    // Power operations
    for (int i = 2; i <= 10; i++) {
        result += std::pow(x, 1.0 / i);
    }
    
    // Newton-Raphson like iteration
    double y = x;
    for (int i = 0; i < 30; i++) {
        y = (y + x / y) / 2.0;
        result += y;
    }
    
    // Hyperbolic functions
    for (int i = 0; i < 25; i++) {
        result += std::sinh(x + i * 0.1) + std::cosh(x - i * 0.1);
    }
    
    // Arc functions
    if (x >= -1.0 && x <= 1.0) {
        result += std::asin(x) + std::acos(x) + std::atan(x);
    }
    
    // Gamma-like computation
    double factorial = 1.0;
    for (int i = 1; i <= 10; i++) {
        factorial *= i;
        result += 1.0 / factorial;
    }
    
    // Conditional floating point adjustments
    if (result > 1000.0) {
        result = std::log(result);
    } else if (result < -1000.0) {
        result = -std::log(-result);
    }
    
    return result;
}

// Helper: Trigonometric series
static double helper_04_sin_series(double x, int terms) {
    double result = 0.0;
    double power = x;
    double fact = 1.0;
    for (int i = 1; i <= terms; i++) {
        if (i > 1) {
            power *= -x * x;
            fact *= (2 * i - 2) * (2 * i - 1);
        }
        result += power / fact;
    }
    return result;
}

// Helper: Exponential series
static double helper_04_exp_series(double x, int terms) {
    double result = 1.0;
    double power = x;
    double fact = 1.0;
    for (int i = 1; i < terms; i++) {
        result += power / fact;
        power *= x;
        fact *= (i + 1);
    }
    return result;
}

// Helper: Combined calculation
static double helper_04_combined(double x) {
    return helper_04_sin_series(x, 15) + helper_04_exp_series(x, 15);
}
