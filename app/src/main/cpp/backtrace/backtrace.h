//
// Created by Jekton Luo on 2019/4/4.
//

#ifndef NATIVECRASHCATCHING_BACKTRACE_H_
#define NATIVECRASHCATCHING_BACKTRACE_H_

#include <unistd.h>

#include <string>

class GetTraceCallback {
 public:
  virtual void OnFrame(size_t frame_num, std::string frame) = 0;
  virtual void OnFail() = 0;
  virtual ~GetTraceCallback() {}
};

/*
 * @ctx can be nullptr or context from signal handler
 */
void GetStackTrace(pid_t tid, void* ctx, GetTraceCallback* callback);

#endif //NATIVECRASHCATCHING_BACKTRACE_H_
