#include <stdbool.h>
#include <stdint.h>
#include "csv.h"
#include "main.h"

#ifndef STUDENTS_H_INCLUDED
#define STUDENTS_H_INCLUDED

typedef struct {
  char crsNo[MAX_COURSE_NO_LEN];
  char description[MAX_COURSE_DES_LEN];
  bool alternate;
} REQUEST;

typedef struct {
  uint32_t pupilNum;
  REQUEST *requests;
  uint8_t requestsLen;
  char schedule[TOTAL_BLOCKS][MAX_COURSE_ID_LEN];
  uint8_t expectedClasses;
  uint8_t classes;
  REQUEST *remainingAlts;
  uint8_t remainingAltsLen;
  uint8_t grade;
} STUDENT;


typedef struct {
  uint32_t uniquePupilNumbers[MAX_STUDENTS];
  size_t numberOfStudents;
} UNIQUE_STUDENTS; 

UNIQUE_STUDENTS getNumberOfStudents(CSV_LINE *lines, size_t lines_len);
STUDENT *getStudents(CSV_LINE *lines, size_t lines_len, int total_blocks, UNIQUE_STUDENTS students_info);
#endif