#include "module.h"
#include <cmath>

// Module 09: Multi-parameter floating point
double module_09_function(double x, double y) {
    double result = x + y;
    
    // Weighted computation
    for (int i = 0; i < 30; i++) {
        result += (x * i + y * i) * 0.1;
    }
    
    // Trigonometric operations
    for (int i = 0; i < 20; i++) {
        result += std::sin(x + i * 0.1) * std::cos(y + i * 0.1);
    }
    
    // Ratio and scaling
    if (y != 0.0) {
        double ratio = x / y;
        for (int i = 1; i <= 15; i++) {
            result += ratio / i;
        }
    }
    
    // Square root iterations
    double sqrt_result = std::sqrt(std::abs(x * y));
    for (int i = 0; i < 25; i++) {
        result += sqrt_result / (i + 1.0);
    }
    
    // Exponential scaling
    for (int i = 1; i <= 10; i++) {
        result += std::exp(-i * 0.1) * (x + y);
    }
    
    // Distance formula iterations
    for (double dx = -2.0; dx <= 2.0; dx += 0.5) {
        for (double dy = -2.0; dy <= 2.0; dy += 0.5) {
            result += std::sqrt(dx * dx + dy * dy);
        }
    }
    
    // Conditional scaling
    if (x > y) {
        result *= 1.1;
    } else {
        result *= 0.9;
    }
    
    // Final normalization
    if (result > 10000.0) {
        result = std::log10(result);
    }
    
    return result;
}

// Helper: Distance between points
static double helper_09_distance(double x1, double y1, double x2, double y2) {
    double dx = x2 - x1;
    double dy = y2 - y1;
    return std::sqrt(dx * dx + dy * dy);
}

// Helper: Dot product calculation
static double helper_09_dot_product(double x1, double y1, double x2, double y2) {
    return x1 * x2 + y1 * y2;
}

// Helper: Angle calculation
static double helper_09_angle(double x, double y) {
    return std::atan2(y, x);
}

// Helper: Magnitude
static double helper_09_magnitude(double x, double y) {
    return std::sqrt(x * x + y * y);
}
