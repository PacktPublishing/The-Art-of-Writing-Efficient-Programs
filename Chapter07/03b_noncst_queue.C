#include <optional>
#include <queue>
#include <mutex>
#include <shared_mutex>
#include <unistd.h>
#include <string.h>

#include "spinlock.h"

#include "benchmark/benchmark.h"

template <typename T> void reset(T& q) { T().swap(q); }

struct LS {
    long x[1024];
    LS(char i = 0) { for (size_t k = 0; k < 1024; ++k) x[k] = i; }
};

// Circular queue of a fixed size.
// This class is not thread-safe, it is supposed to be manipulated by one
// thread at a time.
template <typename T> class subqueue {
    public:
    explicit subqueue(size_t capacity) : capacity_(capacity), begin_(0), size_(0) {}
    size_t capacity() const { return capacity_; }
    size_t size() const { return size_; }
    bool push(const T& x) {
        if (size_ == capacity_) return false;
        size_t end = begin_ + size_++;
        if (end >= capacity_) end -= capacity_;
        data_[end] = x;
        return true;
    }
    bool pop(std::optional<T>& x) {
        if (size_ == 0) return false;
        --size_;
        size_t pos = begin_;
        if (++begin_ == capacity_) begin_ -= capacity_;
        x.emplace(std::move(data_[pos]));
        data_[pos].~T();
        return true;
    }
    static size_t memsize(size_t capacity) {
        return sizeof(subqueue) + capacity*sizeof(T);
    }
    static subqueue* construct(size_t capacity) {
        return new(::malloc(subqueue::memsize(capacity))) subqueue(capacity);
    }
    static void destroy(subqueue* queue) {
        queue->~subqueue();
        ::free(queue);
    }
    void reset() { 
        begin_ = size_ = 0;
    }

    private:
    const size_t capacity_;
    size_t begin_;
    size_t size_;
    T data_[1]; // Actually [capacity_]
};

// Collection of several subqueues, for optimizing of concurrent access.
// Each queue pointer is on a separate cache line.
template <typename Q> struct subqueue_ptr {
  subqueue_ptr() : queue() {}
  std::atomic<Q*> queue;
  char padding[64 - sizeof(queue)]; // Padding to cache line
};

template <typename T> class concurrent_queue {
  typedef subqueue<T> subqueue_t;
  typedef subqueue_ptr<subqueue_t> subqueue_ptr_t;
  public:
    explicit concurrent_queue(size_t capacity = 1UL << 15) {
      for (int i = 0; i < QUEUE_COUNT; ++i) {
        queues_[i].queue.store(subqueue_t::construct(capacity), std::memory_order_relaxed);
      }
    }
    ~concurrent_queue() {
      for (int i = 0; i < QUEUE_COUNT; ++i) {
        subqueue_t* queue = queues_[i].queue.exchange(nullptr, std::memory_order_relaxed);
        subqueue_t::destroy(queue);
      }
    }

    // How many entries are in the queue?
    size_t size() const { return count_.load(std::memory_order_acquire); }

    // Is the queue empty?
    bool empty() const { return size() == 0; }

    // Get an element from the queue.
    // This method blocks until either the queue is empty (size() == 0) or an
    // element is returned from one of the subqueues.
    std::optional<T> pop() {
      std::optional<T> res;
      if (count_.load(std::memory_order_acquire) == 0) {
        return res;
      }
      // Take ownership of a subqueue. The subqueue pointer is reset to NULL
      // while the calling thread owns the subqueue. When done, relinquish
      // the ownership by restoring the pointer.  The subqueue we got may be
      // empty, but this does not mean that we have no entries: we must check
      // other queues. We can exit the loop when we got an element or the element
      // count shows that we have no entries.
      // Note that decrementing the count is not atomic with dequeueing the
      // entries, so we might spin on empty queues for a little while until the
      // count catches up.
      subqueue_t* queue = NULL;
      bool success = false;
      for (size_t i = 0; ;)
      {
        i = dequeue_slot_.fetch_add(1, std::memory_order_relaxed) & (QUEUE_COUNT - 1);
        queue = queues_[i].queue.exchange(nullptr, std::memory_order_acquire);  // Take ownership of the subqueue
        if (queue) {
          success = queue->pop(res);                                            // Dequeue element while we own the queue
          queues_[i].queue.store(queue, std::memory_order_release);             // Relinquish ownership
          if (success) break;                                                   // We have an element
          if (count_.load(std::memory_order_acquire) == 0) goto EMPTY;          // No element, and no more left
        } else {                                                                // queue is NULL, nothing to relinquish
          if (count_.load(std::memory_order_acquire) == 0) goto EMPTY;          // We failed to get a queue but there are no more entries left
        }
        if (success) break;
        if (count_.load(std::memory_order_acquire) == 0) goto EMPTY;
        static const struct timespec ns = { 0, 1 };
        nanosleep(&ns, NULL);
      };
      // If we have an element, decrement the queued element count.
      count_.fetch_add(-1);
EMPTY:
      return success;
    }

    // Add an element to the queue.
    // This method blocks until either the queue is full or an element is added to
    // one of the subqueues. Note that the "queue is full" condition cannot be
    // checked atomically for all subqueues, so it's approximate, we try
    // several subqueues, if they are all full we give up.
    bool push(const T& v) {
      // Preemptively increment the element count: get() will spin on subqueues
      // as long as it thinks there is an element to dequeue, but it will exit as
      // soon as the count is zero, so we want to avoid the situation when we
      // added an element to a subqueue, have not incremented the count yet, but
      // get() exited with no element. If this were to happen, the pool could be
      // deleted with entries still in the queue.
      count_.fetch_add(1);
      // Take ownership of a subqueue. The subqueue pointer is reset to NULL
      // while the calling thread owns the subqueue. When done, relinquish
      // the ownership by restoring the pointer.  The subqueue we got may be
      // full, in which case we try another subqueue, but don't loop forever
      // if all subqueues keep coming up full.
      subqueue_t* queue = NULL;
      bool success = false;
      int full_count = 0;                                             // How many subqueues we tried and found full
      //for (size_t enqueue_slot = 0, i = 0; ;)
      for (size_t i = 0; ;)
      {
        //i = ++enqueue_slot & (QUEUE_COUNT - 1);
        i = enqueue_slot_.fetch_add(1, std::memory_order_relaxed) & (QUEUE_COUNT - 1);
        queue = queues_[i].queue.exchange(nullptr, std::memory_order_acquire);    // Take ownership of the subqueue
        if (queue) {
          success = queue->push(v);                                               // Enqueue element while we own the queue
          queues_[i].queue.store(queue, std::memory_order_release);               // Relinquish ownership
          if (success) return success;                                            // We added the element
          if (++full_count == QUEUE_COUNT) break;                                 // We tried hard enough, probably queue is full
        }
        static const struct timespec ns = { 0, 1 };
        nanosleep(&ns, NULL);
      };
      // If we added the element, the count is already incremented. Otherwise,
      // we must decrement the count now.
      count_.fetch_add(-1);
      return success;
    }

  void reset() { 
    count_ = enqueue_slot_ = dequeue_slot_ = 0;
    for (int i = 0; i < QUEUE_COUNT; ++i) {
      subqueue_t* queue = queues_[i].queue;
      queue->reset();
    }
  }

  private:
    enum { QUEUE_COUNT = 16 };
    subqueue_ptr_t queues_[QUEUE_COUNT];
    std::atomic<int> count_;
    std::atomic<size_t> enqueue_slot_;
    std::atomic<size_t> dequeue_slot_;
};

