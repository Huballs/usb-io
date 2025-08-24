//
// Created by hugo on 10.08.25.
//

#ifndef DEVICE_STATUS_HPP
#define DEVICE_STATUS_HPP

#include "ftxui/component/component_base.hpp"
#include "../device/protocol.hpp"

namespace gui {
    using namespace ftxui;

    class DeviceStatus : public ComponentBase {
    public:
        DeviceStatus();

        Element OnRender() override;

        void make_element(const device::proto::status_t& current_status) noexcept;
    private:
        Element m_element;
    };

}

#endif //DEVICE_STATUS_HPP
