// Copyright 2016 The Go Authors. All rights reserved.
// Use of this source code is governed by a BSD-style
// license that can be found in the LICENSE file.

// +build darwin dragonfly freebsd linux netbsd openbsd solaris

package runtime

import (
	"unsafe"
)

// Functions for gccgo to support signal handling. In the gc runtime
// these are written in OS-specific files and in assembler.

//extern sigaction
func sigaction(signum int32, act *_sigaction, oact *_sigaction) int32

//extern sigprocmask
func sigprocmask(how int32, set *_sigset_t, oldset *_sigset_t) int32

// The argument should be simply *_sigset_t, but that fails on GNU/Linux
// which sometimes uses _sigset_t and sometimes uses ___sigset_t.
//extern sigfillset
func sigfillset(set unsafe.Pointer) int32

//extern sigemptyset
func sigemptyset(set *_sigset_t) int32

//extern sigaddset
func sigaddset(set *_sigset_t, signum int32) int32

//extern sigaltstack
func sigaltstack(ss *_stack_t, oss *_stack_t) int32

//extern raise
func raise(sig int32) int32

//extern getpid
func getpid() _pid_t

//extern kill
func kill(pid _pid_t, sig int32) int32

type sigctxt struct {
	info *_siginfo_t
	ctxt unsafe.Pointer
}

func (c *sigctxt) sigcode() uint64 { return uint64(c.info.si_code) }

//go:nosplit
func sigblock() {
	var set _sigset_t
	sigfillset(unsafe.Pointer(&set))
	sigprocmask(_SIG_SETMASK, &set, nil)
}

//go:nosplit
//go:nowritebarrierrec
func setsig(i int32, fn uintptr, restart bool) {
	var sa _sigaction
	sa.sa_flags = _SA_SIGINFO

	// For gccgo we do not set SA_ONSTACK for a signal that can
	// cause a panic.  Instead, we trust that the split stack has
	// enough room to start the signal handler.  This is because
	// otherwise we have no good way to switch back to the
	// original stack before panicing.
	if sigtable[i].flags&_SigPanic == 0 {
		sa.sa_flags |= _SA_ONSTACK
	}

	if restart {
		sa.sa_flags |= _SA_RESTART
	}
	sigfillset(unsafe.Pointer(&sa.sa_mask))
	setSigactionHandler(&sa, fn)
	sigaction(i, &sa, nil)
}

//go:nosplit
//go:nowritebarrierrec
func setsigstack(i int32) {
	var sa _sigaction
	sigaction(i, nil, &sa)
	handler := getSigactionHandler(&sa)
	if handler == 0 || handler == _SIG_DFL || handler == _SIG_IGN || sa.sa_flags&_SA_ONSTACK != 0 {
		return
	}
	if sigtable[i].flags&_SigPanic != 0 {
		return
	}
	sa.sa_flags |= _SA_ONSTACK
	sigaction(i, &sa, nil)
}

//go:nosplit
//go:nowritebarrierrec
func getsig(i int32) uintptr {
	var sa _sigaction
	if sigaction(i, nil, &sa) < 0 {
		// On GNU/Linux glibc rejects attempts to call
		// sigaction with signal 32 (SIGCANCEL) or 33 (SIGSETXID).
		if GOOS == "linux" && (i == 32 || i == 33) {
			return _SIG_DFL
		}
		throw("sigaction read failure")
	}
	return getSigactionHandler(&sa)
}

//go:nosplit
//go:nowritebarrierrec
func updatesigmask(m sigmask) {
	var mask _sigset_t
	sigemptyset(&mask)
	for i := int32(0); i < _NSIG; i++ {
		if m[(i-1)/32]&(1<<((uint(i)-1)&31)) != 0 {
			sigaddset(&mask, i)
		}
	}
	sigprocmask(_SIG_SETMASK, &mask, nil)
}

func unblocksig(sig int32) {
	var mask _sigset_t
	sigemptyset(&mask)
	sigaddset(&mask, sig)
	sigprocmask(_SIG_UNBLOCK, &mask, nil)
}

//go:nosplit
//go:nowritebarrierrec
func raiseproc(sig int32) {
	kill(getpid(), sig)
}

//go:nosplit
//go:nowritebarrierrec
func sigfwd(fn uintptr, sig uint32, info *_siginfo_t, ctx unsafe.Pointer) {
	f1 := &[1]uintptr{fn}
	f2 := *(*func(uint32, *_siginfo_t, unsafe.Pointer))(unsafe.Pointer(&f1))
	f2(sig, info, ctx)
}
