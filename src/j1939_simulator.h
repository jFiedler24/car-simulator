/**
 * @file j1939_simulator.h
 *
 */

#ifndef J1939_SIMULATOR_H
#define J1939_SIMULATOR_H

#include <cstdint>
#include <string>
#include <memory>
#include <thread>

#include "ecu_lua_script.h"


class J1939Simulator
{
public:
    J1939Simulator() = delete;
    J1939Simulator(uint8_t source_address,
                   const std::string& device);
    virtual ~J1939Simulator();
    int openReceiver() noexcept;
    void closeReceiver() noexcept;
    int readData() noexcept;
    void proceedReceivedData(const uint8_t* buffer, const size_t num_bytes) noexcept;


private:
    uint8_t source_address_;
    std::string device_;
    int receive_skt_ = -1;
    bool isOnExit_ = false;
    std::thread j1939ReceiverThread_;

    sel::State lua_state_;
    uint16_t *pgns_;
};

#endif // J1939_SIMULATOR_H