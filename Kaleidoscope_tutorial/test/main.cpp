//
// Created by zuxinggu on 2025/8/19.
//

#include <iostream>
extern "C" {
    double foo(double, double);
    double squire(double);
    double fib(double);
    double printstar(double);
    double putchard(double X) {
        fputc((char)X, stderr);
        return 0;
    }
}

int main(int argc, char **argv) {
    double x = 1.0;
    double y = 2.0;
    std::cout << squire(x) << std::endl;
    std::cout << squire(y) << std::endl;
    std::cout << foo(x, y) << std::endl;
    std::cout << squire(x) << std::endl;
    std::cout << fib(2) << std::endl;
    std::cout << fib(5) << std::endl; // 5*5 + 4*4 + 3*3 + 2*2 = 54
    printstar(100);
    return 0;
}