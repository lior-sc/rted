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

class PangoBBGClass {
public:
    PangoBBGClass(int16_t client_id);
    ~PangoBBGClass();
    
    void run();

private:
    bool getGPSData(GPS_POINT_T* gps_data, bool verbose = false);   
    void GPSThreadFunction(int client_id);
    int connectToServer();
    bool sendDataToServer(GPS_POINT_T gps_data, bool verbose = false);

    /////////////// class private variables
    // server connection
    int socket_fd = -1;
    bool deliverable_data_exists = false;

    // shared memory
    boost::interprocess::managed_shared_memory shm;
    GPS_POINT_T *gps_data;
    int *new_data_available; 
    std::shared_ptr<boost::interprocess::named_mutex> gps_mutex;
    // threads
    std::thread gps_thread;
    bool gps_thread_running = false;
    // parking logic
    enum PARKING_STATES {
        PARKING_STATE_UNKNOWN = -1,
        PARKING_STATE_START,
        PARKING_STATE_OCCUPIED,
        PARKING_STATE_END
    };
    int16_t _client_id = 0;
    int8_t _parking_state = PARKING_STATE_UNKNOWN;


    

};

}


#endif // PANGOBBG_HPP