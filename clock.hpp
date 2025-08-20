//
// Created by hugo on 20.08.25.
//

#ifndef CLOCK_HPP
#define CLOCK_HPP
#include <chrono>

using namespace std::literals;

template<typename T_COUNT>
class Counter {
public:
    using clock_t = std::chrono::steady_clock;
    using time_point_t = std::chrono::time_point<clock_t>;
    using dur_t = std::chrono::duration<long, std::ratio<1, 1000000000>>;

    explicit Counter(dur_t period) : m_period(period) {
        m_start_time = clock_t::now();
    }

    // void start() {
    //     m_last_time = clock_t::now();
    // }

    T_COUNT cnt() {
        auto diff = clock_t::now() - m_start_time;

        m_cnt = diff / (m_period);

        return m_cnt;
    }

    void reset() {
        m_cnt = 0;
    }

private:
    time_point_t m_start_time;;
    dur_t m_period;

    T_COUNT m_cnt = 0U;
};

#endif //CLOCK_HPP
