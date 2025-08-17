//
// Created by hugo on 08.06.25.
//

#ifndef BUFFERCHUNK_H
#define BUFFERCHUNK_H

#include <ranges>
#include <vector>
#include <span>
#include <algorithm>
#include <cassert>
#include <functional>
#include <optional>
#include "../buffer.h"

namespace buffers {

    template<typename T>
    class ChunkBuffer : public Buffer<T> {
    public:
        ChunkBuffer(size_t chunk_size, size_t chunk_count)
            : m_chunk_size(chunk_size), m_chunk_count(chunk_count), m_total_buffer_size(m_chunk_size * m_chunk_count)
        {

            m_buffer_write.resize(m_total_buffer_size);
            m_buffer_read.resize(m_total_buffer_size);
        }

        ~ChunkBuffer() = default;

        void write(T* data, size_t size) {

            assert((data != nullptr) && (size < m_chunk_size));

            if (m_process_f.has_value()) {
                auto f = m_process_f.value();
                if (!f(&data, size)) {
                    return;
                }
            }

            auto write_chunks = std::views::all(m_buffer_write)
                                | std::views::chunk(m_chunk_size);

            std::copy(data, data + size,
                write_chunks[m_chunk_index].begin());

            m_chunk_index++;

            if (m_chunk_index == m_chunk_count) {
                m_chunk_index = 0U;
                m_read_buffer_ready = true;
                this->write_to_read_buffer();
            }
        }

        void get_data(std::span<T> buffer) {

            std::lock_guard<std::mutex> lock(m_mutex_read);

            std::copy(m_buffer_read.begin(), m_buffer_read.end(), buffer.begin() );
            m_read_buffer_ready = false;
        }

        bool data_ready() const {
            return m_read_buffer_ready;
        }

        size_t total_buffer_size() const {
            return m_total_buffer_size;
        }

        size_t buffer_size() const {
            return m_chunk_size;
        }

        // set function that will be called on each chunk
        void set_process_function(std::optional<std::function<bool(T**, size_t)>> f) {
            m_process_f = f;
        }

    private:

        void write_to_read_buffer() {
            std::lock_guard<std::mutex> lock(m_mutex_read);

            std::copy(m_buffer_write.begin(), m_buffer_write.end(), m_buffer_read.begin());
        }

        std::vector<T> m_buffer_write;
        std::vector<T> m_buffer_read;

        const size_t m_chunk_size;
        const size_t m_chunk_count;
        const size_t m_total_buffer_size;

        size_t m_chunk_index = 0U;

        bool m_read_buffer_ready = false;

        std::optional<std::function<bool(T**, size_t)>> m_process_f = std::nullopt;

        std::mutex m_mutex_read;
    };

} // buffer

#endif //BUFFERCHUNK_H
