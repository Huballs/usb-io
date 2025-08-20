//
// Created by hugo on 19.08.25.
//

#ifndef GUIV2_HPP
#define GUIV2_HPP

#include <so_5/all.hpp>
#include "../device/device_control.hpp"
#include "logger.hpp"
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
    };
}

#endif //GUIV2_HPP
