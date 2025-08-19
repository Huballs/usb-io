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

namespace gui {

    class Gui final : public so_5::agent_t {
    public:
        Gui(context_t ctx, so_5::mbox_t board)
           :  so_5::agent_t{std::move(ctx)}
        ,  m_board{std::move(board)}
        {}

    private:
        so_5::mbox_t m_board;
    };
}

#endif //GUIV2_HPP
