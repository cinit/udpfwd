//
// Created by sulfate on 2024-01-09.
//

#include "EtherPool.h"

#include <atomic>
#include <array>
#include <memory>

#include "platform/log/Assertions.h"

namespace vether {

class BufferPool {
public:
    enum class Status : uint8_t {
        kEmpty = 0,
        kInUseLoading = 1,
        kAvailable = 2,
        kInUseStoring = 3,
    };

    static constexpr size_t kMaxPoolSize = 1024; // is this enough?

    static_assert((size_t(kMaxPoolSize - 1) & size_t(kMaxPoolSize)) == 0, "kMaxPoolSize must be power of 2");

private:
    std::array<std::unique_ptr<std::array<uint8_t, kMaxFrameSize>>, kMaxPoolSize> buffers;
    std::array<std::atomic<Status>, kMaxPoolSize> status = {};
    std::atomic<size_t> head = 0; // begin, is the next available buffer
    std::atomic<size_t> tail = 0; // end, points to one past the last available buffer
public:
    BufferPool() = default;

    [[nodiscard]] std::unique_ptr<std::array<uint8_t, kMaxFrameSize>> ObtainBuffer() noexcept {
        while (true) {
            size_t h = head.load(std::memory_order_acquire);
            size_t t = tail.load(std::memory_order_relaxed);
            if (h == t) {
                return nullptr;
            }
            if (head.compare_exchange_weak(h, (h + 1) % kMaxPoolSize, std::memory_order_release)) {
                Status expected = Status::kAvailable;
                while (!status[h].compare_exchange_weak(expected, Status::kInUseLoading, std::memory_order_acquire)) {
                    expected = Status::kAvailable;
                }
                std::unique_ptr<std::array<uint8_t, kMaxFrameSize>> buffer = std::move(buffers[h]);
                status[h].store(Status::kEmpty, std::memory_order_release);
                return buffer;
            }
        }
    }

    void ReturnBuffer(std::unique_ptr<std::array<uint8_t, kMaxFrameSize>> buffer) noexcept {
        while (true) {
            size_t t = tail.load(std::memory_order_acquire);
            size_t h = head.load(std::memory_order_relaxed);
            if ((t + 1) % kMaxPoolSize == h) {
                return;
            }
            if (tail.compare_exchange_weak(t, (t + 1) % kMaxPoolSize, std::memory_order_release)) {
                // wait for the buffer to be available
                Status expected = Status::kEmpty;
                while (!status[t].compare_exchange_weak(expected, Status::kInUseStoring, std::memory_order_acquire)) {
                    expected = Status::kEmpty;
                }
                CHECK(buffers[t] == nullptr);
                buffers[t] = std::move(buffer);
                status[t].store(Status::kAvailable, std::memory_order_release);
                return;
            }
        }
    }
};

static constinit BufferPool g_bufferPool;

std::unique_ptr<std::array<uint8_t, kMaxFrameSize>> EtherPool::ObtainBuffer() {
    auto buffer = g_bufferPool.ObtainBuffer();
    if (buffer == nullptr) {
        return std::make_unique<std::array<uint8_t, kMaxFrameSize>>();
    }
    return buffer;
}

void EtherPool::ReturnBuffer(std::unique_ptr<std::array<uint8_t, kMaxFrameSize>> buffer) noexcept {
    if (buffer != nullptr) [[likely]] {
        g_bufferPool.ReturnBuffer(std::move(buffer));
    }
}

}
