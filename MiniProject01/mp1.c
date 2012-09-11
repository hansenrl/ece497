/* mp1.c
 * 
 * Miniproject 1
 * Author: Ross Hansen
 *
 * Turns on and off LEDs based off an interrupt-driven switch input,
 * reads temperature data from i2c, and dims a flashing LED based on an analog input.
 *
 * --------------------------------------------------------------------------------------------
 * Usage: mp1 <gpio-pin-switch> <gpio-pin-LED1> <i2c bus> <i2c addr> <pwm-num1> <pwm-num2> <anin-pin>
 * --------------------------------------------------------------------------------------------
 * 
 * For example, the command "mp1 7 60 3 74 1 0 6" uses GPIO 7 for the switch,
 * GPIO 60 for the LED, I2C bus 3, I2C address 74 (0x4a),
 * ehrpwm.1:A for the pwm LED, and analog input 6 (ain6 in Linux)
 *
 * Requires the pwmLib.c library. Compile with gcc: "gcc mp1.c pwmLib.c -o mp1"
 *
 *
 * GPIO source code based off of code from RidgeRun (see license information below)
 *
 */

/* Copyright (c) 2011, RidgeRun
 * All rights reserved.
 *
From https://www.ridgerun.com/developer/wiki/index.php/Gpio-int-test.c

 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 *    This product includes software developed by the RidgeRun.
 * 4. Neither the name of the RidgeRun nor the
 *    names of its contributors may be used to endorse or promote products
 *    derived from this software without specific prior written permission.
 * 
 * THIS SOFTWARE IS PROVIDED BY RIDGERUN ''AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL RIDGERUN BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <poll.h>
#include <signal.h>	// Defines signal-handling functions (i.e. trap Ctrl-C)

// for i2c
#include "i2c-dev.h"
#include <fcntl.h>

// pwm libraries
#include "pwmLib.h"

 /****************************************************************
 * Constants
 ****************************************************************/
 
#define SYSFS_GPIO_DIR "/sys/class/gpio"
#define POLL_TIMEOUT (1 * 1000) /* 3 seconds */
#define MAX_BUF 64

/****************************************************************
 * Global variables
 ****************************************************************/
int keepgoing = 1;	// Set to 0 when ctrl-c is pressed

/****************************************************************
 * signal_handler
 ****************************************************************/
// Callback called when SIGINT is sent to the process (Ctrl-C)
void signal_handler(int sig)
{
	printf( "Ctrl-C pressed, cleaning up and exiting..\n" );
	pwm_stop(1, 0);
	keepgoing = 0;
}

/****************************************************************
 * gpio_export
 ****************************************************************/
int gpio_export(unsigned int gpio)
{
	int fd, len;
	char buf[MAX_BUF];
 
	fd = open(SYSFS_GPIO_DIR "/export", O_WRONLY);
	if (fd < 0) {
		perror("gpio/export");
		return fd;
	}
 
	len = snprintf(buf, sizeof(buf), "%d", gpio);
	write(fd, buf, len);
	close(fd);
 
	return 0;
}

/****************************************************************
 * gpio_unexport
 ****************************************************************/
int gpio_unexport(unsigned int gpio)
{
	int fd, len;
	char buf[MAX_BUF];
 
	fd = open(SYSFS_GPIO_DIR "/unexport", O_WRONLY);
	if (fd < 0) {
		perror("gpio/export");
		return fd;
	}
 
	len = snprintf(buf, sizeof(buf), "%d", gpio);
	write(fd, buf, len);
	close(fd);
	return 0;
}

/****************************************************************
 * gpio_set_dir
 ****************************************************************/
int gpio_set_dir(unsigned int gpio, unsigned int out_flag)
{
	int fd, len;
	char buf[MAX_BUF];
 
	len = snprintf(buf, sizeof(buf), SYSFS_GPIO_DIR  "/gpio%d/direction", gpio);
 
	fd = open(buf, O_WRONLY);
	if (fd < 0) {
		perror("gpio/direction");
		return fd;
	}
 
	if (out_flag)
		write(fd, "out", 4);
	else
		write(fd, "in", 3);
 
	close(fd);
	return 0;
}

/****************************************************************
 * gpio_set_value
 ****************************************************************/
