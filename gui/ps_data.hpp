//
// Created by EDashkevich on 17.09.2025.
//

#ifndef USB_IO_PS_DATA_HPP
#define USB_IO_PS_DATA_HPP
#include "ftxui/component/component_base.hpp"
#include "../device/device_control.hpp"
#include "button.hpp"

namespace gui {

    using namespace ftxui;

    class PSData : public ComponentBase, so_5::agent_t {
        public:

    private:

        Component m_component;

        void make_component();
    };

}

#endif // USB_IO_PS_DATA_HPP
