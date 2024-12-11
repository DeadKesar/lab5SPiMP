#include <coroutine>
#include <iostream>
#include <string>

template<typename T>
struct Generator {
    struct promise_type;
    using handle_type = std::coroutine_handle<promise_type>;

    struct promise_type {
        T current_value;

        auto get_return_object() {
            return Generator{ handle_type::from_promise(*this) };
        }
        auto initial_suspend() { return std::suspend_always{}; }
        auto yield_value(T value) {
            current_value = value;
            return std::suspend_always{};
        }
        void unhandled_exception() { std::exit(1); }
        auto return_void() { return std::suspend_never{}; }
        auto final_suspend() noexcept { return std::suspend_always{}; }
    };

    handle_type coro;

    Generator(handle_type h) : coro(h) {}
    ~Generator() { if (coro) coro.destroy(); }

    Generator(const Generator&) = delete;
    Generator& operator=(const Generator&) = delete;

    Generator(Generator&& other) noexcept : coro(other.coro) {
        other.coro = nullptr;
    }

    Generator& operator=(Generator&& other) noexcept {
        if (this != &other) {
            if (coro) coro.destroy();
            coro = other.coro;
            other.coro = nullptr;
        }
        return *this;
    }

    T next() {
        coro.resume();
        return coro.promise().current_value;
    }

    explicit operator bool() {
        return !coro.done();
    }
};

std::string addNumbers(const std::string& num1, const std::string& num2) {
    int len1 = num1.length();
    int len2 = num2.length();
    int maxLength = std::max(len1, len2);

    std::string result(maxLength+1, '0');

    int carry = 0;
    int i = len1 - 1;
    int j = len2 - 1;
    int k = maxLength;

    while (i >= 0 || j >= 0 || carry != 0) {
        int digit1 = (i >= 0) ? num1[i] - '0' : 0;
        int digit2 = (j >= 0) ? num2[j] - '0' : 0;

        int sum = digit1 + digit2 + carry;
        carry = sum / 10;
        result[k] = (sum % 10) + '0';

        i--;
        j--;
        k--;
    }
    size_t startPos = result.find_first_not_of('0');
    if (startPos != std::string::npos) {
        return result.substr(startPos);
    }
    return "0";
}

Generator<std::string> fibonacci() {
    std::string a = "0", b = "1";
    while (true) {
        co_yield a;
        auto temp = a;
        a = b;
        b = addNumbers(temp, b);
    }
}

int main() {
    auto gen = fibonacci();
    for (int i = 0; i < 1000; ++i) {
        auto value = gen.next();
        std::cout << value << " " << std::endl;
    }
    std::cout << std::endl;
    return 0;
}
