//
// Created by EDashkevich on 17.09.2025.
//
#include "ps_data.hpp"
#include <unordered_map>
#include "button.hpp"
#include <format>

namespace gui {

    using namespace ftxui;

    class PSComponent : public ComponentBase {
    public:

        PSComponent(std::string name)
            : m_name(std::move(name)) {

            refresh();
        }

        void voltage(float v) {
            m_voltage = v;
        }
        void current(float c) {
            m_current = c;
        }

        void over_current(bool oc) {
            m_over_current = oc;
        }

        void on_off(bool on) {
            m_on = on;
        }

        template<typename T>
        void set( power_supply_param_t param, T value) {
            switch (param) {
            case power_supply_param_t::VOLTAGE: voltage(value); break;
                case power_supply_param_t::CURRENT: current(value); break;
                case power_supply_param_t::OVERCURRENT: over_current(value); break;
                default: break;
            }
        }


        void refresh() {
            auto text = std::format(" {} {:.2f}V  {:.2f}mA ", m_name, m_voltage, m_current);

            make_component(text);

        }

    private:
        std::shared_ptr<Button> m_component;

        const std::string m_name;

        float m_voltage = 0.f;
        float m_current = 0.f;
        bool m_over_current = false;
        bool m_on = false;

        Color m_oc_colour = Color::RedLight;
        Color m_on_colour = Color::GreenYellow;
        Color m_off_colour = Color::GrayLight;

        Color m_current_colour;

        void make_component(const std::string& text) {
            m_component = std::make_shared<Button>(text);

            Color colour;

            if (m_on) {
                colour  = (m_over_current ? m_oc_colour : m_on_colour);
            } else {
                colour = m_off_colour;
            }

            m_component->set_colour(colour);

            DetachAllChildren();
            Add(m_component);
        }

    };


    PSData::PSData(context_t ctx, so_5::mbox_t board, Component& this_component)
        :  so_5::agent_t{std::move(ctx)}
        ,  m_board{std::move(board)}{
        make_component();
        this_component = m_component;
    }

    template<typename T>
    void PSData::set(power_supply_t ps, power_supply_param_t param, T value) {
        if (auto it = m_supply_to_component.find(ps); it != m_supply_to_component.end()) {
            it->second->set(param, value);
        }
    }

    void PSData::update() {
        for (auto& [ps, comp] : m_supply_to_component ) {
            comp->refresh();
        }
    }
    void PSData::update(power_supply_t ps) {
        if (auto it = m_supply_to_component.find(ps); it != m_supply_to_component.end()) {
            it->second->refresh();
        }
    }


    void PSData::make_component() {

        m_supply_to_component[power_supply_t::VDD] = std::make_shared<PSComponent>("VDD");
        m_supply_to_component[power_supply_t::V33] = std::make_shared<PSComponent>("3V3");
        m_supply_to_component[power_supply_t::VIO] = std::make_shared<PSComponent>("VIO");

        Components components;

        for (auto& [_, c] : m_supply_to_component) {
            components.emplace_back(c);
            components.push_back(
                Renderer([]() {
                    return separatorLight();
                    }
                )
            );
        }

        m_component = Container::Horizontal(components);

        // DetachAllChildren();
        // Add(m_component);
    }


}