//
// Created by hugo on 06.07.25.
//

#ifndef WINDOW_COMM_STATE_HPP
#define WINDOW_COMM_STATE_HPP

#include <ftxui/component/component.hpp>

#include "ftxui/component/component_base.hpp"
#include "../device/device_control.hpp"

namespace gui {
    using namespace ftxui;

    class WindowCommState : public ComponentBase {
    public:

        WindowCommState(device::DeviceControl& device_ctrl, ScreenInteractive& screen)
            : m_device(device_ctrl)
            , m_screen(screen){};

        Element OnRender() override {

            static bool prev_success = false;

            //m_device.send_data_if_any();

            std::string message{m_device.last_comm_success() ? "OK" : "NOK"};
            Decorator col = color(m_device.last_comm_success() ? Color::Green : Color::Red);

            if (prev_success != m_device.last_comm_success()) {
                prev_success = m_device.last_comm_success();
            }

            return text(message) | col | borderStyled(BorderStyle::LIGHT);
        };

    public:
        static Component make(device::DeviceControl& device_ctrl, ScreenInteractive& screen) {
            return Make<WindowCommState>(device_ctrl, screen);
        };

    private:
        device::DeviceControl& m_device;
        ScreenInteractive& m_screen;
    };
}

#endif //WINDOW_COMM_STATE_HPP
