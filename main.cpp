#include <iostream>
#include <optional>
#include <args.hxx>
#include <string>
#include <thread>
#include <cassert>
#include <csignal>
// #include "gui/gui.hpp"
// #include "device/device_control.hpp"
#include "sob-test.hpp"
#include "device/device_control.hpp"
#include "usb/usb-bulk.hpp"
#include "usb/usb-bulk-agent.hpp"
#include "gui/logger.hpp"

namespace {

    constexpr unsigned char ENDPOINT_ADDRESS    = 0x03U;
    constexpr size_t ENDPOINT_DATA_SIZE         = 64U;
    constexpr int INTERFACE_NUMBER              = 0;
    constexpr int CONFIGURATION                 = 1;

    constexpr size_t BUFFER_MULTI = 4U;
    constexpr size_t BUFFER_TOTAL_SIZE = BUFFER_MULTI * ENDPOINT_DATA_SIZE;

    static std::atomic_bool s_exit{false};

    // device::DeviceControl* s_device;

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

        // if (s_device && !s_device->has_device()) {
        //     std::exit(0);
        // }
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
        , .rx_time_out_ms = 100U
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

    // auto usb_thread = std::jthread([&]() {
    //     //usb.start();
    //     while (true) {
    //         usb_bulk.loop();
    //
    //         auto data = usb_bulk.make_tx_data();
    //
    //         usb_bulk.transmit([](usb::status_t s) {
    //             std::cout << "transmit: " << (int)s << std::endl;
    //         }, &data);
    //
    //         if (s_exit == true) {
    //             std::cout << "exiting" << std::endl;
    //             return;
    //         }
    //         std::this_thread::sleep_for(std::chrono::milliseconds(500));
    //     }
    // });
    so_5::launch([&usb_bulk](so_5::environment_t & env) {
          auto board = env.create_mbox();
          env.introduce_coop(so_5::disp::thread_pool::make_dispatcher(env, 4U).binder(),
              [board, &usb_bulk](so_5::coop_t & coop) mutable {

                coop.make_agent<usb_agent_t>(board, usb_bulk);
                coop.make_agent<device_t>(board, gui::log);
             });
       });

        auto gui_thread = std::jthread([&]() {
            //gui::run(device_ctrl, s_exit);
                while (1){};
        });

        //usb_thread.detach();
        gui_thread.join();
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

extern "C" void __assert_func(bool cond){}

extern "C" void __getreent(){}

extern "C" void* __locale_ctype_ptr = NULL;