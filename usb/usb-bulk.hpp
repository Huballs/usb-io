//
// Created by hugo on 16.08.25.
//

#ifndef USB_BULK_HPP
#define USB_BULK_HPP

#include <cstddef>
#include <array>
#include <cstdint>
#include <functional>
#include <memory>
#include <iostream>

#include "bulk-transfer.hpp"
#include "usb-v2.hpp"

namespace usb {

    struct settings_t {
        int vid;
        int pid;
        unsigned char ep_addr;
        int config;
        int interface;
        unsigned int tx_time_out_ms;
        unsigned int rx_time_out_ms;
    };

    using f_hotplug_t = std::function<void(void)>;

    #define UsbBulk_template_t template<size_t DATA_SIZE>

    UsbBulk_template_t
    class UsbBulk {

        using tx_transfer = detail::BulkTransfer<detail::direction_t::TX, DATA_SIZE>;
        using rx_transfer = detail::BulkTransfer<detail::direction_t::RX, DATA_SIZE>;
    public:

        using tx_transfer_data_t = typename tx_transfer::data_t;
        using rx_transfer_data_t = typename rx_transfer::data_t;

        using f_on_transmit_t = typename tx_transfer::f_on_transmit_t;
        using f_on_recieve_t = typename rx_transfer::f_on_recieve_t;

        UsbBulk(settings_t settings);

        status_t init() noexcept;

        void loop() noexcept;

        status_t transmit   (f_on_transmit_t on_finish, tx_transfer_data_t* data) noexcept;
        status_t recieve    (f_on_recieve_t on_recieve);

        void set_on_hotplug(f_hotplug_t f) noexcept;
        void set_on_hotunplug(f_hotplug_t f) noexcept;

        [[nodiscard]] bool has_device() const noexcept;

        void close_device() noexcept;

        [[nodiscard]] tx_transfer_data_t make_tx_data() const noexcept;
        [[nodiscard]] rx_transfer_data_t make_rx_data() const noexcept;

    private:
        settings_t m_settings;
        libusb_device_handle * m_dev_handle = nullptr;

    protected:
        status_t open_device(libusb_device *dev) noexcept;
        status_t open_device() noexcept;

        std::optional<f_hotplug_t> m_on_hotplug;
        std::optional<f_hotplug_t> m_on_hotunplug;

    };


    struct hot_plug_data_t {
        std::function<status_t(libusb_device *)> open_device;
        std::optional<f_hotplug_t>*  on_hotplug;
    };

    struct hot_unplug_data_t {
        std::function<void(void)> close_device;
        std::optional<f_hotplug_t>*  on_hotunplug;
    };

    static int LIBUSB_CALL hotplug_callback([[maybe_unused]] libusb_context *ctx, [[maybe_unused]] libusb_device *dev
                                            , [[maybe_unused]] libusb_hotplug_event event
                                            , [[maybe_unused]]  void *user_data) {

        auto usb = reinterpret_cast<hot_plug_data_t*>(user_data);

        usb->open_device(dev);

        if (*usb->on_hotplug) {
            usb->on_hotplug->value()();
        }

        return 0;
    }

    static int LIBUSB_CALL hotunplug_callback([[maybe_unused]] libusb_context *ctx, [[maybe_unused]] libusb_device *dev
                                                    , [[maybe_unused]] libusb_hotplug_event event
                                                    , [[maybe_unused]] void *user_data) {

        auto usb = reinterpret_cast<hot_unplug_data_t*>(user_data);

        usb->close_device();

        if (*usb->on_hotunplug) {
            usb->on_hotunplug->value()();
        }

        return 0;
    }

    UsbBulk_template_t
    UsbBulk<DATA_SIZE>::UsbBulk(settings_t settings)
        : m_settings(std::move(settings)) {}

