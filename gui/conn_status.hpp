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

    class ConnStatus : public ComponentBase {
    public:

        ConnStatus() {
            make_element(false);
        };

        Element OnRender() override {
            return m_element;
        }

        static Component make() {
            return Make<ConnStatus>();
        }

        void make_element(bool connected) {
            if (connected) {
                m_element = text(m_name_connected) | color(Color::Green);
            } else {
                m_element = text(m_name_not_connected) | color(Color::Red);
            }
        }

    private:
        const std::string m_name_connected    {"Connected    "};
        const std::string m_name_not_connected{"Not Connected"};

        Element m_element;

    };
}
#endif //WINDOW_CONN_HPP
