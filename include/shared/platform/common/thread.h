#ifndef SHARED_PLATFORM_COMMON_THREAD_H
#define SHARED_PLATFORM_COMMON_THREAD_H

#include <stdbool.h>

#if defined(CRTS_PLATFORM_posix)
#include "shared/platform/posix/thread.h"
#elif defined(CRTS_PLATFORM_windows)
#include "shared/platform/windows/thread.h"
#else
#error "no valid platform defined"
#endif

typedef void *((*thread_func)(void *ctx));

bool thread_create(struct thread *thread, thread_func func, void *ctx);
#endif
