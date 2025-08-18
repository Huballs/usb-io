// //
// // Created by hugo on 15.06.25.
// //

#ifndef DEVICECONTROL_HPP
#define DEVICECONTROL_HPP

#include <assert.h>
#include <ranges>
#include <so_5/all.hpp>
#include "../usb/usb-bulk-agent.hpp"
#include "protocol.hpp"

namespace device {

    constexpr size_t s_gpio_count = 28U;

    struct gpio_state_t {
        proto::io_t io;
        size_t value;
        size_t function;

        bool operator!=(const gpio_state_t& right) {
            return (this->io != right.io) || (this->value != right.value) || (this->function != right.function);
        }
    };

    struct sig_connected{};
    struct sig_disconnected{};
    struct sig_status{proto::status_t status;};
    struct sig_gpio_set{size_t n; gpio_state_t state;};
    struct sig_gpio_new_state{size_t n; gpio_state_t state;};
    struct sig_send_script{std::string_view name; std::string_view script;};

    using f_log_t = std::function<void(std::string_view)>;

    #define DeviceTemplateParams DATA_SIZE, DATA_T
    #define DeviceTemplate template<size_t DATA_SIZE, typename DATA_T>

    DeviceTemplate
    class Device final : public so_5::agent_t {

    public:
        Device(context_t ctx, so_5::mbox_t board, f_log_t logger)
           :  so_5::agent_t{std::move(ctx)}
            ,  m_board{std::move(board)}
            , m_logger(logger)
        {}

        void            set_gpio(size_t n, gpio_state_t state) noexcept;
        gpio_state_t    get_gpio(size_t n) const noexcept;

        void request_gpio() noexcept;
        void send_script(std::string_view name, std::string_view script) noexcept;

        void so_define_agent() override;

    private:

        void on_recieve(mhood_t<usb::sig_recieve_res<DATA_SIZE>>) noexcept;
        void on_error(mhood_t<usb::sig_error_mes>) noexcept;

        void send(proto::data_gpio_t gpio_data) noexcept;
        void send(const DATA_T& data) noexcept;

        void on_connected(mhood_t<usb::sig_hotplug>) noexcept;
        void on_disconnected(mhood_t<usb::sig_hotunplug>) noexcept;

        void parse_gpio(const DATA_T& data) noexcept;
        void parse_text_message(const DATA_T& data) noexcept;

        void on_script(mhood_t<sig_send_script>) noexcept;
        void on_gpio_set(mhood_t<sig_gpio_set>) noexcept;

        f_log_t         m_logger;
        so_5::mbox_t    m_board;
        bool            m_conected {false};
        proto::status_t m_status;
        std::array<gpio_state_t, s_gpio_count> m_gpio_states;
    };

    namespace detail {

        proto::data_gpio_t              gpio_state_to_data(const size_t n, const gpio_state_t gpio_state) noexcept;
        std::pair<size_t, gpio_state_t> gpio_data_to_state(proto::data_gpio_t data) noexcept;

        template<typename T>
        constexpr proto::data_header_t* cast_to_data_header(T& array) noexcept {
            return reinterpret_cast<proto::data_header_t*>(array.data());
        }

        template<typename T>
        constexpr const proto::data_header_t* cast_to_data_header(const T& array) noexcept {
            return reinterpret_cast<const proto::data_header_t*>(array.data());
        }

        template<typename T> // starts from zero
        constexpr proto::command_t* cast_to_data_command(T& array) noexcept {
            return reinterpret_cast<proto::command_t*>(
                array.data() + sizeof(proto::data_header_t)
            );
        }

        template<typename T> // starts from zero
        constexpr proto::data_gpio_t* cast_to_data_gpio(T& array, size_t postition) noexcept {
            return reinterpret_cast<proto::data_gpio_t*>(
                array.data() + sizeof(proto::data_header_t) + sizeof(proto::data_gpio_t) * postition
            );
        }

        template<typename T>
        constexpr proto::status_t* cast_to_data_status(T& array) noexcept {
            return reinterpret_cast<proto::status_t*>(
                array.data() + sizeof(proto::data_header_t)
            );
        }

        template<typename T>
        constexpr uint8_t* cast_to_data(T& array) noexcept {
            return array.data() + sizeof(proto::data_header_t);
        }

    }

    #define DeviceDef(RET)  DeviceTemplate\
                            RET Device<DeviceTemplateParams>::

    DeviceDef(void)  so_define_agent() {
        so_subscribe_self().event(&Device::on_script);
    }

    DeviceDef(void) send(proto::data_gpio_t gpio_data) noexcept {

        using namespace detail;

        DATA_T data{};

        cast_to_data_header(data)->type = proto::data_type_t::GPIO;
        cast_to_data_header(data)->size = 1U;

        cast_to_data_gpio(data, 0U) = gpio_data;

        send(data);
    }

    DeviceDef(void) send(const DATA_T& data) noexcept {
        so_5::send<usb::sig_transmit_data<DATA_SIZE>>(m_board, data);
    }

