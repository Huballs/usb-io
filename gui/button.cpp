//
// Created by hugo on 10.08.25.
//

#include "button.hpp"

namespace gui {

#define IS_MOUSE_CLICK(EV) (EV.is_mouse() && (EV.mouse().button == Mouse::Left) && (EV.mouse().motion == Mouse::Pressed) && this->Focused())

    void Button::on_enter(f_t f) {
        m_f_on_enter = std::move(f);
    }

    Element Button::OnRender() {
        Decorator colour = m_disabled ? color(m_disabled_colour) : color(m_colour);
        return m_renderer->Render() | colour;
    }

    bool Button::OnEvent(Event ev) {

        if (m_disabled) {
            return true;
        }

        if (m_is_hover) {
            this->TakeFocus();
        }

        if ((IS_MOUSE_CLICK(ev) && m_is_hover) || (ev == Event::Return)) {
            m_f_on_enter();
        }
        return m_renderer->OnEvent(ev);
    }

    bool Button::Focusable() const {
        return true;
    }

    const std::string& Button::text() const noexcept {
        return m_text;
    }

    void Button::text(std::string_view txt) noexcept {
        if (txt != m_text) {
            m_text = (txt);
            make_renderer();
        }
    }

    void Button::set_colour(Color color) {
        m_colour = color;
    }

    void Button::set_disabled_clour(Color color) {
        m_disabled_colour = color;
    }

    void Button::disable() noexcept{
        m_disabled = true;
    }

    bool Button::is_disabled() const noexcept{
        return m_disabled;
    }

    void Button::enable() noexcept {
        m_disabled = false;
    }

    void Button::make_renderer() noexcept {
        m_element = bold(ftxui::text(m_text));

        m_renderer = Renderer([this]() {

            if (m_border.has_value()) {
                return (Focused() ? m_element | inverted : m_element) | m_border.value();
            } else {
                return (Focused() ? m_element | inverted : m_element);
            }
        });

        m_renderer |= Hoverable(&m_is_hover);

        this->DetachAllChildren();
        this->Add(m_renderer);
    }


}