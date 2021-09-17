#include <time.h>
#include <stdlib.h>
#include <atomic>

static constexpr timespec ns = { 0, 1 };
inline void nanosleep(int& i) {
  if (++i == 8) {
    i = 0;
    nanosleep(&ns, NULL);
  }
}

class spinlock {
  public:
  spinlock() = default;
  spinlock(const spinlock&) = delete;
  spinlock& operator=(const spinlock&) = delete;
  void lock() {
    for (int i = 0; flag_.load(std::memory_order_relaxed) || flag_.exchange(1, std::memory_order_acquire); ) {
      nanosleep(i);
    }
  }
  void unlock() { flag_.store(0, std::memory_order_release); }
  private:
  std::atomic<unsigned int> flag_ { 0 };
};

class rw_spinlock {
  public:
  rw_spinlock() = default;
  rw_spinlock(const rw_spinlock&) = delete;
  rw_spinlock& operator=(const rw_spinlock&) = delete;
  void lock() {
    while (true) {
      if (flag_.fetch_sub(idle, std::memory_order_acquire) == idle) return; // flag_ == 0
      flag_.fetch_add(idle, std::memory_order_relaxed);    // Undo the lock
      for (int i = 0; flag_.load(std::memory_order_relaxed) != idle; ) {
        nanosleep(i);
      }
    }
  }
  void unlock() {
    flag_.fetch_add(idle, std::memory_order_release);
  }
  void lock_shared() {
    while (true) {
      if (flag_.fetch_sub(1, std::memory_order_acquire) > 0) return; // flag_ >= 0
      flag_.fetch_add(1, std::memory_order_relaxed);    // Undo the lock
      for (int i = 0; flag_.load(std::memory_order_relaxed) <= 0; ) {
        nanosleep(i);
      }
    }
  }
  void unlock_shared() {
    flag_.fetch_add(1, std::memory_order_release);
  }
  private:
  // Bit 63 - exclusive lock flag
  // Bits 0-62 - read lock count
  // 0x0000000000000000 - locked for writing
  // 0x1000000000000000 - unlocked
  // 0x0FFFFFFFFFFFFFFF - locked by 1 reader
  static constexpr long idle = 0x1000000000000000L;
  std::atomic<long> flag_ { idle };
};
