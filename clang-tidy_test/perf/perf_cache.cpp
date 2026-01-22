#include <iostream>
#include <vector>
#include <cstdint>
#include <chrono>

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
    virtual B* get() {
        return b;
    }
};

// 拷贝参数
long long NoCache() {
    long long sum = 0;
    A *a = new A();
    // auto *cache = a.get();
    for(long long i = 0; i < 100000000; ++i) {        
        sum += a->get()->getX() + i;
        sum += a->get()->getY() + i * 2;
        // sum += cache->getX();
        // sum += cache->getY();
    }
    return sum;
}

// const引用参数
long long Cache() {
    long long sum = 0;
    A *a = new A();
    auto *cache = a->get();
    for(long long i = 0; i < 100000000; ++i) {        
        // sum += a.get()->getX();
        // sum += a.get()->getY();
        sum += cache->getX() + i - 1;
        sum += cache->getY() + i / 2;
    }
    return sum;
}

uint64_t current_time_us() {
    return std::chrono::duration_cast<std::chrono::microseconds>(
        std::chrono::steady_clock::now().time_since_epoch()
    ).count();
}

int main() {
    // 创建一个大数据vector
    std::vector<uint64_t> data;
    for(int i=0; i<100000000; ++i) data.push_back(i*i + 1);

    auto t_start = current_time_us();
    std::cout << NoCache() << std::endl;
    auto t_end = current_time_us();
    std::cout << "NoCache: " << (t_end-t_start) / 1000 << " ms" << std::endl;

    t_start = current_time_us();
    std::cout << Cache() << std::endl;
    t_end = current_time_us();
    std::cout << "Cache: " << (t_end-t_start) / 1000 << " ms" << std::endl;

    return 0;
}