//
// Created by hugo on 02.07.25.
//

#ifndef WINDOW_GPIO_HPP
#define WINDOW_GPIO_HPP

#include <vector>

#include "ftxui/component/component_base.hpp"
#include "../device/device_control.hpp"
#include "timer.hpp"

ftxui::Component make_gpio_component(std::string name);

namespace gui {
    using namespace ftxui;

    //struct window_gpio_render{Component render;};

    class GPIOcomponent;

    class WindowGPIO : public so_5::agent_t{
    public:

        WindowGPIO(context_t ctx, so_5::mbox_t board
            , std::function<void(std::string_view)> logger
            , Timer& timer, std::function<void(void)> f_update_screen, Component& this_render);

        Component render();

        void so_define_agent() override;
        void so_evt_start() override{}

    private:

        void on_gpio_recieve(mhood_t<device::sig_gpio_new_state>) noexcept;

        const device::gpio_state_t& get_state(size_t n) noexcept;
        Component create_gpio(device::gpio_state_t state, size_t position);

        GPIOcomponent* get_component(size_t n) noexcept;

        so_5::mbox_t    m_board;

        std::vector<Component> m_components_top;
        std::vector<Component> m_components_bot;

        Component m_final_render;

        std::function<void(std::string_view)> m_logger;
        Timer& m_timer;

        std::function<void(void)> m_f_update_screen;

    };
}

#endif //WINDOW_GPIO_HPP
