// Copyright 2009 The Go Authors. All rights reserved.
// Use of this source code is governed by a BSD-style
// license that can be found in the LICENSE file.

#include "config.h"

#include "go-assert.h"
#include <complex.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <pthread.h>
#include <semaphore.h>
#include <ucontext.h>

#ifdef HAVE_SYS_MMAN_H
#include <sys/mman.h>
#endif

#include "go-alloc.h"

#define _STRINGIFY2_(x) #x
#define _STRINGIFY_(x) _STRINGIFY2_(x)
#define GOSYM_PREFIX _STRINGIFY_(__USER_LABEL_PREFIX__)

/* This file supports C files copied from the 6g runtime library.
   This is a version of the 6g runtime.h rewritten for gccgo's version
   of the code.  */

typedef signed int   int8    __attribute__ ((mode (QI)));
typedef unsigned int uint8   __attribute__ ((mode (QI)));
typedef signed int   int16   __attribute__ ((mode (HI)));
typedef unsigned int uint16  __attribute__ ((mode (HI)));
typedef signed int   int32   __attribute__ ((mode (SI)));
typedef unsigned int uint32  __attribute__ ((mode (SI)));
typedef signed int   int64   __attribute__ ((mode (DI)));
typedef unsigned int uint64  __attribute__ ((mode (DI)));
typedef float        float32 __attribute__ ((mode (SF)));
typedef double       float64 __attribute__ ((mode (DF)));
typedef signed int   intptr __attribute__ ((mode (pointer)));
typedef unsigned int uintptr __attribute__ ((mode (pointer)));

typedef intptr		intgo; // Go's int
typedef uintptr		uintgo; // Go's uint

typedef uintptr		uintreg;

/* Defined types.  */

typedef	uint8			bool;
typedef	uint8			byte;
typedef	struct	g		G;
typedef	struct	mutex		Lock;
typedef	struct	m		M;
typedef	struct	p		P;
typedef	struct	note		Note;
typedef	struct	String		String;
typedef	struct	FuncVal		FuncVal;
typedef	struct	SigTab		SigTab;
typedef	struct	mcache		MCache;
typedef struct	FixAlloc	FixAlloc;
typedef	struct	hchan		Hchan;
typedef	struct	timer		Timer;
typedef	struct	gcstats		GCStats;
typedef	struct	lfnode		LFNode;
typedef	struct	ParFor		ParFor;
typedef	struct	ParForThread	ParForThread;
typedef	struct	cgoMal		CgoMal;
typedef	struct	PollDesc	PollDesc;
typedef	struct	sudog		SudoG;

typedef	struct	__go_open_array		Slice;
typedef	struct	iface			Iface;
typedef	struct	eface			Eface;
typedef	struct	__go_type_descriptor	Type;
typedef	struct	_defer			Defer;
typedef	struct	_panic			Panic;

typedef struct	__go_ptr_type		PtrType;
typedef struct	__go_func_type		FuncType;
typedef struct	__go_interface_type	InterfaceType;
typedef struct	__go_map_type		MapType;
typedef struct	__go_channel_type	ChanType;

typedef struct  tracebackg	Traceback;

typedef struct	location	Location;

struct String
{
	const byte*	str;
	intgo		len;
};

struct FuncVal
{
	void	(*fn)(void);
	// variable-size, fn-specific data here
};

#include "array.h"

// Rename Go types generated by mkrsysinfo.sh from C types, to avoid
// the name conflict.
#define timeval go_timeval
#define timespec go_timespec

#include "runtime.inc"

#undef timeval
#undef timespec

/*
 * Per-CPU declaration.
 */
extern M*	runtime_m(void);
extern G*	runtime_g(void)
  __asm__(GOSYM_PREFIX "runtime.getg");

extern M	runtime_m0;
extern G	runtime_g0;

enum
{
	true	= 1,
	false	= 0,
};
enum
{
	PtrSize = sizeof(void*),
};
enum
{
	// Per-M stack segment cache size.
	StackCacheSize = 32,
	// Global <-> per-M stack segment cache transfer batch size.
	StackCacheBatch = 16,
};

struct	SigTab
{
	int32	sig;
	int32	flags;
	void*   fwdsig;
};

#ifdef GOOS_nacl
enum {
   NaCl = 1,
};
#else
enum {
   NaCl = 0,
};
#endif

#ifdef GOOS_windows
enum {
   Windows = 1
};
#else
enum {
   Windows = 0
};
#endif
#ifdef GOOS_solaris
enum {
   Solaris = 1
};
#else
enum {
   Solaris = 0
};
#endif