    UsbBulk_template_t
    status_t UsbBulk<DATA_SIZE>::init() noexcept {

        constexpr int class_id = LIBUSB_HOTPLUG_MATCH_ANY;

        int rc = libusb_init(nullptr);

        if (LIBUSB_SUCCESS != rc) {
            return status_t::ERROR;
        }

        if (libusb_has_capability (LIBUSB_CAP_HAS_HOTPLUG) == 0) {
            libusb_exit (nullptr);
            return status_t::ERROR;
        }

        auto hot_plug_data = new hot_plug_data_t();
        hot_plug_data->open_device = [this](libusb_device* dev) {
            return this->open_device(dev);
        };

        hot_plug_data->on_hotplug = &this->m_on_hotplug;

        auto hot_unplug_data = new hot_unplug_data_t();
        hot_unplug_data->close_device = [this]() {
            this->close_device();
        };

        hot_unplug_data->on_hotunplug = &this->m_on_hotunplug;

        rc = libusb_hotplug_register_callback (nullptr, LIBUSB_HOTPLUG_EVENT_DEVICE_ARRIVED, 0, m_settings.vid,
            m_settings.pid, class_id, hotplug_callback, hot_plug_data, nullptr);
        if (LIBUSB_SUCCESS != rc) {
            libusb_exit (nullptr);
            delete hot_plug_data;
            return status_t::ERROR;
        }

        rc = libusb_hotplug_register_callback (nullptr, LIBUSB_HOTPLUG_EVENT_DEVICE_LEFT, 0, m_settings.vid,
            m_settings.pid,class_id, hotunplug_callback, hot_unplug_data, nullptr);
        if (LIBUSB_SUCCESS != rc) {
            libusb_exit (nullptr);
            delete hot_unplug_data;
            return status_t::ERROR;
        }

        this->open_device();

        return status_t::SUCCESS;
    }

    UsbBulk_template_t
    void UsbBulk<DATA_SIZE>::loop() noexcept {
        const int rc = libusb_handle_events (nullptr);

        if (LIBUSB_SUCCESS != rc) {
            std::cerr << "libusb_handle_events() failed: "
                        << libusb_strerror(static_cast<enum libusb_error>(rc)) << std::endl;
        }
    }

    UsbBulk_template_t
    status_t UsbBulk<DATA_SIZE>::open_device(libusb_device *dev) noexcept{
        int rc = libusb_open (dev, &m_dev_handle);

        if ((rc != LIBUSB_SUCCESS) || (m_dev_handle == nullptr)) {
            return status_t::ERROR;
        }

        return status_t::SUCCESS;
    }
    UsbBulk_template_t
    status_t UsbBulk<DATA_SIZE>::transmit   (typename tx_transfer::f_on_transmit_t on_finish, typename tx_transfer::data_t* data) noexcept {

        if (!has_device()) {
            return status_t::ERROR;
        }

        return tx_transfer::submit(data, m_dev_handle, m_settings.ep_addr, m_settings.tx_time_out_ms, on_finish);
    }
    UsbBulk_template_t
    status_t UsbBulk<DATA_SIZE>::recieve    (typename rx_transfer::f_on_recieve_t on_recieve) {
        if (!has_device()) {
            return status_t::ERROR;
        }

        return rx_transfer::submit(nullptr, m_dev_handle, m_settings.ep_addr, m_settings.rx_time_out_ms, on_recieve);
    }

    UsbBulk_template_t
    void UsbBulk<DATA_SIZE>::set_on_hotplug(f_hotplug_t f) noexcept {
        m_on_hotplug = f;
    }

    UsbBulk_template_t
    void UsbBulk<DATA_SIZE>::set_on_hotunplug(f_hotplug_t f) noexcept {
        m_on_hotunplug = f;
    }

    UsbBulk_template_t
    status_t UsbBulk<DATA_SIZE>::open_device() noexcept{
        close_device();
        // register_callback requires int as vid/pid but open device uin16_t
        m_dev_handle = libusb_open_device_with_vid_pid(nullptr
                                                    , static_cast<uint16_t>(m_settings.vid)
                                                    , static_cast<uint16_t>(m_settings.pid));

        if (m_dev_handle == nullptr) {
            return status_t::ERROR;
        }

        return status_t::SUCCESS;
    }

    UsbBulk_template_t
    void UsbBulk<DATA_SIZE>::close_device() noexcept {
        if (has_device()) {
            libusb_close (m_dev_handle);
            m_dev_handle = nullptr;
        }
    }

    UsbBulk_template_t
    bool UsbBulk<DATA_SIZE>::has_device() const noexcept {
        return m_dev_handle != nullptr;
    }

    UsbBulk_template_t
    [[nodiscard]] UsbBulk<DATA_SIZE>::tx_transfer_data_t UsbBulk<DATA_SIZE>::make_tx_data() const noexcept {
        return tx_transfer_data_t{};
    }
    UsbBulk_template_t
    [[nodiscard]] UsbBulk<DATA_SIZE>::rx_transfer_data_t UsbBulk<DATA_SIZE>::make_rx_data() const noexcept {
        return rx_transfer_data_t{};
    }
}

#endif //USB_BULK_HPP
