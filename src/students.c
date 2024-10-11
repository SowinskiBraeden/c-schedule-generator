#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <stdint.h>
#include "../include/main.h"
#include "../include/csv.h"
#include "../include/students.h"

bool intInArray(uint32_t val, uint32_t *arr, size_t size) {
  for (size_t i = 0; i < size; i++) 
    if (arr[i] == val)
      return true;
  return false;
}

UNIQUE_STUDENTS getNumberOfStudents(CSV_LINE *lines, size_t lines_len) {
  // Find the number of unique students and store each unique pupil Num
  UNIQUE_STUDENTS students_info = {{0}, 0};
  for (size_t i = 0; i < lines_len; i++) {
    bool exists = intInArray(lines[i].pupilNum, students_info.uniquePupilNumbers, students_info.numberOfStudents);
    if (!exists) {
      students_info.uniquePupilNumbers[students_info.numberOfStudents] = lines[i].pupilNum;
      students_info.numberOfStudents++;
    }
  }
  return students_info;
}

STUDENT *getStudents(CSV_LINE *lines, size_t lines_len, int total_blocks, UNIQUE_STUDENTS students_info) {
  // Calculate each students number of requests to allocate the requests array
  uint8_t numRequests[MAX_STUDENTS] = {0};
  for (size_t i = 0; i < lines_len; i++) {
    for (size_t j = 0; j < students_info.numberOfStudents; j++) {
      if (students_info.uniquePupilNumbers[j] == lines[i].pupilNum) {
        numRequests[j]++;
        break;
      }
    }
  }

  // Create our student array with the correct number of students 
  STUDENT *students = malloc(students_info.numberOfStudents * sizeof(STUDENT));
  for (uint16_t i = 0; i < students_info.numberOfStudents; i++) {
    STUDENT student;
    
    student.pupilNum = students_info.uniquePupilNumbers[i];
    student.expectedClasses = 0;
    student.classes = 0;
    student.grade = 0;
    student.requestsLen = 0;
    student.remainingAltsLen = 0;

    REQUEST *requests = malloc(numRequests[i] * sizeof(REQUEST));
    student.requests = requests;

    char **schedule = malloc(TOTAL_BLOCKS * sizeof(int*));
    student.schedule = schedule;

    REQUEST *remainingAlts = malloc(MAX_REQUEST_ALTS * sizeof(REQUEST));
    student.remainingAlts = remainingAlts;
   
    students[i] = student;
  }
  
  // Read each request in lines and assign to correct student
  for (size_t i = 0; i < lines_len; i++) {
    for (uint16_t j = 0; j < students_info.numberOfStudents; j++) {
      if (lines[i].pupilNum == students[j].pupilNum) {
        REQUEST request;
        strcpy(request.crsNo, lines[i].crsNo);
        strcpy(request.description, lines[i].description);
        request.alternate = lines[i].alternate;
        students[j].requests[students[j].requestsLen] = request;
        students[j].requestsLen++;
        break;
      }
    }
  }

  return students;
}
