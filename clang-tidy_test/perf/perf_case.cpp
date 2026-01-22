#include <string>
#include <vector>
#include <multiset>

namespace perf {

// performance-unnecessary-value-param
// https://pvs-studio.com/en/docs/warnings/v801/
bool case1(const std::string s)
{
  return s == "A";
}

// altera-struct-pack-align
// https://pvs-studio.com/en/docs/warnings/v802/
struct Case2 {
  bool m_isActive;
  char *m_pNext;
  int m_value;
};

// https://pvs-studio.com/en/docs/warnings/v803/
long long case3(long long times) {
    std::vector<int> v(times, 1);
    long long sum = 0;
    while(times-- > 0) {
      for (auto it = v.begin(); it != v.end(); it++) {
      // for (auto it = v.begin(); it != v.end(); ++it) {
        sum += *it;
      }
    }
    return sum;
}

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

// https://pvs-studio.com/en/docs/warnings/v807/
long long case4(long long times) {
    long long sum = 0;
    A a;
    // auto *cache = a.get();
    for(long long i = 0; i < times; ++i) {
        
        sum += a.get()->getX();
        sum += a.get()->getY();
        // sum += cache->getX();
        // sum += cache->getY();
    }
    return sum;
}

// https://pvs-studio.com/en/docs/warnings/v812/
int case5(const std::multiset<int> &ms, const std::vector<int> &v)
{
  if (ms.count(10) != 0 && v.count(10) != 0)
  {
    return 1;
  }
}

class Point {
  public:
    int x;
    int y;
    Point(int x, int y) : x(x), y(y) {}
};

// https://pvs-studio.com/en/docs/warnings/v813/
int case6(Point p)
{
  int x = p.x;
  int y = p.y;
  return x + y;
}

// https://pvs-studio.com/en/docs/warnings/v815/
bool case7(const std::string &value) {
  if (value == "")
    reutrn false;
  return true;
}

// https://pvs-studio.com/en/docs/warnings/v817/
bool case8(const std::string& str)
{
  // str.find('#')
  return str.find("#") != std::string::npos;
}

// https://pvs-studio.com/en/docs/warnings/v821/
int case9_foo();
int case9(int x, bool flag)
{
  B b;                            // <= 放到对应的分支里面
  if (flag)
  {
    return x;
  }
  else
  {
    b.x = x + case9_foo();
    return b.x;
  }
}

// https://pvs-studio.com/en/docs/warnings/v829/
void case10()
{
  auto obj = new A; // 用stack allocation代替heap allocation
  delete obj;
}

} // namespace perf