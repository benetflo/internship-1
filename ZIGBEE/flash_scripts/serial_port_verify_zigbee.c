#include <stdio.h>		// printf, perror, FILE*
#include <stdlib.h>		// exit, malloc, free, system
#include <string.h>		// memset, memcpy, strcmp
#include <unistd.h>		// read, write, close, usleep
#include <fcntl.h>		// open, 0_RDWR, O_NOCTTY, O_NDELAY
#include <termios.h>	// struct termios, tcgetattr, tcsetattr
#include <errno.h>		// errno
#include <signal.h>

#define PORTNAME ("/dev/ttyAMA3")

//void cleanup(int sig, int *fd); // FIX cleanup for SIGINT

int main(void)
{
    //signal(SIGINT, cleanup);

    int fd = open(PORTNAME, O_RDWR);
    
    //check for errors
    if (fd < 0)
    {
        printf("Error %d, from open: %s\n", errno, strerror(errno));
    }
    
    // create new termios struct
    struct termios tty;
    
    // Read in existing settings, and handle any error
    // NOTE: This is important! POSIX states that the struct passed to tcsetattr()
    // must have been initialized with a call to tcgetattr() overwise behaviour
    // is undefined
    if(tcgetattr(fd, &tty) != 0)
    {
        printf("Error %d from tcgetattr: %s\n", errno, strerror(errno));
    }
    
    // SET PARITY
    tty.c_cflag &= ~(PARENB); // disable parity (most common)
    //tty.c_cflag |= (PARENB); // enable parity - can try if problems appear 
    
    // SET NUMBER OF STOP BITS
    tty.c_cflag &= ~(CSTOPB); // one stop bit used (most common)
    //tty.c_cflag |= (CSTOPB); // two stop bits used - can try if problems appear
    
    // CLEAR ALL SIZE BITS FIRST
    tty.c_cflag &= ~(CSTOPB);
    tty.c_cflag |= (CS8); // decides how many bits that are transmitted per byte across the serial port.
    // CS8 MOST COMMON (CS5, CS6, CS7)
    
    // ENABLE OR DISABLE HARDWARE FLOW CONTROL (CRTSCTS)
    //tty.c_cflag &= ~(CRTSCTS);  // disable RTS/CTS hardware flow control
    //tty.c_cflag |= (CRTSCTS); // enable RTS/CTS hardware flow control
    
    tty.c_cflag |= CREAD | CLOCAL;
    
    tty.c_lflag &= ~(ICANON); // disable canonical mode 
    
    // ICANON probably makes these bits useless but clear these bits just in case
    tty.c_lflag &= ~(ECHO);
    tty.c_lflag &= ~(ECHOE);
    tty.c_lflag &= ~(ECHONL);
    
    tty.c_lflag &= ~(ISIG);
    
    tty.c_iflag &= ~(IXON | IXOFF | IXANY); // clearing these bits turns off software flow control
    
    tty.c_iflag &= ~(IGNBRK | BRKINT | PARMRK | ISTRIP | INLCR | IGNCR | ICRNL); //disable any special handling of received bytes
    
    tty.c_oflag &= ~(OPOST);
    tty.c_oflag &= ~(ONLCR);
    
    tty.c_cc[VTIME] = 10;
    tty.c_cc[VMIN] = 0;
    
    
    // SET BAUDRATE
    cfsetispeed(&tty, B115200);
    cfsetospeed(&tty, B115200);
    
    if(tcsetattr(fd, TCSANOW, &tty) != 0) // update with new settings
    {
        printf("Error %d from tcsetattr: %s\n", errno, strerror(errno));
    }
    
    char read_buffer[256];
    
    while(1)
    {
        int bytes_read = read(fd, &read_buffer, sizeof(read_buffer));
        
        if(bytes_read > 0)
        {
            read_buffer[bytes_read] = '\0';
            printf("Received: %s\n", read_buffer);
            ;
        }else if(bytes_read < 0)
        {
            //error
            printf("Error %d from read: %s\n", errno, strerror(errno));
        }
        
        printf("Read %d bytes. Received message: %.*s\n", bytes_read, read_buffer);
        usleep(100);
    }    
    
    close(fd); // close connection
    
    return 0;
}
