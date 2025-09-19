//
// Created by hugo on 19.08.25.
//

#ifndef GUIV2_HPP
#define GUIV2_HPP

#include <so_5/all.hpp>
#include "../device/device_control.hpp"
#include "logger.hpp"
#include "menu.hpp"
#include "timer.hpp"
#include "window_gpio.hpp"
#include "window_open_file.hpp"
#include "window_tabs.hpp"
#include "ftxui/component/component.hpp"           // for Menu
#include "ftxui/component/component_options.hpp"   // for MenuOption
#include "ftxui/component/screen_interactive.hpp"  // for ScreenInteractive
#include "ftxui/component/loop.hpp"
#include "ftxui/dom/elements.hpp"
#include "../fs/fs.hpp"
#include "button.hpp"
#include "conn_status.hpp"
#include "device_status.hpp"
#include "returns.hpp"
#include "../device/device_control.hpp"
#include "ps_data.hpp"

constexpr std::string_view s_open_file_parent_folder {".."};

namespace gui {

    using namespace ftxui;

    struct loop_signal : public so_5::signal_t {};

    class Gui final : public so_5::agent_t {
    public:
        Gui(context_t ctx, so_5::mbox_t board, std::function<void(void)> f_on_exit)
           :  so_5::agent_t{std::move(ctx)}
        ,  m_board{std::move(board)}
        ,  m_f_on_exit(std::move(f_on_exit)){

            //make_agents(coop);
        }

        ~Gui() = default;

        void so_define_agent() override;
        void so_evt_start() override;

        void make_agents(so_5::coop_unique_holder_t& coop) noexcept;
    private:
        so_5::mbox_t m_board;
        so_5::timer_id_t m_timer_loop;

        ScreenInteractive m_screen = ScreenInteractive::FullscreenPrimaryScreen();
        fs::Fs m_fs {s_open_file_parent_folder};

        Component m_menu;
        Component m_gpio;
        Component m_modal_open_file;
        Component m_core_control;
        Component m_render;
        Component m_power_suply_data;
        Component m_settings;

        std::shared_ptr<detail::Logger> m_logger;
        std::shared_ptr<WindowTabs> m_tabs;
        std::shared_ptr<ConnStatus> m_conn_status;
        std::shared_ptr<DeviceStatus> m_device_status;


        Component m_tab_returns;

        std::unique_ptr<Loop> m_render_loop;

        std::function<void(void)> m_f_on_exit;

        bool m_show_modal_open_file {false};

        Timer m_animation_timer{};

        void on_loop(mhood_t<loop_signal>) noexcept;
        void on_status(mhood_t<device::sig_status>) noexcept;
        void on_connected(mhood_t<device::sig_connected>) noexcept;
        void on_disconnected(mhood_t<device::sig_disconnected>) noexcept;
        void on_message(mhood_t<device::sig_message>) noexcept;

        Component make_menu() noexcept;
        Component make_logger() noexcept;
        void make_gpio() noexcept;
        Component make_open_file(std::shared_ptr<WindowTabs> tabs) noexcept;
        void build_render() noexcept;

    };
}

#endif //GUIV2_HPP
