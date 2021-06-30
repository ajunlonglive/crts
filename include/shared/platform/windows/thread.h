#ifndef SHARED_PLATFORM_WINDOWS_THREAD_H
#define SHARED_PLATFORM_WINDOWS_THREAD_H

struct thread {
	void *handle;
	unsigned long id;
};
#endif
