//
// Created by hugo on 11.08.25.
//
//
// Created by hugo on 10.08.25.
//

#include "device_status.hpp"

#include <string.h>

namespace gui {
    using namespace ftxui;

    Color lua_status_colour(device::lua_status_t status) {

        using s = device::lua_status_t;

        switch (status) {
            case s::RUNNING: return Color::Blue; break;
            case s::FINISHED: return Color::Green; break;
            case s::INIT_ERROR: return Color::DarkRed; break;
            case s::INIT_SUCCESS: return Color::GreenLight; break;
            case s::PAUSED: return Color::Yellow; break;
            case s::SCRIPT_ERROR: return Color::Red; break;
            case s::STOPPED: return Color::GrayLight; break;
            default: return Color::White; break;
        }
    }

    std::string lua_status_text(device::lua_status_t status) {

        using s = device::lua_status_t;

        switch (status) {
            case s::RUNNING:        return "Running     "; break;
            case s::FINISHED:       return "Finished    "; break;
            case s::INIT_ERROR:     return "Init Error  "; break;
            case s::INIT_SUCCESS:   return "Init Success"; break;
            case s::PAUSED:         return "Paused      "; break;
            case s::SCRIPT_ERROR:   return "Script Error"; break;
            case s::STOPPED:        return "Stopped     "; break;
            default:                return "Unknown     "; break;
        }
    }

    static bool check_status(device::status_t& current_status) {
        static device::lua_status_t last_lua_status{};
        static bool last_script_loaded = false;
        static std::string last_name{};

        bool ret = false;

        if ((current_status.lua_status != last_lua_status)) {
            last_lua_status = current_status.lua_status;
            ret = true;
        }

        if (strnlen((const char*)current_status.script_name, 32U) == 32U) {
            strncpy(current_status.script_name, "Name Too Big", 32U);
            return true;
        }
        std::string new_name{current_status.script_name};

        if (new_name != last_name) {
            last_name = new_name;
            ret = true;
        }

        if (last_script_loaded != current_status.script_loaded) {
            last_script_loaded = current_status.script_loaded;
            ret = true;
        }

        return ret;
    }

    DeviceStatus::DeviceStatus(device::DeviceControl& device) : m_device(device) {
        make_element(m_device.status());
    }

    Element DeviceStatus::OnRender() {

        auto status = m_device.status();

        if (check_status(status)) {
            make_element(status);
        }

        return m_element;
    }


    void DeviceStatus::make_element(const device::status_t& current_status) {

        Decorator status_colour = color(lua_status_colour(current_status.lua_status));
        Decorator script_colour{};
        std::string script_name{current_status.script_name};

        if (script_name.empty() || !current_status.script_loaded) {
            script_name = "None";
            script_colour = color(Color::GrayLight);
        } else {
            script_colour = color(Color::White);
        }

        m_element = hbox(
            text("Lua Status: ")
            , text(lua_status_text(current_status.lua_status)) | status_colour
            , text(script_name) | script_colour
        );
    }

}

