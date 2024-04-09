#ifndef GPS_HPP
#define GPS_HPP

#include <chrono>

typedef struct GPS_POINT_T {
    std::chrono::system_clock::time_point timestamp;
    double latitude;
    double longitude;
} GPS_POINT_T;

typedef struct PANGO_CLIENT_MSG_T{
    int16_t client_id;
    int8_t parking_state;
    GPS_POINT_T gps_data;
} Pango_MSG_T;


// Your code goes here

#endif // GPS_HPP