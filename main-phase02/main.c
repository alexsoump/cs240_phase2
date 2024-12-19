/*
 * ============================================
 * file: main.c
 * @Author Myron Tsatsarakis (myrontsa@csd.uoc.gr)
 * @Version 26/11/2024
 *
 * @e-mail hy240@csd.uoc.gr
 *
 * @brief Main function
 *        for CS240 Project Phase 2,
 *        Winter Semester 2024-2025
 * ============================================
 */

#include <ctype.h>
#include <limits.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "voting.h"

#define DISTRICTS_SZ 56
#define PARTIES_SZ 5

/*** Parsing Utilities ***/

void TrimLine(char* line) {
    char* start = line;
    // Trim leading whitespace
    while (*start && isspace((unsigned char)*start)) start += 1;
    if (*start == '\0') {    // whitespace-only string case
        *line = '\0';
        return;
    }
    // Trim trailing whitespace
    char* end = start + strlen(start) - 1;
    while (end > start && isspace((unsigned char)*end)) end -= 1;
    *(end + 1) = '\0';
    // Copy the trimmed content to the original buffer (memmove used for overlapping content)
    memmove(line, start, strlen(start));
}

bool IsEmptyLine(const char* line) {
    if (line == NULL || *line == '\0') return true;
    // check for non-whitespace characters
    for (const char* p = line; *p != '\0'; ++p)
        if (!isspace((unsigned char)*p)) return false;
    return true;
}

void ReportOutOfBounds(const char* line, int lineIdx, int arg, int minLimit, int maxLimit) {
    if ((minLimit <= arg) && (arg < maxLimit)) return;
    printf("On line %d: %s\n", lineIdx, line);
    printf("argument %d out of bounds [%d, %d)\n", arg, minLimit, maxLimit);
    exit(EXIT_FAILURE);
}

void ReportInvalidParse(const char* line, int lineIdx, int actual, int expected) {
    if (actual == expected) return;
    printf("On line %d: %s\n", lineIdx, line);
    printf("sscanf argument assignment error: expected %d, assigned %d\n", expected, actual);
    exit(EXIT_FAILURE);
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        printf("Invalid number of arguments\n");
        exit(EXIT_FAILURE);
    }
    FILE* inputFile = fopen(argv[1], "r");
    if (inputFile == NULL) {
        printf("fopen error (Invalid path ?)\n");
        exit(EXIT_FAILURE);
    }
