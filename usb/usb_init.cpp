// //
// // Created by hugo on 01.06.25.
// //
//
// #include "usb_init.h"
//
// #include <cstdio>
// #include <iostream>
//
// #include "usb_recieve.h"
//
// namespace usb {
//
//     static int LIBUSB_CALL hotplug_callback(libusb_context *ctx, libusb_device *dev
//                                             , libusb_hotplug_event event, void *user_data)
//     {
//         struct libusb_device_descriptor desc{};
//         int rc{};
//         auto usb_state = reinterpret_cast<detail::UsbAsyncCtrl*>(user_data);
//
//         (void)ctx;
//         (void)dev;
//         (void)event;
//
//         rc = libusb_get_device_descriptor(dev, &desc);
//         if (LIBUSB_SUCCESS == rc) {
//             usb_state->log("Device attached");
//         } else {
//             std::cerr << "Device Attached." << std::endl
//                         << "Error getting device descriptor: "
//                         << libusb_strerror(static_cast<enum libusb_error>(rc)) << std::endl;
//         }
//
//         try {
//             usb_state->open_device(dev);
//             start_receive(*usb_state);
//         } catch (usbError& e) {
//             std::cout << e.what() << std::endl;
//         }
//         return 0;
//     }
//
//     static int LIBUSB_CALL hotplug_callback_detach(libusb_context *ctx, libusb_device *dev
//                                                     , libusb_hotplug_event event, void *user_data)
//     {
//         struct libusb_device_descriptor desc{};
//         int rc{};
//         auto usb_state = reinterpret_cast<detail::UsbAsyncCtrl*>(user_data);
//
//         (void)ctx;
//         (void)dev;
//         (void)event;
//
//         rc = libusb_get_device_descriptor(dev, &desc);
//         if (LIBUSB_SUCCESS == rc) {
//             usb_state->log("Device detached");
//         } else {
//             usb_state->log("Error getting device descriptor");
//         }
//
//         usb_state->close_device();
//
//         return 0;
//     }
//
//     // init libusb, register hotplug callbacks - if error on these, throws usbInitError
//     // on_attach - opens the device and starts recieve, on_detach - closes
//     // tries to open the the device, if success - start the async receiver if not just returns
//     void init(int vid, int pid, detail::UsbAsyncCtrl& usb_state) {
//
//         constexpr int class_id = LIBUSB_HOTPLUG_MATCH_ANY;
//
//         int rc = libusb_init(nullptr);
//
//         if (LIBUSB_SUCCESS != rc) {
//             throw usbInitError("failed to initialise libusb");
//         }
//
//         if (libusb_has_capability (LIBUSB_CAP_HAS_HOTPLUG) == 0) {
//             libusb_exit (nullptr);
//             throw usbInitError("libusb hotplug is not supported");
//         }
//
//         rc = libusb_hotplug_register_callback (nullptr, LIBUSB_HOTPLUG_EVENT_DEVICE_ARRIVED, 0, vid,
//             pid, class_id, hotplug_callback, &usb_state, nullptr);
//         if (LIBUSB_SUCCESS != rc) {
//             libusb_exit (nullptr);
//             throw usbInitError("failed to register hotplug attach callback");
//         }
//
//         rc = libusb_hotplug_register_callback (nullptr, LIBUSB_HOTPLUG_EVENT_DEVICE_LEFT, 0, vid,
//             pid,class_id, hotplug_callback_detach, &usb_state, nullptr);
//         if (LIBUSB_SUCCESS != rc) {
//             libusb_exit (nullptr);
//             throw usbInitError("failed to register hotplug attach callback");
//         }
//
//         try {
//             usb_state.open_device(vid, pid);
//             usb_state.log("Device attached on start");
//             start_receive(usb_state);
//         } catch (usbError& e) {
//             std::cout << e.what() << std::endl;
//         }
//
//     }
//
//     void loop() {
//
//         const int rc = libusb_handle_events (nullptr);
//
//         if (LIBUSB_SUCCESS != rc) {
//             std::cerr << "libusb_handle_events() failed: "
//                         << libusb_strerror(static_cast<enum libusb_error>(rc)) << std::endl;
//         }
//     }
//
//     void exit (detail::UsbAsyncCtrl& usb_state) {
//         usb_state.close_device();
//         libusb_exit (nullptr);
//     }
//
// }