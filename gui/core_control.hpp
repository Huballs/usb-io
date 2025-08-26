//
// Created by hugo on 12.08.25.
//

#ifndef CORE_CONTROL_HPP
#define CORE_CONTROL_HPP
#include "button.hpp"
#include "logger.hpp"
#include "../device/device_control.hpp"
#include "ftxui/component/component_base.hpp"

namespace gui {

    using namespace ftxui;

    class CoreControl : public so_5::agent_t {
    public:
        CoreControl(context_t ctx, so_5::mbox_t board, Component& this_component);

        void so_define_agent() override;
    private:

        void make_component() noexcept;

        so_5::mbox_t m_board;
        Component m_component;

        std::shared_ptr<Button> m_button_launch;
        std::shared_ptr<Button> m_button_stop;
        std::shared_ptr<Button> m_button_pause;
        std::shared_ptr<Button> m_button_continue;
    };

}

#endif //CORE_CONTROL_HPP
