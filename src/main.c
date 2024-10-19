#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <stdint.h>
#include <math.h>
#include "../include/main.h"
#include "../include/csv.h"
#include "../include/students.h"
#include "../include/courses.h"
#include "../include/json.h"
#include "../include/generator.h"

int main(int argc, char **argv) {
  /*
    read csv data into array of structs that can
    be processed into an array of student structs
    and an array of course structs
  */  
  char data_dir[] = "sample_data/course_selection_data.csv";
  
  int num_lines = count_lines(data_dir);
  if (num_lines == -1) {
    fputs("Failed to read number of lines in the given CSV file!\n", stderr);
    return -1;
  }

  CSV_LINE *lines = csvReader(data_dir, num_lines);
  if (lines == NULL) return -1;

  // Get number of students & number of courses
  UNIQUE_STUDENTS students_info = getNumberOfStudents(lines, num_lines);
  UNIQUE_COURSES courses_info = getNumberOfCourses(lines, num_lines);

  // Create struct array of students & courses
  STUDENT *students = getStudents(lines, num_lines, TOTAL_BLOCKS, students_info);
  if (students == NULL) return -1;

  COURSE *courses = getCourses(lines, num_lines, courses_info);
  if (courses == NULL) return -1;

  free(lines);
 
  // Algorithm here
  TIMETABLE timetable = generateTimetable(students, students_info.numberOfStudents, courses, courses_info.numberOfCourses);
  (void)timetable; // added for now so the compiler sees timetable as used and does not throw error

  // Write data to json for output
  if (createDirectory("output") == -1) return -1;
  if (writeStudentsToJson(students, students_info.numberOfStudents, "output/students.json") == -1) return -1;
  if (writeCoursesToJson(courses, courses_info.numberOfCourses, "output/courses.json") == -1) return -1;

  for (size_t i = 0; i < students_info.numberOfStudents; i++) {
    free(students[i].requests);
    free(students[i].remainingAlts);
  }
  free(students);
  free(courses);
  
  return 0;
}
