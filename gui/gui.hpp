//
// Created by hugo on 12.06.25.
//

#ifndef GUI_HPP
#define GUI_HPP

#include "logger.hpp"
#include "../device/device_control.hpp"

namespace gui {
    void run(device::DeviceControl& device_ctrl, std::atomic_bool& on_exit);
}

#endif //GUI_HPP
