#include <vector>

int foo();
int bar(int);

void Foo();

int test5(std::vector<int> &v1, std::vector<int> &v2) {
    // can not find the error
    // https://pvs-studio.com/en/docs/warnings/v662/
    std::vector<int>::iterator it;
    for (it = v1.begin(); it != v2.end(); ++it)
        *it = 1;
    return 1; 
}

int test4(int n, int y) {
    // can not find the error
    // https://pvs-studio.com/en/docs/warnings/v581/
    if (n > y) {
        Foo();
    }
    if (n > y) {
        Foo();
    }
    return 1;
}

int GetX();
int GetY();
int test3(int n) {
    // can not find the error
    // https://pvs-studio.com/en/docs/warnings/v537/
    int x = GetX() * n;
    int y = GetX() * n;
    return x + y;
}

int test2()
{
    // can not find the error
    // https://pvs-studio.com/en/docs/warnings/v531/
    int a[100] = {0};
    return sizeof(a) * sizeof(int);
}

int test(std::vector<int> &my_container) {
    // can not find the error
    // https://pvs-studio.com/en/docs/warnings/v728/
    if (my_container.empty() || (!my_container.empty() && my_container[0] == 0)) {
        return 0;
    }
    // [modernize-loop-convert]
    for (std::vector<int>::iterator I = my_container.begin(),
                                E = my_container.end();
     I != E; ++I) {
        *I += 1;
    }
    return 1;
}