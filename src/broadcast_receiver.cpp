/** 
 * @file broadcast_receiver.cpp
 * 
 * This file contains the broadcast receiver which handles messages like 
 * `TesterPresent`.
 */

#include "broadcast_receiver.h"
#include "service_identifier.h"
#include <iostream>

using namespace std;

BroadcastReceiver::BroadcastReceiver(canid_t source,
                                     const string& device,
                                     UdsReceiver* pUdsRec)
: IsoTpReceiver(BROADCAST_ADDR, source, device)
, pUdsReceiver_(pUdsRec)
{
}

/**
 * Handles the broadcast messages (e.g. `TesterPresent`).
 * 
 * @param buffer: the buffer of the UDS message
 * @param num_bytes: the number of transmitted data in bytes
 */
void BroadcastReceiver::proceedReceivedData(const uint8_t* buffer,
                                            const size_t num_bytes) noexcept
{
    switch (buffer[0])
    {
        case TESTER_PRESENT_REQ:
        {
            if(buffer[1] != 0x80)
            {
               pUdsReceiver_->pSessionCtrl_->reset();
                // -> beware of arrows ->
                constexpr array<uint8_t, 1> tp = {TESTER_PRESENT_RES};
                pUdsReceiver_->pIsoTpSender_->sendData(tp.data(), tp.size());
            }
            break;
        }
        default:
        {
            pUdsReceiver_->proceedReceivedData(buffer, num_bytes);
        }
    }
}
