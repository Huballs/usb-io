//
// Created by hugo on 10.08.25.
//

#ifndef PROTO_HPP
#define PROTO_HPP

namespace device::proto {

    constexpr uint32_t s_var_name_max_size = 16U;

    enum class data_type_t : uint8_t {
        NONE
        , GPIO
        , COMMAND
        , TEXT_MESSAGE
        , SCRIPT_DATA_START
        , SCRIPT_DATA_START_END
        , SCRIPT_DATA_N
        , SCRIPT_DATA_END
        , SCRIPT_NAME
        , STATUS
        , VARIABLES
    };

    enum class command_t : uint8_t {
        NONE
        , GET_ALL_GPIO
        , LAUNCH_LUA_CORE
        , STOP_LUA_CORE
        , PAUSE_LUA_CORE
        , CONTINUE_LUA_CORE
        , GET_STATUS
    };

    enum class lua_status_t : uint8_t {
        STOPPED
        , PAUSED
        , RUNNING
        , FINISHED
        , INIT_SUCCESS
        , INIT_ERROR
        , SCRIPT_ERROR
    };

    enum class io_t : uint8_t {
        INPUT
        , OUTPUT
    };

    enum class variable_t : uint8_t {
        BOOL
        , INT
        , FLOAT
        , STR
    };

#pragma pack(push,1)
    struct /*__attribute__((packed))*/  data_variable_t  {
        variable_t type;
        uint32_t size;
        char name[s_var_name_max_size];
    };


    struct /*__attribute__((packed))*/ status_t {
        lua_status_t lua_status = lua_status_t::STOPPED;
        char script_name[32U];
        bool script_loaded { false };
    };

    struct /*__attribute__((packed))*/ data_header_t {
        data_type_t type;
        uint32_t size;
    };

    struct /*__attribute__((packed))*/ data_gpio_t {
        uint8_t gpio_n;
        uint8_t value;
        io_t io;
        uint8_t function;

    };
#pragma pack(pop)

}

#endif //PROTO_HPP
