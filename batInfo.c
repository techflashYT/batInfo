#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

static inline void batErr() {
	perror("Failed to find the battery");
	fputs("Please look for any errors above. Likely 'findbat' just isn't installed, or your computer doesn't have a battery.\r\n", stderr);
	exit(1);
}

static inline void modPath(char *tmpPath, char *basePath, char *new) {
	strcpy(tmpPath, basePath);
	strcat(tmpPath, new);
}

int main() {
	FILE* bat = popen("findbat", "r");
	if (bat == NULL) {batErr();}

	char basePath[100] = {0};
	char tmpPath[100] = {0};
	fgets(basePath, sizeof(basePath), bat);
	pclose(bat);

	if (basePath[0] == '\0') {batErr();}

	modPath(tmpPath, basePath, "/capacity");

	FILE* fh = fopen(tmpPath, "r");
	if (fh == NULL) {
		perror("Failed to open capacity file!");
		return 1;
	}

	int percent;
	fscanf(fh, "%d", &percent);
	fclose(fh);

	const char* pre = "";
	const char* post = "\e[0m";
	if (percent < 35) {
		pre = "\e[1;31m!!! ";
		post = " !!! \e[0m";
	}
	else if (percent <  45) {pre = "\e[31m";}
	else if (percent <  65) {pre = "\e[1;33m";}
	else if (percent <  75) {pre = "\e[1;32m";}
	else if (percent >= 75) {pre = "\e[32m";}
	printf("BAT PERCENT: %s%d%%%s\n", pre, percent, post);

	modPath(tmpPath, basePath, "/status");
	fh = fopen(tmpPath, "r");
	if (fh == NULL) {
		perror("Failed to open status file!");
		return 1;
	}
	char statusStr[20];
	fscanf(fh, "%s", statusStr);
	fclose(fh);

	if      (strcmp(statusStr, "Charging")    == 0) {pre = "\e[32m";}
	else if (strcmp(statusStr, "Discharging") == 0) {pre = "\e[1;31m";}
	else                                            {pre = "\e[1;33m";}

	modPath(tmpPath, basePath, "/current_now");
	fh = fopen(tmpPath, "r");
	if (fh == NULL) {
		printf("Failed to open current file!\n");
		return 1;
	}
	int currentNow;
	fscanf(fh, "%d", &currentNow);
	fclose(fh);

	if (currentNow < 1000000 && currentNow > 0) {pre = "\e[1;33mSlow ";}
	printf("BAT STATUS : %s%s\e[0m\n", pre, statusStr);

	// print the battery current nicely
	// step 1: reverse the negative (and log it for later) to make the math easier

	pre = "\e[1;31m";
	// currentNow = 1000000;
	bool negative = false;
	bool amps = false;
	if (currentNow < 0) {currentNow = -currentNow; negative = true;}

	// step 1: detect if it is more than 1A

	if (currentNow > 1000000) {amps = true;}

	// step 2: colors
	if (!negative) {
		// > 1000mA
		if (currentNow >= 1000000) {pre = "\e[1;33m";}
		// > 1500mA
		if (currentNow >= 1500000) {pre = "\e[1;32m";}
		// > 2000mA
		if (currentNow >= 2000000) {pre = "\e[32m";}
	}

	// step 3: print it
	currentNow /= 1000;
	printf("BAT CURRENT: %s%s%d%s\x1b[0m\r\n", pre, negative ? "-" : "", amps ? currentNow / 1000 : currentNow, amps ? "A" : "mA");

	return 0;
}