// Parallel for descriptor.
struct ParFor
{
	const FuncVal *body;		// executed for each element
	uint32 done;			// number of idle threads
	uint32 nthr;			// total number of threads
	uint32 nthrmax;			// maximum number of threads
	uint32 thrseq;			// thread id sequencer
	uint32 cnt;			// iteration space [0, cnt)
	bool wait;			// if true, wait while all threads finish processing,
					// otherwise parfor may return while other threads are still working
	ParForThread *thr;		// array of thread descriptors
	// stats
	uint64 nsteal;
	uint64 nstealcnt;
	uint64 nprocyield;
	uint64 nosyield;
	uint64 nsleep;
};

extern bool runtime_precisestack;
extern bool runtime_copystack;

/*
 * defined macros
 *    you need super-gopher-guru privilege
 *    to add this list.
 */
#define	nelem(x)	(sizeof(x)/sizeof((x)[0]))
#define	nil		((void*)0)
#define USED(v)		((void) v)
#define	ROUND(x, n)	(((x)+(n)-1)&~(uintptr)((n)-1)) /* all-caps to mark as macro: it evaluates n twice */

byte*	runtime_startup_random_data;
uint32	runtime_startup_random_data_len;
void	runtime_get_random_data(byte**, int32*);

enum {
	// hashinit wants this many random bytes
	HashRandomBytes = 32
};
void	runtime_hashinit(void);

void	runtime_traceback(int32)
  __asm__ (GOSYM_PREFIX "runtime.traceback");
void	runtime_tracebackothers(G*)
  __asm__ (GOSYM_PREFIX "runtime.tracebackothers");
enum
{
	// The maximum number of frames we print for a traceback
	TracebackMaxFrames = 100,
};

/*
 * external data
 */
extern	uintptr* runtime_getZerobase(void)
  __asm__(GOSYM_PREFIX "runtime.getZerobase");
extern	G**	runtime_allg;
extern	uintptr runtime_allglen;
extern	G*	runtime_lastg;
extern	M*	runtime_allm;
extern	P**	runtime_allp;
extern	int32	runtime_gomaxprocs;
extern	uint32	runtime_needextram;
extern	uint32	runtime_panicking;
extern	int8*	runtime_goos;
extern	int32	runtime_ncpu;
extern 	void	(*runtime_sysargs)(int32, uint8**);
extern	struct debugVars runtime_debug;

extern	bool	runtime_isstarted;
extern	bool	runtime_isarchive;

/*
 * common functions and data
 */
#define runtime_strcmp(s1, s2) __builtin_strcmp((s1), (s2))
#define runtime_strncmp(s1, s2, n) __builtin_strncmp((s1), (s2), (n))
#define runtime_strstr(s1, s2) __builtin_strstr((s1), (s2))
intgo	runtime_findnull(const byte*)
  __asm__ (GOSYM_PREFIX "runtime.findnull");

void	runtime_gogo(G*);
struct __go_func_type;
void	runtime_args(int32, byte**)
  __asm__ (GOSYM_PREFIX "runtime.args");
void	runtime_osinit();
void	runtime_goargs(void)
  __asm__ (GOSYM_PREFIX "runtime.goargs");
void	runtime_goenvs(void);
void	runtime_goenvs_unix(void)
  __asm__ (GOSYM_PREFIX "runtime.goenvs_unix");
void	runtime_throw(const char*) __attribute__ ((noreturn));
void	runtime_panicstring(const char*) __attribute__ ((noreturn));
bool	runtime_canpanic(G*);
void	runtime_printf(const char*, ...);
int32	runtime_snprintf(byte*, int32, const char*, ...);
#define runtime_mcmp(a, b, s) __builtin_memcmp((a), (b), (s))
#define runtime_memmove(a, b, s) __builtin_memmove((a), (b), (s))
void*	runtime_mal(uintptr);
String	runtime_gostringnocopy(const byte*)
  __asm__ (GOSYM_PREFIX "runtime.gostringnocopy");
void	runtime_schedinit(void);
void	runtime_initsig(bool)
  __asm__ (GOSYM_PREFIX "runtime.initsig");
int32	runtime_gotraceback(bool *crash);
void	runtime_goroutineheader(G*)
  __asm__ (GOSYM_PREFIX "runtime.goroutineheader");
void	runtime_printtrace(Slice, G*)
  __asm__ (GOSYM_PREFIX "runtime.printtrace");
