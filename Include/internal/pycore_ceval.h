#ifndef Py_INTERNAL_CEVAL_H
#define Py_INTERNAL_CEVAL_H
#ifdef __cplusplus
extern "C" {
#endif

#if !defined(Py_BUILD_CORE) && !defined(Py_BUILD_CORE_BUILTIN)
#  error "this header requires Py_BUILD_CORE or Py_BUILD_CORE_BUILTIN define"
#endif

#include "pycore_atomic.h"
#include "pythread.h"

struct _is;  // See PyInterpreterState in cpython/pystate.h.

PyAPI_FUNC(int) _Py_AddPendingCall(struct _is*, unsigned long, int (*)(void *), void *);
PyAPI_FUNC(int) _Py_MakePendingCalls(struct _is*);
PyAPI_FUNC(void) _Py_FinishPendingCalls(struct _is*);

struct _pending_calls {
    int finishing;
    PyThread_type_lock lock;
    /* Request for running pending calls. */
    _Py_atomic_int calls_to_do;
    /* Request for looking at the `async_exc` field of the current
       thread state.
       Guarded by the GIL. */
    int async_exc;
#define NPENDINGCALLS 32
    struct {
        unsigned long thread_id;
        int (*func)(void *);
        void *arg;
    } calls[NPENDINGCALLS];
    int first;
    int last;
};

struct _ceval_interpreter_state {
    /* This single variable consolidates all requests to break out of
       the fast path in the eval loop. */
    _Py_atomic_int eval_breaker;
    struct _pending_calls pending;
};

#include "pycore_gil.h"

struct _ceval_runtime_state {
    int recursion_limit;
    /* Records whether tracing is on for any thread.  Counts the number
       of threads for which tstate->c_tracefunc is non-NULL, so if the
       value is 0, we know we don't have to check this thread's
       c_tracefunc.  This speeds up the if statement in
       PyEval_EvalFrameEx() after fast_next_opcode. */
    int tracing_possible;
    /* Request for dropping the GIL */
    _Py_atomic_int gil_drop_request;
    /* Request for checking signals. */
    _Py_atomic_int signals_pending;
    struct _gil_runtime_state gil;
};

PyAPI_FUNC(void) _PyEval_Initialize(struct _ceval_runtime_state *);

#ifdef __cplusplus
}
#endif
#endif /* !Py_INTERNAL_CEVAL_H */
