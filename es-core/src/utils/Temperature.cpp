#include "Temperature.h"

#if defined(__linux__)
#include <stdio.h>
#include <dirent.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#endif

double ReadTemperature()
{
#if defined(__linux__)
	FILE *temperatureFile;
	double T;
	temperatureFile = fopen("/sys/class/thermal/thermal_zone0/temp", "r");
	if (temperatureFile == NULL)
		; //print some message
	fscanf(temperatureFile, "%lf", &T);
	T /= 1000;
	//printf("The temperature is %6.3f C.\n", T);
	fclose(temperatureFile);
	return T;
#else
	static int temp = 45;
	static bool up = true;
	temp += up ? 1 : -1;
	if (temp >= 55 || temp <= 45) { up = !up; }
	
	return temp;
#endif
}