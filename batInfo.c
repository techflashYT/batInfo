#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <limits.h>

#define MAX_PATH_LENGTH 256  // Adjust size to accommodate longer paths

static const char* pre = "";
static const char* post = "\e[0m";

// Helper function to read an integer value from a file
static int readBatteryFile(const char* path, int* value) {
    FILE* file = fopen(path, "r");
    if (!file) {
        perror("Failed to open file");
        return -1;
    }
    if (fscanf(file, "%d", value) != 1) {
        fprintf(stderr, "Failed to read value from %s\n", path);
        fclose(file);
        return -1;
    }
    fclose(file);
    return 0;
}

// Helper function to calculate watts if power_now is unavailable
static double calculateWatts(int current, int voltage) {
    return (current / 1000000.0) * (voltage / 1000000.0); // Convert µA and µV to A and V
}

int main() {
    char basePath[MAX_PATH_LENGTH] = {0};
    char tmpPath[MAX_PATH_LENGTH] = {0};
    char status[20] = {0};
    int percent, currentNow, voltageNow, powerNow;
    bool isWatts = false;

    // Get the battery base path
    FILE* bat = popen("findbat", "r");
    if (!bat) {
        perror("Failed to launch findbat");
        return 1;
    }

    fgets(basePath, sizeof(basePath), bat);
    pclose(bat);

    if (basePath[0] == '\0') {
        fprintf(stderr, "findbat returned nothing!\n");
        return 1;
    }

    // Ensure basePath does not end with newline
    size_t len = strlen(basePath);
    if (len > 0 && basePath[len - 1] == '\n') {
        basePath[len - 1] = '\0';
    }

    // Get battery capacity
    if (snprintf(tmpPath, sizeof(tmpPath), "%s/capacity", basePath) >= sizeof(tmpPath)) {
        fprintf(stderr, "Error: Path exceeds buffer size\n");
        return 1;
    }
    if (readBatteryFile(tmpPath, &percent) < 0) return 1;

    // Determine battery status
    if (snprintf(tmpPath, sizeof(tmpPath), "%s/status", basePath) >= sizeof(tmpPath)) {
        fprintf(stderr, "Error: Path exceeds buffer size\n");
        return 1;
    }
    FILE* statusFile = fopen(tmpPath, "r");
    if (!statusFile) {
        perror("Failed to read battery status");
        return 1;
    }
    if (!fgets(status, sizeof(status), statusFile)) {
        fprintf(stderr, "Failed to read status from %s\n", tmpPath);
        fclose(statusFile);
        return 1;
    }
    fclose(statusFile);

    // Trim newline from status
    len = strlen(status);
    if (len > 0 && status[len - 1] == '\n') {
        status[len - 1] = '\0';
    }

    // Update color coding based on percent and status
    if (strcmp(status, "Discharging") == 0) {
        if (percent < 35) {
            pre = "\e[1;31m!!! ";
            post = " !!! \e[0m";
        } else if (percent < 65) {
            pre = "\e[31m";
        } else if (percent < 85) {
            pre = "\e[1;33m";
        } else {
            pre = "\e[1;32m";
        }
    } else { // Charging or Full
        pre = "\e[1;32m";
    }

    printf("BAT PERCENT: %s%d%%%s\n", pre, percent, post);

    // Attempt to read power_now
    if (snprintf(tmpPath, sizeof(tmpPath), "%s/power_now", basePath) >= sizeof(tmpPath)) {
        fprintf(stderr, "Error: Path exceeds buffer size\n");
        return 1;
    }
    if (readBatteryFile(tmpPath, &powerNow) == 0) {
        isWatts = true;
    } else {
        // Fallback to current_now and voltage_now
        if (snprintf(tmpPath, sizeof(tmpPath), "%s/current_now", basePath) >= sizeof(tmpPath)) {
            fprintf(stderr, "Error: Path exceeds buffer size\n");
            return 1;
        }
        if (readBatteryFile(tmpPath, &currentNow) < 0) return 1;

        if (snprintf(tmpPath, sizeof(tmpPath), "%s/voltage_now", basePath) >= sizeof(tmpPath)) {
            fprintf(stderr, "Error: Path exceeds buffer size\n");
            return 1;
        }
        if (readBatteryFile(tmpPath, &voltageNow) < 0) return 1;

        powerNow = (int)(calculateWatts(currentNow, voltageNow) * 1000000); // Convert W to µW
        isWatts = true;
    }

    // Adjust sign for discharging
    if (strcmp(status, "Discharging") == 0) {
        powerNow = -abs(powerNow);
        post = "\e[0m";

        // > 2W discharge
        if (isWatts && powerNow <= -2) {
        	pre = "\e[1;31m";
        }
        else if (isWatts && powerNow <= -1) {
			pre = "\e[31m";
		}
		else if (powerNow <= -500) {
			pre = "\e[1;33m";
		}
		else {
			pre = "\e[1;32m";
		}

    }

    // Format the output
    double displayPower = isWatts ? (powerNow / 1000000.0) : powerNow;
    const char* unit = isWatts ? "W" : "mW";

    printf("BAT POWER  : %s%.2f %s%s\n", pre, displayPower, unit, post);

    return 0;
}
