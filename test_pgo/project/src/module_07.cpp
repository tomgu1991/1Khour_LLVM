#include "module.h"
#include <algorithm>
#include <numeric>

// Module 07: Vector aggregation
int module_07_function(std::vector<int>& v) {
    int result = 0;
    
    // Basic sum
    result = std::accumulate(v.begin(), v.end(), 0);
    
    // Find max and min
    if (!v.empty()) {
        int max_val = *std::max_element(v.begin(), v.end());
        int min_val = *std::min_element(v.begin(), v.end());
        result += max_val - min_val;
    }
    
    // Count even and odd
    int even_count = 0, odd_count = 0;
    for (int val : v) {
        if (val % 2 == 0) {
            even_count++;
        } else {
            odd_count++;
        }
    }
    result += even_count * 2 + odd_count * 3;
    
    // Compute sum of squares
    int sq_sum = 0;
    for (int val : v) {
        sq_sum += val * val;
    }
    result += sq_sum % 100000;
    
    // Count elements greater than average
    if (!v.empty()) {
        int avg = result / v.size();
        int count = 0;
        for (int val : v) {
            if (val > avg) {
                count++;
            }
        }
        result += count * 5;
    }
    
    // Sort vector and compute differences
    std::vector<int> sorted_v = v;
    std::sort(sorted_v.begin(), sorted_v.end());
    
    for (int i = 1; i < sorted_v.size(); i++) {
        result += sorted_v[i] - sorted_v[i-1];
    }
    
    // Transform and recompute
    for (int& val : v) {
        val = std::abs(val);
    }
    
    result = (result * 7) % 1000000;
    
    return result;
}

// Helper: Vector average
static double helper_07_average(const std::vector<int>& v) {
    if (v.empty()) return 0.0;
    int sum = std::accumulate(v.begin(), v.end(), 0);
    return static_cast<double>(sum) / v.size();
}

// Helper: Vector variance
static double helper_07_variance(const std::vector<int>& v) {
    if (v.empty()) return 0.0;
    double avg = helper_07_average(v);
    double var = 0.0;
    for (int val : v) {
        var += (val - avg) * (val - avg);
    }
    return var / v.size();
}

// Helper: Find median
static int helper_07_median(std::vector<int> v) {
    std::sort(v.begin(), v.end());
    if (v.size() % 2 == 0) {
        return (v[v.size()/2 - 1] + v[v.size()/2]) / 2;
    }
    return v[v.size() / 2];
}

// Helper: Count occurrences
static int helper_07_count_value(const std::vector<int>& v, int target) {
    return std::count(v.begin(), v.end(), target);
}
