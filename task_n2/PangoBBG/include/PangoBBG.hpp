#ifndef PANGOBBG_HPP
#define PANGOBBG_HPP

#include <thread>
#include <iostream>
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
    GPS_POINT_T* gps_data = nullptr;
    int* new_data_available = nullptr;
    bool gps_thread_running = false;
    std::shared_ptr<boost::interprocess::named_mutex> gps_mutex;

    std::thread gps_read_thread;
    void GpsReadThreadFunction();
    
};

}


#endif // PANGOBBG_HPP