#define runtime_open(p, f, m) open((p), (f), (m))
#define runtime_read(d, v, n) read((d), (v), (n))
#define runtime_write(d, v, n) write((d), (v), (n))
#define runtime_close(d) close(d)
void	runtime_ready(G*);
String	runtime_getenv(const char*);
int32	runtime_atoi(const byte*, intgo);
void*	runtime_mstart(void*);
G*	runtime_malg(int32, byte**, uintptr*);
void	runtime_mpreinit(M*);
void	runtime_minit(void);
void	runtime_unminit(void);
void	runtime_needm(void)
  __asm__ (GOSYM_PREFIX "runtime.needm");
void	runtime_dropm(void)
  __asm__ (GOSYM_PREFIX "runtime.dropm");
void	runtime_signalstack(byte*, int32);
MCache*	runtime_allocmcache(void);
void	runtime_freemcache(MCache*);
void	runtime_mallocinit(void);
void	runtime_mprofinit(void);
#define runtime_malloc(s) __go_alloc(s)
#define runtime_free(p) __go_free(p)
#define runtime_getcallersp(p) __builtin_frame_address(0)
int32	runtime_mcount(void)
  __asm__ (GOSYM_PREFIX "runtime.mcount");
int32	runtime_gcount(void);
void	runtime_mcall(void(*)(G*));
uint32	runtime_fastrand1(void) __asm__ (GOSYM_PREFIX "runtime.fastrand1");
int32	runtime_timediv(int64, int32, int32*)
  __asm__ (GOSYM_PREFIX "runtime.timediv");
int32	runtime_round2(int32 x); // round x up to a power of 2.

// atomic operations
#define runtime_cas(pval, old, new) __sync_bool_compare_and_swap (pval, old, new)
#define runtime_cas64(pval, old, new) __sync_bool_compare_and_swap (pval, old, new)
#define runtime_casp(pval, old, new) __sync_bool_compare_and_swap (pval, old, new)
// Don't confuse with XADD x86 instruction,
// this one is actually 'addx', that is, add-and-fetch.
#define runtime_xadd(p, v) __sync_add_and_fetch (p, v)
#define runtime_xadd64(p, v) __sync_add_and_fetch (p, v)
#define runtime_xchg(p, v) __atomic_exchange_n (p, v, __ATOMIC_SEQ_CST)
#define runtime_xchg64(p, v) __atomic_exchange_n (p, v, __ATOMIC_SEQ_CST)
#define runtime_xchgp(p, v) __atomic_exchange_n (p, v, __ATOMIC_SEQ_CST)
#define runtime_atomicload(p) __atomic_load_n (p, __ATOMIC_SEQ_CST)
#define runtime_atomicstore(p, v) __atomic_store_n (p, v, __ATOMIC_SEQ_CST)
#define runtime_atomicstore64(p, v) __atomic_store_n (p, v, __ATOMIC_SEQ_CST)
#define runtime_atomicload64(p) __atomic_load_n (p, __ATOMIC_SEQ_CST)
#define runtime_atomicloadp(p) __atomic_load_n (p, __ATOMIC_SEQ_CST)
#define runtime_atomicstorep(p, v) __atomic_store_n (p, v, __ATOMIC_SEQ_CST)

void runtime_setg(G*)
  __asm__ (GOSYM_PREFIX "runtime.setg");
void runtime_newextram(void);
#define runtime_exit(s) exit(s)
#define runtime_breakpoint() __builtin_trap()
void	runtime_gosched(void);
void	runtime_gosched0(G*);
void	runtime_schedtrace(bool);
void	runtime_park(bool(*)(G*, void*), void*, const char*);
void	runtime_parkunlock(Lock*, const char*);
void	runtime_tsleep(int64, const char*);
M*	runtime_newm(void);
void	runtime_goexit(void);
void	runtime_entersyscall(int32)
  __asm__ (GOSYM_PREFIX "runtime.entersyscall");
void	runtime_entersyscallblock(int32)
  __asm__ (GOSYM_PREFIX "runtime.entersyscallblock");
void	runtime_exitsyscall(int32)
  __asm__ (GOSYM_PREFIX "runtime.exitsyscall");
G*	__go_go(void (*pfn)(void*), void*);
int32	runtime_callers(int32, Location*, int32, bool keep_callers);
int64	runtime_nanotime(void)	// monotonic time
  __asm__(GOSYM_PREFIX "runtime.nanotime");
int64	runtime_unixnanotime(void) // real time, can skip
  __asm__ (GOSYM_PREFIX "runtime.unixnanotime");
void	runtime_dopanic(int32) __attribute__ ((noreturn));
void	runtime_startpanic(void)
  __asm__ (GOSYM_PREFIX "runtime.startpanic");
