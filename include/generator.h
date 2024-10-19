#include <stdint.h>
#include <string.h>
#include "main.h"
#include "students.h"
#include "courses.h"

#ifndef GENERATOR_H_INCLUDED
#define GENERATOR_H_INCLUDED

const char FLEX[2][11] = {"XAT--12A-S", "XAT--12B-S"};

typedef struct {
  uint8_t numberOfClasses;
  char classes[CLASSROOMS][MAX_COURSE_ID_LEN];
} TIMETABLE_BLOCK;

typedef struct {
  TIMETABLE_BLOCK timetable[TOTAL_BLOCKS];
  bool success;
} TIMETABLE;

typedef enum {
  FirstToSecondSemester,
  SecondToFirstSemester
} StepType;

TIMETABLE generateTimetable(STUDENT *students, size_t size_students, COURSE *courses, size_t size_courses);

bool isFlex(char crsNo[MAX_COURSE_NO_LEN]);

#endif
