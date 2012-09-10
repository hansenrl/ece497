#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int set_pin_mux(int num, int letter);
int set_pwm_value(int num, int letter, char * func, int value);
int pwm_on(int num, int letter, int period_freq, int dutyCycle);

int pwm_update_duty_cycle(int num, int letter, int dutyCycle);
int pwm_update_frequency(int num, int letter, int period_freq);
