#include "PangoSTM32.hpp"

using boost::interprocess::read_write;
using boost::interprocess::open_only;

#define SHARED_MEMORY_NAME "RTED_PANGO"
#define THREAD_SLEEP_TIME_MS 100

namespace PangoSTM32 {

PangoSTM32Node::PangoSTM32Node() {
    // Open the existing shared memory segment
    boost::interprocess::managed_shared_memory segment(boost::interprocess::open_only, "RTED_PANGO");

    // Find the existing object named "NewDataAvailable"
    new_data_available = segment.find<int>("NewDataAvailable").first;

    // Find the existing object named "GPSData"
    gps_data = segment.find<GPS_POINT_T>("GPSData").first;

    // Create a named mutex
    _gps_mutex = std::make_shared<boost::interprocess::named_mutex>(open_only, "RTED_PANGO_GPS_MUTEX");
    
    return;
}

PangoSTM32Node::~PangoSTM32Node() {

    // stop the GPS read thread
    _gps_thread_running = false;
    gps_write_thread.join();
    
    // alert the user
    printf("Exiting PangoSTM32Node.\n");

    return;
}

void PangoSTM32Node::run() {
    // start the GPS read thread
    _gps_thread_running = true;
    gps_write_thread = std::thread(&PangoSTM32Node::GpsWriteThreadFunction, this);

    return;
}

void PangoSTM32Node::GpsWriteThreadFunction() {
    // Your code goes here
    GPS_POINT_T gps_data_loc;
    bool data_sent_flag = false;

    while (this->_gps_thread_running) {
        data_sent_flag = false;
        gps_data_loc = readGPS();   // GPS readings are relatively slow. no need to update 

        while(std::chrono::system_clock::now() - gps_data_loc.timestamp < std::chrono::milliseconds(THREAD_SLEEP_TIME_MS)){
            if(data_sent_flag == true) {
                // data has been sent. sleep for a while
                std::this_thread::sleep_for(std::chrono::milliseconds(10));
                continue;
            }

            // try to lock the mutex and send the data
            if(_gps_mutex->try_lock() == true) {
                *gps_data = gps_data_loc;
                *new_data_available = 1;
                _gps_mutex->unlock();
                data_sent_flag = true;
            }
            else {
                // mutex is locked. sleep for a while
                std::this_thread::sleep_for(std::chrono::milliseconds(1));
            }
        }
    }

    printf("Exiting GPS thread\n");

}

GPS_POINT_T PangoSTM32Node::readGPS() {
    GPS_POINT_T gps_reading;
    gps_reading.latitude = 37.563936;
    gps_reading.longitude = -116.851230;
    gps_reading.timestamp = std::chrono::system_clock::now();

    return gps_reading;
}

}; // namespace PangoSTM32