    DeviceDef(void) set_gpio(size_t n, gpio_state_t state) noexcept {

        assert(n < s_gpio_count);

        proto::data_gpio_t gpio = detail::gpio_state_to_data(n, state);

        send(gpio);
    }

     DeviceDef(gpio_state_t) get_gpio(size_t n) const noexcept {
        assert(n < s_gpio_count);

        return m_gpio_states[n];
    }

    DeviceDef(void) request_gpio() noexcept {
        DATA_T data;

        detail::cast_to_data_header(data)->type = proto::data_type_t::COMMAND;
        *detail::cast_to_data_command(data) = proto::command_t::GET_ALL_GPIO;

        send(data);
    }

    DeviceDef(void) send_script(std::string_view name, std::string_view script) noexcept {

        size_t data_size = DATA_SIZE - sizeof(proto::data_header_t);

        size_t start = 0U;

        while (start < script.size()) {
            DATA_T data;

            auto tx_data_no_header = detail::cast_to_data(data);

            size_t end = std::min(start + data_size, script.size());

            auto header = detail::cast_to_data_header(data);

            if ((start == 0U) && (script.size() > data_size)) {
                header->type = proto::data_type_t::SCRIPT_DATA_START;
            } else if (start == 0U) {
                header->type = proto::data_type_t::SCRIPT_DATA_START_END;
            } else if ((end-start) > data_size) {
                header->type = proto::data_type_t::SCRIPT_DATA_N;
            } else {
                header->type = proto::data_type_t::SCRIPT_DATA_END;
            }

            (void)std::copy_n(script.begin() + start, end-start, tx_data_no_header);
            header->size = end - start;

            start = end;

            send(data);
        }

        DATA_T data;

        detail::cast_to_data_header(data)->type = proto::data_type_t::SCRIPT_NAME;

        strncpy(reinterpret_cast<char*>(detail::cast_to_data(data)), name.data()
            , std::min(static_cast<size_t>(name.size()), (size_t)32U));

        send(data);
    }

    DeviceDef(void) on_recieve(mhood_t<usb::sig_recieve_res<DATA_SIZE>> result) noexcept {

        if (result->status() == usb::status_t::SUCCESS) {

            auto header = detail::cast_to_data_header(result->data);

            switch (header->type) {
                case proto::data_type_t::STATUS: {
                    m_status = *detail::cast_to_data_status(result->data);
                    so_5::send<sig_status>(m_board, m_status);
                    break;
                }
                case proto::data_type_t::GPIO: {
                    parse_gpio(result->data);
                    break;
                }
                case proto::data_type_t::TEXT_MESSAGE: {
                    parse_text_message(result->data());
                }
                default: break;
            }
        }
    }

    DeviceDef(void) on_error(mhood_t<usb::sig_error_mes> res) noexcept {
        switch (res->error) {
            case usb::usb_error_t::INIT: m_logger("Usb Init Error"); break;
            case usb::usb_error_t::TRANSMIT: m_logger("Usb Transmit Error"); break;
            default: m_logger("Usb Unknown error");
        }
    }

    DeviceDef(void) on_connected(mhood_t<usb::sig_hotplug>) noexcept {
        m_conected = true;
        m_logger("Device attached");
        so_5::send<sig_connected>(m_board);
        request_gpio();
    }

    DeviceDef(void) on_disconnected(mhood_t<usb::sig_hotunplug>) noexcept {
        m_conected = false;
        m_logger("Device dettached");
        so_5::send<sig_disconnected>(m_board);
    }

    DeviceDef(void) parse_gpio(const DATA_T& data) noexcept {

        auto header = detail::cast_to_data_header(data);

        assert(header->type == proto::data_type_t::GPIO);

        if ((header->size() > s_gpio_count)
            || (header->size() * sizeof(proto::data_gpio_t) + sizeof(proto::data_header_t)) > DATA_SIZE
        ) {
            m_logger("GPIO packet size too big");
            return;
        }

        for (auto i : std::ranges::views::iota(0U, header->size)) {
            auto gpio_data = detail::cast_to_data_gpio(data, i);

            auto [n, gpio_state] = detail::gpio_data_to_state(*gpio_data);

            if (n >= m_gpio_states.size()) {
                m_logger("GPIO incorrect N");
                continue;
            }

            m_gpio_states.at(n) = gpio_state;

            so_5::send<sig_gpio_new_state>(m_board, n, gpio_state);
        }
    }

    DeviceDef(void)  parse_text_message(const DATA_T& data) noexcept {
        auto mess_ptr = detail::cast_to_data(data.data());

        auto max_size =  DATA_SIZE - sizeof(proto::data_header_t);

        auto size = strnlen(mess_ptr, max_size);

        if ((size == max_size) || (size == 0U)) {
            m_logger("Recieved a text message that's too big");
        } else {
            m_logger(mess_ptr);
        }
    }

    DeviceDef(void) on_script(mhood_t<sig_send_script> s) noexcept {
        send_script(s->name, s->script);
    }

    DeviceDef(void) on_gpio_set(mhood_t<sig_gpio_set> s) noexcept {
        set_gpio(s->n, s->state);
    }
}


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
 #endif //DEVICECONTROL_HPP
