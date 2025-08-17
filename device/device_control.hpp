// //
// // Created by hugo on 15.06.25.
// //
//
// #include "../usb/usb.hpp"
// #include <queue>
// #include "proto.hpp"
// #include <map>
// #include <bits/this_thread_sleep.h>
//
// #ifndef DEVICECONTROL_HPP
// #define DEVICECONTROL_HPP
//
// namespace device {
//
//     struct gpio_state_t {
//         io_t io;
//         size_t value;
//         size_t function;
//
//         bool operator!=(const gpio_state_t& right) {
//             return (this->io != right.io) || (this->value != right.value) || (this->function != right.function);
//         }
//     };
//
//     constexpr size_t s_gpio_count = 28U;
//
//     class DeviceControl {
//
//     public:
//         DeviceControl(usb::Usb& usb_device, std::function<void(std::string_view)> f_log);
//
//         void set_gpio(size_t n, gpio_state_t gpio_state) noexcept;
//
//         void send_data_if_any() noexcept;
//
//
//         [[nodiscard]] bool last_comm_success() const noexcept {
//             return m_last_comm_success;
//         }
//
//         [[nodiscard]] bool state_changed() noexcept{
//             bool state_changed = m_state_changed;
//             m_state_changed = false;
//             return state_changed || has_new_data();
//         }
//
//         [[nodiscard]] bool data_recieved() const noexcept;
//
//         [[nodiscard]] gpio_state_t get_gpio(size_t n) const noexcept {
//             return m_gpio_states.at(n);
//         }
//
//         [[nodiscard]] bool has_device() noexcept;
//
//         void request_gpio() noexcept;
//
//         void send_script(std::string_view name, std::string_view script) noexcept;
//
//         [[nodiscard]] status_t status() const noexcept {
//             return m_status;
//         }
//
//         const std::string& script_name() const noexcept {
//             return m_script_name;
//         }
//
//         bool script_name_changed() noexcept {
//             bool temp = m_script_name_changed;
//             m_script_name_changed = false;
//             return temp;
//         }
//
//     private:
//
//         [[nodiscard]] bool has_new_data() noexcept;
//
//         [[nodiscard]] bool has_gpio_data() const noexcept;
//         [[nodiscard]] bool has_message() const noexcept;
//         [[nodiscard]] bool has_status() const noexcept;
//
//         void parse_gpio_data() noexcept;
//         void parse_message() noexcept;
//         void parse_status();
//
//         usb::Usb& m_usb_device;
//
//         using tx_buffer_t = std::vector<uint8_t>;
//         std::queue<tx_buffer_t> m_tx_data;
//
//         tx_buffer_t make_tx_buffer() {
//             tx_buffer_t buff;
//             buff.resize(m_usb_device.get_data_size());
//             return buff;
//         }
//
//         std::mutex m_mutex_tx_data;
//
//         bool m_are_we_sending = false;
//         bool m_last_comm_success = false;
//         bool m_state_changed = false;
//         bool m_buffer_has_data = false;
//         bool m_prev_device_status = false;
//         bool m_is_gpio_init = false;
//         bool m_script_name_changed = false;
//
//         std::array<gpio_state_t, s_gpio_count> m_gpio_states;
//
//         std::function<void(std::string_view)> m_log;
//
//         std::vector<uint8_t> m_rec_buffer;
//
//         std::string m_script_name;
//
//         status_t m_status;
//     };
//
// } // device
//
// #endif //DEVICECONTROL_HPP
