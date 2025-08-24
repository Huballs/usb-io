//
// Created by hugo on 23.07.25.
//

#ifndef WINDOW_TABS_HPP
#define WINDOW_TABS_HPP
#include <ftxui/component/component.hpp>
#include <ftxui/component/screen_interactive.hpp>

#include "ftxui/component/component_base.hpp"
#include "../device/device_control.hpp"
#include "../fs/fs.hpp"
#include "../device/device_control.hpp"

namespace gui {

    using namespace ftxui;

    class WindowTabs : public ComponentBase {
    public:

        using f_send_script_t = std::function<void(std::string_view, std::string_view)>;

        WindowTabs(f_send_script_t f_send_script);

        Element OnRender() override;
        //
        // [[nodiscard]] bool OnEvent(Event ev) override;

        [[nodiscard]] bool Focusable() const override;

        void build_component();

        void add_programm_tab(const std::string& tab_name, fs::ItemConst fs_item);

        bool Active() const {
            return m_final_component->Active();
        }

        bool Focused() const {
            return m_final_component->Focused();
        }

        static Component make(f_send_script_t f) noexcept {
            return Make<WindowTabs>(f);
        }

    private:

        int m_selected = 0;
        std::vector<std::string> m_tabs_names;
        std::map<size_t, std::string> m_tabs_text_contents;
        //std::vector<Component> m_tabs_menu;
        //std::vector<Component> m_tabs;

        Component m_final_tabs_menu;
        Component m_final_tabs;
        Component m_final_component;
        Components m_tabs_contents;

        Element m_final_element;

        std::function<void(std::string_view, std::string_view)> m_f_send_script;

        void make_tabs_menu() noexcept;
        void make_tabs() noexcept ;
        void make_final() noexcept;

    };

}

#endif //WINDOW_TABS_HPP