#define LINE_SZ 4096
    char line[LINE_SZ] = { 0 };
    for (int lineIdx = 1; fgets(line, LINE_SZ, inputFile) != NULL; ++lineIdx) {    // Read next line from input file
        line[strcspn(line, "\r\n")] = '\0';
        TrimLine(line);
        if (IsEmptyLine(line)) {
            continue;    // Skip empty lines
        }
        if (line[0] == '#') {
            continue;    // Skip comment lines
        }
        if (line[0] == 'A') {
            const char* args = &(line[1]);
            int maxStationsCount, maxSid;
            int count = sscanf(args, "%d %d", &maxStationsCount, &maxSid);
            ReportInvalidParse(line, lineIdx, count, 2);
            ReportOutOfBounds(line, lineIdx, maxStationsCount, 0, INT_MAX);
            ReportOutOfBounds(line, lineIdx, maxSid, 0, INT_MAX);
            EventAnnounceElections(maxStationsCount, maxSid);
            continue;
        }
        if (line[0] == 'D') {
            const char* args = &(line[1]);
            int did, seats;
            int count = sscanf(args, "%d %d", &did, &seats);
            ReportInvalidParse(line, lineIdx, count, 2);
            ReportOutOfBounds(line, lineIdx, did, 0, DISTRICTS_SZ);
            ReportOutOfBounds(line, lineIdx, seats, 0, INT_MAX);
            EventCreateDistrict(did, seats);
            continue;
        }
        if (line[0] == 'S') {
            const char* args = &(line[1]);
            int sid, did;
            int count = sscanf(args, "%d %d", &sid, &did);
            ReportInvalidParse(line, lineIdx, count, 2);
            ReportOutOfBounds(line, lineIdx, sid, 0, INT_MAX);
            ReportOutOfBounds(line, lineIdx, did, 0, DISTRICTS_SZ);
            EventCreateStation(sid, did);
            continue;
        }
        if (line[0] == 'R') {
            const char* args = &(line[1]);
            int vid, sid;
            int count = sscanf(args, "%d %d", &vid, &sid);
            ReportInvalidParse(line, lineIdx, count, 2);
            ReportOutOfBounds(line, lineIdx, vid, 0, INT_MAX);
            ReportOutOfBounds(line, lineIdx, sid, 0, INT_MAX);
            EventRegisterVoter(vid, sid);
            continue;
        }
        if (line[0] == 'C') {
            const char* args = &(line[1]);
            int cid, pid, did;
            int count = sscanf(args, "%d %d %d", &cid, &pid, &did);
            ReportInvalidParse(line, lineIdx, count, 3);
            ReportOutOfBounds(line, lineIdx, cid, 0, INT_MAX);
            ReportOutOfBounds(line, lineIdx, pid, 0, PARTIES_SZ);
            ReportOutOfBounds(line, lineIdx, did, 0, DISTRICTS_SZ);
            EventRegisterCandidate(cid, pid, did);
            continue;
        }
        if (line[0] == 'V') {
            const char* args = &(line[1]);
            int vid, sid, cid, pid;
            int count = sscanf(args, "%d %d %d %d", &vid, &sid, &cid, &pid);
            ReportInvalidParse(line, lineIdx, count, 4);
            ReportOutOfBounds(line, lineIdx, vid, 0, INT_MAX);
            ReportOutOfBounds(line, lineIdx, sid, 0, INT_MAX);
            ReportOutOfBounds(line, lineIdx, cid, -2, INT_MAX);
            ReportOutOfBounds(line, lineIdx, pid, 0, PARTIES_SZ);
            EventVote(vid, sid, cid, pid);
            continue;
        }
        if (line[0] == 'M') {
            const char* args = &(line[1]);
            int did;
            int count = sscanf(args, "%d", &did);
            ReportInvalidParse(line, lineIdx, count, 1);
            ReportOutOfBounds(line, lineIdx, did, 0, DISTRICTS_SZ);
            EventCountVotes(did);
            continue;
        }
        if (line[0] == 'N') {
            EventFormParliament();
            continue;
        }
        if (line[0] == 'I') {
            const char* args = &(line[1]);
            int did;
            int count = sscanf(args, "%d", &did);
            ReportInvalidParse(line, lineIdx, count, 1);
            ReportOutOfBounds(line, lineIdx, did, 0, DISTRICTS_SZ);
            EventPrintDistrict(did);
            continue;
        }
        if (line[0] == 'J') {
            const char* args = &(line[1]);
            int sid;
            int count = sscanf(args, "%d", &sid);
            ReportInvalidParse(line, lineIdx, count, 1);
            ReportOutOfBounds(line, lineIdx, sid, 0, INT_MAX);
            EventPrintStation(sid);
            continue;
        }
        if (line[0] == 'K') {
            const char* args = &(line[1]);
            int pid;
            int count = sscanf(args, "%d", &pid);
            ReportInvalidParse(line, lineIdx, count, 1);
            ReportOutOfBounds(line, lineIdx, pid, 0, PARTIES_SZ);
            EventPrintParty(pid);
            continue;
        }
        if (line[0] == 'L') {
            EventPrintParliament();
            continue;
        }
        if (line[0] == 'B' && line[1] == 'U') {
            const char* args = &(line[2]);
            int vid, sid;
            int count = sscanf(args, "%d %d", &vid, &sid);
            ReportInvalidParse(line, lineIdx, count, 2);
            ReportOutOfBounds(line, lineIdx, vid, 0, INT_MAX);
            ReportOutOfBounds(line, lineIdx, sid, 0, INT_MAX);
            EventBonusUnregisterVoter(vid, sid);
            continue;
        }
        if (line[0] == 'B' && line[1] == 'F') {
            EventBonusFreeMemory();
            continue;
        }
        printf("Invalid line %d: %s\n", lineIdx, line);
        exit(EXIT_FAILURE);
    }
    int err = fclose(inputFile);
    if (err != 0) {
        printf("fclose error\n");
        exit(EXIT_FAILURE);
    }
}
