//
// Created by hugo on 14.06.25.
//

#ifndef WINDOW_PROGRAM_HPP
#define WINDOW_PROGRAM_HPP

#include <mutex>
#include <map>

#include "ftxui/component/component_base.hpp"

namespace gui {
    using namespace ftxui;

    class WindowPrograms : public ComponentBase {
    public:
        Element OnRender() override;

        static void Add(Element contents, std::string name);
        static void Delete(std::string_view name);
    public:
        static Component make();

        static Elements m_small_windows;
        static std::mutex m_mutex_data;
        static std::map<const std::string, size_t> m_name_to_place;

    };
}

#endif //WINDOW_PROGRAM_HPP
