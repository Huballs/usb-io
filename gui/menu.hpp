//
// Created by hugo on 03.08.25.
//

#ifndef MENU_HPP
#define MENU_HPP

#include <functional>
#include <ftxui/component/component.hpp>
#include <ftxui/component/component_base.hpp>
#include <ftxui/component/event.hpp>

namespace gui {

    using namespace ftxui;

    #define IS_MOUSE_CLICK(EV) (EV.is_mouse() && (EV.mouse().button == Mouse::Left) && (EV.mouse().motion == Mouse::Pressed) && this->Focused())

    class Menu : public ComponentBase {
    public:

        using MenuItem = std::pair<std::string, std::function<void(void)>>;

        Menu(std::initializer_list<MenuItem> items, MenuOption options) : m_options(std::move(options)) {
            for (auto& [name, f] : items) {
                m_names.push_back(name);
                m_functions.push_back(f);
            }

            make_menu();
        }

        // Element OnRender() override {
        //     return m_component->Render();
        // }

        bool OnEvent(Event ev) override {

            if (m_is_hover) {
                this->TakeFocus();
            }

            if(IS_MOUSE_CLICK(ev) && m_is_hover) {
                on_enter();
                return true;
            }

            return m_component->OnEvent(ev);
        }

        bool Focusable() const override {
            return true;
        }

        static Component make(std::initializer_list<MenuItem> items, MenuOption options) {
            return Make<Menu>(items, options);
        }

    private:

        MenuOption m_options;
        int m_selected;
        bool m_is_hover;
        Component m_component;

        void make_menu() noexcept {

            m_options.selected = &m_selected;
            m_options.on_enter = [this]() {on_enter();};
            m_options.entries = &m_names;

            auto menu = ftxui::Menu(m_options) | Hoverable(&m_is_hover);

            m_component = menu;

            DetachAllChildren();
            Add(m_component);
        }

        void on_enter() const noexcept {

            auto selected = static_cast<size_t>(m_selected);

            if (selected < m_functions.size()) {
                m_functions[selected]();
            }
        }

        std::vector<std::string> m_names;
        std::vector<std::function<void(void)>> m_functions;

    };
}

#endif //MENU_HPP
