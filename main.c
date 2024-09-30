#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

/*
  Using define instead of constants prevents
  "Variable modified error"
*/
#define MAX_COURSE_DES_LEN 50
#define MAX_COURSE_NO_LEN 20
#define MAX_PUPIL_NUM_LEN 8

#define TOTAL_BLOCKS 10 // the number of blocks between 2 semesters i.e 8 = 4 blocks per semester, 10 = 5 blocks per semester
#define MAX_REQUEST_ALTS 6
#define CLASSROOMS 40
#define MIN_REQ 18
#define CLASS_CAP 30

#define MAX_CHAR 85 // MAX_PUPIL_NUM_LEN + ',' + MAX_COURSE_NO_LEN + ',' + MAX_COURSE_DES_LEN, + ',' + 5 (5 is the number of characters in the word "FALSE")

/*** DEFINE CSV READER FUNCTIONS & DATASTRUCTURES ***/

typedef struct {
  char pupilNum[MAX_PUPIL_NUM_LEN];
  char crsNo[MAX_COURSE_NO_LEN];
  char description[MAX_COURSE_DES_LEN];
  bool alternate;
} CSV_LINE;

size_t count_lines(char data_dir[]) {
  FILE *stream = fopen(data_dir, "r");
  char buff[MAX_CHAR];
  size_t counter = 0;
  for (;;) {
    size_t res = fread(buff, 1, MAX_CHAR, stream);
    if (ferror(stream))
      return -1;

    for (int i = 0; i < res; i++) {
      if (buff[i] == '\n')
        counter++;
    }

    if (feof(stream))
      break;
  }

  fclose(stream);
  return counter;
}

CSV_LINE *csvReader(char data_dir[], size_t size) {

  FILE *stream = fopen(data_dir, "r");
  if (!stream) {
    fputs("Failed to open CSV file", stderr);
    return NULL;
  }

  CSV_LINE *lines = malloc(size * sizeof(CSV_LINE));

  char buff[MAX_CHAR];
  int i = 0;

  // Read each line in CSV file
  while (fgets(buff, sizeof(buff), stream) && i < size) {
    CSV_LINE csv_line;
    lines[i] = csv_line;

    // Tokenize the line to extract data
    char *token = strtok(buff, ",");
    if (token) {
      strncpy(lines[i].pupilNum, token, MAX_PUPIL_NUM_LEN);
      // lines[i].pupilNum[strcspn(lines[i].pupilNum, "\n")] = 0; // Remove newline
    }

    token = strtok(NULL, ",");
    if (token) strncpy(lines[i].crsNo, token, MAX_COURSE_NO_LEN);

    token = strtok(NULL, ",");
    if (token) strncpy(lines[i].description, token, MAX_COURSE_DES_LEN);

    token = strtok(NULL, ",");
    if (token) {
      char alternate[5];
      strncpy(alternate, token, 5);
      alternate[strcspn(alternate, "\n")] = 0; // Remove newline
      lines[i].alternate = strcmp(alternate, "TRUE") == 0 ? true : false;
    }

    i++;
  }

  fclose(stream);
  return lines;
}

/*** DEFINE STUDENT DATA STRUCTURES & COURSE DATA STRUCTURES***/

typedef struct {
  char crsNo[MAX_COURSE_NO_LEN];
  char description[MAX_COURSE_DES_LEN];
  bool alt;
} REQUEST;

typedef struct {
  char pupilNum[MAX_PUPIL_NUM_LEN];
  REQUEST request[TOTAL_BLOCKS + MAX_REQUEST_ALTS];
  char schedule[TOTAL_BLOCKS]; // blocks 1 through (TOTAL_BLOCKS / 2) represents the first semester in this array
  short expectedClasses;
  short classes;
  REQUEST remainingAlts[MAX_REQUEST_ALTS];
  short studentIndex;
} STUDENT;

typedef struct {
  char crsNo[MAX_COURSE_NO_LEN];
  short requests;
  char description[MAX_COURSE_DES_LEN];
  short credits;
  bool core; // Is this a core class? prioritize this course over alternates
  STUDENT students[CLASS_CAP];
} COURSE;

/*** DEFINE GET FUNCTIONS FOR STUDENTS & COURSES FROM CSV ***/

STUDENT *getStudents(CSV_LINE *lines, size_t lines_len, int total_blocks) {
  return NULL;
  // for (size_t i = 0; i < lines_len; i++) {
    
  // }
}

COURSE *getCourses(CSV_LINE *lines) {
  return NULL;
}

int main(int argc, char **argv) {

  /*
    read csv data into array of structs that can
    be processed into an array of student structs
    and an array of course structs
  */
  char data_dir[] = "sample_data/course_selection_data.csv";
  
  size_t num_lines = count_lines(data_dir);
  if (num_lines == -1) {
    fputs("Failed to read number of lines in the given CSV file!\n", stderr);
    return -1;
  }
  
  CSV_LINE *lines = csvReader(data_dir, num_lines);
  if (lines == NULL) return -1;

  STUDENT *students = getStudents(lines, num_lines, TOTAL_BLOCKS);
  if (students == NULL) return -1;

  free(lines);

  // Algorithm

  return 0;
}