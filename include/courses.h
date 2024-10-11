#include <stdint.h>
#include "main.h"
#include "csv.h"

#ifndef COURSES_H_INCLUDED
#define COURSES_H_INCLUDED

typedef struct {
  char uniqueCrsNos[MAX_COURSES][MAX_COURSE_NO_LEN];
  uint16_t numberOfCourses;
} UNIQUE_COURSES;

typedef struct {
  char crsNo[MAX_COURSE_NO_LEN];
  uint16_t requests;
  char description[MAX_COURSE_DES_LEN];
  uint8_t credits;
  uint32_t students[CLASS_CAP * CLASSROOMS * TOTAL_BLOCKS]; // Will be large enough to handle all pupil numbers taking this course, even if it is highly requested.
  uint8_t globalNumberOfStudents;
} COURSE; // Course is the general information for selection and stuff?

typedef struct {
  char crsNo[MAX_COURSE_NO_LEN];
  char description[MAX_COURSE_DES_LEN];
  uint32_t students[CLASS_CAP];
  uint8_t numberOfStudents;
} CLASS; // A class is an actual active course, there can be many classes for 1 course, each with its own number of students.

UNIQUE_COURSES getNumberOfCourses(CSV_LINE *lines, size_t lines_len);
COURSE *getCourses(CSV_LINE *lines, size_t lines_len, UNIQUE_COURSES unique_course_info);

#endif