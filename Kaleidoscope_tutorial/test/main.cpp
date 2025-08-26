//
// Created by zuxinggu on 2025/8/19.
//

#include <iostream>
extern "C" {
    double foo(double, double);
    double squire(double);
    double fib(double);
    double printstar(double);
    double testunary(double);
    double testbinary(double, double);
    double putchard(double X) {
        fputc((char)X, stderr);
        return 0;
    }
    double printd(double X) {
        fprintf(stderr, "%f\n", X);
        return 0;
    }
    double mysquire(double);
    double specialsum(double x);
}

int main(int argc, char **argv) {
    double x = 1.0;
    double y = 2.0;
    std::cout << "squire(x)=" << squire(x) << std::endl;
    std::cout << "squire(y)=" << squire(y) << std::endl;
    std::cout << "foo(x,y)=" << foo(x, y) << std::endl;
    std::cout << "fib(2)=" << fib(2) << std::endl;
    std::cout << "fib(5)" << fib(5) << std::endl; // 5*5 + 4*4 + 3*3 + 2*2 = 54
    printstar(100);
    std::cout << std::endl;
    std::cout << "!2 = " << testunary(2);
    std::cout << std::endl;
    std::cout << "!0 = " << testunary(0);
    std::cout << std::endl;
    std::cout << "testbinary(0, 1) = " << testbinary(0, 1);
    std::cout << std::endl;
    std::cout << "testbinary(0, 0) = " << testbinary(0, 0);
    std::cout << std::endl;
    std::cout << mysquire(1) << std::endl;
    std::cout << mysquire(2) << std::endl;
    std::cout << mysquire(3) << std::endl;
    std::cout << mysquire(4) << std::endl;
    std::cout << mysquire(5) << std::endl;
    std::cout << mysquire(6) << std::endl;
    std::cout << specialsum(3.0) << std::endl;
    std::cout << specialsum(6.0) << std::endl;
    return 0;
}