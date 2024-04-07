#include <csignal>
#include <cstdlib>
#include <iostream>

#include "PangoBBG.hpp"

using namespace PangoBBG;
PangoBBGNode pango_bbg;

void signalHandler(int signal) {
    std::cout << "\n\nHalt signal received. closing pango server..." << std::endl;
    // Perform cleanup or other actions...
    pango_bbg.~PangoBBGNode(); // Terminate the program with the received signal
    std::exit(signal); // Terminate the program with the received signal
}

int main (int argc, char *argv[]) {
    std::signal(SIGINT, signalHandler);
    std::signal(SIGTERM, signalHandler);
    std::signal(SIGQUIT, signalHandler);
    std::cout << "Starting PangoBBGNode..." << std::endl;
    pango_bbg.run();

    // Wait for halt signal (Ctrl+C)
    while (true) {
        // Do nothing here. The signal handler will take care of the rest
    }

    return 0;
}