int gpio_set_value(unsigned int gpio, unsigned int value)
{
	int fd, len;
	char buf[MAX_BUF];
 
	len = snprintf(buf, sizeof(buf), SYSFS_GPIO_DIR "/gpio%d/value", gpio);
 
	fd = open(buf, O_WRONLY);
	if (fd < 0) {
		perror("gpio/set-value");
		return fd;
	}
 
	if (value)
		write(fd, "1", 2);
	else
		write(fd, "0", 2);
	//printf("Set value for GPIO %d to %d", gpio, value); 
	close(fd);
	return 0;
}

/****************************************************************
 * gpio_get_value
 ****************************************************************/
int gpio_get_value(unsigned int gpio, unsigned int *value)
{
	int fd, len;
	char buf[MAX_BUF];
	char ch;

	len = snprintf(buf, sizeof(buf), SYSFS_GPIO_DIR "/gpio%d/value", gpio);
 
	fd = open(buf, O_RDONLY);
	if (fd < 0) {
		perror("gpio/get-value");
		return fd;
	}
 
	read(fd, &ch, 1);

	if (ch != '0') {
		*value = 1;
	} else {
		*value = 0;
	}
 
	close(fd);
	return 0;
}


/****************************************************************
 * gpio_set_edge
 ****************************************************************/

int gpio_set_edge(unsigned int gpio, char *edge)
{
	int fd, len;
	char buf[MAX_BUF];

	len = snprintf(buf, sizeof(buf), SYSFS_GPIO_DIR "/gpio%d/edge", gpio);
 
	fd = open(buf, O_WRONLY);
	if (fd < 0) {
		perror("gpio/set-edge");
		return fd;
	}
 
	write(fd, edge, strlen(edge) + 1); 
	close(fd);
	return 0;
}

/****************************************************************
 * gpio_fd_open
 ****************************************************************/

int gpio_fd_open(unsigned int gpio)
{
	int fd, len;
	char buf[MAX_BUF];

	len = snprintf(buf, sizeof(buf), SYSFS_GPIO_DIR "/gpio%d/value", gpio);
 
	fd = open(buf, O_RDONLY | O_NONBLOCK );
	if (fd < 0) {
		perror("gpio/fd_open");
	}
	return fd;
}

/****************************************************************
 * gpio_fd_close
 ****************************************************************/

int gpio_fd_close(int fd)
{
	return close(fd);
}

int setup_gpio(unsigned int gpio){
	int gpio_fd;

	gpio_export(gpio);
	gpio_set_dir(gpio, 0);
	gpio_set_edge(gpio, "both");  // Can be rising, falling or both
	gpio_fd = gpio_fd_open(gpio);

	gpio_export(60);
	gpio_set_dir(60,1);

	return gpio_fd;
}

int read_i2c(int i2cbus, int address){
	char *end;
	int res, size, file;
	int daddress;
	char filename[20];

	/* handle (optional) flags first 
	if(argc < 3) {
		fprintf(stderr, "Usage:  %s <i2c-bus> <i2c-address> <register>\n", argv[0]);
		exit(1);
	}*/

	daddress = 0;
	size = I2C_SMBUS_BYTE;

	sprintf(filename, "/dev/i2c-%d", i2cbus);
	file = open(filename, O_RDWR);
	if (file<0) {
		if (errno == ENOENT) {
			fprintf(stderr, "Error: Could not open file "
				"/dev/i2c-%d: %s\n", i2cbus, strerror(ENOENT));
		} else {
			fprintf(stderr, "Error: Could not open file "
				"`%s': %s\n", filename, strerror(errno));
			if (errno == EACCES)
				fprintf(stderr, "Run as root?\n");
		}
		return -1;
	}

	if (ioctl(file, I2C_SLAVE, address) < 0) {
		fprintf(stderr,
			"Error: Could not set address to 0x%02x: %s\n",
			address, strerror(errno));
		return -errno;
	}

	res = i2c_smbus_write_byte(file, daddress);
	if (res < 0) {
		fprintf(stderr, "Warning - write failed, filename=%s, daddress=%d\n",
			filename, daddress);
	}
	res = i2c_smbus_read_byte_data(file, daddress);
	close(file);

	if (res < 0) {
		fprintf(stderr, "Error: Read failed, res=%d\n", res);
		return -1;
	}

	printf("Temperature: %d deg C \n", res, res);
	return res;
}

