//
// Created by hugo on 15.06.25.
//

#include "device_control.hpp"
#include <cstring>
#include <ranges>
#include <utility>

namespace device {

    #define DATA_HEADER_CONST(BUFFER_) reinterpret_cast<const data_header_t*>(BUFFER_)
    #define DATA_HEADER(BUFFER_) reinterpret_cast<data_header_t*>(BUFFER_)

    DeviceControl::DeviceControl(usb::Usb& usb_device, std::function<void(std::string_view)> f_log)
        : m_usb_device(usb_device)
        , m_log(std::move(f_log)){

        m_rec_buffer.resize(m_usb_device.get_data_size());

        std::ranges::fill(m_gpio_states.begin(), m_gpio_states.end(), gpio_state_t{});
    }

    void DeviceControl::set_gpio(const size_t n, const gpio_state_t gpio_state) noexcept {

        data_header_t header{};
        header.type = data_type_t::GPIO;
        header.size = 1U;

        data_gpio_t data{};
        data.gpio_n = static_cast<uint8_t>(n);
        data.io = gpio_state.io;
        data.value = static_cast<uint8_t>(gpio_state.value);
        data.function = static_cast<uint8_t>(gpio_state.function);

        auto tx_data = make_tx_buffer();

        (void) memcpy(tx_data.data(),                         &header,    sizeof(data_header_t));
        (void) memcpy(tx_data.data() + sizeof(data_header_t), &data,      sizeof(data_gpio_t));

        std::lock_guard l(m_mutex_tx_data);
        m_tx_data.push(tx_data);
    }

    void DeviceControl::send_data_if_any() noexcept {

        static bool pop_data = false;

        if (pop_data && !m_tx_data.empty()) {
            std::lock_guard l(m_mutex_tx_data);
            m_tx_data.pop();
            pop_data = false;
        }

        if (m_tx_data.empty() || m_are_we_sending || !has_device())
            return;

        auto f_succ = [&](bool success) {
            m_last_comm_success = success;
            m_state_changed = true;
            m_are_we_sending = false;
            // std::lock_guard l(m_mutex_tx_data);
            // m_tx_data.pop();
            pop_data = true;
        };
        try {
            m_usb_device.submit_tx(m_tx_data.front(), m_tx_data.front().size(), f_succ);
            m_are_we_sending = true;
        } catch (usb::usbError& e) {
            m_last_comm_success = false;
            m_log(e.what());
        }
    }

    bool DeviceControl::has_gpio_data() const noexcept {
        auto data_header = DATA_HEADER_CONST(m_rec_buffer.data());
        return data_header->type == data_type_t::GPIO;
    }

    bool DeviceControl::has_message() const noexcept {
        auto data_header = DATA_HEADER_CONST(m_rec_buffer.data());
        return data_header->type == data_type_t::TEXT_MESSAGE;
    }

    bool DeviceControl::has_status() const noexcept {
        auto data_header = DATA_HEADER_CONST(m_rec_buffer.data());
        return data_header->type == data_type_t::STATUS;
    }

    [[nodiscard]] bool DeviceControl::data_recieved() const noexcept {
        return (m_usb_device.get_buffer().data_ready());
    }

    void DeviceControl::parse_gpio_data() noexcept {

        if (!has_gpio_data()) {
            return;
        }

        auto data = m_rec_buffer.data();

        auto data_header = DATA_HEADER_CONST(data);

        auto max_count_of_gpio = ((m_rec_buffer.size() - sizeof(data_header_t)) / sizeof(data_gpio_t));

        if (data_header->size >= max_count_of_gpio) {
            m_log("Broken GPIO data");
            return;
        }

        auto new_data_ptr = reinterpret_cast<const data_gpio_t*> (data + sizeof(data_header_t));

        for (auto i : std::ranges::views::iota(0U, data_header->size)) {

            if (new_data_ptr[i].gpio_n >= m_gpio_states.size()) {
                continue;
            }

            //m_log(std::format("parse new gpio {}, io {}, value {}", new_data_ptr[i].gpio_n, (int)new_data_ptr[i].io, new_data_ptr[i].value));
            m_gpio_states.at(new_data_ptr[i].gpio_n).io = new_data_ptr[i].io;
            m_gpio_states.at(new_data_ptr[i].gpio_n).value = new_data_ptr[i].value;
            m_gpio_states.at(new_data_ptr[i].gpio_n).function = new_data_ptr[i].function;
        }

        m_is_gpio_init = (m_is_gpio_init) || (data_header->size == s_gpio_count);

    }

