//
// Created by hugo on 07.06.25.
//

#ifndef USB_TRANSFER_H
#define USB_TRANSFER_H

#include <array>
#include <functional>
#include <libusb.h>
#include "usb-v2.hpp"

namespace usb::detail {

    enum class direction_t {
        RX
        , TX
    };

    constexpr unsigned char ENDPOINT_TRANSMIT_BIT   = 0x00U;
    constexpr unsigned char ENDPOINT_RECEIVE_BIT    = 0x80U;

    #define BulkTransferTemplate template<direction_t DIR, size_t DATA_SIZE>

    BulkTransferTemplate
    class BulkTransfer;

    BulkTransferTemplate
    struct to_transmit_t;

    BulkTransferTemplate
    struct to_recieve_t;

    // BulkTransferTemplate
    // void LIBUSB_CALL lib_usb_recieve_callback (struct libusb_transfer *transfer) {
    //     auto to_recieve = reinterpret_cast<to_recieve_t<DIR, DATA_SIZE>*>(transfer->user_data);
    //
    //     status_t status = status_t::ERROR;
    //     typename BulkTransfer<DIR, DATA_SIZE>::data_t data;
    //
    //     if ((LIBUSB_TRANSFER_COMPLETED == transfer->status) && (transfer->length == to_recieve->data->size())) {
    //         std::copy(transfer->buffer, transfer->buffer + transfer->length, data);
    //         status = status_t::SUCCESS;
    //     } else {
    //         status = status_t::ERROR;
    //     }
    //
    //     to_recieve->f_on_recieve_t(status, data);
    //
    //     delete to_recieve;
    // }
    //
    // BulkTransferTemplate
    // void lib_usb_transmit_callback(struct libusb_transfer * transfer) {
    //
    //     auto to_transmit = reinterpret_cast<to_recieve_t<DIR, DATA_SIZE>*>(transfer->user_data);
    //
    //     status_t status = status_t::ERROR;
    //
    //     if (transfer->status == LIBUSB_TRANSFER_COMPLETED) {
    //         status = status_t::SUCCESS;
    //     }
    //
    //     to_transmit->on_finish(status);
    //
    //     delete to_transmit;
    // }

    BulkTransferTemplate
    class BulkTransfer {

    public:

        using data_t = std::array<uint8_t, DATA_SIZE>;

        using f_on_recieve_t = std::function<void(status_t, data_t)>;
        using f_on_transmit_t = std::function<void(status_t)>;

        BulkTransfer() = default;

        template<typename F_CALLBACK>
        static status_t submit(data_t* data, libusb_device_handle * libusb_handle
                                , unsigned char endpoint, unsigned int timeout, F_CALLBACK callback) {

            auto transfer = alloc_transfer();

            if (transfer == NULL) {
                return status_t::ERROR;
            }

            if constexpr (DIR == direction_t::RX) {
                auto to_recieve = new to_recieve_t<DIR, DATA_SIZE>();
                to_recieve->libusb_transfer = transfer;
                to_recieve->libusb_handle = libusb_handle;
                to_recieve->on_recieve = callback;

                libusb_fill_bulk_transfer(transfer, libusb_handle, endpoint | (ENDPOINT_RECEIVE_BIT)
                                            , to_recieve->data.data(), DATA_SIZE
                                            , lib_usb_recieve_callback, to_recieve, timeout
                                            );
            } else if (DIR == direction_t::TX) {

                if (data == nullptr) {
                    return status_t::ERROR;
                }

                auto to_transmit = new to_transmit_t<DIR, DATA_SIZE>();
                to_transmit->libusb_transfer = transfer;
                to_transmit->libusb_handle = libusb_handle;
                to_transmit->on_finish = callback;
                std::copy(std::begin(*data), std::end(*data), std::begin(to_transmit->data));
                libusb_fill_bulk_transfer(transfer, libusb_handle, endpoint | (ENDPOINT_TRANSMIT_BIT)
                                            , to_transmit->data.data(), DATA_SIZE
                                            , lib_usb_transmit_callback, to_transmit, timeout
                                            );
            } else {
                return status_t::ERROR;
            }

            return submit_(transfer);
        }

        static status_t submit_ (struct libusb_transfer * transfer) {
            return libusb_submit_transfer(transfer) == LIBUSB_SUCCESS ? status_t::SUCCESS : status_t::ERROR;
        }

    private:

         static libusb_transfer * alloc_transfer() {
            return libusb_alloc_transfer(0);
        }

        static void LIBUSB_CALL lib_usb_recieve_callback (struct libusb_transfer *transfer) {
             auto to_recieve = reinterpret_cast<to_recieve_t<DIR, DATA_SIZE>*>(transfer->user_data);

             status_t status = status_t::ERROR;
             typename BulkTransfer<DIR, DATA_SIZE>::data_t data;

             if ((LIBUSB_TRANSFER_COMPLETED == transfer->status)
                    && (transfer->length == static_cast<int>(to_recieve->data.size()))) {
                 std::copy(transfer->buffer, transfer->buffer + transfer->length, std::begin(data));
                 status = status_t::SUCCESS;
             } else {
                 status = status_t::ERROR;
             }

             to_recieve->on_recieve(status, data);

             delete to_recieve;
         }

        static void lib_usb_transmit_callback(struct libusb_transfer * transfer) {

            auto to_transmit = reinterpret_cast<to_transmit_t<DIR, DATA_SIZE>*>(transfer->user_data);

            status_t status = status_t::ERROR;

            if (transfer->status == LIBUSB_TRANSFER_COMPLETED) {
                status = status_t::SUCCESS;
            }

            to_transmit->on_finish(status);

            delete to_transmit;
        }
    };



    BulkTransferTemplate
    struct to_recieve_t {

        ~to_recieve_t() {
            if (libusb_transfer != nullptr) {
                libusb_free_transfer(libusb_transfer);
            }
        }

        typename BulkTransfer<DIR, DATA_SIZE>::data_t data;
        typename BulkTransfer<DIR, DATA_SIZE>::f_on_recieve_t on_recieve;
        struct libusb_transfer * libusb_transfer = nullptr;
        libusb_device_handle * libusb_handle = nullptr;
    };

    BulkTransferTemplate
    struct to_transmit_t {

        ~to_transmit_t() {
            if (libusb_transfer != nullptr) {
                libusb_free_transfer(libusb_transfer);
            }
        }
        typename BulkTransfer<DIR, DATA_SIZE>::data_t data;
        typename BulkTransfer<DIR, DATA_SIZE>::f_on_transmit_t on_finish;
        struct libusb_transfer * libusb_transfer = nullptr;
        libusb_device_handle * libusb_handle = nullptr;
    };
}

#endif //USB_TRANSFER_H
