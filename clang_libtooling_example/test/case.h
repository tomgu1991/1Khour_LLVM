//
// Created by zuxinggu on 2024/8/28.
//

#ifndef CASE_H
#define CASE_H

class Helper {
public:
  bool init() {
    return true;
  };
};

class Handler {
public:
  Handler(const unsigned int head_uin);

  int operator()();

protected:
  int A();
  int B(bool& could_open);
  int C();
  int D();
  int E();
  int F();

private:
  const unsigned int head_uin_;
  Helper helper_;
};

#endif //CASE_H
