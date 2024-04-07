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
    bool writeLog(PANGO_CLIENT_MSG_T client_msg);

    bool _client_thread_running = false;
    bool _accept_new_connections = false;
    std::thread _accept_clients_thread;
    std::vector<std::thread> _client_threads;
    int _server_port;
    int _socket_fd;
    std::ofstream _log_file;
    std::mutex _log_mutex;
};

} // namespace PangoServer
