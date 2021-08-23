C#include <coroutine>
#include <iostream>
#include <thread>

template <typename T>
struct generator {
  struct Promise;

// compiler looks for promise_type
  using promise_type=Promise;
  std::coroutine_handle<Promise> h_;

  generator(std::coroutine_handle<Promise> h): h_(h) {}

  ~generator() {
    if (h_) h_.destroy();
  }

// get current value of coroutine
  int value() {
    return h_.promise().val;
  }

// advance coroutine past suspension
  bool next() {
    h_.resume();
    return !h_.done();
  }

  struct Promise {
// current value of suspended coroutine
    T val;

// called by compiler first thing to get coroutine result
    generator get_return_object() {
      return generator{std::coroutine_handle<Promise>::from_promise(*this)};
    }

// called by compiler first time co_yield occurs
    std::suspend_always initial_suspend() {
      return {};
    }

// required for co_yield
    std::suspend_always yield_value(T x) {
      val=x;
      return {};
    }

// called by compiler for coroutine without return
    std::suspend_never return_void() {
      return {};
    }

void unhandled_exception() {}

// called by compiler last thing to await final result
// coroutine cannot be resumed after this is called
    std::suspend_always final_suspend() noexcept {
      return {};
    }
  };

};

generator<int> coro(int n) {

  for(int i = 0; i < n; ++i) {
    co_yield i;
  }

}

int main ()
{
  int n=10;

  generator gen = coro(n);

  for(int i=0; i < n; ++i) {
    gen.next();
    printf("%d ", gen.value());
  }

  return 0;
}