    void DeviceControl::parse_message() noexcept {
        if (!has_message()) {
            return;
        }

        auto data = m_rec_buffer.data();

        auto data_header = DATA_HEADER_CONST(data);

        if (data_header->size > (m_rec_buffer.size() - sizeof(data_header_t))) {
            m_log("Broken text message");
            return;
        }

        *(data + sizeof(data_header_t) + data_header->size) = 0U;

        m_log(reinterpret_cast<const char *>(data + sizeof(data_header_t)));

    }

    void DeviceControl::parse_status() {
        m_status = *reinterpret_cast<status_t*>(m_rec_buffer.data() + sizeof(data_header_t));

    }

    [[nodiscard]] bool DeviceControl::has_device() noexcept {
        if (m_usb_device.has_device() && (m_prev_device_status == false)) {
            request_gpio();
            m_prev_device_status = true;
        } else if (!m_usb_device.has_device()) {
            m_prev_device_status = false;
        }
        return m_prev_device_status;
    }

    [[nodiscard]] bool DeviceControl::has_new_data() noexcept {

        if (m_usb_device.get_buffer().data_ready()) {
            m_usb_device.get_buffer().get_data(m_rec_buffer);
            m_state_changed = true;

            if (has_gpio_data()) {
                parse_gpio_data();
            } else if (has_message()) {
                parse_message();
            } else if (has_status()) {
                parse_status();
            }

            return true;
        }

        if (has_device() && !m_is_gpio_init && (m_tx_data.size() < 2U))
            this->request_gpio();

        return false;
    }

    void DeviceControl::request_gpio() noexcept {
        tx_buffer_t tx_data = make_tx_buffer();

        auto header = DATA_HEADER(tx_data.data());
        header->type = data_type_t::COMMAND;

        auto command = reinterpret_cast<command_t*>(tx_data.data() + sizeof(data_header_t));
        *command = command_t::GET_ALL_GPIO;

        std::lock_guard l(m_mutex_tx_data);
        m_tx_data.push(tx_data);
    }

    void DeviceControl::send_script(std::string_view name, std::string_view script) noexcept {

        const size_t data_size = m_usb_device.get_data_size() - sizeof(data_header_t);

        size_t start = 0U;

        while (start < script.size()) {
            tx_buffer_t tx_data = make_tx_buffer();

            auto tx_data_no_header = tx_data.begin() + sizeof(data_header_t);

            size_t end = std::min(start + data_size, script.size());

            data_header_t header;

            if ((start == 0U) && (script.size() > data_size)) {
                header.type = data_type_t::SCRIPT_DATA_START;
            } else if (start == 0U) {
                header.type = data_type_t::SCRIPT_DATA_START_END;
            } else if ((end-start) > data_size) {
                header.type = data_type_t::SCRIPT_DATA_N;
            } else {
                header.type = data_type_t::SCRIPT_DATA_END;
            }

            (void)std::copy_n(script.begin() + start, end-start, tx_data_no_header);
            header.size = end - start;
            *DATA_HEADER(tx_data.data()) = header;

            start = end;

            std::lock_guard l(m_mutex_tx_data);
            m_tx_data.push(std::move(tx_data));
        }

        tx_buffer_t tx_data = make_tx_buffer();

        (DATA_HEADER(tx_data.data()))->type = data_type_t::SCRIPT_NAME;

        strncpy((char*)tx_data.data() + sizeof(data_header_t), name.data(), std::min(name.size(), 32UL));

        std::lock_guard l(m_mutex_tx_data);
        m_tx_data.push(std::move(tx_data));

        m_script_name = name;
    }
} // device