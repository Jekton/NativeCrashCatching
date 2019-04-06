//
// Created by Jekton Luo on 2019/4/4.
//

#include <syscall.h>
#include <unistd.h>

pid_t gettid() {
  return (pid_t) syscall(__NR_gettid);
}

