#include <vector>
using namespace std;

int foo();
int bar(int);

int test(vector<int> &my_container) {
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
    int x = 1;
    // int y = bar(y);
    // x = foo();
    return x;
}