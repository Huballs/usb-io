#include <iostream>
#include <optional>
#include <args.hxx>
#include <string>
#include <thread>
#include <cassert>
#include <csignal>
// #include "gui/gui.hpp"
// #include "device/device_control.hpp"

#include "device/device_control.hpp"
#include "usb/usb-bulk.hpp"
#include "usb/usb-bulk-agent.hpp"
#include "gui/logger.hpp"
#include "clock.hpp"
#include "gui/guiv2.hpp"

namespace {

    constexpr unsigned char ENDPOINT_ADDRESS    = 0x03U;
    constexpr size_t ENDPOINT_DATA_SIZE         = 64U;
    constexpr int INTERFACE_NUMBER              = 0;
    constexpr int CONFIGURATION                 = 1;

    constexpr size_t BUFFER_MULTI = 4U;
    constexpr size_t BUFFER_TOTAL_SIZE = BUFFER_MULTI * ENDPOINT_DATA_SIZE;

    static std::atomic_bool s_exit{false};

    so_5::wrapped_env_t* s_env;

    void signal_handler ([[maybe_unused]] int sig) {
        std::cout << "Received signal ";
        if (sig == SIGINT) {
            std::cout << "INT";
        } else if (sig == SIGTERM) {
            std::cout << "TERM";
        } else {
            std::cout << "nubmber " << sig;
        }

        std::cout << std::endl;
        s_exit.store(true);
        s_exit.notify_one();

        if (s_env) {
            s_env->stop();
        }
    };

    struct arguments_t {
        int vid;
        int pid;
    };

arguments_t parse_arguments(int argc, char *argv[]) {

    args::ArgumentParser        parser("This is a test program.", "This goes after the options.");
    args::HelpFlag              help(parser, "help", "Display this help menu", {'h', "help"});
    args::ValueFlag<std::string>   vid(parser, "VID", "device VID", {'v'}, args::Options::Required);
    args::ValueFlag<std::string>   pid(parser, "PID", "device PID", {'p'}, args::Options::Required);

    try
    {
        parser.ParseCLI(argc, argv);
    }
    catch (args::Help& e)
    {
        std::cout << parser;
        std::rethrow_exception(std::current_exception());
    }
    catch (args::ParseError& e)
    {
        std::cerr << e.what() << std::endl;
        std::cerr << parser;
        std::rethrow_exception(std::current_exception());
    }
    catch (args::RequiredError& e) {
        std::cout << e.what() << std::endl;
        std::rethrow_exception(std::current_exception());
    }
    catch (args::ValidationError& e)
    {
        std::cerr << e.what() << std::endl;
        std::cerr << parser;
        std::rethrow_exception(std::current_exception());
    }

    arguments_t result {
        .vid = std::stoi(args::get(vid), nullptr, 0)
        , .pid = std::stoi(args::get(pid), nullptr, 0)
    };

    return result;
}

}

class my_run_time_stats_listener : public so_5::agent_t
{
public :
    my_run_time_stats_listener( context_t ctx )
        :    so_5::agent_t( ctx )
{}

    void so_define_agent() override
{
    so_default_state().event(
        so_environment().stats_controller().mbox(),
        &my_run_time_stats_listener::evt_quantity );

    so_default_state().event(
    so_environment().stats_controller().mbox(),
    &my_run_time_stats_listener::evt_activity );
}

    void so_evt_start() override
{
    so_environment().stats_controller().turn_on();
}

private :
    void evt_quantity( const so_5::stats::messages::quantity< std::size_t > & evt ) {
    // Just show to standard output.
        std::cout << evt.m_prefix << evt.m_suffix << ": " << evt.m_value << std::endl;
    }

    void evt_activity( const so_5::stats::messages::work_thread_activity& evt) {
        std::cout << evt.m_prefix
                    << ": " << "avrg_time: "
                    << evt.m_stats.m_working_stats.m_avg_time.count()
                    << ", " << "count: "
                    << evt.m_stats.m_working_stats.m_count
                    << ", " << "total_time: "
                    << evt.m_stats.m_working_stats.m_total_time.count()
                    << std::endl;
    }
};

