#ifndef PANGOSTM32_HPP
#define PANGOSTM32_HPP

#include <thread>
#include <iostream>
#include <boost/interprocess/shared_memory_object.hpp>
#include <boost/interprocess/mapped_region.hpp>
#include <boost/interprocess/sync/named_mutex.hpp>
#include <boost/interprocess/managed_shared_memory.hpp>

#include "GPS.hpp"

namespace PangoSTM32 {

class PangoSTM32Node {
public:
    PangoSTM32Node();
    ~PangoSTM32Node();

    void run();
private:
    GPS_POINT_T readGPS();
    void GpsWriteThreadFunction();

    // class variables
    GPS_POINT_T* gps_data = nullptr;
    int* new_data_available = nullptr;
    bool _gps_thread_running = false;
    std::shared_ptr<boost::interprocess::named_mutex> _gps_mutex;
    std::thread gps_write_thread;
};

}


#endif // PANGOSTM32_HPP