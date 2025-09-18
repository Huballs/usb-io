//
// Created by EDashkevich on 17.09.2025.
//

#ifndef USB_IO_PS_DATA_HPP
#define USB_IO_PS_DATA_HPP
#include "ftxui/component/component_base.hpp"
#include "../device/device_control.hpp"
#include "button.hpp"

namespace gui {

    using namespace ftxui;

    enum class power_supply_t {
        VIO
        , VDD
        , V33
    };

    enum class power_supply_param_t {
        VOLTAGE
        , CURRENT
        , OVERCURRENT
        , ONOFF
    };

    class PSComponent;

    class PSData :public so_5::agent_t {
    public:

        PSData(context_t ctx, so_5::mbox_t board, Component& this_component);

        template<typename T>
        void set(power_supply_t ps, power_supply_param_t param, T value);
        void update();
        void update(power_supply_t ps);
    private:

        so_5::mbox_t m_board;

        Component m_component;
        std::unordered_map<power_supply_t, std::shared_ptr<PSComponent>> m_supply_to_component;

        void make_component();
    };

}

#endif // USB_IO_PS_DATA_HPP
