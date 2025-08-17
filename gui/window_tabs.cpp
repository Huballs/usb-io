//
// Created by hugo on 28.07.25.
//

#include <ftxui/component/event.hpp>
#include "window_tabs.hpp"
#include "button.hpp"

namespace gui {

using namespace ftxui;

class TabProgramContent : public ComponentBase {
public:
    TabProgramContent(std::string text) : m_text(std::move(text)) {
        build_component();
    }

    bool OnEvent(Event ev) override {

        if (m_is_hover) {
            if (ev.is_mouse()) {
                if (ev.mouse().button == Mouse::WheelUp) {
                    set_prev_line_pos();
                    return true;
                }
                else if (ev.mouse().button == Mouse::WheelDown) {
                    set_next_line_pos();
                    return true;
                }
            }
        }

        return m_final_component->OnEvent(ev);
    }

    // Element OnRender() override {
    //     auto elem = m_final_component->Render();
    //
    //     if (Focused())
    //         return  elem | inverted;
    //     return elem ;
    // }

    std::string_view text() const noexcept {
        return m_text;
    }

    bool Focusable() const override {
        return true;
    }

private:
    Component m_final_component{};
    std::string m_text{};
    bool m_is_hover{};
    int m_line{};
    int m_cursor_pos{0};
    size_t m_line_pos{0U};

    void build_component() noexcept {
        auto options = InputOption::Default();
        options.cursor_position = &m_cursor_pos;

        auto input = Input(&m_text, options) | vscroll_indicator | hscroll_indicator | frame  | size(WIDTH, LESS_THAN, 400) | flex_shrink;

        m_final_component = input;

        m_final_component |= Hoverable(&m_is_hover);

        DetachAllChildren();
        Add(m_final_component);

    }

    bool set_next_line_pos() {

        auto pos = m_text.find('\n', m_cursor_pos );

        if (pos < (m_text.size() - 1U) ){
            m_cursor_pos = pos + 1U;
            return true;
        }
        return false;
    }

    bool set_prev_line_pos() {
        if (m_cursor_pos == 0)
            return false;

        auto pos = m_text.rfind('\n', m_cursor_pos - 1);

        if (pos < m_text.size()) {
            m_cursor_pos = pos;
            return true;
        }

        return false;
    }
};

class TabProgramm : public ComponentBase {
public:
    TabProgramm(fs::ItemConst file, size_t index
            , std::function<void(size_t)> f_close_tab
            , std::function<void(std::string_view)> f_send)
        : m_file(file), m_index(index)
        , m_f_close_tab(std::move(f_close_tab))
        , m_f_send(std::move(f_send)) {

        build_component();
    }

    // bool OnEvent(Event ev) override {
    //     return m_final_component->OnEvent(ev);
    // }

    // Element OnRender() override {
    //     return m_final_component->Render();
    // }

    bool Focusable() const override {
        return true;
    }

private:
    Component m_final_component;
    std::shared_ptr<TabProgramContent> m_content;
    fs::ItemConst m_file;
    bool m_is_hover;
    size_t m_index;
    std::function<void(size_t)> m_f_close_tab;
    std::function<void(std::string_view)> m_f_send;

    void build_component() noexcept {
        m_content = std::make_shared<TabProgramContent>(m_file->read_file());

        auto button_save = std::make_shared<Button>("Save");
        auto button_send = std::make_shared<Button>("Send");
        auto button_close = std::make_shared<Button>("Close");

        button_save->on_enter([this]() {
           m_file->write_file(m_content->text());
        });

        button_close->on_enter([this]() {
           m_f_close_tab(m_index);
        });

        button_send->on_enter([this]() {
            m_f_send(m_content->text());
        });

        auto button_send_renderer = Renderer(button_send, [button_send, this]() {
            std::string text{"Send ("};
            text += std::to_string(m_content->text().size());
            text += ')';
            button_send->text(text);
            return button_send->OnRender();
        });

        #define sep Renderer([]() { \
            return ftxui::separatorEmpty(); \
        })

        m_final_component = Container::Vertical({
            Container::Horizontal({button_close, sep, button_save, sep, button_send_renderer, sep})
            ,   sep
            , m_content
        });

        DetachAllChildren();
        Add(m_final_component);
    }
};

WindowTabs::WindowTabs(device::DeviceControl& device) : m_device(device) {
    build_component();
}

Element WindowTabs::OnRender() {
    return m_final_component->Render() | border/*| size(HEIGHT, LESS_THAN, m_screen.dimy() * 0.5f)*/;
}

// bool WindowTabs::OnEvent(Event ev) {
//     return m_final_component->OnEvent(ev);
// }

bool WindowTabs::Focusable() const {
    return m_final_component->Focusable();
}

void WindowTabs::build_component() {
    make_tabs_menu();
    make_tabs();
    make_final();
}

void WindowTabs::add_programm_tab(const std::string& tab_name, fs::ItemConst fs_item) {

    auto close_tab = [this](size_t index) {
        m_tabs_names.erase(m_tabs_names.begin() + index);
        m_tabs_contents.erase(m_tabs_contents.begin() + index);
        build_component();
    };

    auto send_script = [this, fs_item](std::string_view text) {
        m_device.send_script(fs_item->name(), text);
    };

    auto content = std::make_shared<TabProgramm>(fs_item, m_tabs_names.size(), close_tab, send_script);

    m_tabs_names.push_back(tab_name);
    m_tabs_contents.push_back(content  | xflex);

    build_component();
}

void WindowTabs::make_tabs_menu() noexcept {
    m_final_tabs_menu = Menu(m_tabs_names, &m_selected);
}

void WindowTabs::make_tabs() noexcept {
    m_final_tabs = Container::Tab(m_tabs_contents, &m_selected);
}

void WindowTabs::make_final() noexcept {
    auto s = Renderer([]() {
        return separator();
    });

    m_final_component = Container::Horizontal({m_final_tabs_menu, s, m_final_tabs | xflex});

    this->DetachAllChildren();

    this->Add(m_final_component);
}

}
