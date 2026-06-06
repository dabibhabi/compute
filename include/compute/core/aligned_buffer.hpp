#pragma once
#include <cstddef>
#include <cstdlib>
#include <memory>
#include <span>
#include <stdexcept>

namespace compute::core {

// Heap-allocated buffer with guaranteed alignment — required for AVX-512 (64-byte alignment).
// Owns its memory; moveable, not copyable.
template<typename T, std::size_t Alignment = 64>
class aligned_buffer {
    static_assert(Alignment >= alignof(T) && (Alignment & (Alignment - 1)) == 0,
                  "Alignment must be a power-of-two >= alignof(T)");
public:
    aligned_buffer() = default;

    explicit aligned_buffer(std::size_t n)
        : size_(n)
    {
        if (n == 0) return;
        void* raw = std::aligned_alloc(Alignment, n * sizeof(T));
        if (!raw) throw std::bad_alloc{};
        ptr_ = static_cast<T*>(raw);
    }

    ~aligned_buffer() { std::free(ptr_); }

    aligned_buffer(aligned_buffer&& o) noexcept
        : ptr_(o.ptr_), size_(o.size_) { o.ptr_ = nullptr; o.size_ = 0; }

    aligned_buffer& operator=(aligned_buffer&& o) noexcept {
        if (this != &o) {
            std::free(ptr_);
            ptr_ = o.ptr_; size_ = o.size_;
            o.ptr_ = nullptr; o.size_ = 0;
        }
        return *this;
    }

    aligned_buffer(const aligned_buffer&)            = delete;
    aligned_buffer& operator=(const aligned_buffer&) = delete;

    T*          data()        noexcept { return ptr_; }
    const T*    data()  const noexcept { return ptr_; }
    std::size_t size()  const noexcept { return size_; }
    bool        empty() const noexcept { return size_ == 0; }

    T&       operator[](std::size_t i)       noexcept { return ptr_[i]; }
    const T& operator[](std::size_t i) const noexcept { return ptr_[i]; }

    std::span<T>       as_span()       noexcept { return {ptr_, size_}; }
    std::span<const T> as_span() const noexcept { return {ptr_, size_}; }

private:
    T*          ptr_  = nullptr;
    std::size_t size_ = 0;
};

} // namespace compute::core
