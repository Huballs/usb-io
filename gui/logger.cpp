//
// Created by hugo on 13.06.25.
//

#include <mutex>
#include <string>
#include <string_view>

#include "ftxui/component/component.hpp"           // for Menu
#include "ftxui/component/component_options.hpp"   // for MenuOption
#include "ftxui/component/screen_interactive.hpp"  // for ScreenInteractive
#include "ftxui/dom/elements.hpp"

#include "logger.hpp"

namespace gui {
    void log(std::string_view mess) {
        detail::Logger::AddMessage(mess);
    }
}

namespace gui::detail {

    int                     m_selected{};
    bool                    m_is_hovered{};
    Component               m_component{};
    std::mutex              m_mutex{};
    MenuOption              m_menu_option{};
    ScreenInteractive*      m_screen{};
    std::vector<std::string> m_text{};
    bool m_was_changed{false};
    int m_last_text_size{0};

    std::string get_current_time(){

        auto now = std::chrono::system_clock::now();
        auto in_time_t = std::chrono::system_clock::to_time_t(now);

        std::stringstream ss;
        ss << std::put_time(std::localtime(&in_time_t), "[%H:%M:%S] ");

        return ss.str();
    }

    using namespace ftxui;

        Element Logger::OnRender() {


            m_selected = static_cast<int>(m_last_text_size - 1U) == m_selected ? m_text.size() - 1 : m_selected;

            m_last_text_size = m_text.size();

            auto element = m_component->Render() | vscroll_indicator  | frame;

            return window(text("Log"), element);
            //return text("hello");
        }

        bool Logger::OnEvent(Event event) {
            //if (m_is_hovered) {
                // if (event.is_mouse() && (event.mouse().button == Mouse::WheelDown)) {
                //     if (m_selected > 0) m_selected--;
                // } else if (event.is_mouse() && (event.mouse().button == Mouse::WheelUp)) {
                //     if (m_selected < (m_text.size() - 1U)) m_selected++;
                //}
            //}
                return m_component->OnEvent(event);
            //return false;
        }

        Component Logger::make() {

            m_menu_option.direction = Direction::Down;
            m_menu_option.selected = &m_selected;

            AddMessage("Start");

            return Make<Logger>();
        }

        void Logger::AddMessage(std::string_view mess) {

            std::lock_guard lk(m_mutex);

            if (m_text.size() > 50U) {
                m_text.erase(m_text.begin());
            }

            auto line = get_current_time();
            line += mess;

            m_text.push_back(std::move(line));

            m_menu_option.entries = m_text;
            m_component = Menu(m_menu_option) | Hoverable(&m_is_hovered);

            UpdateScreen();
        }

        void Logger::SetScreen(ScreenInteractive* screen) {
            m_screen = screen;
        }

        void Logger::UpdateScreen() {
            if (m_screen != nullptr) {
                m_screen->PostEvent(Event::Custom);
            }
        }

}