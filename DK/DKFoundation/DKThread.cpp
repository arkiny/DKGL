//
//  File: DKThread.cpp
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2004-2016 Hongtae Kim. All rights reserved.
//

#ifdef _WIN32
#include <process.h>
#include <windows.h>
#else
#include <pthread.h>
#include <sys/select.h>
#include <sched.h>		// to using sched_yield() in DKThread::Yield()
#include <errno.h>
#include <limits.h>
#endif

#include "DKThread.h"
#include "DKObject.h"
#include "DKError.h"
#include "DKMap.h"
#include "DKMutex.h"
#include "DKCondition.h"
#include "DKSpinLock.h"
#include "DKFunction.h"
#include "DKLog.h"

#ifndef POSIX_USE_SELECT_SLEEP
/// Set POSIX_USE_SELECT_SLEEP to 1 if you want use 'select' instead of 'nanosleep'.
/// ignored on Win32.
#define POSIX_USE_SELECT_SLEEP	1
#endif

namespace DKFoundation::Private
{
#if defined(__APPLE__) && defined(__MACH__)
    bool InitializeMultiThreadedEnvironment();
    void PerformOperationInsidePool(DKOperation* op);
#else
    FORCEINLINE bool InitializeMultiThreadedEnvironment() { return true; }
    FORCEINLINE void PerformOperationInsidePool(DKOperation* op) { op->Perform(); }
#endif
    void PerformOperationWithErrorHandler(const DKOperation*, size_t);

    struct ThreadCreationInfo
    {
        DKThread::ThreadId		id;
        DKObject<DKOperation>	op;
    };
    struct ThreadContext
    {
        DKThread::ThreadId		id;
        DKObject<DKOperation>	op;
        bool					running;
    };
    typedef DKMap<DKThread::ThreadId, ThreadContext> RunningThreadsMap;
    static RunningThreadsMap runningThreads;
    static DKCondition threadCond;

#ifdef _WIN32
    DKString GetWin32ErrorString(DWORD dwError); // defined DKError.cpp

