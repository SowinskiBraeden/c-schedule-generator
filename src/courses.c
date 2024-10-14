#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <stdint.h>
#include "../include/main.h"
#include "../include/csv.h"
#include "../include/courses.h"

/*
  TODO: find a way to make the arr paramter work with any size, i.e char **arr
  However, when I call this array I can't seem to be able to pass the string array
  I want without it throwing an error when using char **arr as the parameter
*/
bool strInArray(char *str, char arr[MAX_CLASSES][MAX_COURSE_NO_LEN], size_t size) {
  for (size_t i = 0; i < size; i++)
    if (strcmp(arr[i], str) == 0)
      return true;
  return false;
}

UNIQUE_COURSES getNumberOfCourses(CSV_LINE *lines, size_t lines_len) {
  // Find the number of unique courses and store each unique course no.
  UNIQUE_COURSES unique_course_info = {{{"\0"}}, 0};
  for (size_t i = 0; i < lines_len; i++) {
    bool exists = strInArray(lines[i].crsNo, unique_course_info.uniqueCrsNos, unique_course_info.numberOfCourses);
    if (!exists) {
      strcpy(unique_course_info.uniqueCrsNos[unique_course_info.numberOfCourses], lines[i].crsNo);
      unique_course_info.numberOfCourses++;
    }
  }
  return unique_course_info;
}

COURSE *getCourses(CSV_LINE *lines, size_t lines_len, UNIQUE_COURSES unique_course_info) {
  COURSE *courses = malloc(unique_course_info.numberOfCourses * sizeof(COURSE));
  for (size_t i = 0; i < unique_course_info.numberOfCourses; i++) {
    for (size_t j = 0; j < lines_len; j++) {
      if (strcmp(lines[j].crsNo, unique_course_info.uniqueCrsNos[j]) == 0) {
        COURSE course = {"\0", 0, "\0", 4, {0}, 0}; // Ensure array of pupil numbers is null till we populate it later with the correct size
        strcpy(course.crsNo, lines[i].crsNo);
        strcpy(course.description, lines[i].description);
        courses[i] = course;
        break;
      }
    }
  }
  return courses;
}
