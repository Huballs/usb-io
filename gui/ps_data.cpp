//
// Created by EDashkevich on 17.09.2025.
//
#include "ps_data.hpp"
#include <unordered_map>

namespace gui {

    using namespace ftxui;

    class PSComponent : public ComponentBase {
    public:

        PSComponent(std::string name) : m_name(std::move(name)) {}

        void voltage(float v) {
            m_voltage = v;
        }
        void current(float c) {
            m_current = c;
        }

        void over_current(bool oc) {
            m_over_current = oc;
        }

    private:
        Component m_component;

        const std::string m_name;
        float m_voltage = 0.f;
        float m_current = 0.f;
        bool m_over_current = false;

        Color m_oc_colour = Color::RedLight;
        Color m_on_colour = Color::GreenYellow;
        Color m_off_colour = Color::GrayLight;

    };

    void PSData::make_component() {
        m_component = Container::Horizontal({

        });
    }


}