#include "PangoBBG.hpp"
#include <iostream>
#include <cstring>
#include <errno.h>

using boost::interprocess::read_write;
using boost::interprocess::create_only;
using boost::interprocess::open_or_create;

#define SHARED_MEMORY_NAME "RTED_PANGO"
#define GPS_MUTEX_NAME "RTED_PANGO_GPS_MUTEX"
#define SHARED_MEMORY_SIZE 1024
#define THREAD_SLEEP_TIME_MS 100

namespace PangoBBG {
PangoBBGNode::PangoBBGNode() : shm(open_or_create, SHARED_MEMORY_NAME, SHARED_MEMORY_SIZE){
    /** @note
     * one cannot open 2 instances of PaangoBBGNode at the same time on the same machine since the
     * shared memory is created with the same name. This is a limitation of the current implementation.
    */

    // create or open the managed shared memory
    std::cout << "Creating shared memory\n";

    // allocate memory for GPS_POINT_T object
    gps_data = shm.find_or_construct<GPS_POINT_T>("gps_data")();
    new_data_available = shm.find_or_construct<int>("new_data_available")(0);

    // create a named mutex
    gps_mutex = std::make_shared<boost::interprocess::named_mutex>(open_or_create, GPS_MUTEX_NAME);

     // initialize the GPS data
    gps_mutex->lock();
    gps_data->latitude = 0;
    gps_data->longitude = 0;
    gps_data->timestamp = std::chrono::system_clock::now();
    gps_mutex->unlock();

    std::cout << "PangoBBGNode created" << std::endl;

    return;
}

PangoBBGNode::~PangoBBGNode() {

    // stop the GPS read thread
    gps_thread_running = false;
    
    if(gps_thread.joinable()){
        gps_thread.join();
    }  
    
    // destroy shared memory
    boost::interprocess::named_mutex::remove(GPS_MUTEX_NAME);
    boost::interprocess::shared_memory_object::remove(SHARED_MEMORY_NAME);
    // alert the user
    printf("Exiting PangoBBGNode. shared memory destroyed\n");

    return;
}

void PangoBBGNode::run() {
    // connect to the pango server
    std::cout << "Connecting to the pango server" << std::endl;
    int client_id = connectToServer();

    if(client_id == -1){
        std::cerr << "Error connecting to server. Exiting. Errno: " << errno << "  " << strerror(errno) << std::endl;
        return;
    }

    // start the GPS read thread
    std::cout << "Starting GPS thread\n";
    gps_thread_running = true;
    gps_thread = std::thread(&PangoBBGNode::GPSThreadFunction, this, client_id);

    std::cout << "PangoBBGNode running\n";
    return;
}

int PangoBBGNode::connectToServer(){
    // Create a socket
    socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    
    if (socket_fd == -1) {
        std::cerr << "Error creating socket file descriptor. Errno: " << errno << "  " << strerror(errno) << std::endl;
        return -1;
    }

    // Specify the server address and port
    sockaddr_in server_address;
    server_address.sin_family = AF_INET; // IPv4
    server_address.sin_port = htons(8080); // Port 8080
    server_address.sin_addr.s_addr = inet_addr("127.0.0.1"); // Localhost

    // Connect to the server
    if (connect(socket_fd, (struct sockaddr *)&server_address, sizeof(server_address)) < 0) {
        std::cerr << "Error connecting to socket. Errno: " << errno << "  " << strerror(errno) << std::endl;
        return -1;
    }

    std::cout << "Connected to server. socket_fd = " << socket_fd << std::endl;

    return socket_fd;

}

void PangoBBGNode::GPSThreadFunction(int client_id) {
    PANGO_CLIENT_MSG_T msg;
    GPS_POINT_T gps_data_loc;
    msg.client_id = client_id;

    while(gps_thread_running == true){
        
        if(getGPSData(&gps_data_loc) == true){
            std::cout << "New GPS data available. msg size [bytes]: " << *new_data_available << std::endl;
            if(*new_data_available == sizeof(GPS_POINT_T)){
                // new data available. send to server
                msg.gps_data = gps_data_loc;
                // send the message to the server
                int ret = send(client_id, &msg, sizeof(msg), 0);
                // check if the message was sent successfully
                if(ret == -1){
                    std::cerr << "Error sending data to server. Errno: " << errno << "  " << strerror(errno) << std::endl;
                }     
            }
     
        }
        // sleep for a while. allow other threads to run
        std::this_thread::sleep_for(std::chrono::milliseconds(THREAD_SLEEP_TIME_MS));
    }

    std::cout << "Exiting GPS thread\n";

    return;
}

bool PangoBBGNode::getGPSData(GPS_POINT_T* ref_var) {
    bool got_new_data = false;

    gps_mutex->lock();
    if(*new_data_available == 1){
        // if new data is available read it and update the flag.
        *ref_var = *gps_data;
        *new_data_available = 0;
        got_new_data = true;
    }
    gps_mutex->unlock();    

    return got_new_data;
}


}; // namespace PangoBBG

