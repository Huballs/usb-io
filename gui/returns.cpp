//
// Created by hugo on 07.09.2025.
//

#include "returns.hpp"

#include <ftxui/component/component.hpp>
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
        make_component();
    }

    void ReturnValueElement::make_component() {
        m_element = text(m_value);
    }

    // --- ReturnComponent
    ReturnComponent::ReturnComponent(return_name_t name, std::shared_ptr<ReturnValueElement> ret_val)
        : m_name(std::move(name)), m_ret_val(std::move(ret_val)){
        make_component();
    }

    Element ReturnComponent::OnRender() {
        return ComponentBase::OnRender();
    }

    bool ReturnComponent::OnEvent(Event event) {
        return ComponentBase::OnEvent(event);
    }

    void ReturnComponent::make_component() {

        #define sep_empty Renderer([]() { \
            return separatorEmpty(); \
        })

        auto sep_line = Renderer([]() {
            return separatorDashed();
        });

        auto name = Renderer([&]() {
            return text(m_name);
        });

        m_component = Container::Horizontal({
            sep_empty
            , name
            , sep_empty
            , m_ret_val
            , sep_empty
            , sep_line
        });

        DetachAllChildren();
        Add(m_component);
    }



    // --- Returns
    void Returns::so_define_agent() {
        so_subscribe(m_board).event(&Returns::on_variable);
    }

    void Returns::on_variable(mhood_t<device::sig_variable> s) {

        auto it = m_map_name_to_value.find(s->name);

        if (it != m_map_name_to_value.end()) {
            it->second->value(s->var);
        } else {
            add_new_variable(s->name, s->var);
            make_component();
        }

        m_f_update_screen();
    }

    Element Returns::OnRender() {
        return ComponentBase::OnRender();
    }

    bool Returns::OnEvent(Event ev) {
        return ComponentBase::OnEvent(ev);
    }

    void Returns::add_new_variable(const return_name_t& name, const return_value_t& value) {
        auto value_el = std::make_shared<ReturnValueElement>(value);
        auto component = std::make_shared<ReturnComponent>(name, value_el);

        m_map_name_to_value.emplace(name, value_el);

        if (m_rows.empty()) {
            m_rows.push_back({component});
        } else {
            if (m_rows.back().size() >= m_max_rows) {
                m_rows.push_back({component});
            } else {
                m_rows.back().push_back(component);
            }
        }
    }

    void Returns::make_component() {

        Components vertical;

        for (auto& row : m_rows) {
            Component row_container = Container::Horizontal(row);
            vertical.push_back(row_container);
        }

        m_component = Container::Vertical(vertical) | frame | vscroll_indicator | flex;

        DetachAllChildren();
        Add(m_component);
    }

}
