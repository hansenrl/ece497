/***************************************
#* accel.c
#* 
#* 
#*
#*
***************************************/

#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/time.h>

#define ANINX 7
#define ANINY 3
#define ANINZ 1

int keepGoing = 1;

void signal_handler(int sig)
{
	printf( "Ctrl-C pressed, cleaning up and exiting..\n" );
	keepGoing = 0;
}

/*
*	read_anin
*
*	It passes the fileName instead of the integer because this will be called a lot, for a fixed number of filenames
*
*/
int readAnin(char * fileName){
	FILE *fp;
	char readValue[5];

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

long long timevalSubtract(struct timeval *difference,
             struct timeval *end_time,
             struct timeval *start_time
            )
{
  struct timeval temp_diff;

  if(difference==NULL)
  {
    difference=&temp_diff;
  }

  difference->tv_sec =end_time->tv_sec -start_time->tv_sec ;
  difference->tv_usec=end_time->tv_usec-start_time->tv_usec;

  /* Using while instead of if below makes the code slightly more robust. */

  while(difference->tv_usec<0)
  {
    difference->tv_usec+=1000000;
    difference->tv_sec -=1;
  }

  return 1000000LL*difference->tv_sec+
                   difference->tv_usec;

}

float averageValue(int anin, int sleepLength){
	char fileName[50];
	sprintf(fileName, "/sys/devices/platform/omap/tsc/ain%d", anin);
	int sum = 0, i = 0;

	for(i = 0; i < 1000; i++){
		usleep(sleepLength);
		sum += readAnin(fileName);
	}	
	return sum/(float)i;
}

int findVelocity(int anin){
	char fileName[50];
	sprintf(fileName, "/sys/devices/platform/omap/tsc/ain%d", anin);
	
	struct timeval startTime,endTime, globalStartTime;
	long long timeDiff;
	int value1, value2, i;
	long long sum = 0;
	double integralResult = 0.0;
	float average;

	printf("Average value: %2.2f\n", average = averageValue(ANINX, 100));

	gettimeofday(&startTime,0x0);
	globalStartTime.tv_sec = startTime.tv_sec;
	globalStartTime.tv_usec = startTime.tv_usec;

	for(i = 0; i < 100; i++){
		value1 = readAnin(fileName);
		usleep(500);
		gettimeofday(&endTime,0x0);
		timeDiff = timevalSubtract(NULL,&endTime,&startTime);
		startTime.tv_sec = endTime.tv_sec;
		startTime.tv_usec = endTime.tv_usec;
		sum += timeDiff * value1;
	}

	timeDiff = timevalSubtract(NULL,&endTime,&globalStartTime);
	integralResult = ((double) sum - (double) timeDiff * average) / 1000.0; //in somethings/ms
	printf("Global time difference: %lld\n", timeDiff);
	//printf("Start time:\t%ld\n",startTime.tv_usec);
	//printf("End time:\t%ld\n",endTime.tv_usec);
	//printf("Time difference: %lld, value1: %d\n", timeDiff, value1);
	printf("Sum: %lld, integral result: %2.2f\n", sum, integralResult);
}

int main(int argc, char **argv, char **envp){
	int i;

	char fileNameX[50], fileNameY[50], fileNameZ[50];
	sprintf(fileNameX, "/sys/devices/platform/omap/tsc/ain%d", ANINX);
	sprintf(fileNameY, "/sys/devices/platform/omap/tsc/ain%d", ANINY);
	sprintf(fileNameZ, "/sys/devices/platform/omap/tsc/ain%d", ANINZ);
	
	signal(SIGINT, signal_handler);

	findVelocity(ANINY);
/*
	for(i = 0; keepGoing;i++){
		printf("The value X was: %d\n", read_anin(fileNameX));
		printf("The value Y was: %d\n", read_anin(fileNameY));
		printf("The value Z was: %d\n", read_anin(fileNameZ));
	}
	printf("%d loops run\n",i);
*/
}
