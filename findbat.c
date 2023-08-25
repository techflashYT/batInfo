#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>


int main() {
	DIR *dir;
	struct dirent *entry;
	char path[289];

	dir = opendir("/sys/class/power_supply");
	if (dir == NULL) {
		perror("ERROR: Failed to open directory");
		exit(1);
	}

	while ((entry = readdir(dir)) != NULL) {
		if (strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "..") != 0) {
			snprintf(path, sizeof(path), "/sys/class/power_supply/%s/type", entry->d_name);
			if (access(path, F_OK) == 0) {
				// we found one with type
				int fd = open(path, O_RDONLY);
				if (fd < 0) {
					perror("ERROR: Failed to open type");
					exit(1);
				}
				char type[256];
				if (read(fd, type, sizeof(type)) < 0) {
					perror("ERROR: Failed to read type");
					exit(1);
				}
				close(fd);

				if (strcmp(type, "Battery\n") == 0) {
					goto goodbat;
				}
			}
		}
	}

	closedir(dir);
	fprintf(stderr, "FINDBAT-ERROR: No battery found!!!\n");
	exit(1);

	goodbat:
	printf("/sys/class/power_supply/%s", entry->d_name);
	closedir(dir);
	return 0;
}
