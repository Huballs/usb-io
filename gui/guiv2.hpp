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
#include "ftxui/component/component.hpp"           // for Menu
#include "ftxui/component/component_options.hpp"   // for MenuOption
#include "ftxui/component/screen_interactive.hpp"  // for ScreenInteractive
#include "ftxui/dom/elements.hpp"
#include "../fs/fs.hpp"
#include "../device/device_control.hpp"

constexpr std::string_view s_open_file_parent_folder {".."};

namespace gui {

    using namespace ftxui;

    class Gui final : public so_5::agent_t {
    public:
        Gui(context_t ctx, so_5::mbox_t board)
           :  so_5::agent_t{std::move(ctx)}
        ,  m_board{std::move(board)}
        {}

    private:
        so_5::mbox_t m_board;
        ScreenInteractive m_screen = ScreenInteractive::FullscreenPrimaryScreen();
        fs::Fs m_fs {s_open_file_parent_folder};
        Component m_modal_open_file;
        Component m_menu;
        Component m_logger;

        std::shared_ptr<WindowGPIO> m_gpio;

        bool m_show_modal_open_file {false};

        Timer m_animation_timer{};

        void make_menu() noexcept;
        void make_logger() noexcept;
        void make_gpio() noexcept;
    };

    void Gui::make_menu() noexcept {
        auto m_menu = Menu::make({
            {"Open", [&](){m_show_modal_open_file = true;}}
        }
        , MenuOption::Horizontal());
    }

    void Gui::make_logger() noexcept {
        m_logger = detail::Logger::make();
    }

    void Gui::make_gpio() noexcept {
        m_gpio = std::make_shared<WindowGPIO>(m_logger, m_animation_timer);
    }
}

#endif //GUIV2_HPP
