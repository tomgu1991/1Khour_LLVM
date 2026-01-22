#include <vector>
#include <memory>

int foo();
int bar(int);

void Foo();


class A {
public:
  int x;
  int y;
};

void test7() {
  A *p; // can find
  p->x = 1;
  p->y = 2;
  A *p1 = nullptr; // fails to find
  p1->x = 1;
  std::shared_ptr<A> sp; // fails to find
  sp->x = 1;
}

enum msg {
    ERROR = -1,
    OK = 0
};

#define ASSERT_RETURN(stat, ret) \
    if (!(stat)) { \
        return ret; \
    } \

bool test6(std::vector<int> &container) {
    for (auto &elem : container) {
        // can not find the error
        ASSERT_RETURN(elem > 100, msg::ERROR)
    }
    for (auto &elem : container) {
        if (!(elem > 100)) {
            // readability-implicit-bool-conversion
            return msg ::OK;
        }
    }

    return true;
}

int test5(std::vector<int> &v1, std::vector<int> &v2) {
    // can not find the error
    // https://pvs-studio.com/en/docs/warnings/v662/
    std::vector<int>::iterator it;
    for (it = v1.begin(); it != v2.end(); ++it)
        *it = 1;
    return 1; 
}

int test4(int left, int right) {
    // can not find the error
    // https://pvs-studio.com/en/docs/warnings/v581/
    // Conditional expressions of 'if' statements located next to each other are identical
    if (left > 0) {
        bar(left);
    }
    if (left > 0) {
        bar(right);
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
    // can find the error, bugprone-sizeof-expression,cert-arr39-c
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