int read_anin(int anin){
	FILE *fp;
	char fileName[60]; 
	char readValue[5];

	sprintf(fileName, "/sys/devices/platform/omap/tsc/ain%d", anin);

	if ((fp = fopen(fileName,  "r")) == NULL) {
		printf("Cannot open anin file: %s.\n", fileName);
		exit(1);
	}

	//Set pointer to begining of the file
	rewind(fp);
	//Write our value of "out" to the file
	fread(readValue, sizeof(char), 10, fp);
	readValue[4] = '\0'; //for some reason when reading 4 digit numbers you get weird garbage after the value
	//printf("anin: %s\n", readValue);
	//fwrite(&setValue, sizeof(char), strlen(setValue), fp);
	fclose(fp);

	return atoi(readValue);
}

/****************************************************************
 * Main
 ****************************************************************/
int main(int argc, char **argv, char **envp)
{

//USE ME: ./mp1 7 60 3 74 1 0 6

	struct pollfd fdset[2];
	int nfds = 2;
	int gpio_fd, timeout, rc, gpio_led;
	char *buf[MAX_BUF];
	unsigned int gpio;
	int len, pot_input, pwm_num, pwm_letter, anin_pin;

	if (argc < 8) {
		printf("Usage: mp1 <gpio-pin-switch> <gpio-pin-LED1> <i2c bus> <i2c addr> <pwm-num1> <pwm-num2> <anin-pin>\n\n");
		printf("For example, the command \"mp1 7 60 3 74 1 0 6\" uses GPIO 7 for the switch,\n"
			"GPIO 60 for the LED, I2C bus 3, I2C address 74 (0x4a),\n"
			"ehrpwm.1:A for the pwm LED, and analog input 6 (ain6 in Linux)\n\n");
		printf("Turns on and off lights based off an interrupt-driven switch input,\nreads temp from i2c, and dims the flashing LED based on an analog input.\n\n");
		exit(-1);
	}

	// Set the signal callback for Ctrl-C
	signal(SIGINT, signal_handler);
	gpio = atoi(argv[1]);
	gpio_fd = setup_gpio(gpio);

	gpio_led = atoi(argv[2]);

	pwm_num = atoi(argv[5]);
	pwm_letter = atoi(argv[6]);
	anin_pin = atoi(argv[7]);

	timeout = POLL_TIMEOUT;

	if(pwm_on(pwm_num,pwm_letter,50,50) != 0){
		printf("Something went wrong with the pwm's...probably the MUXing.\nIt may still work, but no gaurantees.\n");
	}
 
	while (keepgoing) {
		memset((void*)fdset, 0, sizeof(fdset));

		fdset[0].fd = STDIN_FILENO;
		fdset[0].events = POLLIN;
      
		fdset[1].fd = gpio_fd;
		fdset[1].events = POLLPRI;

		rc = poll(fdset, nfds, timeout);      

		if (rc < 0) {
			printf("\npoll() failed!\n");
			return -1;
		}
      
		if (rc == 0) {
			printf(".");
		}
            
		if (fdset[1].revents & POLLPRI) {
			lseek(fdset[1].fd, 0, SEEK_SET);  // Read from the start of the file
			len = read(fdset[1].fd, buf, MAX_BUF);
			//printf("\npoll() GPIO %d interrupt occurred, value=%c, len=%d\n", gpio, buf[0], len);
			if((unsigned int) buf[0] & 0x01 == 1){
				gpio_set_value(gpio_led, 1);
				pwm_update_frequency(pwm_num,pwm_letter,100);
			} else {
				gpio_set_value(gpio_led, 0);
				pwm_update_frequency(pwm_num,pwm_letter,3);
			}
			
			read_i2c(atoi(argv[3]),atoi(argv[4]));
			//printf("Set the value to %d", (unsigned int) buf[0]);
		}

		if (fdset[0].revents & POLLIN) {
			(void)read(fdset[0].fd, buf, 1);
			printf("\npoll() stdin read 0x%2.2X\n", (unsigned int) buf[0]);
		}

		pot_input = read_anin(anin_pin);
		//printf("Value from read_anin: %d\n", pot_input);
		//pwm_update_frequency(1,0,100*pot_input/4100.0);
		pwm_update_duty_cycle(pwm_num,pwm_letter,100*pot_input/4100.0);

		fflush(stdout);
	}

	gpio_fd_close(gpio_fd);
	return 0;
}

