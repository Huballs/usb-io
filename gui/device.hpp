//
// Created by hugo on 10.08.25.
//

#ifndef DEVICE_HPP
#define DEVICE_HPP

#include <ftxui/component/component.hpp>

#include "ftxui/component/component_base.hpp"
#include "../device/device_control.hpp"

namespace gui {

    using namespace ftxui;

    class Device : public ComponentBase {
    public:
        Device(device::DeviceControl& device) : m_device(device) {}
    private:
        device::DeviceControl& m_device;
    };
}

#endif //DEVICE_HPP
