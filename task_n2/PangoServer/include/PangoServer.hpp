#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <vector>
#include <thread>
#include <fstream>
#include <mutex>
#include "GPS.hpp"


namespace PangoServer{

class PangoServerClass {
public:
    PangoServerClass(int port);
    ~PangoServerClass();

    bool run();
    


private:
    bool acceptConnections();
    void clientThread(int client_socket);
    void acceptClientsThread();
    bool updateLog(PANGO_CLIENT_MSG_T client_msg, double total_parking_duration, double current_area_pricing, double total_price);
    double updateParkingCost(PANGO_CLIENT_MSG_T client_msg, PANGO_CLIENT_MSG_T prev_client_msg, double total_price, double current_area_pricing);
    double calculateAreaPricing(double latitude, double longitude);
    unsigned long int updateParkingDuration(PANGO_CLIENT_MSG_T client_msg, unsigned long int* start_time);
    

    bool _client_thread_running = false;
    bool _accept_new_connections = false;
    std::thread _accept_clients_thread;
    std::vector<std::thread> _client_threads;
    int _server_port;
    int _socket_fd;
    std::ofstream _log_file;
    std::mutex _log_mutex;
    enum PARKING_STATES {
        PARKING_STATE_UNKNOWN = -1,
        PARKING_STATE_START,
        PARKING_STATE_OCCUPIED,
        PARKING_STATE_END
    };
};

} // namespace PangoServer
