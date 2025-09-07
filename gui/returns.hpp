//
// Created by hugo on 07.09.2025.
//

#ifndef USB_IO_RETURNS_HPP
#define USB_IO_RETURNS_HPP

#include "ftxui/component/component_base.hpp"
#include "../device/device_control.hpp"

namespace gui {

    using namespace ftxui;

    class Returns : public ComponentBase, public so_5::agent_t {

    public:
        Returns(context_t ctx, so_5::mbox_t board) : agent_t(std::move(ctx)), m_board(board) {}

        void so_define_agent() override;

    private:
        so_5::mbox_t m_board;
    };
}

#endif //USB_IO_RETURNS_HPP