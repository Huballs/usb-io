//
// Created by hugo on 12.06.25.
//

// Copyright 2020 Arthur Sonzogni. All rights reserved.
// Use of this source code is governed by the MIT license that can be found in
// the LICENSE file.
#include <functional>  // for function
#include <iostream>  // for basic_ostream::operator<<, operator<<, endl, basic_ostream, basic_ostream<>::__ostream_type, cout, ostream
#include <string>    // for string, basic_string, allocator
#include <vector>    // for vector
#include <mutex>

#include "ftxui/component/captured_mouse.hpp"      // for ftxui
#include "ftxui/component/component.hpp"           // for Menu
#include "ftxui/component/component_options.hpp"   // for MenuOption
#include "ftxui/component/screen_interactive.hpp"  // for ScreenInteractive
#include "ftxui/dom/elements.hpp"
#include "ftxui/screen/box.hpp"

#include "gui.hpp"
#include "window_program.hpp"
#include "window_gpio.hpp"
#include "window_comm_state.hpp"
#include "conn_status.hpp"
#include "window_open_file.hpp"
#include "window_tabs.hpp"
#include "button.hpp"
#include "menu.hpp"
#include "device_status.hpp"
#include "../device/device_control.hpp"
#include "../fs/fs.hpp"
#include "timer.hpp"

namespace gui {

    using namespace ftxui;

    void run(device::DeviceControl& device_ctrl, std::atomic_bool& on_exit) {

        auto screen = ScreenInteractive::FullscreenPrimaryScreen();
        screen.TrackMouse(true);

        bool show_open_file_modal = false;
        auto fs = fs::Fs("..");
        auto window_tabs = std::make_shared<WindowTabs>(device_ctrl);

        auto w_open = WindowOpenFile::make(fs, window_tabs, &show_open_file_modal);

        auto menu = Menu::make(
            {
                {"Open", [&](){show_open_file_modal = true;}}
            }
            , MenuOption::Horizontal());

        auto messager = detail::Logger::make();
        detail::Logger::SetScreen(&screen);

        Timer timer{};
        WindowGPIO w_gpio(log, device_ctrl, timer);

        auto conn_state = WindowConn::make(device_ctrl);
        auto comm_state = WindowCommState::make(device_ctrl, screen);

        auto screen_updater = std::jthread([&]() {

            std::this_thread::sleep_for(std::chrono::milliseconds(500U));

            while (true) {
                if (device_ctrl.state_changed()) {
                    //w_gpio.render();
                    screen.PostEvent(Event::Custom);
                }
                device_ctrl.send_data_if_any();

                if (timer.check_all()) {
                    screen.PostEvent(Event::Custom);
                }

                std::this_thread::sleep_for(std::chrono::milliseconds(6U));
            }
        });

        screen_updater.detach();

        int split_size = 10;

        auto bottom = Container::Vertical({
            messager  | yflex_shrink
            , Container::Horizontal({w_gpio.render(), comm_state})
        });

        auto device_status = std::make_shared<DeviceStatus>(device_ctrl);

        #define sep Renderer([]() { \
            return hbox(separatorEmpty(), separator() | color(Color::GrayLight), separatorEmpty()); \
        })

        auto layout = Container::Vertical({
            Container::Horizontal({menu, sep, conn_state, sep, device_status, sep})
            , ResizableSplitTop(
            window_tabs | flex_grow
                    , bottom | xflex_grow, &split_size)
        } ) | flex_grow;


        auto& window_open_file = *reinterpret_cast<WindowOpenFile*>(&*w_open);

        auto back_button = std::make_shared<gui::Button>("<", border);
        auto close_button = std::make_shared<gui::Button>("x", border);

        back_button->on_enter([&]() {
            window_open_file.go_back();
        });

        close_button->on_enter([&]() {
            window_open_file.close();
        });

        auto title = WindowOpenFileTitle::make(window_open_file);

        auto modal_open_file = Container::Vertical({
            Container::Horizontal({back_button, title, close_button})
            , w_open
        });

        layout |= Modal(modal_open_file, &show_open_file_modal);
        screen.Loop(layout);

        on_exit.store(true);
    }

}