    unsigned int __stdcall ThreadProc(void* p)
    {
#else
    void* ThreadProc(void* p)
    {
        //pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
        //pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, NULL);
#endif
        ThreadCreationInfo* param = reinterpret_cast<ThreadCreationInfo*>(p);
        // register running threads;
        threadCond.Lock();
        DKThread::ThreadId tid = DKThread::CurrentThreadId();
        ThreadContext& ctxt = runningThreads.Value(tid);
        ctxt.id = tid;
        ctxt.op = param->op;
        ctxt.running = false;
        param->id = tid;
        threadCond.Broadcast();

        // waiting for ctxt.running is being flagged.
        while (!ctxt.running)
            threadCond.Wait();
        threadCond.Unlock();

        if (ctxt.op)
        {
            // Calling thread procedure.
            // To use NSAutoreleasePool in Cocoa,
            // Wrap operation with PerformOperatonInsidePool.
            struct OpWrapper : public DKOperation
            {
                OpWrapper(DKOperation* o) : op(o) {}
                DKOperation* op;
                void Perform() const override
                {
                    PerformOperationWithErrorHandler(op, DKERROR_DEFAULT_CALLSTACK_TRACE_DEPTH);
                }
            };
            OpWrapper wrapper(ctxt.op);
            PerformOperationInsidePool(&wrapper);
        }

        threadCond.Lock();
        runningThreads.Remove(tid);
        threadCond.Broadcast();
        threadCond.Unlock();

        // terminate thread.
#ifdef _WIN32
            //ExitThread(0);
        _endthreadex(0);
#else
        pthread_exit(0);
#endif
        return 0;
    }

    static ThreadContext* CreateThread(DKOperation* op, size_t stackSize)
    {
        if (op == NULL)
            return NULL;

        InitializeMultiThreadedEnvironment();

        ThreadCreationInfo param;
        param.id = DKThread::invalidId;
        param.op = op;

        bool failed = true;
#ifdef _WIN32
        if (stackSize > 0)
        {
            size_t pageSize = DKMemoryPageSize();
            if (stackSize % pageSize)
                stackSize += pageSize - (stackSize % pageSize);
        }
        unsigned int id;
        HANDLE h = (HANDLE)_beginthreadex(0, stackSize, ThreadProc, reinterpret_cast<void*>(&param), 0, &id);
        if (h)
        {
            static const int numProcessorGroups = GetMaximumProcessorGroupCount();
            if (numProcessorGroups > 1)
            {
                static WORD groupId = 0;
                WORD affinityGroup = groupId++;
                affinityGroup = affinityGroup % numProcessorGroups;
                DKLogI("Setting thread group affinity: %d / %d", affinityGroup, numProcessorGroups);

                GROUP_AFFINITY groupAffinity = {};
                groupAffinity.Mask = ~KAFFINITY(0);
                groupAffinity.Group = affinityGroup;
                if (!SetThreadGroupAffinity(h, &groupAffinity, NULL))
                {
                    DKLogE("SetThreadGroupAffinity Error: %ls", (const wchar_t*)GetWin32ErrorString(GetLastError()));
                }
            }
            // a closed handle could be recycled by system.
            CloseHandle(h);
            failed = false;
        }
#else
        // set pthread to detached, unable to be joined.
        pthread_t id;
        pthread_attr_t attr;
        pthread_attr_init(&attr);
        pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
        if (stackSize > 0)
        {
            stackSize = Max(stackSize, (size_t)PTHREAD_STACK_MIN);
            size_t pageSize = DKMemoryPageSize();
            if (stackSize % pageSize)
                stackSize += pageSize - (stackSize % pageSize);
            pthread_attr_setstacksize(&attr, stackSize);
        }
        failed = (bool)pthread_create((pthread_t*)&id, &attr, ThreadProc, reinterpret_cast<void*>(&param));
        pthread_attr_destroy(&attr);
#endif
        if (!failed)
        {
            DKCriticalSection<DKCondition> guard(threadCond);

            while (param.id == DKThread::invalidId)
                threadCond.Wait();

            DKASSERT_DEBUG(param.id == (DKThread::ThreadId)id);

            while (true)
            {
                RunningThreadsMap::Pair* p = runningThreads.Find(param.id);
                if (p)
                    return &p->value;
                threadCond.Wait();
            }
        }
        return NULL;
    }
}
using namespace DKFoundation;
using namespace DKFoundation::Private;

const DKThread::ThreadId DKThread::invalidId = (DKThread::ThreadId)0;

DKThread::DKThread()
	: threadId(invalidId)
{
}

DKThread::~DKThread()
{
}

DKObject<DKThread> DKThread::FindThread(ThreadId id)
{
	DKCriticalSection<DKCondition> guard(threadCond);
	if (runningThreads.Find(id) != NULL)
	{
		DKObject<DKThread> ret = DKObject<DKThread>::New();
		ret->threadId = id;
		return ret;
	}
	return NULL;
}

DKObject<DKThread> DKThread::CurrentThread()
{
	return FindThread(CurrentThreadId());
}

DKThread::ThreadId DKThread::CurrentThreadId()
{
#ifdef _WIN32
	return (ThreadId)::GetCurrentThreadId();
#else
	return (ThreadId)pthread_self();	
#endif
}

void DKThread::Yield()
{
#ifdef _WIN32
	if (SwitchToThread() == 0)
		Sleep(0);
#else
	if (sched_yield() != 0)
		Sleep(0);
#endif
}

void DKThread::Sleep(double d)
{
	d = Max(d, 0.0);

#ifdef _WIN32
	DWORD dwTime = static_cast<DWORD>(d * 1000.0f);
	::Sleep(dwTime);
#elif POSIX_USE_SELECT_SLEEP
	timeval tm;
	uint64_t ms = (uint64_t)(d * 1000000.0);
	tm.tv_sec = ms / 1000000;
	tm.tv_usec = ms % 1000000;
	select(0, 0, 0, 0, &tm);
#else
	long sec = static_cast<long>(d);
	long usec = (d - sec) * 1000000;
	struct timespec req = {sec, usec * 1000};
	while ( nanosleep(&req, &req) != 0 )
	{
		// internal error! (except for signal, intrrupt)
		if (errno != EINTR)
			break;
	}
#endif
}

DKObject<DKThread> DKThread::Create(const DKOperation* operation, size_t stackSize)
{
	if (operation == NULL)
		return NULL;

	ThreadContext* ctxt = CreateThread(const_cast<DKOperation*>(operation), stackSize);
	if (ctxt)
	{
		DKObject<DKThread> ret = DKObject<DKThread>::New();

		DKCriticalSection<DKCondition> guard(threadCond);

		if (true)
		{
			ctxt->running = true;
			threadCond.Broadcast();
		}
		ret->threadId = ctxt->id;
		// param will be deleted by thread.
		return ret;
	}
	return NULL;
}

void DKThread::WaitTerminate() const
{
	if (threadId != invalidId)
	{
		DKASSERT_DESC(threadId != CurrentThreadId(), "Cannot wait current thread");

		DKCriticalSection<DKCondition> guard(threadCond);
		while (runningThreads.Find(threadId))
			threadCond.Wait();

		threadId = invalidId;
		Yield();
	}
}

DKThread::ThreadId DKThread::Id() const
{
	if (threadId != invalidId)
	{
		DKCriticalSection<DKCondition> guard(threadCond);
		if (runningThreads.Find(threadId))
			return threadId;
		threadId = invalidId;
	}
	return invalidId;
}

bool DKThread::IsAlive() const
{
	if (threadId != invalidId)
	{
		if (CurrentThreadId() == threadId)
			return true;

		DKCriticalSection<DKCondition> guard(threadCond);
		if (runningThreads.Find(threadId))
			return true;
		threadId = invalidId;
	}
	return false;
}

bool DKThread::SetPriority(double p)
{
	ThreadId tid = Id();
#if _WIN32
	HANDLE hThread = OpenThread(THREAD_SET_INFORMATION, FALSE, (DWORD)tid);
	if (hThread)
	{
		const int priorities[5] = {
			THREAD_PRIORITY_LOWEST,
			THREAD_PRIORITY_BELOW_NORMAL,
			THREAD_PRIORITY_NORMAL,
			THREAD_PRIORITY_ABOVE_NORMAL,
			THREAD_PRIORITY_HIGHEST
		};
		int prio = Clamp<int>(floor(p * 4.0 + 0.5), 0, 4);
		bool ret = ::SetThreadPriority(hThread, priorities[prio]) != 0;
		CloseHandle(hThread);
		return ret;
	}
	else
	{
		DKLogE("DKThread::SetPriority Error: OpenThread Error");
	}
	return false;
#else
	int policy = SCHED_FIFO;
	struct sched_param schedule;
	if (pthread_getschedparam((pthread_t)tid, &policy, &schedule) != 0)
	{
		return false;
	}

	p = Clamp(p, 0.0, 1.0);

	int min_priority = sched_get_priority_min(policy);
	int max_priority = sched_get_priority_max(policy);

	schedule.sched_priority = (int)p * (max_priority - min_priority) + min_priority;
	return pthread_setschedparam((pthread_t)tid, policy, &schedule) == 0;
#endif
}

double DKThread::Priority() const
{
	ThreadId tid = Id();
#if _WIN32
	HANDLE hThread = OpenThread(THREAD_QUERY_INFORMATION, FALSE, (DWORD)tid);
	if (hThread)
	{
		int prio = ::GetThreadPriority(hThread);
		CloseHandle(hThread);

		if (prio == THREAD_PRIORITY_ERROR_RETURN)
		{
			DKLogE("DKThread::Priority Error: GetThreadPriority failed");
		}
		else
		{
			const int priorities[5] = {
				THREAD_PRIORITY_LOWEST,
				THREAD_PRIORITY_BELOW_NORMAL,
				THREAD_PRIORITY_NORMAL,
				THREAD_PRIORITY_ABOVE_NORMAL,
				THREAD_PRIORITY_HIGHEST
			};
			for (int i = 0; i < 4; ++i)
			{
				if (prio < priorities[i+1])
				{
					return Clamp(double(i) / 4.0, 0.0, 1.0);
				}
			}
		}
	}
	else
	{
		DKLogE("DKThread::SetPriority Error: OpenThread Error");
	}
	return 1.0;
#else
	int policy = SCHED_FIFO;
	struct sched_param schedule;
	if (pthread_getschedparam((pthread_t)tid, &policy, &schedule) != 0)
	{
		return 1.0;
	}
	int min_priority = sched_get_priority_min(policy);
	int max_priority = sched_get_priority_max(policy);
	return (double)(schedule.sched_priority - min_priority) / (double)(max_priority - min_priority);
#endif
}
