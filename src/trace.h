#pragma once
// #define NOTRACE
// #define TRACENOEXIT
#define MAXSTACKDEPTH 32

#include <stdint.h>
#include <stddef.h>

void _trace(const char *text);
void _trace2(const char *text, const char *text2);
void _trace2e(const char *text, const char *text2);
void _traceint(const char *text, uint64_t val);
void _traceinth(const char *text, uint64_t val);
void _stackprot();
extern int64_t tracestackdepth;

#ifndef NOTRACE
#define trace(...) _trace(__VA_ARGS__)
#define trace2(...) _trace2(__VA_ARGS__)
#define trace2e(...) _trace2e(__VA_ARGS__)
#define traceint(...) _traceint(__VA_ARGS__)
#define traceinth(...) _traceinth(__VA_ARGS__)
#else
#define trace(...)
#define trace2(...)
#define trace2e(...)
#define traceint(...)
#define traceinth(...)
#endif

#define trace_error(errormsg) trace("\033[1;31mError: " errormsg "\033[0m");

#ifdef MAXSTACKDEPTH
#define trace_enter() trace2e("\033[1;32mEnter\033[0m ", __func__); tracestackdepth++; _stackprot()
#else
#define trace_enter() trace2e("\033[1;32mEnter\033[0m ", __func__); tracestackdepth++
#endif

#define trace_call() trace2("\033[1;34mCall\033[0m ", __func__)
#define trace_log_addr(addr) traceinth("Addr", (uintptr_t)addr)

#ifdef TRACENOEXIT
#define trace_exit() tracestackdepth--; trace("\033[2m╵\033[0m")
#else
#define trace_exit() tracestackdepth--; trace2("\033[1;33mExit\033[0m ", __func__)
#endif
