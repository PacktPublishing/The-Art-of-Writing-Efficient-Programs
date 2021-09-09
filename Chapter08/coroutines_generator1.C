#include <coroutine>
#include <iostream>

template <typename T> struct generator {
  struct promise_type {
    T value_ = -1;

    generator get_return_object() {
      using handle_type=std::coroutine_handle<promise_type>;
      return generator{handle_type::from_promise(*this)};
    }
    std::suspend_never initial_suspend() { return {}; }
    std::suspend_never final_suspend() noexcept { return {}; }
    void unhandled_exception() {}
    std::suspend_always yield_value(T value) {
        std::cout << "suspend " << value << " was " << value_ << std::endl;
      value_ = value;
      return {};
    }
  };

  std::coroutine_handle<promise_type> h_;
};

generator<int> coro()
{
  for (int i = 0;; ++i) {
    co_yield i;       // co yield i => co_await promise.yield_value(i)
  }
}

int main()
{
    std::cout << "A" << std::endl;
  auto h = coro().h_;
      std::cout << "B" << std::endl;
  auto &promise = h.promise();
  for (int i = 0; i < 3; ++i) {
    std::cout << "counter: " << h.promise().value_ << std::endl;
    h();
  }
  h.destroy();
}
