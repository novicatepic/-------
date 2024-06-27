#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <gps.h>
#include <errno.h>
#include <unistd.h>

int main() {
    //Variable to hold GPS data
    struct gps_data_t gps_data;

    // Default port for GPSD is 2947, can be changed with -S command when starting gpsd
    // Attempting to connect to gpsd with gps_open function and store data
    if (gps_open("localhost", "2947", &gps_data) != 0) {
        fprintf(stderr, "Error connecting to GPSD: %s\n", gps_errstr(errno));
        return 1;
    }

    // Enable streaming of data, later it is disabled (after infinite while loop if it exists for whatever reason)
    // WATCH_ENABLE starts the streaming and WATCH_JSON specifies the format (in this case JSON) for the data that is goind to be received
    gps_stream(&gps_data, WATCH_ENABLE | WATCH_JSON, NULL);

while (1) {
    if (gps_waiting(&gps_data, 5000000)) {  // 5 second timeout
        if (gps_read(&gps_data, NULL, 0) == -1) {
            fprintf(stderr, "Error reading GPS data: %s\n", gps_errstr(errno));
            return 1;
        } else {
            //For each parametar bitwise & is used to check if the parameter is set
            if (gps_data.set) 
                printf("GPS is online\n");
            if (gps_data.set & TIME_SET) 
                printf("Time: %ld.%09ld\n", (long)gps_data.fix.time.tv_sec, (long)gps_data.fix.time.tv_nsec);
            if (gps_data.set & LATLON_SET)
                printf("Latitude: %f, Longitude: %f\n", gps_data.fix.latitude, gps_data.fix.longitude);
            if (gps_data.set & ALTITUDE_SET)
                printf("Altitude: %f\n", gps_data.fix.altitude);
            if (gps_data.set & SPEED_SET)
                printf("Speed: %f\n", gps_data.fix.speed);
            if (gps_data.set & TRACK_SET)
                printf("Track: %f\n", gps_data.fix.track);
            if (gps_data.set & CLIMB_SET)
                printf("Climb: %f\n", gps_data.fix.climb);
            if (gps_data.set & MODE_SET)
                printf("Mode: %d\n", gps_data.fix.mode);
            if (gps_data.set & DOP_SET)
                printf("DOP: {pDOP: %f, hDOP: %f, vDOP: %f}\n", gps_data.dop.pdop, gps_data.dop.hdop, gps_data.dop.vdop);
            if (gps_data.set & SATELLITE_SET) {
                printf("Satellites visible: %d\n", gps_data.satellites_visible);
                for (int i = 0; i < gps_data.satellites_visible; i++) {
                    printf("Satellite PRN: %d, Elevation: %f, Azimuth: %f, SNR: %f\n",
                        gps_data.skyview[i].PRN,
                        gps_data.skyview[i].elevation,
                        gps_data.skyview[i].azimuth,
                        gps_data.skyview[i].ss);
                }
            }
        }
    } else {
    	printf("GPS STOPPED WAITING!\n");
    }
}
    // Disable streaming and close the connection
    gps_stream(&gps_data, WATCH_DISABLE, NULL);
    gps_close(&gps_data);

    return 0;
}
