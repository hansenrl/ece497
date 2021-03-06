/* pwmLib.c
 * 
 * Contains various libraries for interfacing with PWMs
 *
 */

#include "pwmLib.h"

#define SYSFS_PWM_DIR "/sys/class/pwm"

FILE *fp;

int set_pin_mux(int num, int letter){
	char fileName[50];
	char setValue[5];

	if(num == 1 && letter == 0){
		strcpy(fileName,"/sys/kernel/debug/omap_mux/gpmc_a2");
	} else {
		printf("Sorry, I don't know how to pin mux that....\n");
		printf("Right now, I only know how to pin mux ehrpwm.1:A (numbers 1 0)\n\n");
		printf("If you know what you're doing and have already pin mux'd your output,\nit shouldn't"
			" be a problem. If you don't know what pin muxing is you might \nhave unexpected results.\n\n");
		return -1;
	}
	

	if ((fp = fopen(fileName,  "rb+")) == NULL) {
		printf("Cannot open pin mux file.\n");
		return 1;
	}

	//Set pointer to begining of the file
	rewind(fp);
	//Write our value of "out" to the file
	strcpy(setValue, "6");
	fwrite(&setValue, sizeof(char), strlen(setValue), fp);
	fclose(fp);

	return 0;
}

int set_pwm_value(int num, int letter, char * func, int value){
	char fileName[50]; 
	char setValue[10];
	//printf(SYSFS_PWM_DIR "/ehrpwm.%d:%d/%s\n", num, letter, func);

	sprintf(fileName, SYSFS_PWM_DIR "/ehrpwm.%d:%d/%s", num, letter, func);
	//printf("fileName: %s, len: %d \n", fileName, strlen(fileName));

	if ((fp = fopen(fileName,  "rb+")) == NULL) {
		printf("Cannot open pwm file %s.\n", func);
		return 1;
	}

	//Set pointer to begining of the file
	rewind(fp);
	//Write our value of "out" to the file
	sprintf(setValue, "%d", value);
	//printf("setValue: %s, len: %d \n", setValue, strlen(setValue));
	fwrite(&setValue, sizeof(char), strlen(setValue), fp);
	fclose(fp);

	return 0;
}

int pwm_update_duty_cycle(int num, int letter, int dutyCycle){
	set_pwm_value(num,letter,"duty_percent",dutyCycle);
}

int pwm_update_frequency(int num, int letter, int period_freq){
	set_pwm_value(num,letter,"duty_percent",0);
	set_pwm_value(num,letter,"period_freq",period_freq);
}

int pwm_stop(int num, int letter){
	set_pwm_value(num,letter,"run",0);
}

int pwm_on(int num, int letter, int period_freq, int dutyCycle){
	if(set_pin_mux(num,letter) != 0){
		return -1;
	}
	set_pwm_value(num,letter,"run",1);
	set_pwm_value(num,letter,"duty_percent",0);
	set_pwm_value(num,letter,"period_freq",period_freq);
	set_pwm_value(num,letter,"duty_percent",dutyCycle);

	return 0;
}
