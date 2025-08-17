//
// Created by hugo on 12.08.25.
//

#ifndef CORE_CONTROL_HPP
#define CORE_CONTROL_HPP

namespace gui {
#include "logger.hpp"
#include "ftxui/component/component_base.hpp"
#include "../device/device_control.hpp"

using namespace ftxui;

class CoreControl : public ComponentBase {
public:
    CoreControl(device::DeviceControl& device);
private:

    void make_component();

    device::DeviceControl& m_device;
    device::status_t m_last_status;
};

}

#endif //CORE_CONTROL_HPP
