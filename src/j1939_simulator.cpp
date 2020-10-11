
#include "j1939_simulator.h"
#include "can/j1939.h"
#include <linux/can.h>
#include <iostream>
#include <iomanip>
#include <cstdio>
#include <cstring>

#include <net/if.h>
#include <sys/socket.h>
#include <sys/ioctl.h>

using namespace std;

constexpr size_t MAX_BUFSIZE = 1788; // 255*7 Byte + 3 byte PGN

J1939Simulator::J1939Simulator(uint8_t source_address,
                               const std::string& device,
                               EcuLuaScript *pEcuScript)
: source_address_(source_address)
, device_(device)
, pEcuScript_(pEcuScript)
, j1939ReceiverThread_(&J1939Simulator::readData, this)
{
    pgns_ = new uint16_t[1];
    // This is just a demo for the getKeys function I implemented into Selene
    // One could use that to fetch a list of configured PDNs from the lua file
    cout << "Requests:" << endl;
    for(auto const &request : pEcuScript_->getRawRequests()) {
        cout << request << " -> "<< pEcuScript_->getRaw(request) << endl;
    }
}

J1939Simulator::~J1939Simulator()
{

}

/**
 * Opens the socket for receiving J1939 PGNs
 *
 * @return 0 on success, otherwise a negative value
 * @see J1939Simulator::closeReceiver()
 */
int J1939Simulator::openReceiver() noexcept
{
    // see also: https://www.kernel.org/doc/html/latest/networking/j1939.html
    isOnExit_ = false;
    struct sockaddr_can addr;
    addr.can_family = AF_CAN;

    addr.can_addr.j1939.pgn = J1939_NO_PGN; // Receive all PGNs
    addr.can_addr.j1939.name = J1939_NO_NAME; // Don't do name lookups
    addr.can_addr.j1939.addr = 3; // source address

    int skt = socket(PF_CAN, SOCK_DGRAM, CAN_J1939);
    if (skt < 0)
    {
        cerr << __func__ << "() socket: " << strerror(errno) << '\n';
        return -1;
    }

    struct ifreq ifr;
    strncpy(ifr.ifr_name, device_.c_str(), device_.length() + 1);
    ioctl(skt, SIOCGIFINDEX, &ifr);
    addr.can_ifindex = ifr.ifr_ifindex;

    auto bind_res = bind(skt,
                         reinterpret_cast<struct sockaddr*> (&addr),
                         sizeof(addr));
    if (bind_res < 0)
    {
        cerr << __func__ << "() bind: " << strerror(errno) << '\n';
        close(skt);
        return -2;
    }

    receive_skt_ = skt;
    return 0;
}

/**
 * Closes the socket for receiving data.
 * 
 * @see J1939Simulator::openReceiver()
 */
void J1939Simulator::closeReceiver() noexcept
{
    isOnExit_ = true;

    if (receive_skt_ < 0)
    {
        cerr << __func__ << "() Receiver socket is already closed!\n";
        return;
    }
    close(receive_skt_);
    receive_skt_ = -1;
}

/**
 * Reads the data from the opened receiver socket. The handling of the received
 * data is controlled by the inside nested `proceedReceivedData()` function.
 * 
 * @return 0 on success, otherwise a negative value
 */
int J1939Simulator::readData() noexcept
{
    int err = openReceiver();
    if (err != 0)
    {
        throw exception();
    }

    if (receive_skt_ < 0)
    {
        cerr << __func__ << "() Can not read data. J1939 receiver socket invalid!\n";
        return -1;
    }

    // Sending some dummy PGN to see that it works
    struct sockaddr_can saddr;
    saddr.can_family = AF_CAN;
    saddr.can_addr.j1939.name = J1939_NO_NAME;
    saddr.can_addr.j1939.pgn = 0x12300;
    saddr.can_addr.j1939.addr = 0x30;

    uint8_t dat[] = {0x01, 0xff, 0xab, 0xa3};
    sendto(receive_skt_, dat, sizeof(dat), 0, (const struct sockaddr *)&saddr, sizeof(saddr));

    // the actual receive
    uint8_t msg[MAX_BUFSIZE];
    size_t num_bytes;
    do
    {
        cout << "Reading from socket" << endl;
        num_bytes = read(receive_skt_, msg, MAX_BUFSIZE);
        if (num_bytes > 0 && num_bytes < MAX_BUFSIZE)
        {
            proceedReceivedData(msg, num_bytes);
        }
    }
    while (!isOnExit_);

    return 0;
}

/**
 * Prints out the received data and should be called once per received message
 * 
 * @see J1939Simulator::readData()
 */
void J1939Simulator::proceedReceivedData(const uint8_t* buffer, const size_t num_bytes) noexcept
{
    // Print out what we received
    cout << __func__ << "() Received " << dec << num_bytes << " bytes.\n";
    for (size_t i = 0; i < num_bytes; i++)
    {
        cout << " 0x"
                << hex
                << setw(2)
                << setfill('0')
                << static_cast<int> (buffer[i]);
    }
    cout << endl;
}
