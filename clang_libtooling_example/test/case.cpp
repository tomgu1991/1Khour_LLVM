//
// Created by zuxinggu on 2024/8/28.
//

#include "case.h"

Handler::Handler(const unsigned int head_uin): head_uin_(head_uin) {}

static void local_func() {}

int Handler::operator()() {
  A();
  bool flag = false;
  B(flag);
  if(flag) {
    C();
    local_func();
  } else {
    D();
  }
  return 0;
}

int Handler::A() {
  E();
  if(!helper_.init()) {
    return -1;
  }
  return 0;
}

int Handler::B(bool& could_open) {
  return 0;
}

int Handler::C() {
  return 0;
}

int Handler::D() {
  return 0;
}