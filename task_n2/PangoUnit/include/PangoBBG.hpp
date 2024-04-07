#ifndef PANGOBBG_HPP
#define PANGOBBG_HPP

#include <thread>
#include <iostream>
#include <cstring>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <boost/interprocess/shared_memory_object.hpp>
#include <boost/interprocess/managed_shared_memory.hpp>
#include <boost/interprocess/mapped_region.hpp>
#include <boost/interprocess/sync/named_mutex.hpp>
#include "GPS.hpp"

namespace PangoBBG {

class PangoBBGNode {
public:
    PangoBBGNode();
    ~PangoBBGNode();

    void run();
private:
    bool getGPSData(GPS_POINT_T* gps_data);   
    void GPSThreadFunction(int client_id);
    int connectToServer();

    int socket_fd = -1;
    bool deliverable_data_exists = false;

    GPS_POINT_T* gps_data = nullptr;
    int* new_data_available = nullptr;
    std::shared_ptr<boost::interprocess::named_mutex> gps_mutex;

    std::thread gps_thread;
    bool gps_thread_running = false;

    

};

}


#endif // PANGOBBG_HPP