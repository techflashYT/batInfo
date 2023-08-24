#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static inline void batErr() {
	perror("Failed to find the battery");
	fputs("Please look for any errors above. Likely 'findbat' just isn't installed, or your computer doesn't have a battery.\r\n", stderr);
	exit(1);
}

int main() {
	FILE* bat = popen("findbat", "r");
	if (bat == NULL) {
		batErr();
	}
	char path[100] = {0};
	fgets(path, sizeof(path), bat);


	if (path[0] == '\0') {
		batErr();
	}

	FILE* capacity = fopen(path, "r");
	if (capacity == NULL) {
		perror("Failed to open capacity file!");
		return 1;
	}
	int percent;
	fscanf(capacity, "%d", &percent);
	fclose(capacity);

	const char* pre;
	const char* post;
	if (percent < 35) {
		pre = "\033[1;31m!!! ";
		post = " !!! \033[0m";
	}
	else if (percent < 45) {
		pre = "\033[31m";
		post = "\033[0m";
	}
	else if (percent < 65) {
		pre = "\033[1;33m";
		post = "\033[0m";
	}
	else if (percent < 75) {
		pre = "\033[1;32m";
		post = "\033[0m";
	}
	else if (percent >= 75) {
		pre = "\033[32m";
		post = "\033[0m";
	}
	printf("BAT PERCENT: %s%d%s\n", pre, percent, post);

	FILE* status = fopen(path, "r");
	if (status == NULL) {
		printf("Failed to open status file!\n");
		return 1;
	}
	char statusStr[20];
	fscanf(status, "%s", statusStr);
	fclose(status);

	if (strcmp(statusStr, "Charging") == 0) {
		pre = "\033[32m";
	}
	else if (strcmp(statusStr, "Discharging") == 0) {
		pre = "\033[1;31m";
	}
	else {
		pre = "\033[1;33m";
	}

	FILE* current = fopen(path, "r");
	if (current == NULL) {
		printf("Failed to open current file!\n");
		return 1;
	}
	int currentNow;
	fscanf(current, "%d", &currentNow);
	fclose(current);

	if (currentNow < 1000000 && currentNow > 0) {
		pre = "\033[1;33mSlow ";
	}
	printf("BAT STATUS : %s%s\033[0m\n", pre, statusStr);

	return 0;
}
