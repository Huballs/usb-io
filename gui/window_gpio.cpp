//
// Created by hugo on 02.07.25.
//

#include "window_gpio.hpp"

#include <assert.h>
#include <iostream>
#include <ranges>
#include <ftxui/component/component.hpp>
#include <ftxui/component/event.hpp>

#include "logger.hpp"
#include "ftxui/component/component_base.hpp"

namespace gui {

    class GPIOcomponent : public ComponentBase {
    public:

        GPIOcomponent(size_t gpio_n, Timer& timer)
            : m_gpio_n(gpio_n), m_timer(timer) {

            m_name += (m_gpio_n < 10U ? "0" : "");
            m_name += std::to_string(m_gpio_n);

            make_element(device::gpio_state_t{});
        };

        Element OnRender() override {

            auto& this_timer = m_timer.get(m_name);

            auto colour = m_color;

            if (this_timer.running) {
                colour = m_color_on_change;
            } else {
                this_timer.reset();
            }

            if (m_is_hovered) {
                this->TakeFocus();
            }

            return (Focused() ? inverted(m_element) : m_element) | color(colour);
            //return (Focused() ? inverted(text("text")) : text("text")) | color(colour);
        }


        bool Focusable() const override {
            return true;
        }

        bool& get_is_hovered() noexcept {
            return m_is_hovered;
        }

        void make_element(const device::gpio_state_t& gpio_state) {
            std::stringstream text_stream;
            text_stream << m_name;
            text_stream << '[' << (gpio_state.io == device::proto::io_t::INPUT ? 'I' : 'O') << ']';
            text_stream << '[' << gpio_state.value << "] ";

            m_text = text_stream.str();
            m_element = text(m_text);

            if (gpio_state.function == 5U) {
                m_color = Color::White;
            } else {
                m_color = Color::Yellow;
            }
            m_gpio_state = gpio_state;

            auto& this_timer = m_timer.get(m_name);
            this_timer.start(500ms);
        }

        const device::gpio_state_t& state() const noexcept {
            return m_gpio_state;
        }

    private:

        std::string m_name{" IO"};
        const size_t m_gpio_n;
        std::string m_text;
        Element m_element;
        Color m_color;
        const Color m_color_on_change = Color::Blue;
        device::gpio_state_t m_gpio_state;
        Timer& m_timer;

        bool m_is_hovered = false;

    };

    Component make_gpio_component(size_t gpio_n, Timer& timer) {
        Component g = std::make_shared<GPIOcomponent>(gpio_n, timer);
        g |= Hoverable(&(dynamic_cast<GPIOcomponent*>(&*g)->get_is_hovered()));
        return g;
    }

    Component make_final_container(const std::vector<Component>& gpio) {

        assert(gpio.size() == device::s_gpio_count);

        auto chunk_view = std::ranges::views::all(gpio) | std::views::chunk(device::s_gpio_count / 2);

        return Container::Vertical({

            Container::Horizontal(std::vector<Component>(chunk_view[0].begin(), chunk_view[0].end()))
            , Renderer([=](){return separator();})
            , Container::Horizontal(std::vector<Component>(chunk_view[1].begin(), chunk_view[1].end()))
        }) | flex;
    }

    WindowGPIO::WindowGPIO(context_t ctx, so_5::mbox_t board
            , std::function<void(std::string_view)> logger
            , Timer& timer
            , std::function<void(void)> f_update_screen
            , Component& this_render)
            : so_5::agent_t{std::move(ctx)}
            , m_board(std::move(board))
            , m_logger(std::move(logger))
            , m_timer(timer)
            , m_f_update_screen(f_update_screen){

        m_components_top.resize(device::s_gpio_count / 2U);
        m_components_bot.resize(device::s_gpio_count / 2U);

        for (auto pos : std::ranges::views::iota(0U, device::s_gpio_count / 2U)) {
            m_components_top[pos] = create_gpio(device::gpio_state_t{}, pos);
            m_components_bot[pos] = create_gpio(device::gpio_state_t{}, pos + (device::s_gpio_count / 2U));
        }

        m_final_render = Container::Vertical({
            Container::Horizontal(m_components_top)
            , Renderer([=](){return separator();})
            , Container::Horizontal(m_components_bot)
        }) | flex;

        this_render = m_final_render;
    }

    void WindowGPIO::so_define_agent() {
        so_subscribe(m_board).event(&WindowGPIO::on_gpio_recieve);
    }

    Component WindowGPIO::create_gpio([[maybe_unused]]device::gpio_state_t state, size_t position) {

        auto new_component = make_gpio_component(position, m_timer);

        new_component |= CatchEvent([this, position](const Event& ev) {

            auto current_state = get_state(position);

            if (current_state.function != 5U)
                return false;

            if (ev == Event::Return) {
                //m_logger(std::to_string(position));
                if (current_state.value == 1U)
                    current_state.value = 0U;
                else
                    current_state.value = 1U;
                // m_device.set_gpio(position, current_state);
                so_5::send<device::sig_gpio_set>(m_board, position, current_state);
            } else if (ev == Event::i) {
                current_state.io = device::proto::io_t::INPUT;

                // m_device.set_gpio(position, current_state);
                so_5::send<device::sig_gpio_set>(m_board, position, current_state);
            } else if (ev == Event::o) {
                current_state.io = device::proto::io_t::OUTPUT;

                // m_device.set_gpio(position, current_state);
                so_5::send<device::sig_gpio_set>(m_board, position, current_state);
            }
           return false;
        });

        return new_component;
    }

    GPIOcomponent* WindowGPIO::get_component(size_t n) noexcept {
        Component comp;

        if (n < device::s_gpio_count / 2U) {
            comp = m_components_top.at(n)->ActiveChild()->ActiveChild();//->children_[0U]->children_[0];
        } else {
            comp = m_components_bot.at(n - (device::s_gpio_count / 2U))->ActiveChild()->ActiveChild();//->children_[0U]->children_[0];
        }

        return reinterpret_cast<GPIOcomponent*>(&*comp);
    }

    const device::gpio_state_t& WindowGPIO::get_state(size_t n) noexcept {
        return get_component(n)->state();
    }

    void WindowGPIO::on_gpio_recieve(mhood_t<device::sig_gpio_new_state> s) noexcept {

        auto state = s->state;

        get_component(s->n)->make_element(state);

        m_f_update_screen();
    }

    Component WindowGPIO::render() {

        return m_final_render;
    }
}
