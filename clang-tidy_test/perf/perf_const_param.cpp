#include <iostream>
#include <vector>
#include <cstdint>
#include <chrono>

// 拷贝参数
void ProcessByValue(const std::vector<uint64_t> vec) {
    // 模拟遍历
    uint64_t sum = 0;
    for (auto v : vec) sum += v;
}

// const引用参数
void ProcessByConstRef(const std::vector<uint64_t>& vec) {
    // 模拟遍历
    uint64_t sum = 0;
    for (auto v : vec) sum += v;
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
    ProcessByConstRef(data);
    auto t_end = current_time_us();
    std::cout << "ProcessByRef: " << (t_end-t_start) / 1000 << " ms" << std::endl;

    t_start = current_time_us();
    ProcessByValue(data);
    
    t_end = current_time_us();
    std::cout << "ProcessByConstValue: " << (t_end-t_start) / 1000 << " ms" << std::endl;

    return 0;
}