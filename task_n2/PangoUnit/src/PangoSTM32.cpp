#include "PangoSTM32.hpp"

#define SHARED_MEMORY_NAME "RTED_PANGO"
#define GPS_MUTEX_NAME "RTED_PANGO_GPS_MUTEX"
#define SHARED_MEMORY_SIZE 1024
#define THREAD_SLEEP_TIME_MS 1000

using boost::interprocess::read_write;
using boost::interprocess::open_only;
namespace PangoSTM32 {

PangoSTM32Node::PangoSTM32Node() : shm(open_only, SHARED_MEMORY_NAME) {
    // Open the existing shared memory segment
    std::cout << "Opening shared memory\n";
    // Find the existing object named "NewDataAvailable"
    std::cout << "Find the existing object named \"NewDataAvailable\"\n";
    // new_data_available = shm.find_or_construct<int>("NewDataAvailable").first;

    // // Find the existing object named "GPSData"
    // gps_data = shm.find_or_construct<GPS_POINT_T>("GPSData").first;

    gps_data = shm.find_or_construct<GPS_POINT_T>("gps_data")();
    new_data_available = shm.find_or_construct<int>("new_data_available")();

    // Create a named mutex
    _gps_mutex = std::make_shared<boost::interprocess::named_mutex>(open_only, GPS_MUTEX_NAME);
    
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
    std::cout << "Starting GPS thread\n";
    _gps_thread_running = true;
    gps_write_thread = std::thread(&PangoSTM32Node::GpsWriteThreadFunction, this);

    return;
}

void PangoSTM32Node::GpsWriteThreadFunction() {
    std::cout << "hello from GPS write thread function\n";
    // Your code goes here
    GPS_POINT_T gps_data_loc;
    bool data_sent_flag = false;

    while (this->_gps_thread_running) {
        data_sent_flag = false;
        std::cout << "Reading GPS data and updating shm\n";
        gps_data_loc = readGPS(); 
        time_t timestamp = std::chrono::system_clock::to_time_t(gps_data_loc.timestamp);
        std::string timestamp_str = std::ctime(&timestamp);
        std::cout << "GPS timestamp: " << timestamp_str << std::endl;  

        _gps_mutex->lock();
        *gps_data = gps_data_loc;
        *new_data_available = 1;
        _gps_mutex->unlock();

        std::this_thread::sleep_for(std::chrono::milliseconds(THREAD_SLEEP_TIME_MS));
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
