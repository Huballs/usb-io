//
// Created by hugo on 19.08.25.
//

#include "guiv2.hpp"
#include "core_control.hpp"
#include "returns.hpp"
#include "settings.hpp"

namespace gui {

void Gui::make_agents(so_5::coop_unique_holder_t & coop) noexcept {
        auto update_screen = [this]() {
            m_screen.PostEvent(Event::Custom);
        };

        coop->make_agent<WindowGPIO>(m_board, log, m_animation_timer,update_screen, m_gpio);
        coop->make_agent<CoreControl>(m_board, m_core_control);
        coop->make_agent<PSData>(m_board, m_power_suply_data);
        coop->make_agent<Settings>(m_board, m_settings);

        auto return_tab = coop->make_agent<Returns>(m_board, update_screen);
        Component c{return_tab};
        m_tab_returns = c;
    }

    void Gui::so_define_agent() {
        m_timer_loop = so_5::send_periodic<loop_signal>(*this, 0ms, 30ms);
    }

    void Gui::so_evt_start() {
        so_subscribe_self().event(&Gui::on_loop);
        so_subscribe(m_board).event(&Gui::on_connected);
        so_subscribe(m_board).event(&Gui::on_disconnected);
        so_subscribe(m_board).event(&Gui::on_status);
        so_subscribe(m_board).event(&Gui::on_message);
        build_render();
    }

    void Gui::on_loop(mhood_t<loop_signal>) noexcept {

        if (m_render_loop && !m_render_loop->HasQuitted()) {

            if (m_animation_timer.check_all()) {
                m_screen.PostEvent(Event::Custom);
            }

            m_render_loop->RunOnce();

        }

        if (m_render_loop->HasQuitted()) {
            m_screen.Exit();
            m_f_on_exit();
        }
    }

    void Gui::on_status(mhood_t<device::sig_status> s) noexcept {
        m_device_status->make_element(s->status);
        m_screen.PostEvent(Event::Custom);
    }

    void Gui::on_connected(mhood_t<device::sig_connected>) noexcept {
        m_conn_status->make_element(true);
        m_logger->AddMessage("Device attached");
        m_screen.PostEvent(Event::Custom);
    }

    void Gui::on_disconnected(mhood_t<device::sig_disconnected>) noexcept {
        m_conn_status->make_element(false);
        m_logger->AddMessage("Device dettached");
        m_screen.PostEvent(Event::Custom);
    }

    void Gui::on_message(mhood_t<device::sig_message> s) noexcept {
        //std::cout << s->text << std::endl;
        m_logger->AddMessage(s->text);
    }

    Component Gui::make_menu() noexcept {
        return Menu::make({
            {"Open", [&](){m_show_modal_open_file = true;}}
            , {"Request Status", [&]() {so_5::send<device::sig_command>(m_board, device::command_t::GET_STATUS);}}
            , {"Request GPIO", [&]() {so_5::send<device::sig_command>(m_board, device::command_t::GET_ALL_GPIO);}}
            , {"Exit", [&]() {
                m_screen.Exit();
                m_f_on_exit();
            }}
        }
        , MenuOption::Horizontal());
    }

    Component Gui::make_logger() noexcept {
       return detail::Logger::make();
    }

    void Gui::make_gpio() noexcept {
        //m_gpio = std::make_shared<WindowGPIO>(log, m_animation_timer);
    }

    Component Gui::make_open_file(std::shared_ptr<WindowTabs> tabs) noexcept {
        auto window_open_file = std::make_shared<WindowOpenFile>(m_fs, tabs, &m_show_modal_open_file);

        auto back_button = std::make_shared<gui::Button>("<", border);
        auto close_button = std::make_shared<gui::Button>("x", border);

        back_button->on_enter([window_open_file]() {
            window_open_file->go_back();
        });

        close_button->on_enter([window_open_file]() {
            window_open_file->close();
        });

        auto title = WindowOpenFileTitle::make(*window_open_file);

        return Container::Vertical({
            Container::Horizontal({back_button, title, close_button})
            , window_open_file
        });
    }

    void Gui::build_render() noexcept {

        static int split_size = 10;

        auto send_script = [this](std::string_view name, std::string_view text) {
            so_5::send<device::sig_send_script>(m_board, name, text);
        };

        m_logger = std::make_shared<detail::Logger>();
        m_tabs = std::make_shared<WindowTabs>(send_script);
        m_modal_open_file = make_open_file(m_tabs);
        m_menu = make_menu();
        m_conn_status = std::make_shared<ConnStatus>();
        m_device_status = std::make_shared<DeviceStatus>();

        m_tabs->add_tab("returns", m_tab_returns);
        m_tabs->add_tab("settings", m_settings);

        auto bottom = Container::Vertical({
            m_logger  | yflex_shrink
            , Container::Horizontal({m_gpio/*, comm_state*/})
        });

        #define sep Renderer([]() { \
        return hbox(separatorEmpty(), separator() | color(Color::GrayLight), separatorEmpty()); \
        })

        auto left_layout = Container::Vertical({
            Container::Horizontal({m_menu, sep, m_conn_status, sep, m_device_status, sep, m_power_suply_data})
            , ResizableSplitTop(
            m_tabs | flex_grow
                    , bottom | xflex_grow, &split_size)
        } ) | flex_grow;

        auto layout = Container::Horizontal({
            left_layout
            , m_core_control
        });

        layout |= Modal(m_modal_open_file, &m_show_modal_open_file);

        m_render = layout;

        m_render_loop = std::make_unique<Loop>(&m_screen, m_render);
    }
    
}