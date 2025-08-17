//
// Created by hugo on 09.06.25.
//

#ifndef BUFFER_H
#define BUFFER_H

#include <vector>
#include <atomic>
#include <span>

namespace buffers {

    template<typename T>
    class Buffer {
    public:
        Buffer() = default;
        virtual ~Buffer(){};

        virtual void write(T* data, size_t size) = 0;
        virtual void get_data(std::span<T> buffer) = 0;
        [[nodiscard]] virtual bool data_ready() const = 0;
        [[nodiscard]] virtual size_t buffer_size() const = 0;
        [[nodiscard]] virtual size_t total_buffer_size() const = 0;
    };

    template <typename T>
    class SimpleBuffer : public Buffer<T> {
    public:

        SimpleBuffer(size_t size) : m_size(size) {
            m_data.resize(size);
        }

        ~SimpleBuffer() = default;

        void write(T* data, size_t size) override {
            assert(size <= this->buffer_size());

            std::lock_guard<std::mutex> lk(m_mutex_data);

            std::copy(data, data + size, m_data.begin());

            m_data_ready.store(true);
            m_data_ready.notify_one();
        }
        void get_data(std::span<T> buffer) override {
            std::lock_guard<std::mutex> lk(m_mutex_data);

            std::copy(m_data.begin(), m_data.end(), buffer.begin());

            m_data_ready.store(false);
        }

        void wait_for_data(std::span<T> buffer) {
            m_data_ready.wait(false);

            return get_data(buffer);
        }

        [[nodiscard]] bool data_ready() const override {
            return m_data_ready;
        }
        [[nodiscard]] size_t buffer_size() const override {
            return m_size;
        }
        [[nodiscard]] size_t total_buffer_size() const override {
            return m_size;
        }

    private:
        std::vector<T> m_data;
        size_t m_size;

        std::mutex m_mutex_data;

        std::atomic<bool> m_data_ready{false};
    };
}

#endif //BUFFER_H
