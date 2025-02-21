#include <stdint.h>
#include <string.h>
#include "main.h"
#include "students.h"
#include "courses.h"

#ifndef GENERATOR_H_INCLUDED
#define GENERATOR_H_INCLUDED

typedef struct {
  uint8_t numberOfClasses;
  CLASS classes[CLASSROOMS];
} TIMETABLE_BLOCK;

typedef struct {
  TIMETABLE_BLOCK blocks[TOTAL_BLOCKS];
  bool success;
} TIMETABLE;

typedef enum {
  FirstToSecondSemester,
  SecondToFirstSemester
} StepType;

TIMETABLE generateTimetable(STUDENT *students, size_t size_students, COURSE *courses, size_t size_courses);

#endif
