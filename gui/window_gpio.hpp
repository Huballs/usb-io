//
// Created by hugo on 02.07.25.
//

#ifndef WINDOW_GPIO_HPP
#define WINDOW_GPIO_HPP

#include <mutex>
#include <vector>

#include <ftxui/component/component.hpp>
#include "ftxui/component/component_base.hpp"

#include "../device/device_control.hpp"
#include "timer.hpp"

ftxui::Component make_gpio_component(std::string name);

namespace gui {
    using namespace ftxui;

    class WindowGPIO {
    public:

        WindowGPIO(std::function<void(std::string_view)> logger, device::DeviceControl& device_ctrl, Timer& timer);

        Component render();

    private:

        // ComponentDecorator

        void set_gpio(device::gpio_state_t state, size_t position);

        std::vector<Component> m_components;

        Component m_final_render;

        std::function<void(std::string_view)> m_logger;

        device::DeviceControl& m_device;
        Timer& m_timer;

    };
}

#endif //WINDOW_GPIO_HPP
