//
// Created by hugo on 10.08.25.
//

#ifndef DEVICE_STATUS_HPP
#define DEVICE_STATUS_HPP
#include "logger.hpp"
#include "ftxui/component/component_base.hpp"
#include "../device/device_control.hpp"

namespace gui {
    using namespace ftxui;

    class DeviceStatus : public ComponentBase {
    public:
        DeviceStatus(device::DeviceControl& device);

        Element OnRender() override;

    private:
        device::DeviceControl& m_device;
        Element m_element;
        device::status_t m_last_status;

        void make_element(const device::status_t& current_status);
    };

}

#endif //DEVICE_STATUS_HPP
