#include <unistd.h>
// --------------------------------------------------------------------
#ifndef INCLUDED_INTR_SHARED_PTR_H_
#define INCLUDED_INTR_SHARED_PTR_H_
#include <cstdlib>
#include <time.h>
#include <atomic>

// Intrusive reference-counted pointer:
// T - type the pointer points to
// U - T with reference counter
//     U must have the following member functions:
//     void AddRef() - atomically increment reference count by 1
//     bool DelRef() - atomically decrement reference count by 1, return true iff the counter dropped to 0
//     operator T&(), operator const T&() const - conversions to T (implicit if U is derived from T)
template <typename T, typename U = T> class intr_shared_ptr
{
  struct get_ptr {
    std::atomic<U*>& aptr;
    U* p;
    get_ptr(std::atomic<U*>& ptr) : aptr(ptr), p() {
      static const timespec ns = { 0, 1 };
      for (int i = 0; aptr.load(std::memory_order_relaxed) == (U*)(locked) || (p = aptr.exchange((U*)(locked), std::memory_order_acquire)) == (U*)(locked); ++i) {
        if (i == 8) {
          i = 0;
          nanosleep(&ns, NULL);
        }
      }
    }
    ~get_ptr() {
      aptr.store(p, std::memory_order_release);
    }
    static const uintptr_t locked = uintptr_t(-1);
  };

  public:
  // Non-threadsafe shared pointer, used to hold non-zero reference counter to
  // safely dereference intr_shared_ptr.
  class shared_ptr {
    public:
    shared_ptr() : p_(nullptr) {
    }
    T& operator*() const { return *p_; }
    T* operator->() const { return p_; }
    shared_ptr(const shared_ptr& x) : p_(x.p_) {
      if (p_) p_->AddRef();
    }
    ~shared_ptr() {
      if (p_ && p_->DelRef()) {
        delete p_;
      }
    }
    explicit operator bool() const { return p_ != NULL; }
    shared_ptr& operator=(const shared_ptr& x) {
      if (this == &x) return *this;
      if (p_ && p_->DelRef()) {
        delete p_;
      }
      p_ = x.p_;
      if (p_) p_->AddRef();
      return *this;
    }
    bool operator==(const shared_ptr& rhs) const {
        return p_ == rhs.p_;
    }
    bool operator!=(const shared_ptr& rhs) const {
        return p_ != rhs.p_;
    }

    private:
    friend class intr_shared_ptr;
    explicit shared_ptr(U* p) : p_(p) {
      if (p_) p_->AddRef();
    }
    void reset(U* p) {
      if (p_ == p) return;
      if (p_ && p_->DelRef()) {
        delete p_;
      }
      p_ = p;
      if (p_) p_->AddRef();
    }
    U* p_;
  };

  explicit intr_shared_ptr(U* p = NULL) : p_(p) {
    if (p) p->AddRef();
  }
  explicit intr_shared_ptr(const shared_ptr& x) : p_() {
    if (x.p_) x.p_->AddRef();
    p_.store(x.p_, std::memory_order_relaxed);
  }
  explicit intr_shared_ptr(const intr_shared_ptr& x) : p_() {
    get_ptr px(x.p_);
    if (px.p) px.p->AddRef();
    p_.store(px.p, std::memory_order_relaxed);
  }
  ~intr_shared_ptr() {
    get_ptr p(p_);
    if (p.p && p.p->DelRef()) {
      delete p.p;
    }
    p.p = NULL; // Destructor of p will copy this to p_
  }
  intr_shared_ptr& operator=(const intr_shared_ptr& x) {
    if (this == &x) return *this;
    // Beware of a deadlock:
    // get_ptr px(x.p_);
    // get_ptr p(p_);
    // Deadlock will happen if a = b; and b = a; are executed at the same time.
    U* pxp;
    {
      get_ptr px(x.p_);
      pxp = px.p;
      if (px.p) px.p->AddRef();
    }
    get_ptr p(p_);
    if (p.p && p.p->DelRef()) {
      delete p.p;
    }
    p.p = pxp;  // Destructor of p will copy this to p_
    return *this;
  }
  void reset(U* x) {
    get_ptr p(p_);
    if (p.p == x) return;
    if (p.p && p.p->DelRef()) {
      delete p.p;
    }
    p.p = x;  // Destructor of p will copy this to p_
    if (x) x->AddRef();
  }
  void reset(const shared_ptr& x) {
    get_ptr p(p_);
    if (p.p == x.p_) return;
    if (p.p && p.p->DelRef()) {
      delete p.p;
    }
    p.p = x.p_;  // Destructor of p will copy this to p_
    if (x.p_) x.p_->AddRef();
  }
  explicit operator bool() const { return p_.load(std::memory_order_relaxed) != NULL; }

  shared_ptr get() const {
    get_ptr p(p_);
    return shared_ptr(p.p);
  }

  bool compare_exchange_strong(shared_ptr& expected_ptr, const shared_ptr& new_ptr) {
    get_ptr p(p_);
    if (p.p == expected_ptr.p_) {
      if (p.p && p.p->DelRef()) {
        delete p.p;
      }
      p.p = new_ptr.p_;  // Destructor of p will copy this to p_
      if (p.p) p.p->AddRef();
      return true;
    } else {
      expected_ptr.reset(p.p);
      return false;
    }
  }

  bool compare_exchange_strong(shared_ptr& expected_ptr, U* new_ptr) {
    get_ptr p(p_);
    if (p.p == expected_ptr.p_) {
      if (p.p && p.p->DelRef()) {
        delete p.p;
      }
      p.p = new_ptr;  // Destructor of p will copy this to p_
      if (p.p) p.p->AddRef();
      return true;
    } else {
      expected_ptr.reset(p.p);
      return false;
    }
  }

  private:
  mutable std::atomic<U*> p_;
};
#endif // INCLUDED_INTR_SHARED_PTR_H_
// --------------------------------------------------------------------