// TIP To <b>Run</b> code, press <shortcut actionId="Run"/> or click the <icon src="AllIcons.Actions.Execute"/> icon in the gutter.
int main(const int argc, char *argv[3]) {

    arguments_t arguments{};

    try {
        arguments = parse_arguments(argc, argv);
        std::cout << "Will be connecting to device with:" << std::endl
                    << "VID: " << std::hex << arguments.vid << std::endl
                    << "PID: " << std::hex << arguments.pid
                    << std::dec << std::endl
                    << std::endl;
    } catch (args::Error&) {
        // message is handled by parse_arguments
        // return EXIT_FAILURE;
        arguments.vid = 0xcafe;
        arguments.pid = 0x4014;
    } catch (...) {
        std::cerr << "An unknown argument parse error has occured!" << std::endl;
        return EXIT_FAILURE;
    }

    usb::settings_t settings{
        .vid = arguments.vid
        , .pid = arguments.pid
        , .ep_addr = ENDPOINT_ADDRESS
        , .config = CONFIGURATION
        , .interface = INTERFACE_NUMBER
        , .tx_time_out_ms = 2000U
        , .rx_time_out_ms = 5000U
    };

    using usb_t = usb::UsbBulk<BUFFER_TOTAL_SIZE>;
    usb_t usb_bulk(settings);

    using usb_agent_t = usb::UsbBulkAgent<BUFFER_TOTAL_SIZE>;
    using device_t = device::Device<BUFFER_TOTAL_SIZE
                    , usb_t::tx_transfer_data_t>;

    //buffers::SimpleBuffer<uint8_t> rx_buffer(BUFFER_TOTAL_SIZE);
    //buffer.set_process_function(processor);

    //device::DeviceControl device_ctrl( usb, gui::log);

    //s_device = &device_ctrl;

    //usb.set_logger(gui::log);

    auto sig_res = std::signal(SIGINT, signal_handler);

    if (sig_res == SIG_ERR) {
        std::cerr << "Failed to set SIGINT handler" << std::endl;
    }

    sig_res = std::signal(SIGTERM, signal_handler);

    if (sig_res == SIG_ERR) {
        std::cerr << "Failed to set SIGTERM handler" << std::endl;
    }

    so_5::mbox_t mbox_usb;
    so_5::mbox_t mbox_device;

    so_5::wrapped_env_t sobj([&](so_5::environment_t & env) {

            mbox_usb = env.create_mbox();
            mbox_device = env.create_mbox();

            auto parent_coop = env.make_coop(/*so_5::disp::thread_pool::make_dispatcher(env, 2U).binder()*/);
            parent_coop->make_agent<usb_agent_t>(mbox_usb, usb_bulk);
            parent_coop->make_agent<device_t>(mbox_usb, mbox_device);

            //parent_coop->make_agent<my_run_time_stats_listener>();

            auto gui_coop = env.make_coop(parent_coop->handle());
            auto gui_agent = gui_coop->make_agent<gui::Gui>(mbox_device, [](){signal_handler(1);});
            gui_agent->make_agents(gui_coop);

            env.register_coop(std::move(parent_coop));
            env.register_coop(std::move(gui_coop));

            // env.introduce_coop(so_5::disp::thread_pool::make_dispatcher(env, 6U).binder(),
            //     [&](so_5::coop_t & coop) mutable {
            //
            //         coop.make_agent<usb_agent_t>(mbox, usb_bulk);
            //         coop.make_agent<device_t>(mbox);
            //
            //         s_env = &env;
            // });

            // env.introduce_coop(
            //     [&](so_5::coop_t & coop) mutable {
            //
            //     //mbox = coop.make_agent<main_agent>(...)->so_direct_mbox();
            //         coop.make_agent<gui::Gui>(mbox, coop, [](){signal_handler(1);});
            // });

        });

    s_env = &sobj;

    sobj.join();

    // so_5::launch([&usb_bulk](so_5::environment_t & env) {
    //       auto board = env.create_mbox();
    //       env.introduce_coop(so_5::disp::thread_pool::make_dispatcher(env, 4U).binder(),
    //           [board, &usb_bulk, &env](so_5::coop_t & coop) mutable {
    //
    //             coop.make_agent<usb_agent_t>(board, usb_bulk);
    //             coop.make_agent<device_t>(board, gui::log);
    //               /*auto a = */coop.make_agent<gui::Gui>(board);
    //               //a->make_agents(coop);
    //               s_env = &env;
    //          });
    //    });

        //usb_thread.detach();
        //gui_thread.join();
        // device_ctrl.set_gpio(1, device::gpio_state_t{});
        // device_ctrl.send_data_if_any();
        // while (true) {
        //     rx_buffer.wait_for_data(buffer_local);
        //
        //     for (size_t i = 0U; i < 2 /*rx_buffer.total_buffer_size()*/; i++) {
        //         //std::cout << (int)buffer_local[i] << " ";
        //     }
        //
        //     //std::cout << std::endl;
        // }

    //usb.stop();

    return EXIT_SUCCESS;
}

// extern "C" int __assert_func(int cond){return 0;}
//
// extern "C" int __getreent(){return 1;}
//
// extern "C" void* __locale_ctype_ptr = NULL;