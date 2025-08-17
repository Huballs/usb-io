//
// Created by hugo on 01.06.25.
//

#ifndef USB_COMMON_H
#define USB_COMMON_H

#include <stdexcept>
#include <format>

namespace usb {

    namespace detail {

        constexpr int ENDPOINT_BULK_SIZE = 64;

        constexpr unsigned char ENDPOINT_TRANSMIT_BIT   = 0x00U;
        constexpr unsigned char ENDPOINT_RECEIVE_BIT    = 0x80U;

        enum class transferType {
            RX
            , TX
        };

    }

    class usbError : public std::runtime_error {
    public:
        explicit usbError(const std::string &problem) : std::runtime_error(problem) {}
        virtual ~usbError() = default;
    };

    class usbInitError final : public usbError {
    public:
        explicit usbInitError(const std::string &problem) : usbError(problem) {}
        virtual ~usbInitError() = default;
    };

    class usbIOError final : public usbError {
    public:
        explicit usbIOError(const std::string &problem) : usbError(problem) {}
        virtual ~usbIOError() = default;
    };

}

#endif //USB_COMMON_H
