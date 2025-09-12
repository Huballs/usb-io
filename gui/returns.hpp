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



    class ReturnComponent : public ComponentBase {
    public:
        ReturnComponent(return_name_t name, std::shared_ptr<ReturnValueElement> ret_val);

        Element OnRender() override;
        bool OnEvent(Event) override;

    private:
        return_name_t m_name;
        std::shared_ptr<ReturnValueElement> m_ret_val;

        Component m_component;

        void make_component();
    };



    class Returns : public ComponentBase, public so_5::agent_t {

    public:
        Returns(context_t ctx, so_5::mbox_t board, std::function<void(void)> f_update_screen)
            : agent_t(std::move(ctx)), m_board(board), m_f_update_screen(f_update_screen) {}

        void so_define_agent() override;

        void on_variable(mhood_t<device::sig_variable>);

        Element OnRender() override;
        bool OnEvent(Event ev) override;

    private:
        using map_name_to_value_t = std::unordered_map<return_name_t, std::shared_ptr<ReturnValueElement>>;

        so_5::mbox_t        m_board;
        map_name_to_value_t m_map_name_to_value;
        Component           m_component;
        const size_t        m_max_rows = 6U;
        std::function<void(void)> m_f_update_screen;

        std::vector<Components> m_rows;

        void make_component();

        void add_new_variable(const return_name_t& name, const return_value_t& value);
    };
}

#endif //USB_IO_RETURNS_HPP