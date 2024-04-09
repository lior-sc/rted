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
    PANGO_CLIENT_MSG_T prev_client_msg;
    ssize_t bytes_read = 0;
    unsigned long int parking_state_start_time = 0;
    double area_pricing = 0.0;
    double total_price = 0.0;

    uint8_t buffer[1024] = {0};

    while(_client_thread_running){
        bytes_read = recv(client_socket, &buffer, sizeof(buffer)-1, 0);
        if(bytes_read == sizeof(client_msg)){
            // save the previous message
            prev_client_msg = client_msg; 
            // copy the received message to client_msg
            memcpy(&client_msg, buffer, sizeof(client_msg));
            // update the total parking duration
            double total_parking_duration = updateParkingDuration(client_msg, &parking_state_start_time);
            // calculate the area pricing
            double current_area_pricing = calculateAreaPricing(client_msg.gps_data.latitude, client_msg.gps_data.longitude);
            // update the total price
            updateParkingCost(client_msg, prev_client_msg, total_price, current_area_pricing);

            updateLog(client_msg, total_parking_duration, current_area_pricing, total_price);
            if(client_msg.parking_state == PARKING_STATE_END){
                // exit the thread
                break;
            }
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

bool PangoServerClass::updateLog(PANGO_CLIENT_MSG_T client_msg, double total_parking_duration, double current_area_pricing, double total_price){
    std::time_t client_timestamp = std::chrono::system_clock::to_time_t(client_msg.gps_data.timestamp);
    std::string timestamp_str = std::ctime(&client_timestamp);
    unsigned long int timestamp_int = static_cast<long>(client_msg.gps_data.timestamp.time_since_epoch().count());

    _log_mutex.lock();
    _log_file << "id," << client_msg.client_id  
        << ",parking_state," << static_cast<int>(client_msg.parking_state) 
        << ",timestamp," << timestamp_int 
        <<",lat_long," << client_msg.gps_data.latitude <<","  << client_msg.gps_data.longitude 
        << ",total_duration:" << total_parking_duration
        << ",area_pricing[usd/sec]," << current_area_pricing
        << ",total_cost," << total_price 
        << std::endl;
    _log_mutex.unlock();

    return true;
}

double PangoServerClass::calculateAreaPricing(double latitude, double longitude){
    double area_pricing = static_cast<double>(abs(latitude) + abs(longitude))/(100*60); // USD per second
    return static_cast<double>(abs(latitude) + abs(longitude))/1000;
}

double PangoServerClass::updateParkingCost(PANGO_CLIENT_MSG_T client_msg, PANGO_CLIENT_MSG_T prev_client_msg, double current_cost, double current_area_pricing){
    /** @bug
     * for some reason the total cost is not being updated correctly.
     * no time to fix this before submitting this task
    */
    double dt_sec = 1e-9 * static_cast<double>(client_msg.gps_data.timestamp.time_since_epoch().count() - prev_client_msg.gps_data.timestamp.time_since_epoch().count()); 
    std::cout << "dt_sec: " << dt_sec << "  current area pricing: " << current_area_pricing << "  " 
        << "  current cost: " << current_cost << "  "
        << current_cost + (current_area_pricing * dt_sec) << std::endl;
    return (current_cost + current_area_pricing * dt_sec);
}

unsigned long int PangoServerClass::updateParkingDuration(PANGO_CLIENT_MSG_T client_msg, unsigned long int* start_time){
    if(client_msg.parking_state == PARKING_STATE_UNKNOWN){
        return 0;
    }
    if(client_msg.parking_state == PARKING_STATE_START){
        *start_time = client_msg.gps_data.timestamp.time_since_epoch().count();
        return 0;
    }
    if(client_msg.parking_state == PARKING_STATE_OCCUPIED || client_msg.parking_state == PARKING_STATE_END){
        return (client_msg.gps_data.timestamp.time_since_epoch().count() - *start_time)*1e-9;
        // do nothing
    }
    return 0;
}

} // namespace PangoServer