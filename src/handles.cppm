// SPDX-FileCopyrightText: 2026 AlgorIT Software Consultancy <https://github.com/algoritnl>
// SPDX-License-Identifier: MIT

export module sqlixx:handles;
import std;

namespace sqlixx {

template <typename Handle>
    requires std::is_pointer_v<Handle>
class shallow_handle {
public:
    constexpr shallow_handle() noexcept = default;
    explicit constexpr shallow_handle(Handle handle) noexcept : handle_(handle) {}

    constexpr shallow_handle(const shallow_handle&) noexcept = default;
    constexpr auto operator=(const shallow_handle&) noexcept -> shallow_handle& = default;
    constexpr shallow_handle(shallow_handle&&) noexcept = default;
    constexpr auto operator=(shallow_handle&&) noexcept -> shallow_handle& = default;

    ~shallow_handle() = default;
    constexpr auto release() noexcept -> Handle { return std::exchange(handle_, Handle{}); }

    [[nodiscard]] constexpr auto c_handle() const noexcept -> Handle { return handle_; }
    [[nodiscard]] constexpr explicit operator bool() const noexcept { return handle_ != Handle{}; }

private:
    Handle handle_{};
};

template <typename Handle>
struct handle_deleter;

template <typename Handle>
    requires std::is_pointer_v<Handle>
class owning_handle {
public:
    constexpr owning_handle() noexcept = default;
    explicit constexpr owning_handle(Handle handle) noexcept : handle_(handle) {}

    constexpr owning_handle(const owning_handle&) = delete;
    constexpr auto operator=(const owning_handle&) -> owning_handle& = delete;

    constexpr owning_handle(owning_handle&& other) noexcept : handle_(other.release()) {}
    constexpr auto operator=(owning_handle&& other) noexcept -> owning_handle& {
        if (this != &other) {
            owning_handle deallocator{release()};
            handle_ = other.release();
        }
        return *this;
    }

    ~owning_handle() {
        if (Handle handle = release(); handle != Handle{}) {
            handle_deleter<Handle>{}(handle);
        }
    }
    constexpr auto release() noexcept -> Handle { return std::exchange(handle_, Handle{}); }

    [[nodiscard]] constexpr auto c_handle() const noexcept -> Handle { return handle_; }
    [[nodiscard]] constexpr explicit operator bool() const noexcept { return handle_ != Handle{}; }
    [[nodiscard]] constexpr operator shallow_handle<Handle>() const noexcept { return shallow_handle<Handle>{handle_}; }

private:
    Handle handle_{};
};

} // namespace sqlixx
