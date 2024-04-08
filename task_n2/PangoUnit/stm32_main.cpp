#include <csignal>
#include <cstdlib>
#include <iostream>

#include "PangoSTM32.hpp"

using namespace PangoSTM32;
PangoSTM32Node pango_stm32;

void signalHandler(int signal) {
    std::cout << "\n\nHalt signal received. closing pango server..." << std::endl;
    // Perform cleanup or other actions...
    pango_stm32.~PangoSTM32Node(); // Terminate the program with the received signal
    std::exit(signal); // Terminate the program with the received signal
}

int main (int argc, char *argv[]) {
    std::signal(SIGINT, signalHandler);

    std::cout << "Starting PangoSTM32 node operation." << std::endl;
    pango_stm32.run();

    // Wait for halt signal (Ctrl+C)
    while (true) {
        // Do nothing here. The signal handler will take care of the rest
    }
    
    return 0;
}