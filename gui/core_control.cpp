//
// Created by hugo on 12.08.25.
//
#include "core_control.hpp"
#include "button.hpp"

namespace gui {

    CoreControl::CoreControl(context_t ctx, so_5::mbox_t board, Component& this_component)
       :  so_5::agent_t{std::move(ctx)}
    ,  m_board{std::move(board)} {

        make_component();
        this_component = m_component;
    }

    void CoreControl::so_define_agent() {
        agent_t::so_define_agent();
    }

    void CoreControl::make_component() noexcept {

        m_button_launch = std::make_shared<Button>("Launch");
        m_button_stop = std::make_shared<Button>("Stop");
        m_button_pause = std::make_shared<Button>("Pause");
        m_button_continue = std::make_shared<Button>("Continue");

        m_button_launch->on_enter([this]() {
            so_5::send<device::sig_command>(m_board, device::command_t::LAUNCH_LUA_CORE);
        });

        m_button_stop->on_enter([this]() {
            so_5::send<device::sig_command>(m_board, device::command_t::STOP_LUA_CORE);
        });

        m_button_pause->on_enter([this]() {
            so_5::send<device::sig_command>(m_board, device::command_t::PAUSE_LUA_CORE);
        });

        m_button_continue->on_enter([this]() {
            so_5::send<device::sig_command>(m_board, device::command_t::CONTINUE_LUA_CORE);
        });

        auto title = Renderer([]() {
            return hcenter(text("Core Control"));
        });

        #define sep_line Renderer([]() { \
            return separator();\
        })

        #define sep_empty Renderer([]() { \
        return separatorEmpty();\
        })

        m_component = Container::Vertical({
            title
            , sep_line
            , m_button_launch
            , m_button_pause
            , m_button_continue
            , sep_empty
            , m_button_stop
            , sep_empty
        }) | border;

    }
}
