//
// Created by hugo on 17.08.25.
//

#ifndef USB_BULK_AGENT_HPP
#define USB_BULK_AGENT_HPP

#include <iostream>
#include <so_5/all.hpp>
#include "usb-bulk.hpp"

namespace usb {

    using namespace std::literals;

    struct transmit_res {status_t val;};

    UsbBulk_template_t
    struct recieve_res  {
        status_t val;
        typename UsbBulk<DATA_SIZE>::rx_transfer_data_t data;
    };

    struct hotplug{};
    struct hotunplug{};

    enum class usb_error_t {
        INIT
    };

    struct error_mes {usb_error_t val;};

    UsbBulk_template_t
    class UsbBulkAgent : public so_5::agent_t {
    public:

        using usb_bulk_t = UsbBulk<DATA_SIZE>;

        UsbBulkAgent(context_t ctx, so_5::mbox_t board, usb_bulk_t& usb_bulk)
           :  so_5::agent_t{std::move(ctx)}
            ,  m_board{std::move(board)}
            , m_usb_bulk(usb_bulk) {}

        void so_evt_start() override {
            if (m_usb_bulk.init() == status_t::ERROR) {
                so_5::send<error_mes>(m_board, usb_error_t::INIT);
            }

            if (m_usb_bulk.recieve(m_f_on_recieve) == status_t::ERROR) {
                so_5::send<error_mes>(m_board, usb_error_t::INIT);
            }

            m_usb_bulk.set_on_hotplug(m_f_on_hotplug);
            m_usb_bulk.set_on_hotunplug(m_f_on_hotunplug);

            m_timer_loop = so_5::send_periodic<loop_signal>(*this, 0ms, 5ms);
        }

        void so_define_agent() override {
            so_subscribe_self().event(&UsbBulkAgent::on_timer);
        }

        status_t transmit(typename usb_bulk_t::tx_transfer_data_t* data) {
            return m_usb_bulk.transmit(m_f_on_transmit, data);
        }

    private:
        const so_5::mbox_t m_board;
        usb_bulk_t& m_usb_bulk;
        so_5::timer_id_t m_timer_loop;

        typename usb_bulk_t::f_on_transmit_t m_f_on_transmit = [this](status_t res) {
            so_5::send<transmit_res>(m_board, res);
        };

        typename usb_bulk_t::f_on_recieve_t m_f_on_recieve
            = [this](status_t res, typename usb_bulk_t::rx_transfer_data_t data) {
                so_5::send<recieve_res<DATA_SIZE>>(m_board, res, data);
                m_usb_bulk.recieve(m_f_on_recieve);
        };

        f_hotplug_t m_f_on_hotplug = [this]() {
            so_5::send<hotplug>(m_board);
        };

        f_hotplug_t m_f_on_hotunplug = [this]() {
            so_5::send<hotunplug>(m_board);
        };

        struct loop_signal : public so_5::signal_t {};

        void on_timer(mhood_t<loop_signal>) {
            m_usb_bulk.loop();
        }

    };

}

#endif //USB_BULK_AGENT_HPP
