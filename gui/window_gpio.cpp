//
// Created by hugo on 02.07.25.
//

#include "window_gpio.hpp"

#include <assert.h>
#include <iostream>
#include <ranges>
#include <ftxui/component/event.hpp>

#include "logger.hpp"
#include "ftxui/component/component_base.hpp"

namespace gui {

    class GPIOcomponent : public ComponentBase {
    public:

        GPIOcomponent(size_t gpio_n, device::DeviceControl& device, Timer& timer)
            : m_gpio_n(gpio_n), m_device(device), m_timer(timer) {

            m_name += (m_gpio_n < 10U ? "0" : "");
            m_name += std::to_string(m_gpio_n);

            make_element();
        };

        Element OnRender() override {

            auto new_state = m_device.get_gpio(m_gpio_n);
            auto& this_timer = m_timer.get(m_name);

            if (new_state != m_gpio_state) {
                m_gpio_state = new_state;
                make_element();
                this_timer.start(std::chrono::milliseconds(400U));
                //log(std::format("new gpio {} io {}, value {}\n", m_gpio_n, (int)new_state.io, new_state.value));
            }

            if (m_gpio_state.function == 5U) {
                m_color = Color::White;
            } else {
                m_color = Color::Yellow;
            }

            if (this_timer.running) {
                m_color = m_color_on_change;
            } else {
                this_timer.reset();
            }

            if (m_is_hovered) {
                this->TakeFocus();
            }

            return (Focused() ? inverted(m_element) : m_element) | color(m_color);
        }

        // bool OnEvent(Event ev) override {
        //
        // }

        bool Focusable() const override {
            // if (m_gpio_state.function == 5U) {
            //     return true;
            // }
            //
            // return false;
            return true;
        }

        bool& get_is_hovered() noexcept {
            return m_is_hovered;
        }

        static Component make(size_t gpio_n, device::DeviceControl& device, Timer& timer) {
            return Make<GPIOcomponent>(gpio_n, device, timer);
        }
    private:

        void make_element() {
            std::stringstream text_stream;
            text_stream << m_name
                        << '[' << (m_gpio_state.io == device::io_t::INPUT ? 'I' : 'O') << ']'
                        << '[' << m_gpio_state.value << "] ";
            m_text = text_stream.str();
            m_element = text(m_text);
        }

        std::string m_name{"GPIO"};
        const size_t m_gpio_n;
        std::string m_text;
        device::gpio_state_t m_gpio_state;
        Element m_element;
        Color m_color;
        const Color m_color_on_change = Color::Blue;

        device::DeviceControl& m_device;
        Timer& m_timer;

        bool m_is_hovered = false;

    };

    Component make_gpio_component(size_t gpio_n, device::DeviceControl& device, Timer& timer) {
        // return Renderer([=] (bool focus) {
        //     w->render();
        //     return focus ? inverted(ftxui::text(name + " "))
        //         : text(name + " ");
        // });
        auto g = std::make_shared<GPIOcomponent>(gpio_n, device, timer);
        Component c(g);
        c |= Hoverable(&g->get_is_hovered());
        return c;
    }

    std::string make_gpio_name(const size_t gpio_n, const device::gpio_state_t state) {
        std::string name = "GPIO" + std::to_string(gpio_n);
        if (gpio_n < 10U) {
            name += ' ';
        }

        name += std::string{"["} + (state.io == device::io_t::INPUT ? "I" : "O");
        name += ']';

        name += std::string{"["} + (state.value == 0U ? "0" : "1");
        name += ']';
        return name;
    }

    Component make_final_container(const std::vector<Component>& gpio) {

        assert(gpio.size() == 28U);

        auto chunk_view = std::ranges::views::all(gpio) | std::views::chunk(14);

        return Container::Vertical({

            Container::Horizontal(std::vector<Component>(chunk_view[0].begin(), chunk_view[0].end()))
            , Renderer([=](){return separator();})
            , Container::Horizontal(std::vector<Component>(chunk_view[1].begin(), chunk_view[1].end()))
        }) | flex;
    }

    WindowGPIO::WindowGPIO(std::function<void(std::string_view)> logger, device::DeviceControl& device_ctrl, Timer& timer)
        : m_logger(std::move(logger))
        , m_device(device_ctrl)
        , m_timer(timer){

        m_components.resize(device::s_gpio_count);

        for (auto pos : std::ranges::views::iota(0U, device::s_gpio_count)) {
            set_gpio(device::gpio_state_t{}, pos);
        }

        m_final_render = make_final_container(m_components);
    }

    void WindowGPIO::set_gpio([[maybe_unused]]device::gpio_state_t state, size_t position) {

        auto new_component = make_gpio_component(position, m_device, m_timer);

        new_component |= CatchEvent([this, position](const Event& ev) {

            auto current_state = m_device.get_gpio(position);

            if (current_state.function != 5U)
                return false;

            if (ev == Event::Return) {
                //m_logger(std::to_string(position));
                if (current_state.value == 1U)
                    current_state.value = 0U;
                else
                    current_state.value = 1U;
                m_device.set_gpio(position, current_state);
            } else if (ev == Event::i) {
                current_state.io = device::io_t::INPUT;

                m_device.set_gpio(position, current_state);
            } else if (ev == Event::o) {
                current_state.io = device::io_t::OUTPUT;

                m_device.set_gpio(position, current_state);
            }
           return false;
        });

        m_components.at(position) = new_component;
    }

    Component WindowGPIO::render() {

        return m_final_render;
    }
}
