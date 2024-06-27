#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <termios.h>
#include <ctype.h>

#define GPS_UART_DEVICE "/dev/ttyS1"

int configure_uart(int uart_fd) {
    struct termios options;

    // Get current UART options
    if (tcgetattr(uart_fd, &options) != 0) {
        perror("tcgetattr");
        return -1;
    }

    // Set baud rate to 9600
    if (cfsetispeed(&options, B9600) != 0 || cfsetospeed(&options, B9600) != 0) {
        perror("cfsetispeed/cfsetospeed");
        return -1;
    }

    // 8 data bits, no parity, 1 stop bit
    options.c_cflag &= ~PARENB; // No parity
    options.c_cflag &= ~CSTOPB; // 1 stop bit
    options.c_cflag &= ~CSIZE;  // Mask the character size bits
    options.c_cflag |= CS8;     // Select 8 data bits

    // Disable hardware flow control
    options.c_cflag &= ~CRTSCTS;

    // Enable receiver and set local mode
    options.c_cflag |= (CLOCAL | CREAD);

    // Raw input mode (no processing of input characters)
    options.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG);

    // Raw output mode
    options.c_oflag &= ~OPOST;

    // Set input mode (non-canonical, no echo,...)
    options.c_iflag &= ~(IXON | IXOFF | IXANY); // Turn off software flow control
    options.c_iflag &= ~(ICRNL | INLCR); // Disable CR to NL mapping

    // Apply the options
    tcflush(uart_fd, TCIFLUSH);
    if (tcsetattr(uart_fd, TCSANOW, &options) != 0) {
        perror("tcsetattr");
        return -1;
    }

    return 0;
}

void read_gps_data(int uart_fd) {
    printf("In read\n");
    char buffer[512];
    int n;
    char sentence[1024];
    int sentence_index = 0;
    int start_found = 0;

    while (1) {
        // Read data from UART
        n = read(uart_fd, buffer, sizeof(buffer) - 1);
        printf("n = %d\n", n);
        printf("Buffer = %s\n", buffer);
        if (n > 0) {
            buffer[n] = '\0'; // Null-terminate the string
	    
            // Process buffer to extract NMEA sentences
            for (int i = 0; i < n; i++) {
                if (buffer[i] == '$') {
                    start_found = 1;
                    sentence_index = 0;
                }

                if (start_found) {
                    sentence[sentence_index++] = buffer[i];
                    if (buffer[i] == '\n') {
                        sentence[sentence_index] = '\0';
                        printf("%s", sentence);
                        start_found = 0;
                    } else if (sentence_index >= sizeof(sentence) - 1) {
                        // In case the sentence is too long (shouldn't happen with valid NMEA)
                        sentence[sentence_index] = '\0';
                        printf("%s\n", sentence);
                        start_found = 0;
                    }
                }
            }
        }
        
        usleep(500000); // Sleep for 500 milliseconds
    }
}

int main() {
    int uart_fd;
    
    // Open the UART device
    uart_fd = open(GPS_UART_DEVICE, O_RDWR | O_NOCTTY | O_NDELAY);
    if (uart_fd == -1) {
        printf("Unable to open UART device");
        return 1;
    }
    printf("After open\n");
    
    // Configure the UART
    //configure_uart(uart_fd);
    
    printf("After configure\n");
    
    //  and print GPS data
    read_gps_data(uart_fd);
    
    // Close the UART device
    close(uart_fd);
    
    return 0;
}