template <typename T> class concurrent_std_queue {
  using subqueue_t = std::queue<T>;
  struct subqueue_ptr_t {
    subqueue_ptr_t() : queue() {}
    std::atomic<subqueue_t*> queue;
    char padding[64 - sizeof(queue)]; // Padding to cache line
  };

  public:
    explicit concurrent_std_queue() {
      for (int i = 0; i < QUEUE_COUNT; ++i) {
        queues_[i].queue.store(new subqueue_t, std::memory_order_relaxed);
      }
    }
    ~concurrent_std_queue() {
      for (int i = 0; i < QUEUE_COUNT; ++i) {
        subqueue_t* queue = queues_[i].queue.exchange(nullptr, std::memory_order_relaxed);
        delete queue;
      }
    }

    // How many entries are in the queue?
    size_t size() const { return count_.load(std::memory_order_acquire); }

    // Is the queue empty?
    bool empty() const { return size() == 0; }

    // Get an element from the queue.
    // This method blocks until either the queue is empty (size() == 0) or an
    // element is returned from one of the subqueues.
    std::optional<T> pop() {
      std::optional<T> res;
      if (count_.load(std::memory_order_acquire) == 0) {
        return res;
      }
      // Take ownership of a subqueue. The subqueue pointer is reset to NULL
      // while the calling thread owns the subqueue. When done, relinquish
      // the ownership by restoring the pointer.  The subqueue we got may be
      // empty, but this does not mean that we have no entries: we must check
      // other queues. We can exit the loop when we got a element or the element
      // count shows that we have no elements.
      // Note that decrementing the count is not atomic with dequeueing the
      // entries, so we might spin on empty queues for a little while until the
      // count catches up.
      subqueue_t* queue = NULL;
      bool success = false;
      for (size_t i = 0; ;)
      {
        i = dequeue_slot_.fetch_add(1, std::memory_order_relaxed) & (QUEUE_COUNT - 1);
        queue = queues_[i].queue.exchange(nullptr, std::memory_order_acquire);  // Take ownership of the subqueue
        if (queue) {
          if (!queue->empty()) {                                                // Dequeue element while we own the queue
            success = true;
            res.emplace(std::move(queue->front()));
            queue->pop();
            queues_[i].queue.store(queue, std::memory_order_release);           // Relinquish ownership
            break;                                                              // We have an element
          } else {                                                              // Subqueue is empty, try the next one
            queues_[i].queue.store(queue, std::memory_order_release);           // Relinquish ownership
            if (count_.load(std::memory_order_acquire) == 0) goto EMPTY;        // No element, and no more left
          }
        } else {                                                                // queue is NULL, nothing to relinquish
          if (count_.load(std::memory_order_acquire) == 0) goto EMPTY;          // We failed to get a queue but there are no more entries left
        }
        if (success) break;
        if (count_.load(std::memory_order_acquire) == 0) break; 
        static const struct timespec ns = { 0, 1 };
        nanosleep(&ns, NULL);
      };
      // If we have an element, decrement the queued element count.
      count_.fetch_add(-1);
EMPTY:
      return res;
  }

  // Add an element to the queue.
  // This method blocks until either the queue is full or an element is added to
  // one of the subqueues. Note that the "queue is full" condition cannot be
  // checked atomically for all subqueues, so it's approximate, we try
  // several subqueues, if they are all full we give up.
  bool push(const T& v) {
    // Preemptively increment the element count: get() will spin on subqueues
    // as long as it thinks there is an element to dequeue, but it will exit as
    // soon as the count is zero, so we want to avoid the situation when we
    // added an element to a subqueue, have not incremented the count yet, but
    // get() exited with no element. If this were to happen, the pool could be
    // deleted with entries still in the queue.
    count_.fetch_add(1);
    // Take ownership of a subqueue. The subqueue pointer is reset to NULL
    // while the calling thread owns the subqueue. When done, relinquish
    // the ownership by restoring the pointer.  The subqueue we got may be
    // full, in which case we try another subqueue, but don't loop forever
    // if all subqueues keep coming up full.
    subqueue_t* queue = NULL;
    bool success = false;
    for (size_t i = 0; ;)
    {
      i = enqueue_slot_.fetch_add(1, std::memory_order_relaxed) & (QUEUE_COUNT - 1);
      queue = queues_[i].queue.exchange(nullptr, std::memory_order_acquire);    // Take ownership of the subqueue 
      if (queue) {
        success = true;
        queue->push(v);                                                         // Enqueue element while we own the queue 
        queues_[i].queue.store(queue, std::memory_order_release);               // Relinquish ownership
        return success;
      }
      static const struct timespec ns = { 0, 1 };
      nanosleep(&ns, NULL);
    };
    return success;
  }

  void reset() { 
    count_ = enqueue_slot_ = dequeue_slot_ = 0;
    for (int i = 0; i < QUEUE_COUNT; ++i) {
      subqueue_t* queue = queues_[i].queue;
      subqueue_t().swap(*queue);
    }
  }

  private:
  enum { QUEUE_COUNT = 16 };
  subqueue_ptr_t queues_[QUEUE_COUNT];
  std::atomic<int> count_;
  std::atomic<size_t> enqueue_slot_;
  std::atomic<size_t> dequeue_slot_;
};

