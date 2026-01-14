#include "module.h"

// Module 12: Vector floating point operations
std::vector<double> module_12_function(int size) {
    std::vector<double> result(size);
    
    // Fill with computed values
    for (int i = 0; i < size; i++) {
        double val = i * 0.1;
        result[i] = val * val * val;
    }
    
    // Apply scaling
    double scale_factor = 1.5;
    for (auto& val : result) {
        val *= scale_factor;
        scale_factor *= 0.99;
    }
    
    // Normalize to range [0, 1]
    double max_val = result[0];
    for (double val : result) {
        if (val > max_val) {
            max_val = val;
        }
    }
    
    if (max_val > 0.0) {
        for (auto& val : result) {
            val /= max_val;
        }
    }
    
    // Apply sine transformation
    for (int i = 0; i < result.size(); i++) {
        result[i] = std::sin(result[i] * 3.14159 / 2.0);
    }
    
    // Cumulative sum
    for (int i = 1; i < result.size(); i++) {
        result[i] += result[i-1];
    }
    
    // Apply exponential decay
    for (int i = 0; i < result.size(); i++) {
        result[i] *= std::exp(-i * 0.05);
    }
    
    // Final smoothing
    if (size > 1) {
        std::vector<double> smoothed(size);
        for (int i = 0; i < size; i++) {
            smoothed[i] = result[i];
            if (i > 0) smoothed[i] += result[i-1];
            if (i < size - 1) smoothed[i] += result[i+1];
            smoothed[i] /= (i == 0 || i == size-1 ? 2.0 : 3.0);
        }
        result = smoothed;
    }
    
    return result;
}

// Helper: Vector dot product
static double helper_12_dot_product(const std::vector<double>& a, 
                                     const std::vector<double>& b) {
    double sum = 0.0;
    int min_size = a.size() < b.size() ? a.size() : b.size();
    for (int i = 0; i < min_size; i++) {
        sum += a[i] * b[i];
    }
    return sum;
}

// Helper: Vector magnitude
static double helper_12_magnitude(const std::vector<double>& v) {
    double sum = 0.0;
    for (double val : v) {
        sum += val * val;
    }
    return std::sqrt(sum);
}

// Helper: Vector normalization
static std::vector<double> helper_12_normalize(std::vector<double> v) {
    double mag = helper_12_magnitude(v);
    if (mag > 0.0) {
        for (auto& val : v) {
            val /= mag;
        }
    }
    return v;
}

// Helper: Interpolation
static double helper_12_interpolate(double v1, double v2, double t) {
    return v1 * (1.0 - t) + v2 * t;
}
