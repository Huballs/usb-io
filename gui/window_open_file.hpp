//
// Created by hugo on 19.07.25.
//

#ifndef WINDOW_OPEN_FILE_HPP
#define WINDOW_OPEN_FILE_HPP

#include "logger.hpp"
#include "ftxui/component/component_base.hpp"
#include "../fs/fs.hpp"
#include "window_tabs.hpp"

namespace gui {

    using namespace ftxui;

    #define IS_MOUSE_CLICK(EV) (EV.is_mouse() && (EV.mouse().button == Mouse::Left) && (EV.mouse().motion == Mouse::Pressed) && this->Focused())

    class WindowOpenFile : public ComponentBase  {

    public:
        WindowOpenFile(fs::Fs& fs, std::shared_ptr<WindowTabs> tabs, bool* on_escape = nullptr)
            : m_fs(fs), m_tabs(std::move(tabs)), m_on_escape(on_escape) {

            m_fs.fill();
            m_current_item = m_fs.top_level();
            m_list = create_list();
        }

        Element OnRender() override {
            return m_list->OnRender() | vscroll_indicator | frame | border | size(HEIGHT, EQUAL, 15);
        }

        Component create_list() noexcept {

            m_title = text(m_current_item->path()) | border;

            std::vector<std::string> res;
            for (auto& item : m_current_item->contents()) {
                res.push_back((item->name()));
            }

            auto m = ftxui::Menu(res, &m_selected);
            m |= Hoverable(&m_hover);
            return m;
        }

        static Component make(fs::Fs& fs, std::shared_ptr<WindowTabs> tabs,  bool* on_escape) {
            return Make<WindowOpenFile>(fs, std::move(tabs), on_escape);
        }

        bool Focusable() const override {
            return true;
        }

        bool OnEvent(Event ev) override {

            if (m_hover) {
                this->TakeFocus();
            }

            bool pressed = (ev == Event::Return)
                || (IS_MOUSE_CLICK(ev) && m_hover);

            bool is_handled = m_list->OnEvent(ev);

            int contents_size = static_cast<int>(m_current_item->contents().size());

            if ( pressed && (contents_size > m_selected)) {
                auto selected = m_current_item->contents().at(m_selected);

                if (selected->type() == fs::item_t::FILE) {
                    m_tabs->add_programm_tab(selected->name(), selected);
                    this->close();
                } else {
                    m_current_item = selected;
                    m_list = create_list();
                }
            } else if ((ev == Event::Backspace)) {
                go_back();
            } else if ((ev == Event::Escape) && m_on_escape) {
                *m_on_escape = false;
            }

            return is_handled;
        }

        void go_back() {
            if (m_current_item->parent()) {
                m_current_item = m_current_item->parent();
                m_list = create_list();
            }
        }

        [[nodiscard]] Element title() const noexcept {
            return m_title;
        }

        void close() noexcept {
            if (m_on_escape) {
                *m_on_escape = false;
            }
        }

    private:
        fs::Fs& m_fs;
        std::shared_ptr<WindowTabs> m_tabs;
        Component m_list;
        Element m_title;
        fs::ItemConst m_current_item;
        bool m_hover = false;
        int m_selected;
        bool* m_on_escape = nullptr;
    };

    class WindowOpenFileTitle : public ComponentBase {
    public:
        WindowOpenFileTitle(WindowOpenFile& w_open) : m_w_open(w_open) {}

        Element OnRender() override {
            return m_w_open.title() | flex;
        }

        static Component make(WindowOpenFile& w_open) {
            return Make<WindowOpenFileTitle>(w_open);
        }
    private:
        WindowOpenFile& m_w_open;
    };

    class WindowOpenFileClose : public ComponentBase {
    public:
        WindowOpenFileClose(WindowOpenFile& w_open)
            : m_w_open(w_open){

            m_component = Renderer([this]() {
                static auto t = text("x");
                return (Focused() ? t | inverted : t) | border;
            });

            m_component |= Hoverable(&m_hover);
        }

        Element OnRender() override {
            return m_component->OnRender();
        }

        bool OnEvent(Event ev) override {
            if (m_hover) {
                this->TakeFocus();
            }

            if (IS_MOUSE_CLICK(ev)) {
                m_w_open.close();
            }

            return m_component->OnEvent(ev);
        }

        bool Focusable() const {
            return true;
        }

        static Component make(WindowOpenFile& w_open) {
            return Make<WindowOpenFileClose>(w_open);
        }
    private:
        WindowOpenFile& m_w_open;
        Component m_component;
        bool m_hover = false;
    };

    class WindowOpenFileBackButton : public ComponentBase {
    public:
        WindowOpenFileBackButton(WindowOpenFile& w_open)
            : m_w_open(w_open) {
            m_element = text("<") ;
            m_component = Renderer([this]() {
               return (Focused() ? m_element | inverted : m_element) | border;
            });
            m_component |= Hoverable(&m_hover);
        }

        Element OnRender() override {
            return m_component->OnRender();
        }

        bool OnEvent(Event ev) override {

            if (m_hover) {
                this->TakeFocus();
            }

            if (IS_MOUSE_CLICK(ev)) {
                m_w_open.go_back();
            }
            return m_component->OnEvent(ev);
        }

        bool Focusable() const {
            return true;
        }

        static Component make(WindowOpenFile& w_open) {
            return Make<WindowOpenFileBackButton>(w_open);
        }
    private:
        WindowOpenFile& m_w_open;
        Element m_element;
        Component m_component;
        bool m_hover = false;
    };

}

#endif //WINDOW_OPEN_FILE_HPP