void	runtime_freezetheworld(void);
void	runtime_unwindstack(G*, byte*);
void	runtime_sigprof()
  __asm__ (GOSYM_PREFIX "runtime.sigprof");
void	runtime_resetcpuprofiler(int32)
  __asm__ (GOSYM_PREFIX "runtime.resetcpuprofiler");
void	runtime_setcpuprofilerate_m(int32)
     __asm__ (GOSYM_PREFIX "runtime.setcpuprofilerate_m");
void	runtime_cpuprofAdd(Slice)
     __asm__ (GOSYM_PREFIX "runtime.cpuprofAdd");
void	runtime_usleep(uint32)
     __asm__ (GOSYM_PREFIX "runtime.usleep");
int64	runtime_cputicks(void)
     __asm__ (GOSYM_PREFIX "runtime.cputicks");
int64	runtime_tickspersecond(void)
     __asm__ (GOSYM_PREFIX "runtime.tickspersecond");
void	runtime_blockevent(int64, int32);
extern int64 runtime_blockprofilerate;
G*	runtime_netpoll(bool)
  __asm__ (GOSYM_PREFIX "runtime.netpoll");
void	runtime_crash(void)
  __asm__ (GOSYM_PREFIX "runtime.crash");
void	runtime_parsedebugvars(void)
  __asm__(GOSYM_PREFIX "runtime.parsedebugvars");
void	_rt0_go(void);
intgo	runtime_setmaxthreads(intgo)
  __asm__ (GOSYM_PREFIX "runtime.setmaxthreads");
G*	runtime_timejump(void);
void	runtime_iterate_finq(void (*callback)(FuncVal*, void*, const FuncType*, const PtrType*));

void	runtime_stopTheWorldWithSema(void)
  __asm__(GOSYM_PREFIX "runtime.stopTheWorldWithSema");
void	runtime_startTheWorldWithSema(void)
  __asm__(GOSYM_PREFIX "runtime.startTheWorldWithSema");
void	runtime_acquireWorldsema(void)
  __asm__(GOSYM_PREFIX "runtime.acquireWorldsema");
void	runtime_releaseWorldsema(void)
  __asm__(GOSYM_PREFIX "runtime.releaseWorldsema");

/*
 * mutual exclusion locks.  in the uncontended case,
 * as fast as spin locks (just a few user-level instructions),
 * but on the contention path they sleep in the kernel.
 * a zeroed Lock is unlocked (no need to initialize each lock).
 */
void	runtime_lock(Lock*)
  __asm__(GOSYM_PREFIX "runtime.lock");
void	runtime_unlock(Lock*)
  __asm__(GOSYM_PREFIX "runtime.unlock");

/*
 * sleep and wakeup on one-time events.
 * before any calls to notesleep or notewakeup,
 * must call noteclear to initialize the Note.
 * then, exactly one thread can call notesleep
 * and exactly one thread can call notewakeup (once).
 * once notewakeup has been called, the notesleep
 * will return.  future notesleep will return immediately.
 * subsequent noteclear must be called only after
 * previous notesleep has returned, e.g. it's disallowed
 * to call noteclear straight after notewakeup.
 *
 * notetsleep is like notesleep but wakes up after
 * a given number of nanoseconds even if the event
 * has not yet happened.  if a goroutine uses notetsleep to
 * wake up early, it must wait to call noteclear until it
 * can be sure that no other goroutine is calling
 * notewakeup.
 *
 * notesleep/notetsleep are generally called on g0,
 * notetsleepg is similar to notetsleep but is called on user g.
 */
void	runtime_noteclear(Note*)
  __asm__ (GOSYM_PREFIX "runtime.noteclear");
void	runtime_notesleep(Note*)
  __asm__ (GOSYM_PREFIX "runtime.notesleep");
void	runtime_notewakeup(Note*)
  __asm__ (GOSYM_PREFIX "runtime.notewakeup");
bool	runtime_notetsleep(Note*, int64)  // false - timeout
  __asm__ (GOSYM_PREFIX "runtime.notetsleep");
bool	runtime_notetsleepg(Note*, int64)  // false - timeout
  __asm__ (GOSYM_PREFIX "runtime.notetsleepg");

/*
 * Lock-free stack.
 * Initialize uint64 head to 0, compare with 0 to test for emptiness.
 * The stack does not keep pointers to nodes,
 * so they can be garbage collected if there are no other pointers to nodes.
 */
void	runtime_lfstackpush(uint64 *head, LFNode *node)
  __asm__ (GOSYM_PREFIX "runtime.lfstackpush");
