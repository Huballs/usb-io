//
// Created by hugo on 14.06.25.
//

#include "window_program.hpp"

#include <ftxui/component/component.hpp>
#include <ftxui/component/event.hpp>
#include <utility>

namespace gui {

        Elements                                WindowPrograms::m_small_windows{};
        std::mutex                              WindowPrograms::m_mutex_data{};
        std::map<const std::string, size_t>     WindowPrograms::m_name_to_place{};

        Element WindowPrograms::OnRender() {
                std::lock_guard lk(m_mutex_data);

                return window(text("Programs"), hbox((m_small_windows)) | focusPosition(0, 1) );
        }

        void WindowPrograms::Add(Element contents, std::string name) {
                std::lock_guard lk(m_mutex_data);

                (void)m_name_to_place.emplace(name, m_small_windows.size());

                m_small_windows.push_back(
                        window(text(std::move(name)), std::move(contents)
                                )
                        );
        }

        void WindowPrograms::Delete(std::string_view name_to_delete) {
                std::lock_guard lk(m_mutex_data);

                auto it = std::find_if(m_name_to_place.begin(), m_name_to_place.end()
                        , [name_to_delete](std::pair<const std::string&, size_t> p)
                                {
                                return p.first == name_to_delete;
                                });

                if (it != m_name_to_place.end()) {
                        (void)m_small_windows.erase(m_small_windows.begin() + it->second);
                        (void)m_name_to_place.erase(it);
                }
        }

        Component WindowPrograms::make() {
                return Make<WindowPrograms>();
        }

}