concurrent_std_queue<int> q;
concurrent_std_queue<LS> lq;

void BM_queue(benchmark::State& state) {
  if (state.thread_index == 0) q.reset();
  const size_t N = state.range(0);
  for (auto _ : state) {
    for (size_t i = 0; i < N; ++i) q.push(i);
    for (size_t i = 0; i < N; ++i) benchmark::DoNotOptimize(q.pop());
  }
  state.SetItemsProcessed(state.iterations()*N);
}

void BM_queue_prod_cons(benchmark::State& state) {
  if ((state.threads & 1) == 1) state.SkipWithError("Need even number of threads!");
  if (state.thread_index == 0) q.reset();
  const bool producer = state.thread_index & 1;
  const size_t N = state.range(0);
  for (auto _ : state) {
    if (producer) {
      for (size_t i = 0; i < N; ++i) q.push(i);
    } else {
      for (size_t i = 0; i < N; ++i) benchmark::DoNotOptimize(q.pop());
    }
  }
  state.SetItemsProcessed(state.iterations()*N);
}

void BM_lqueue_prod_cons(benchmark::State& state) {
  if ((state.threads & 1) == 1) state.SkipWithError("Need even number of threads!");
  if (state.thread_index == 0) lq.reset();
  const bool producer = state.thread_index & 1;
  const size_t N = state.range(0);
  for (auto _ : state) {
    if (producer) {
      for (size_t i = 0; i < N; ++i) lq.push(i);
    } else {
      for (size_t i = 0; i < N; ++i) benchmark::DoNotOptimize(lq.pop());
    }
  }
  state.SetItemsProcessed(state.iterations()*N);
}

static const long numcpu = sysconf(_SC_NPROCESSORS_CONF);

#define ARGS \
  ->Arg(1) \
  ->ThreadRange(1, numcpu) \
  ->UseRealTime()

BENCHMARK(BM_queue) ARGS;
BENCHMARK(BM_queue_prod_cons) ARGS;
BENCHMARK(BM_lqueue_prod_cons) ARGS;

BENCHMARK_MAIN();
