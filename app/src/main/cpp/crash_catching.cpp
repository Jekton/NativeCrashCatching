//
// Created by Jekton Luo on 2019/4/3.
//

#include "crash_catching.h"

#include <dlfcn.h>
#include <signal.h>
#include <string.h>
#include <ucontext.h>

#include <map>
#include <memory>
#include <mutex>
#include <new>
#include <string>
#include <sstream>
#include <thread>

#include "backtrace/backtrace.h"
#include "dlopen.h"
#include "log_util.h"
#include "util.h"

static const char* kTag = "Jekton CrashCatching";

static std::map<int, struct sigaction> sOldHandlers;

static void SetUpStack();
static void SetUpSigHandler();

static void CallOldHandler(int signo, siginfo_t* info, void* context);
static void SignalHandler(int signo, siginfo_t* info, void* context);
static void DumpSignalInfo(siginfo_t* info);
static void DumpStacks(void* context);

static pid_t sTidToDump;    // guarded by sMutex
static void* sContext;
static std::mutex sMutex;
static std::condition_variable sCondition;
static void StackDumpingThread();

void InitCrashCaching() {
  LOGD(kTag, "InitCrashCaching");
  SetUpStack();
  SetUpSigHandler();
  std::thread{StackDumpingThread}.detach();
}

static void SetUpStack() {
  stack_t stack{};
  stack.ss_sp = new(std::nothrow) char[SIGSTKSZ];

  if (!stack.ss_sp) {
    LOGW(kTag, "fail to alloc stack for crash catching");
    return;
  }
  stack.ss_size = SIGSTKSZ;
  stack.ss_flags = 0;
  if (stack.ss_sp) {
    if (sigaltstack(&stack, nullptr) != 0) {
      LOGERRNO(kTag, "fail to setup signal stack");
    }
  }
}

static void SetUpSigHandler() {
  struct sigaction action{};
  action.sa_sigaction = SignalHandler;
  action.sa_flags = SA_SIGINFO | SA_ONSTACK;
  int signals[] = {
      SIGABRT, SIGBUS, SIGFPE, SIGILL, SIGSEGV, SIGPIPE
  };
  struct sigaction old_action;
  for (auto signo : signals) {
    if (sigaction(signo, &action, &old_action) == -1) {
      LOGERRNO(kTag, "fail to set signal handler for signo %d", signo);
    } else {
      if (old_action.sa_handler != SIG_DFL && old_action.sa_handler != SIG_IGN) {
        sOldHandlers[signo] = old_action;
      }
    }
  }
}


static void SignalHandler(int signo, siginfo_t* info, void* context) {
  DumpSignalInfo(info);
  DumpStacks(context);

  CallOldHandler(signo, info, context);
  exit(0);
}

static void CallOldHandler(int signo, siginfo_t* info, void* context) {
  auto it = sOldHandlers.find(signo);
  if (it != sOldHandlers.end()) {
    if (it->second.sa_flags & SA_SIGINFO) {
      it->second.sa_sigaction(signo, info, context);
    } else {
      it->second.sa_handler(signo);
    }
  }
}

static void DumpSignalInfo(siginfo_t* info) {
  switch (info->si_signo) {
  case SIGILL:
    LOGI(kTag, "signal SIGILL caught");
    switch (info->si_code) {
    case ILL_ILLOPC:
      LOGI(kTag, "illegal opcode");
      break;
    case ILL_ILLOPN:
      LOGI(kTag, "illegal operand");
      break;
    case ILL_ILLADR:
      LOGI(kTag, "illegal addressing mode");
      break;
    case ILL_ILLTRP:
      LOGI(kTag, "illegal trap");
      break;
    case ILL_PRVOPC:
      LOGI(kTag, "privileged opcode");
      break;
    case ILL_PRVREG:
      LOGI(kTag, "privileged register");
      break;
    case ILL_COPROC:
      LOGI(kTag, "coprocessor error");
      break;
    case ILL_BADSTK:
      LOGI(kTag, "internal stack error");
      break;
    default:
      LOGI(kTag, "code = %d", info->si_code);
      break;
    }
    break;
  case SIGFPE:
    LOGI(kTag, "signal SIGFPE caught");
    switch (info->si_code) {
    case FPE_INTDIV:
      LOGI(kTag, "integer divide by zero");
      break;
    case FPE_INTOVF:
      LOGI(kTag, "integer overflow");
      break;
    case FPE_FLTDIV:
      LOGI(kTag, "floating-point divide by zero");
      break;
    case FPE_FLTOVF:
      LOGI(kTag, "floating-point overflow");
      break;
    case FPE_FLTUND:
      LOGI(kTag, "floating-point underflow");
      break;
    case FPE_FLTRES:
      LOGI(kTag, "floating-point inexact result");
      break;
    case FPE_FLTINV:
      LOGI(kTag, "invalid floating-point operation");
      break;
    case FPE_FLTSUB:
      LOGI(kTag, "subscript out of range");
      break;
    default:
      LOGI(kTag, "code = %d", info->si_code);
      break;
    }
    break;
  case SIGSEGV:
    LOGI(kTag, "signal SIGSEGV caught");
    switch (info->si_code) {
    case SEGV_MAPERR:
      LOGI(kTag, "address not mapped to object");
      break;
    case SEGV_ACCERR:
      LOGI(kTag, "invalid permissions for mapped object");
      break;
    default:
      LOGI(kTag, "code = %d", info->si_code);
      break;
    }
    break;
  case SIGBUS:
    LOGI(kTag, "signal SIGBUS caught");
    switch (info->si_code) {
    case BUS_ADRALN:
      LOGI(kTag, "invalid address alignment");
      break;
    case BUS_ADRERR:
      LOGI(kTag, "nonexistent physical address");
      break;
    case BUS_OBJERR:
      LOGI(kTag, "object-specific hardware error");
      break;
    default:
      LOGI(kTag, "code = %d", info->si_code);
      break;
    }
    break;
  case SIGABRT:
    LOGI(kTag, "signal SIGABRT caught");
    break;
  case SIGPIPE:
    LOGI(kTag, "signal SIGPIPE caught");
    break;
  default:
    LOGI(kTag, "signo %d caught", info->si_signo);
    LOGI(kTag, "code = %d", info->si_code);
  }
  LOGI(kTag, "errno = %d", info->si_errno);
}

static void DumpStacks(void* context) {
  std::unique_lock<std::mutex> lock{sMutex};
  sTidToDump = gettid();
  sContext = context;
  sCondition.notify_one();
  sCondition.wait(lock, []{ return sTidToDump == 0; });
}

static void StackDumpingThread() {
  std::unique_lock<std::mutex> lock{sMutex};
  sCondition.wait(lock, [] { return sTidToDump > 0; });
  class Callback : public GetTraceCallback {
   public:
    void OnFrame(size_t frame_num, std::string frame) override {
      LOGD(kTag, "%s", frame.c_str());
    }

    void OnFail() override {
      LOGW(kTag, "Fail to get stack trace");
    }
  };
  std::unique_ptr<Callback> callback{new Callback};
  GetStackTrace(sTidToDump, sContext, callback.get());
  sTidToDump = 0;
  sCondition.notify_one();
}

