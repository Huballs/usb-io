//
// Created by EDashkevich on 19.09.2025.
//

#include "settings.hpp"
#include "button.hpp"

namespace gui {
    using namespace ftxui;

    std::string type_to_name(device::power_supply_t type) {
        switch (type) {
        case device::power_supply_t::VDD: return "Vdd";
        case device::power_supply_t::VIO: return "Vio";
        case device::power_supply_t::V33: return "3V3";
        }

        return "Unknown";
    }

    void Settings::make_component() {

        m_supply_to_settings[device::power_supply_t::V33]
            = std::make_shared<PowerSupplySettings>(device::power_supply_t::V33);

        m_supply_to_settings[device::power_supply_t::VIO]
            = std::make_shared<PowerSupplySettings>(device::power_supply_t::VIO);

        m_supply_to_settings[device::power_supply_t::VDD]
            = std::make_shared<PowerSupplySettings>(device::power_supply_t::VDD);

        Components horiz {
            m_supply_to_settings[device::power_supply_t::V33]
            , m_supply_to_settings[device::power_supply_t::VIO]
            , m_supply_to_settings[device::power_supply_t::VDD]
        };
        static int s = 0;
        m_component = Container::Horizontal(horiz, &s);
    }

    PowerSupplySettings::PowerSupplySettings(device::power_supply_t type) {
            m_name = type_to_name(type);
            make_component();
        }

        void PowerSupplySettings::set(device::power_supply_param_t param) {
            switch (param) {
            case device::power_supply_param_t::VOLTAGE: m_set_voltage = true; break;
            case device::power_supply_param_t::CURRENT: m_set_voltage = true; break;
            default: m_set_voltage = 0.f; break;
            }
        }

        void PowerSupplySettings::make_component() noexcept {
            auto button = std::make_shared<Button>("Set");

            InputOption opt_v {
                .content = std::format("{:.2f}", m_set_voltage)
                , .multiline = false
            };

            InputOption opt_i {
                .content = std::format("{:.2f}", m_set_current)
                , .multiline = false
            };

            Component voltage = Container::Horizontal({
                Input(opt_v)
                , Renderer([](){return text("V");})
            });

            Component current = Container::Horizontal({
                Input(opt_i)
                , Renderer([](){return text("mA");})
            });

            Component inner = Container::Vertical({
                voltage
                , current
                , button
            });

            WindowOptions window_options{
                .inner = inner
                , .title = &m_name
                , .left = 0
                , .top = 0
                , .width = 12
                , .height = 5
                , .resize_left = false
                , .resize_right = false
                , .resize_top = false
                , .resize_down = false
            };

            m_component = Window(window_options) | flex_shrink;

            DetachAllChildren();
            Add(m_component);
        };

    Settings::Settings(context_t ctx, so_5::mbox_t board, Component& this_component)
            : so_5::agent_t(std::move(ctx)), m_board(std::move(board)) {
        make_component();
        this_component = m_component;
    }
} // namespace gui
