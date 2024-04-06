#include "PangoBBG.hpp"

using boost::interprocess::read_write;
using boost::interprocess::create_only;
using boost::interprocess::open_or_create;

#define SHARED_MEMORY_NAME "RTED_PANGO"
#define SHARED_MEMORY_SIZE 1024
#define THREAD_SLEEP_TIME_MS 100

namespace PangoBBG {
PangoBBGNode::PangoBBGNode() {
    // create or open the managed shared memory
    boost::interprocess::managed_shared_memory shm(create_only, "RTED_PANGO", SHARED_MEMORY_SIZE);

    // allocate memory for GPS_POINT_T object
    GPS_POINT_T* gps_data = shm.construct<GPS_POINT_T>("gps_data")();

    // allocate memory for new_data_available variable
    int* new_data_available = shm.construct<int>("new_data_available")(0);

    // create a named mutex
    gps_mutex = std::make_shared<boost::interprocess::named_mutex>(create_only, "RTED_PANGO_GPS_MUTEX");

    return;
}

PangoBBGNode::~PangoBBGNode() {

    // stop the GPS read thread
    gps_thread_running = false;
    gps_read_thread.join();
    
    // destroy shared memory
    boost::interprocess::shared_memory_object::remove("RTED_PANGO");
    boost::interprocess::named_mutex::remove("RTED_PANGO_GPS_MUTEX");

    // alert the user
    printf("Exiting PangoBBGNode. shared memory destroyed\n");

    return;
}

void PangoBBGNode::run() {
    // start the GPS read thread
    gps_thread_running = true;
    gps_read_thread = std::thread(&PangoBBGNode::GpsReadThreadFunction, this);

    return;
}

void PangoBBGNode::GpsReadThreadFunction() {
    // Your code goes here
    GPS_POINT_T gps_data_loc;
    while (this->gps_mutex) {
        gps_mutex->lock();
        if (*new_data_available !=0) {
            gps_data_loc = *gps_data;
            *new_data_available = 0;
        }
        gps_mutex->unlock();
        
        /** @todo
         * Send the GPS data to the server node
        */
        std::this_thread::sleep_for(std::chrono::milliseconds(THREAD_SLEEP_TIME_MS));

    }

    printf("Exiting GPS thread\n");

}
}; // namespace PangoBBG
