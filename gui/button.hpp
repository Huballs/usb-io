//
// Created by hugo on 01.08.25.
//

#ifndef BUTTON_HPP
#define BUTTON_HPP

#include <ftxui/component/component.hpp>
#include <ftxui/component/event.hpp>

#include "button.hpp"
#include "ftxui/component/component_base.hpp"

namespace gui {

using namespace ftxui;

class Button : public ComponentBase {
public:

    using f_t = std::function<void(void)>;
    using border_t = std::function<Element(Element)>;

    Button(std::string_view text, std::optional<border_t> border = std::nullopt)
        : m_text(text), m_border(std::move(border)) {
        make_renderer();
    }

    void on_enter(f_t f);

    Element OnRender() override;

    bool OnEvent(Event ev) override;

    bool Focusable() const override;

    const std::string& text() const noexcept;

    void text(std::string_view txt) noexcept;

    void set_colour(Color color);
    void set_disabled_clour(Color color);

    void disable() noexcept;
    bool is_disabled() const noexcept;
    void enable() noexcept;

private:

    void make_renderer() noexcept;

    std::string m_text;
    f_t m_f_on_enter;
    Element m_element;
    Component m_renderer;
    bool m_is_hover;
    std::optional<border_t> m_border;
    bool m_disabled = false;
    Color m_disabled_colour{Color::GrayLight};
    Color m_colour{Color::White};
};
}

#endif //BUTTON_HPP
