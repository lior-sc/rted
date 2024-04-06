#ifndef GPS_HPP
#define GPS_HPP

#include <chrono>

typedef struct GPS_POINT_T {
    std::chrono::system_clock::time_point timestamp;
    double latitude;
    double longitude;
} GPS_POINT_T;



// Your code goes here

#endif // GPS_HPP