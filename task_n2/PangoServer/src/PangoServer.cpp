#include "PangoServer.hpp"
#include <iostream>
#include <cstring> 
#include <errno.h>


namespace PangoServer{

PangoServerClass::PangoServerClass(int port) : _server_port(port), _socket_fd(0){
    // Create a socket
    _socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (_socket_fd == -1) {
        std::cerr << "Failed to create socket. errno: " << errno << " - " << "  " << strerror(errno) << std::endl;
        return;
    }

    // define the server address
    sockaddr_in server_address;
    server_address.sin_family = AF_INET;  // set the address family to AF_INET (IPv4)
    server_address.sin_addr.s_addr = INADDR_ANY;  // accept any incoming address
    server_address.sin_port = htons(_server_port);  // convert port to network byte order

    // bind the socket to the server address
    if (bind(_socket_fd, (struct sockaddr*)&server_address, sizeof(server_address)) < 0) {
        std::cerr << "Failed to bind socket. errno: " << errno << " - " << "  " << strerror(errno) << std::endl;
        return;
    }

    // listen for incoming connections (max 5 connections)
    if (listen(_socket_fd, 5) < 0) {
        std::cerr << "Failed to listen for connections. errno: " << errno << " - " << "  " << strerror(errno) << std::endl;
        return;
    }

    // create a shared file for all threads to store data. delete all previous data if it exists
    _log_file.open("log.txt", std::ios::out | std::ios::trunc);
    if (!_log_file.is_open()) {
        std::cerr << "Error opening file. Errno: " << errno << " " << strerror(errno) << std::endl;
        return;
    }

    std::cout << "Server listening on port " << _server_port << std::endl;
    system("hostname -I | awk '{print \"Server IP Address: \"$1}'");
    std::cout << "Log file created" <<std::endl;
    std::cout << "Server started" << std::endl;

    return;
}

PangoServerClass::~PangoServerClass(){
    // close the socket
    _accept_new_connections = false;
    _client_thread_running = false;
    
    // close all threads
    _accept_clients_thread.join();

    for(auto& thread : _client_threads){
        thread.join();
    }

    // close the socket and log file
    close(_socket_fd);
    _log_file.close();

    return;
}

bool PangoServerClass::run(){
    // Accept incoming connections
    _client_thread_running = true;
    _accept_new_connections = true;

    // create thread to accept incoming connections
    _accept_clients_thread = std::thread(&PangoServerClass::acceptClientsThread, this);

    std::cout << "Server listening for incoming connections...\n";

    return true;
}

void PangoServerClass::acceptClientsThread(){
    while(_accept_new_connections){
        if (!acceptConnections()) {
            std::cerr << "Failed to accept connection. Errno " << errno << "  " << strerror(errno) << std::endl;
        }
    }
    std::cout << "Exiting acceptClientsThread" << std::endl;
}

void PangoServerClass::clientThread(int client_socket){
    // Read messages from client socket
    PANGO_CLIENT_MSG_T client_msg;
    ssize_t bytes_read = 0;

    uint8_t buffer[1024] = {0};

    while(_client_thread_running){
        bytes_read = recv(client_socket, &buffer, sizeof(buffer)-1, 0);
        if(bytes_read == sizeof(client_msg)){
            memcpy(&client_msg, buffer, sizeof(client_msg));
            std::cout << "Client [" << client_msg.client_id << "] sent message: lat: " << client_msg.gps_data.latitude << std::endl;
            updateLog(client_msg);
        }
        else{
            // do nothing. read again
        }
    }
    std::cout << "Exiting client [" << client_msg.client_id << "] Thread" << std::endl;
}

bool PangoServerClass::acceptConnections(){
    sockaddr_in client_address{};
    socklen_t client_address_len = sizeof(client_address);

    // Accept incoming connections
    int client_socket = accept(_socket_fd, (struct sockaddr*)&client_address, &client_address_len);
    if (client_socket < 0) {
        std::cerr << "Failed to accept connection" << std::endl;
        return false;
    }
    else{
        std::cout << "Connection accepted from " << inet_ntoa(client_address.sin_addr) << ":" << ntohs(client_address.sin_port) << std::endl;
    }

    _client_threads.push_back(std::thread(&PangoServerClass::clientThread, this, client_socket));

    return true;
}

bool PangoServerClass::updateLog(PANGO_CLIENT_MSG_T client_msg){
    std::time_t client_timestamp = std::chrono::system_clock::to_time_t(client_msg.gps_data.timestamp);
    std::string timestamp_str = std::ctime(&client_timestamp);
    unsigned long int timestamp_int = static_cast<long>(client_msg.gps_data.timestamp.time_since_epoch().count());

    _log_mutex.lock();
    _log_file << "id," << client_msg.client_id  << ",timestamp," << timestamp_int <<",latlong," << client_msg.gps_data.latitude <<","  << client_msg.gps_data.longitude << std::endl;
    _log_mutex.unlock();

    return true;
}


} // namespace PangoServer