//
// Created by zuxinggu on 2025/8/19.
//

#include <iostream>
extern "C" {
    double foo(double, double);
    double squire(double);
}

int main(int argc, char **argv) {
    double x = 1.0;
    double y = 2.0;
    std::cout << squire(x) << std::endl;
    std::cout << squire(y) << std::endl;
    std::cout << foo(x, y) << std::endl;
    return 0;
}