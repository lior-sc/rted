#include "PangoServer.hpp"
#include <csignal>
#include <cstdlib>
#include <iostream>

void signalHandler(int signal) {
    std::cout << "\n\nHalt signal received. closing pango server..." << std::endl;
    // Perform cleanup or other actions...
    std::exit(signal); // Terminate the program with the received signal
}

int main (int argc, char *argv[]) {
    std::signal(SIGINT, signalHandler);
    PangoServer::PangoServerClass pango_server(8080);
    pango_server.run();

    // Wait for halt signal (Ctrl+C)
    while (true) {
        // Do nothing here. The signal handler will take care of the rest
        }
}

