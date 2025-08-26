//
// Created by hugo on 13.06.25.
//

#ifndef LOGGER_HPP
#define LOGGER_HPP

#include <string_view>
#include <memory>

#include "ftxui/component/component_base.hpp"
#include "ftxui/component/screen_interactive.hpp"
#include "ftxui/component/component_options.hpp"

namespace gui {
    void log(std::string_view);

}

namespace gui::detail {
    using namespace ftxui;

    class Logger : public ComponentBase {

    public:
        Logger();

        Element OnRender() override;
        bool OnEvent(Event event) override;

        static Component make();

        static void AddMessage(std::string_view mess);

        static void SetScreen(ScreenInteractive* screen);

    private:
        static void UpdateScreen();
    };
}

#endif //LOGGER_HPP
