#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <string.h>
#include <unistd.h>

int main() {
	DIR *dir;
	struct dirent *entry;
	char path[256];

	dir = opendir("/sys/class/power_supply");
	if (dir == NULL) {
		perror("ERROR: Failed to open directory");
		exit(1);
	}

	while ((entry = readdir(dir)) != NULL) {
		if (strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "..") != 0) {
			snprintf(path, sizeof(path), "/sys/class/power_supply/%s/capacity", entry->d_name);
			if (access(path, F_OK) == 0) {
				// we found one with capacity
				printf("/sys/class/power_supply/%s", entry->d_name);
				closedir(dir);
				exit(0);
			}
		}
	}

	closedir(dir);
	fprintf(stderr, "FINDBAT-ERROR: No battery found!!!\n");
	exit(1);
}
