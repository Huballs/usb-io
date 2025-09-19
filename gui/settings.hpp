//
// Created by EDashkevich on 19.09.2025.
//

#ifndef USB_IO_SETTINGS_HPP
#define USB_IO_SETTINGS_HPP
#include "ftxui/component/component_base.hpp"
#include "../device/device_control.hpp"
#include "ps_data.hpp"
#include <unordered_map>

namespace gui {
    using namespace ftxui;

    class PowerSupplySettings;

    class Settings : public so_5::agent_t {

        public:

        Settings(context_t ctx, so_5::mbox_t board, Component& this_component);

    private:

        Component m_component;
        so_5::mbox_t m_board;
        std::unordered_map<device::power_supply_t, std::shared_ptr<PowerSupplySettings>> m_supply_to_settings;

        void make_component();

    };


    class PowerSupplySettings : public ComponentBase {

    public:
        PowerSupplySettings(device::power_supply_t type);

        void set(device::power_supply_param_t param);

    private:

        Component m_component;
        std::string m_name;
        float m_set_voltage = 0.f;
        float m_set_current = 0.f;

        void make_component() noexcept;

    };
}

#endif // USB_IO_SETTINGS_HPP
