//
// Created by hugo on 13.07.25.
//

#ifndef WINDOW_CONN_HPP
#define WINDOW_CONN_HPP

#include <ftxui/component/event.hpp>

#include "logger.hpp"
#include "ftxui/component/component_base.hpp"
#include "../device/device_control.hpp"

namespace gui {

    using namespace ftxui;

    class WindowConn : public ComponentBase {
    public:

        WindowConn(device::DeviceControl& device) : m_device(device) {
            make_element();
        };

        Element OnRender() override {
            if (m_device.has_device() != m_prev_status) {
                make_element();
                m_prev_status = m_device.has_device();
            }

            return m_element;
        }

        static Component make(device::DeviceControl& device) {
            return Make<WindowConn>(device);
        }
    private:

        void make_element() {
            if (m_device.has_device()) {
                m_element = text(m_name_connected) | color(Color::Green);
            } else {
                m_element = text(m_name_not_connected) | color(Color::Red);
            }
        }

        const std::string m_name_connected    {"Connected    "};
        const std::string m_name_not_connected{"Not Connected"};

        Element m_element;

        bool m_prev_status = false;

        device::DeviceControl& m_device;
    };
}
#endif //WINDOW_CONN_HPP
