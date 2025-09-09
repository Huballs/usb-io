//
// Created by hugo on 07.09.2025.
//

#ifndef USB_IO_RETURNS_HPP
#define USB_IO_RETURNS_HPP

#include "ftxui/component/component_base.hpp"
#include "../device/device_control.hpp"
#include <unordered_map>

namespace gui {

    using namespace ftxui;

    using return_name_t = std::string;
    using return_value_t = std::string;

    class ReturnValueElement : public ComponentBase {
    public:
        ReturnValueElement(return_value_t value);

        Element OnRender() override;
        bool OnEvent(Event) override;

        void value(return_value_t value);

    private:
        return_value_t m_value;
        Element m_element;

        void make_component();
    };

    class Returns : public ComponentBase, public so_5::agent_t {

    public:
        Returns(context_t ctx, so_5::mbox_t board) : agent_t(std::move(ctx)), m_board(board) {}

        void so_define_agent() override;

    private:
        so_5::mbox_t m_board;


        using map_name_to_value_t = std::unordered_map<return_name_t, ReturnValueElement>;

        map_name_to_value_t m_map_name_to_value;



        void make_component();


    };
}

#endif //USB_IO_RETURNS_HPP