#include <iostream>
#include <vector>

class B {
public:
    int x = 1;
    int y = 2;
    int getX() {
        return x;
    }
    int getY() {
        return y;
    }
};
class A {
public:
    B* b = new B();
    B* get() {
        return b;
    }
};


void test1(long long times) {
    long long sum = 0;
    A a;
    auto *cache = a.get();
    for(long long i = 0; i < times; ++i) {
        
        // sum += a.get()->getX();
        // sum += a.get()->getY();
        sum += cache->getX();
        sum += cache->getY();
    }
    std::cout << sum << std::endl;
}

void test2(long long times) {
    std::vector<int> v(1000000, 1);
    long long sum = 0;
    while(times-- > 0)
        // for (auto it = v.begin(); it != v.end(); it++) {
        for (auto it = v.begin(); it != v.end(); ++it) {
            sum += *it;
        }
    std::cout << sum << std::endl;
}

long long call_by_value(std::vector<int> v) {
    long long sum = 0 + v[100];
    return sum;
}
long long call_by_ref(const std::vector<int>& v) {
    long long sum = 0 + v[100];
    return sum;
}
void test3(long long times) {
    std::vector<int> v(1000000, 1);
    long long sum = 0;
    while(times-- > 0)
    // long long sum = call_by_value(v);
        sum += call_by_ref(v);
        // sum = call_by_value(v);
    std::cout << sum << std::endl;
}

int main(int argc, char** argv) {
    long long times = std::atoll(argv[1]);
    test2(times);
    return 0;
}