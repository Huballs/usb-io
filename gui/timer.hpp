//
// Created by hugo on 18.07.25.
//

#ifndef TIMER_HPP
#define TIMER_HPP

#include <unordered_map>
#include <string>
#include <mutex>
#include <chrono>
#include <ranges>

namespace gui {

class Timer {
public:

    struct state_t {

        friend Timer;

        bool running = false;
        bool finished = false;

        void start(std::chrono::milliseconds time_out) {
            m_start = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now().time_since_epoch());
            m_time_out = time_out;
            running = true;
        }

        bool check() {

            if (!running)
                return false;

            auto current = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now().time_since_epoch());
            auto diff = std::chrono::duration_cast<std::chrono::milliseconds>(current - m_start);

            finished = (diff >= m_time_out);
            running = !finished;

            return finished;
        }

        void reset() {
            running = false;
            finished = false;
        }

    private:
        std::chrono::milliseconds m_start{0};
        std::chrono::milliseconds m_time_out{0};
    };

    state_t& get(const std::string& name) {
        auto it = m_timers.find(name);

        if (it == m_timers.end()) {
            std::lock_guard l(m_mutex);
            m_timers.insert({name, state_t{}});
        }

        return m_timers[name];
    }

    bool check_all() {
        uint32_t ret = 0U;
        for (auto& state: m_timers | std::views::values) {
            ret |= state.check() ? 1U : 0U;
        }

        return ret != 0U;
    }

private:

    std::mutex m_mutex;
    std::unordered_map<std::string, state_t> m_timers;
};

} // gui

#endif //TIMER_HPP
