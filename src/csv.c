#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <stdint.h>
#include "../include/main.h"

typedef struct {
  uint32_t pupilNum;
  char crsNo[MAX_COURSE_NO_LEN];
  char description[MAX_COURSE_DES_LEN];
  bool alternate;
} CSV_LINE;

int count_lines(char data_dir[]) {
  FILE *stream = fopen(data_dir, "r");
  if (!stream) {
    fputs("Failed to open CSV file\n", stderr);
    return -1;
  }
  char buff[MAX_CHAR];
  size_t counter = 1;
  while (fgets(buff, sizeof(buff), stream) != NULL) {
    if (strchr(buff, '\n') != NULL) counter++;
  }
  fclose(stream);
  return counter;
}

CSV_LINE *csvReader(char data_dir[], size_t size) {

  FILE *stream = fopen(data_dir, "r");
  if (!stream) {
    fputs("Failed to open CSV file\n", stderr);
    return NULL;
  }

  CSV_LINE *lines = malloc(size * sizeof(CSV_LINE));

  char buff[MAX_CHAR];
  int i = 0;

  // Read each line in CSV file
  while (fgets(buff, sizeof(buff), stream)) {
    CSV_LINE csv_line = {0, "\0", "\0", false};
    lines[i] = csv_line;

    // Tokenize the line to extract data
    char *token = strtok(buff, ",");
    if (token)
      lines[i].pupilNum = atoi(token); // convert string to uint32_t
    
    token = strtok(NULL, ",");
    if (token)
      strncpy(lines[i].crsNo, token, MAX_COURSE_NO_LEN);

    token = strtok(NULL, ",");
    if (token)
      strncpy(lines[i].description, token, MAX_COURSE_DES_LEN);

    token = strtok(NULL, ",");
    if (token) {
      char alternate[5];
      strncpy(alternate, token, 5);
      alternate[strcspn(alternate, "\n")] = 0; // Remove newline
      lines[i].alternate = !strcmp(alternate, "TRUE") ? true : false;
    }

    i++;
  }

  fclose(stream);
  return lines;
}