void*	runtime_lfstackpop(uint64 *head)
  __asm__ (GOSYM_PREFIX "runtime.lfstackpop");

/*
 * Parallel for over [0, n).
 * body() is executed for each iteration.
 * nthr - total number of worker threads.
 * if wait=true, threads return from parfor() when all work is done;
 * otherwise, threads can return while other threads are still finishing processing.
 */
ParFor*	runtime_parforalloc(uint32 nthrmax);
void	runtime_parforsetup(ParFor *desc, uint32 nthr, uint32 n, bool wait, const FuncVal *body);
void	runtime_parfordo(ParFor *desc);
void	runtime_parforiters(ParFor*, uintptr, uintptr*, uintptr*);

/*
 * low level C-called
 */
#define runtime_mmap mmap
#define runtime_munmap munmap
#define runtime_madvise madvise
#define runtime_memclr(buf, size) __builtin_memset((buf), 0, (size))
#define runtime_getcallerpc(p) __builtin_return_address(0)

#ifdef __rtems__
void __wrap_rtems_task_variable_add(void **);
#endif

/*
 * runtime go-called
 */
void reflect_call(const struct __go_func_type *, FuncVal *, _Bool, _Bool,
		  void **, void **)
  __asm__ (GOSYM_PREFIX "reflect.call");
#define runtime_panic __go_panic

/*
 * runtime c-called (but written in Go)
 */
void	runtime_printany(Eface)
     __asm__ (GOSYM_PREFIX "runtime.Printany");
void	runtime_newTypeAssertionError(const String*, const String*, const String*, const String*, Eface*)
     __asm__ (GOSYM_PREFIX "runtime.NewTypeAssertionError");
void	runtime_newErrorCString(const char*, Eface*)
     __asm__ (GOSYM_PREFIX "runtime.NewErrorCString");

/*
 * wrapped for go users
 */
void	runtime_semacquire(uint32 volatile *, bool)
     __asm__ (GOSYM_PREFIX "runtime.semacquire");
void	runtime_semrelease(uint32 volatile *)
     __asm__ (GOSYM_PREFIX "runtime.semrelease");
int32	runtime_gomaxprocsfunc(int32 n);
void	runtime_procyield(uint32)
  __asm__(GOSYM_PREFIX "runtime.procyield");
void	runtime_osyield(void)
  __asm__(GOSYM_PREFIX "runtime.osyield");
void	runtime_lockOSThread(void);
void	runtime_unlockOSThread(void);
bool	runtime_lockedOSThread(void);

void	runtime_printcreatedby(G*)
  __asm__(GOSYM_PREFIX "runtime.printcreatedby");

uintptr	runtime_memlimit(void);

#define ISNAN(f) __builtin_isnan(f)

enum
{
	UseSpanType = 1,
};

#define runtime_setitimer setitimer

void	runtime_check(void)
  __asm__ (GOSYM_PREFIX "runtime.check");

// A list of global variables that the garbage collector must scan.
struct root_list {
	struct root_list *next;
	struct root {
		void *decl;
		size_t size;
	} roots[];
};

void	__go_register_gc_roots(struct root_list*);

// Size of stack space allocated using Go's allocator.
// This will be 0 when using split stacks, as in that case
// the stacks are allocated by the splitstack library.
extern uintptr runtime_stacks_sys;

struct backtrace_state;
extern struct backtrace_state *__go_get_backtrace_state(void);
extern _Bool __go_file_line(uintptr, int, String*, String*, intgo *);
extern void runtime_main(void*);
extern uint32 runtime_in_callers;

int32 getproccount(void);

#define PREFETCH(p) __builtin_prefetch(p)

bool	runtime_gcwaiting(void);
void	runtime_badsignal(int);
Defer*	runtime_newdefer(void);
void	runtime_freedefer(Defer*);

struct time_now_ret
{
  int64_t sec;
  int32_t nsec;
};

struct time_now_ret now() __asm__ (GOSYM_PREFIX "time.now")
  __attribute__ ((no_split_stack));

extern void _cgo_wait_runtime_init_done (void);
extern void _cgo_notify_runtime_init_done (void);
extern _Bool runtime_iscgo;
extern _Bool runtime_cgoHasExtraM;
extern Hchan *runtime_main_init_done;
extern uintptr __go_end __attribute__ ((weak));
extern void *getitab(const struct __go_type_descriptor *,
		     const struct __go_type_descriptor *,
		     _Bool)
  __asm__ (GOSYM_PREFIX "runtime.getitab");
