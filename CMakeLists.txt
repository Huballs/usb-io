cmake_minimum_required(VERSION 3.28)
project(usb_io)

set(CMAKE_CXX_STANDARD 23)
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall")

set(CMAKE_TOOLCHAIN_FILE ${CMAKE_BINARY_DIR}/conan_toolchain.cmake)

find_package(libusb)
find_package(args)
find_package(ftxui)
find_package(sobjectizer)

add_executable(usb_io main.cpp
#        gui/gui.cpp
#        gui/gui.hpp
        gui/logger.cpp
        gui/logger.hpp
#        gui/window_program.cpp
#        gui/window_program.hpp
        device/device_control.cpp
        device/device_control.hpp
#        device/program.hpp
#        device/program.cpp
        gui/window_gpio.cpp
        gui/window_gpio.hpp
#        gui/window_comm_state.hpp
#        gui/window_conn.hpp
#        gui/timer.hpp
#        fs/fs.hpp
#        gui/window_open_file.hpp
#        gui/window_tabs.hpp
#        gui/window_tabs.cpp

#        gui/menu.hpp
#        device/proto.hpp
#        gui/device_status.hpp
#        gui/device.hpp
#        gui/button.cpp
#        gui/device_status.cpp
#        gui/core_control.hpp
 #       sob-test.hpp
        usb/usb-bulk.cpp
        usb/usb-bulk.hpp
        usb/usb-v2.hpp
        usb/bulk-transfer.hpp
        usb/usb-bulk-agent.hpp
        gui/guiv2.cpp
        gui/guiv2.hpp
        clock.hpp
        gui/window_tabs.cpp
        gui/window_open_file.hpp
        gui/button.cpp
        gui/button.hpp
        gui/conn_status.hpp
        gui/device_status.cpp
        gui/core_control.hpp
        gui/core_control.cpp
)

target_link_libraries(usb_io taywee::args ftxui::ftxui sobjectizer::StaticLib libusb::libusb)
target_include_directories(usb_io PUBLIC taywee::args ftxui::ftxui)
