#include "module.h"
#include <iostream>
#include <iomanip>

int main() {
    std::cout << "=== Large Project with 12 Modules ===" << std::endl;
    std::cout << std::endl;
    
    // Module 01: Basic arithmetic operations
    std::cout << "Module 01: Basic arithmetic operations" << std::endl;
    int result_01 = module_01_function();
    std::cout << "  Result: " << result_01 << std::endl;
    
    // Module 02: Numeric computation with parameter
    std::cout << "\nModule 02: Numeric computation with parameter" << std::endl;
    int param_02 = 42;
    int result_02 = module_02_function(param_02);
    std::cout << "  Input: " << param_02 << std::endl;
    std::cout << "  Result: " << result_02 << std::endl;
    
    // Module 03: Computation with two parameters
    std::cout << "\nModule 03: Computation with two parameters" << std::endl;
    int param_03_x = 12;
    int param_03_y = 18;
    int result_03 = module_03_function(param_03_x, param_03_y);
    std::cout << "  Input: (" << param_03_x << ", " << param_03_y << ")" << std::endl;
    std::cout << "  Result: " << result_03 << std::endl;
    
    // Module 04: Floating point operations
    std::cout << "\nModule 04: Floating point operations" << std::endl;
    double param_04 = 3.14159;
    double result_04 = module_04_function(param_04);
    std::cout << "  Input: " << param_04 << std::endl;
    std::cout << "  Result: " << std::fixed << std::setprecision(6) << result_04 << std::endl;
    
    // Module 05: Vector operations
    std::cout << "\nModule 05: Vector operations" << std::endl;
    int vector_size_05 = 15;
    std::vector<int> result_05 = module_05_function(vector_size_05);
    std::cout << "  Vector size: " << vector_size_05 << std::endl;
    std::cout << "  Result vector size: " << result_05.size() << std::endl;
    if (!result_05.empty()) {
        std::cout << "  First 5 elements: ";
        for (int i = 0; i < std::min(5, (int)result_05.size()); i++) {
            std::cout << result_05[i] << " ";
        }
        std::cout << std::endl;
    }
    
    // Module 06: String operations
    std::cout << "\nModule 06: String operations" << std::endl;
    std::string param_06 = "hello";
    std::string result_06 = module_06_function(param_06);
    std::cout << "  Input: " << param_06 << std::endl;
    std::cout << "  Result length: " << result_06.size() << std::endl;
    std::cout << "  Result: " << result_06 << std::endl;
    
    // Module 07: Vector aggregation
    std::cout << "\nModule 07: Vector aggregation" << std::endl;
    std::vector<int> vector_07 = {10, 20, 30, 40, 50, 15, 25, 35, 45};
    int result_07 = module_07_function(vector_07);
    std::cout << "  Vector size: " << vector_07.size() << std::endl;
    std::cout << "  Result: " << result_07 << std::endl;
    
    // Module 08: Boolean logic and conditions
    std::cout << "\nModule 08: Boolean logic and conditions" << std::endl;
    int param_08 = 121;
    bool result_08 = module_08_function(param_08);
    std::cout << "  Input: " << param_08 << std::endl;
    std::cout << "  Result: " << (result_08 ? "true" : "false") << std::endl;
    
    // Module 09: Multi-parameter floating point
    std::cout << "\nModule 09: Multi-parameter floating point" << std::endl;
    double param_09_x = 2.5;
    double param_09_y = 3.7;
    double result_09 = module_09_function(param_09_x, param_09_y);
    std::cout << "  Input: (" << param_09_x << ", " << param_09_y << ")" << std::endl;
    std::cout << "  Result: " << std::fixed << std::setprecision(6) << result_09 << std::endl;
    
    // Module 10: Fibonacci computation
    std::cout << "\nModule 10: Fibonacci computation" << std::endl;
    int param_10 = 20;
    int result_10 = module_10_function(param_10);
    std::cout << "  Input: " << param_10 << std::endl;
    std::cout << "  Result: " << result_10 << std::endl;
    
    // Module 11: GCD and LCM operations
    std::cout << "\nModule 11: GCD and LCM operations" << std::endl;
    int param_11_x = 48;
    int param_11_y = 18;
    int result_11 = module_11_function(param_11_x, param_11_y);
    std::cout << "  Input: (" << param_11_x << ", " << param_11_y << ")" << std::endl;
    std::cout << "  Result: " << result_11 << std::endl;
    
    // Module 12: Vector floating point operations
    std::cout << "\nModule 12: Vector floating point operations" << std::endl;
    int vector_size_12 = 20;
    std::vector<double> result_12 = module_12_function(vector_size_12);
    std::cout << "  Vector size: " << vector_size_12 << std::endl;
    std::cout << "  Result vector size: " << result_12.size() << std::endl;
    if (!result_12.empty()) {
        std::cout << "  First 5 elements: ";
        for (int i = 0; i < std::min(5, (int)result_12.size()); i++) {
            std::cout << std::fixed << std::setprecision(6) << result_12[i] << " ";
        }
        std::cout << std::endl;
    }
    
    // Summary
    std::cout << "\n=== Summary ===" << std::endl;
    std::cout << "Total modules executed: 12" << std::endl;
    int total_int_result = result_01 + result_02 + result_03 + result_07 + result_10 + result_11;
    std::cout << "Combined integer results: " << total_int_result << std::endl;
    double total_double_result = result_04 + result_09;
    std::cout << "Combined floating point results: " << std::fixed << std::setprecision(6) << total_double_result << std::endl;
    std::cout << "\n=== Program completed successfully ===" << std::endl;
    
    return 0;
}
