//
// Created by hugo on 16.08.25.
//
#include "bulk-transfer.hpp"

// void LIBUSB_CALL on_receive (struct libusb_transfer *transfer) {
//
//     auto to_recieve = reinterpret_cast<usb::detail::to_recieve_t*>(transfer->user_data);
//
//     if ((LIBUSB_TRANSFER_COMPLETED == transfer->status) && (transfer->length == to_recieve->data->size())) {
//         std::copy(transfer->buffer, transfer->buffer + transfer->length, *transfer->data);
//     }
// }