#include <string.h>
#include <atomic>
#include <memory>

#include "benchmark/benchmark.h"

#define REPEAT2(x) {x} {x}
#define REPEAT4(x) REPEAT2(x) REPEAT2(x)
#define REPEAT8(x) REPEAT4(x) REPEAT4(x)
#define REPEAT16(x) REPEAT8(x) REPEAT8(x)
#define REPEAT32(x) REPEAT16(x) REPEAT16(x)
#define REPEAT64(x) REPEAT32(x) REPEAT32(x)
#define REPEAT(x) REPEAT64(x)

using namespace std;

struct A {
  int i;
  A(int i = 0) : i(i) {}
  A& operator=(const A& rhs) { i = rhs.i; return *this; }
  volatile A& operator=(const A& rhs) volatile { i = rhs.i; return *this; }
};

static unsigned long B_count = 0;
struct B : public A {
  B(int i = 0) : A(i), ref_cnt_(0) {
    ++B_count;
  }
  ~B() { --B_count; }
  B(const B& x) = delete;
  B& operator=(const B& x) = delete;
  atomic<unsigned long> ref_cnt_;
  void AddRef() { ref_cnt_.fetch_add(1, std::memory_order_acq_rel); }
  bool DelRef() { return ref_cnt_.fetch_sub(1, std::memory_order_acq_rel) == 1; }
};

intr_shared_ptr<A, B> p(new B(42));

void BM_ptr_deref(benchmark::State& state) {
  volatile A x;
  for (auto _ : state) {
    benchmark::DoNotOptimize(x = *p.get());
  }
  state.SetItemsProcessed(state.iterations());
}

void BM_ptr_copy(benchmark::State& state) {
  for (auto _ : state) {
    volatile intr_shared_ptr<A, B> q(p);
  }
  state.SetItemsProcessed(state.iterations());
}

intr_shared_ptr<A, B> q(new B(7));

void BM_ptr_assign(benchmark::State& state) {
  for (auto _ : state) {
    benchmark::DoNotOptimize(q = p);
  }
  state.SetItemsProcessed(state.iterations());
}

void BM_ptr_xassign(benchmark::State& state) {
  if (state.thread_index == 0) p = intr_shared_ptr<A, B>(new B(42)), q = intr_shared_ptr<A, B>(new B(7));
  if (state.thread_index & 1) {
    for (auto _ : state) {
      benchmark::DoNotOptimize(q = p);
    }
  } else {
    for (auto _ : state) {
      benchmark::DoNotOptimize(p = q);
    }
  }
  state.SetItemsProcessed(state.iterations());
}

static const long numcpu = sysconf(_SC_NPROCESSORS_CONF);

#define ARGS \
  ->ThreadRange(1, numcpu) \
  ->UseRealTime()

BENCHMARK(BM_ptr_deref) ARGS;
BENCHMARK(BM_ptr_copy) ARGS;
BENCHMARK(BM_ptr_assign) ARGS;
BENCHMARK(BM_ptr_xassign) ARGS;

BENCHMARK_MAIN();
