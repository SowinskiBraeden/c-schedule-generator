#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

/*
  We define the constants here so the the processor
  replaces the var name with the actual value to prevent
  "Variable modified error during compile"
  - give me a break I am learning and need this note to
  tell me why I am doing this
*/
#define MAX_COURSE_DES_LEN 50
#define MAX_COURSE_NO_LEN 20
#define MAX_PUPIL_NUM_LEN 7

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

const char* getField(char *line, int num) {
  const char* tok;
  for (tok = strtok(line, ","); tok && *tok; tok = strtok(NULL, ",\n")) {
    if (!--num)
      return tok;
  }
  return NULL;
}

int count_lines(char data_dir[]) {
  FILE *stream = fopen(data_dir, "r");
  char buff[MAX_CHAR];
  int counter = 0;
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

  return counter;
}

CSV_LINE *csvReader(char data_dir[]) {
  int num_lines = count_lines(data_dir) + 1;
  if (num_lines == -1) {
    fputs("Failed to read number of lines in the given CSV file!\n", stderr);
    return NULL;
  }

  FILE *stream = fopen(data_dir, "r");

  CSV_LINE *lines = malloc(num_lines * sizeof(CSV_LINE));
  if (lines == NULL) {
    fputs("Failed to allocate memory!\n", stderr);
    return NULL;
  }

  int i = 0;
  char buff[MAX_CHAR];
  while (fgets(buff, sizeof(buff), stream) != NULL) {
    CSV_LINE csv_line;
    strcpy(csv_line.pupilNum, getField(strdup(buff), 1));
    strcpy(csv_line.crsNo, getField(strdup(buff), 2));
    strcpy(csv_line.description, getField(strdup(buff), 3));
    csv_line.alternate = strcmp(getField(strdup(buff), 4), "TRUE") ? true : false;
    lines[i] = csv_line;
    i++;
  }

  return lines;
}

/*** DEFINE STUDENT DATA STRUCTURES & COURSE DATA STRUCTURES***/

// REQUEST
typedef struct {
  char crsNo[MAX_COURSE_NO_LEN];
  char description[MAX_COURSE_DES_LEN];
  bool alt;
} REQUEST;

// STUDENT
typedef struct {
  char pupilNum[MAX_PUPIL_NUM_LEN];
  REQUEST request[TOTAL_BLOCKS + MAX_REQUEST_ALTS];
  char schedule[TOTAL_BLOCKS]; // blocks 1 through (TOTAL_BLOCKS / 2) represents the first semester in this array
  short expectedClasses;
  short classes;
  REQUEST remainingAlts[MAX_REQUEST_ALTS];
  short studentIndex;
} STUDENT;

// COURSE
typedef struct {
  char crsNo[MAX_COURSE_NO_LEN];
  short requests;
  char description[MAX_COURSE_DES_LEN];
  short credits;
  bool core; // Is this a core class? prioritize this course over alternates
  STUDENT students[CLASS_CAP];
} COURSE;

/*** DEFINE GET FUNCTIONS FOR STUDENTS & COURSES FROM CSV ***/

STUDENT *getStudents(CSV_LINE *lines, int total_blocks) {
  return NULL;
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
  CSV_LINE *lines = csvReader(data_dir);
  if (lines == NULL) return -1;

  STUDENT *students = getStudents("../sample_data/course_selection_data.csv", TOTAL_BLOCKS);
  if (students == NULL) return -1;

  free(lines);

  // Algorithm

  return 0;
}