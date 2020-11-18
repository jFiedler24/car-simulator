/**
 * @file electronic_control_unit.cpp
 *
 * This file contains a class which constitutes a Electronic Control Unit (ECU).
 */

#include "electronic_control_unit.h"
#include <array>
#include <unistd.h>

using namespace std;

ElectronicControlUnit::ElectronicControlUnit(const string& device, EcuLuaScript *pEcuScript)
: requId_(pEcuScript->getRequestId())
, respId_(pEcuScript->getResponseId())
, j1939SourceAddress_(pEcuScript->getJ1939SourceAddress())
, sender_(respId_, requId_, device)
, broadcastReceiver_(pEcuScript->getBroadcastId(), device, &udsReceiver_)
, udsReceiver_(respId_, requId_, device, pEcuScript, &sender_, &sessionControl_)
, udsReceiverThread_(&IsoTpReceiver::readData, &udsReceiver_)
, broadcastReceiverThread_(&IsoTpReceiver::readData, &broadcastReceiver_)
, j1939Simulator_(j1939SourceAddress_, device, pEcuScript)
{
}

ElectronicControlUnit::~ElectronicControlUnit()
{
    broadcastReceiverThread_.join();
    udsReceiverThread_.join();
}
