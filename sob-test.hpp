//
// Created by hugo on 16.08.25.
//

#ifndef SOB_TEST_HPP
#define SOB_TEST_HPP

#include <so_5/all.hpp>

namespace so {

    using namespace std::literals;

    struct acquired_value {
        std::chrono::steady_clock::time_point acquired_at_;
        int value_;
    };

    class producer final : public so_5::agent_t {
        const so_5::mbox_t board_;
        so_5::timer_id_t timer_;
        int counter_{};

        struct acquisition_time final : public so_5::signal_t {};

        void on_timer(mhood_t<acquisition_time>) {
            // Publish the next value for all consumers.
            so_5::send<acquired_value>(
                  board_, std::chrono::steady_clock::now(), ++counter_);
        }

    public:
        producer(context_t ctx, so_5::mbox_t board)
           :  so_5::agent_t{std::move(ctx)}
        ,  board_{std::move(board)}
        {}

        void so_define_agent() override {
            so_subscribe_self().event(&producer::on_timer);
        }

        void so_evt_start() override {
            // Agent will periodically recive acquisition_time signal
            // without initial delay and with period of 750ms.
            timer_ = so_5::send_periodic<acquisition_time>(*this, 0ms, 750ms);
        }
    };

    class consumer final : public so_5::agent_t {
        const so_5::mbox_t board_;
        const std::string name_;

        void on_value(mhood_t<acquired_value> cmd) {
            std::cout << name_ << ": " << cmd->value_ << std::endl;
        }

    public:
        consumer(context_t ctx, so_5::mbox_t board, std::string name)
           :  so_5::agent_t{std::move(ctx)}
        ,  board_{std::move(board)}
        ,  name_{std::move(name)}
        {}

        void so_define_agent() override {
            so_subscribe(board_).event(&consumer::on_value);
        }
    };

    int main() {
        so_5::launch([](so_5::environment_t & env) {
              auto board = env.create_mbox();
              env.introduce_coop(so_5::disp::thread_pool::make_dispatcher(env, 16).binder(),
                  [board](so_5::coop_t & coop) {
                    coop.make_agent<producer>(board);
                    coop.make_agent<consumer>(board, "first"s);
                    coop.make_agent<consumer>(board, "second"s);
                 });

              std::this_thread::sleep_for(std::chrono::seconds(4));
              env.stop();
           });

        return 0;
    }
}

#endif //SOB_TEST_HPP
