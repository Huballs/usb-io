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

    enum class command_t {
        LAUNCH_LUA_CORE
        , STOP_LUA_CORE
        , PAUSE_LUA_CORE
        , CONTINUE_LUA_CORE
    };

    struct sig_connected{};
    struct sig_disconnected{};
    struct sig_status{proto::status_t status;};
    struct sig_gpio_set{size_t n; gpio_state_t state;};
    struct sig_gpio_new_state{size_t n; gpio_state_t state;};
    struct sig_send_script{std::string_view name; std::string_view script;};
    struct sig_message{std::string text;};
    struct sig_command{command_t command;};
    struct sig_variable{std::string name; std::string var;};

    using f_log_t = std::function<void(std::string_view)>;

    #define DeviceTemplateParams DATA_SIZE, DATA_T
    #define DeviceTemplate template<size_t DATA_SIZE, typename DATA_T>

    DeviceTemplate
    class Device final : public so_5::agent_t {

    public:
        Device(context_t ctx, so_5::mbox_t board)
           :  so_5::agent_t{std::move(ctx)}
            ,  m_board{std::move(board)}
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
        void parse_variables(const DATA_T& data) noexcept;

        void on_script(mhood_t<sig_send_script>) noexcept;
        void on_gpio_set(mhood_t<sig_gpio_set>) noexcept;

        void on_command(mhood_t<sig_command>) noexcept;

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

        template<typename T> // starts from zero
        constexpr const proto::data_gpio_t* cast_to_data_gpio(const T& array, size_t postition) noexcept {
            return reinterpret_cast<const proto::data_gpio_t*>(
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
        constexpr const proto::status_t* cast_to_data_status(const T& array) noexcept {
            return reinterpret_cast<const proto::status_t*>(
                array.data() + sizeof(proto::data_header_t)
            );
        }

        template<typename T>
        constexpr proto::command_t* cast_to_command(T& array) noexcept {
            return reinterpret_cast<proto::command_t*>(
                array.data() + sizeof(proto::data_header_t)
            );
        }

        template<typename T>
        constexpr proto::data_variable_t* cast_to_var_data(T& array, size_t offset) noexcept {
            return reinterpret_cast<proto::data_variable_t*>(
                array.data() + sizeof(proto::data_header_t) + offset
            );
        }

        template<typename T>
        constexpr const proto::data_variable_t* cast_to_var_data(const T& array, size_t offset) noexcept {
            return reinterpret_cast<const proto::data_variable_t*>(
                array.data() + sizeof(proto::data_header_t) + offset
            );
        }

        template<typename T>
        constexpr uint8_t* cast_to_var(T& array, size_t offset) noexcept {
            return
                array.data() + sizeof(proto::data_header_t) + sizeof(proto::data_variable_t) + offset;
        }

        template<typename T>
        constexpr const uint8_t* cast_to_var(const T& array, size_t offset) noexcept {
            return
                array.data() + sizeof(proto::data_header_t) + sizeof(proto::data_variable_t) + offset;
        }

        template<typename T>
        constexpr uint8_t* cast_to_data(T& array) noexcept {
            return array.data() + sizeof(proto::data_header_t);
        }

        template<typename T>
        constexpr const uint8_t* cast_to_data(const T& array) noexcept {
            return array.data() + sizeof(proto::data_header_t);
        }

    }

    #define DeviceDef(RET)  DeviceTemplate\
                            RET Device<DeviceTemplateParams>::

    DeviceDef(void)  so_define_agent() {
        so_subscribe(m_board).event(&Device::on_script);
        so_subscribe(m_board).event(&Device::on_connected);
        so_subscribe(m_board).event(&Device::on_disconnected);
        so_subscribe(m_board).event(&Device::on_error);
        so_subscribe(m_board).event(&Device::on_gpio_set);
        so_subscribe(m_board).event(&Device::on_recieve);
        so_subscribe(m_board).event(&Device::on_command);
    }

    DeviceDef(void) send(proto::data_gpio_t gpio_data) noexcept {

        using namespace detail;

        DATA_T data{};

        cast_to_data_header(data)->type = proto::data_type_t::GPIO;
        cast_to_data_header(data)->size = 1U;

        *cast_to_data_gpio(data, 0U) = gpio_data;

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

        using namespace std;

        while (start < script.size()) {
            DATA_T data;

            auto tx_data_no_header = detail::cast_to_data(data);

            size_t end = min(start + data_size, script.size());

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

        auto size = min(static_cast<size_t>(name.size()), (size_t)32U);

        strncpy(reinterpret_cast<char*>(detail::cast_to_data(data)), name.data()
            , size);

        *(detail::cast_to_data(data) + size) = 0U;

        send(data);
    }

    DeviceDef(void) on_recieve(mhood_t<usb::sig_recieve_res<DATA_SIZE>> result) noexcept {

        if (result->status == usb::status_t::SUCCESS) {

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
                    parse_text_message(result->data);
                    break;
                }
                case proto::data_type_t::VARIABLES: {
                    parse_variables(result->data);
                    break;
                }
                default: break;
            }
        }
    }

    DeviceDef(void) on_error(mhood_t<usb::sig_error_mes> res) noexcept {

        std::string mess;

        switch (res->error) {
            case usb::usb_error_t::INIT: mess = ("Usb Init Error"); break;
            case usb::usb_error_t::TRANSMIT: mess = ("Usb Transmit Error"); break;
            default: mess = ("Usb Unknown error");
        }

        so_5::send<sig_message>(m_board, mess);
    }

    DeviceDef(void) on_connected(mhood_t<usb::sig_hotplug>) noexcept {
        m_conected = true;
        so_5::send<sig_connected>(m_board);
        request_gpio();
    }

    DeviceDef(void) on_disconnected(mhood_t<usb::sig_hotunplug>) noexcept {
        m_conected = false;
        so_5::send<sig_disconnected>(m_board);
    }

    DeviceDef(void) parse_gpio(const DATA_T& data) noexcept {

        auto header = detail::cast_to_data_header(data);

        assert(header->type == proto::data_type_t::GPIO);

        if ((header->size > s_gpio_count)
            || (header->size * sizeof(proto::data_gpio_t) + sizeof(proto::data_header_t)) > DATA_SIZE
        ) {
            so_5::send<sig_message>(m_board, "GPIO packet size too big");
            return;
        }

        for (auto i : std::ranges::views::iota(0U, header->size)) {
            auto gpio_data = detail::cast_to_data_gpio(data, i);

            auto [n, gpio_state] = detail::gpio_data_to_state(*gpio_data);

            if (n >= m_gpio_states.size()) {
                so_5::send<sig_message>(m_board, "GPIO incorrect N");
                continue;
            }

            m_gpio_states.at(n) = gpio_state;

            so_5::send<sig_gpio_new_state>(m_board, n, gpio_state);
        }
    }

    DeviceDef(void)  parse_text_message(const DATA_T& data) noexcept {
        auto mess_ptr = reinterpret_cast<const char*>(detail::cast_to_data(data));

        auto max_size =  DATA_SIZE - sizeof(proto::data_header_t);

        auto size = strnlen(mess_ptr, max_size);

        if ((size == max_size) || (size == 0U)) {
            so_5::send<sig_message>(m_board, "Recieved a text message that's too big");
        } else {
            so_5::send<sig_message>(m_board, mess_ptr);
        }
    }

    DeviceDef(void) parse_variables(const DATA_T& data) noexcept {
        auto var_count = detail::cast_to_data_header(data)->size;

        size_t offset = 0U;

        auto max_var_size = [&]() {
            return data.size() - sizeof(proto::data_header_t) - offset;
        };

        for (auto _ : std::views::iota(0U, var_count)) {
            const auto var_data = detail::cast_to_var_data(data, offset);

            auto var_ptr = detail::cast_to_var(data, offset);

            if (var_data->size > max_var_size()) {
                so_5::send<sig_message>(m_board, "Error in var size");
                return;
            }

            const auto name_size = strnlen(var_data->name, proto::s_var_name_max_size);

            if (name_size == proto::s_var_name_max_size) {
                so_5::send<sig_message>(m_board, "Error in varnamee size");
                return;
            }

            switch (var_data->type) {
                case proto::variable_t::BOOL: {
                    const bool var = *reinterpret_cast<const bool*>(var_ptr);
                    so_5::send<sig_variable>(m_board, var_data->name, std::to_string(var));
                    break;
                }
                case proto::variable_t::FLOAT: {
                    const float var = *reinterpret_cast<const float*>(var_ptr);
                    so_5::send<sig_variable>(m_board, var_data->name, std::to_string(var));
                    break;
                }
                case proto::variable_t::STR: {
                    const auto var = reinterpret_cast<const char*>(var_ptr);

                    const auto max_str_size = max_var_size() - sizeof(proto::data_variable_t);

                    auto str_size = strnlen(var, max_str_size);

                    if (str_size == max_str_size) {
                        so_5::send<sig_message>(m_board, "Error in str var size");
                        return;
                    }

                    so_5::send<sig_variable>(m_board, var_data->name, var);
                    break;
                }
                default: so_5::send<sig_message>(m_board, "Unknown var type received");
            }

            offset += var_data->size;
        }
    }

    DeviceDef(void) on_script(mhood_t<sig_send_script> s) noexcept {
        send_script(s->name, s->script);
    }

    DeviceDef(void) on_gpio_set(mhood_t<sig_gpio_set> s) noexcept {
        set_gpio(s->n, s->state);
    }

    DeviceDef(void) on_command(mhood_t<sig_command> s) noexcept {
        DATA_T tx_data;

        detail::cast_to_data_header(tx_data)->type = proto::data_type_t::COMMAND;

        auto command = detail::cast_to_command(tx_data);

        switch (s->command) {
            case(command_t::LAUNCH_LUA_CORE): {
                *command = proto::command_t::LAUNCH_LUA_CORE;
                break;
            }

            case(command_t::PAUSE_LUA_CORE): {
                *command = proto::command_t::PAUSE_LUA_CORE;
                break;
            }

            case(command_t::STOP_LUA_CORE): {
                *command = proto::command_t::STOP_LUA_CORE;
                break;
            }

            case(command_t::CONTINUE_LUA_CORE): {
                *command = proto::command_t::CONTINUE_LUA_CORE;
                break;
            }

            default: {
                so_5::send<sig_message>(m_board, "Unknown command");
                return;
            }
        }

        send(tx_data);
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
