//
// Created by hugo on 07.09.2025.
//

#include "returns.hpp"

#include <ftxui/component/event.hpp>

namespace gui {

    using namespace ftxui;

    // --- ReturnValue
    ReturnValueElement::ReturnValueElement(return_value_t value) : m_value(std::move(value)){
        make_component();
    }

    Element ReturnValueElement::OnRender() {
        return m_element;
    }

    bool ReturnValueElement::OnEvent(Event event) {
        return ComponentBase::OnEvent(event);
    }

    void ReturnValueElement::value(return_value_t value) {
        m_value = std::move(value);
    }

    void ReturnValueElement::make_component() {
        m_element = text(m_value);
    }


    // --- Returns
    void Returns::make_component() {

    }

}
