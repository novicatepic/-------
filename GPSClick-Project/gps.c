#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <gps.h>
#include <errno.h>
#include <unistd.h>

int main() {
    //Variable to hold GPS data
    struct gps_data_t gps_data;

    // Default port for GPSD is 2947, can be changed with -S command when starting gpsd (works only on native OS)
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
      		//tv_sec: This is the number of seconds since the Unix epoch (January 1, 1970, 00:00:00 UTC).
      		//tv_nsec: This is the number of nanoseconds past the last full second.
                printf("Time: %ld.%09ld\n", (long)gps_data.fix.time.tv_sec, (long)gps_data.fix.time.tv_nsec);
            if (gps_data.set & LATLON_SET)
                printf("Latitude: %f, Longitude: %f\n", gps_data.fix.latitude, gps_data.fix.longitude);
            if (gps_data.set & ALTITUDE_SET)
                printf("Altitude: %f\n", gps_data.fix.altitude);
            if (gps_data.set & SPEED_SET)
            	//gps_data.fix.speed represents the speed at which the GPS receiver is moving
                printf("Speed: %f\n", gps_data.fix.speed);
            if (gps_data.set & TRACK_SET)
            	//gps_data.fix.track represents the direction in which the GPS receiver is moving
            	//0 degrees representing north, 90 degrees representing east, 180 degrees representing south, and 270 degrees representing west
                printf("Track: %f\n", gps_data.fix.track);
            if (gps_data.set & CLIMB_SET)
            	//rate of vertical movement (m/s)
                printf("Climb: %f\n", gps_data.fix.climb);
            if (gps_data.set & MODE_SET)
            	//0: No fix
	    	//1: 2D fix (only latitude and longitude)
		//2: 3D fix (latitude, longitude, and altitude)
                printf("Mode: %d\n", gps_data.fix.mode);
            if (gps_data.set & DOP_SET)
            	//accuracy of horizontal precision (hdop), vertical precision (vdop) and both combined (pdop)
                printf("DOP: {pDOP: %f, hDOP: %f, vDOP: %f}\n", gps_data.dop.pdop, gps_data.dop.hdop, gps_data.dop.vdop);
            if (gps_data.set & SATELLITE_SET) {
                printf("Satellites visible: %d\n", gps_data.satellites_visible);
                for (int i = 0; i < gps_data.satellites_visible; i++) {
                    printf("Satellite PRN: %d, Elevation: %f, Azimuth: %f, SNR: %f\n",
                        gps_data.skyview[i].PRN, //Pseudo-random noise code, unique for each satellite
                        gps_data.skyview[i].elevation, //angle between the satellite and the local horizon at the GPS receiver's location, 0° (satellite on the horizon) to 90° (satellite directly //// overhead).
                        gps_data.skyview[i].azimuth, //angle between the north direction and the projection of the satellite's position onto the local horizontal plane, measured in degrees from true north, with values ranging from 0° (north) to 360° (full circle).
                        gps_data.skyview[i].ss); //signal to noise ratio, strength and quality of the satellite signal as received by the GPS receiver, in decibels
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
