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

const char FLEX[2][11] = {"XAT--12A-S", "XAT--12B-S"};

typedef struct {
  //                0-9 block idx |    CRS_NO_ID     | pupilNums
  uint16_t timetable[TOTAL_BLOCKS][MAX_COURSE_ID_LEN][CLASS_CAP];
  bool success;
} TIMETABLE;

void appendChar(char *str, char ch) {
  int len = strlen(str);
  str[len] = ch;
  str[len + 1] = '\0';
}

// Equally disperses the sum of an array to each index
uint8_t *equal(uint8_t *arr, size_t size) {
  size_t sum = 0;

  // Calculate the sum of the array
  for (size_t i = 0; i < size; i++)
    sum += arr[i];

  // Calculate quotient and remainder
  size_t q = sum / size;
  size_t r = sum % size;

  // Fill the result array
  for (size_t i = 0; i < r; i++)
    arr[i] = q + 1;

  for (size_t i = r; i < size; i++)
    arr[i] = q;
  
  return arr;
}

TIMETABLE generateTimetable(STUDENT *students, size_t size_students, COURSE *courses, size_t size_courses) {
  TIMETABLE timetable = {{{{0}}}, false};

  uint8_t MEDIAN = floor((float) (MIN_REQ + CLASS_CAP) / 2);
  // uint8_t BLOCKS_PER_SEMESTER = TOTAL_BLOCKS / 2;


  /*** STEP 1 - Tally requests to check which courses are eligable to run ***/
  char activeCourses[MAX_COURSES][MAX_COURSE_NO_LEN] = {{"\0"}};
  size_t activeCoursesIndexes[MAX_COURSES] = {0};
  uint16_t activeCoursesLen = 0; // also acts as the length of activeCourses
  for (size_t i = 0; i < size_students; i++) {
    for (size_t j = 0; j < students[i].requestsLen; j++) {
      if (students[i].requests[j].alternate) continue;
      if (strcmp(students[i].requests[j].crsNo, FLEX[0]) == 0) continue;
      if (strcmp(students[i].requests[j].crsNo, FLEX[1]) == 0) continue;

      for (size_t k = 0; k < size_courses; k++) {
        if (strcmp(courses[k].crsNo, students[i].requests[j].crsNo) == 0) {
          courses[k].requests++;
          if (courses[k].requests >= MIN_REQ) {
            if (activeCoursesLen == 0) {
              strcpy(activeCourses[activeCoursesLen], courses[k].crsNo);
              activeCoursesIndexes[activeCoursesLen] = k;
              activeCoursesLen++;
            } else {
              bool exists = false;
              for (size_t l = 0; l < activeCoursesLen; l++) {
                if (strcmp(activeCourses[l], courses[k].crsNo) == 0) {
                  exists = true;
                  break;
                }
              }

              if (!exists) {
                strcpy(activeCourses[activeCoursesLen], courses[k].crsNo);
                activeCoursesIndexes[activeCoursesLen] = k;
                activeCoursesLen++;
              }
            }
          }
          break;
        }
      }
    }  
  }


  /*** STEP 2 - Generate classes with no students, but calculate the number of expected students per class ***/

  char hex[] = "0123456789abcdefABCDEF";

  uint8_t *allClassRunCounts = malloc(activeCoursesLen * sizeof(uint8_t));
  // max this out to total number of classrooms available between both semesters
  CLASS *classes = malloc(CLASSROOMS * TOTAL_BLOCKS * sizeof(CLASS));
  size_t classesLen = 0;
  for (size_t i = 0; i < activeCoursesLen; i++) {
    uint16_t index = activeCoursesIndexes[i];
    uint8_t classRunCount = floor((float) courses[index].requests / MEDIAN);
    uint8_t remaining = courses[index].requests % MEDIAN;

    // add 1 to classRunCount in case we need to create an extra class with remaining
    size_t *courseClassIndexes = malloc((classRunCount + 1) * sizeof(size_t));
    for (size_t j = 0; j < classRunCount; j++) {
      CLASS newClass;
      strcpy(newClass.baseCrsNo, courses[index].crsNo);
      char courseID[MAX_COURSE_ID_LEN];
      strcpy(courseID, courses[index].crsNo);
      appendChar(courseID, hex[j]);
      strcpy(newClass.crsNo, courseID);
      strcpy(newClass.description, courses[index].description);
      newClass.numberOfStudents = MEDIAN; // The expected number of students in this class

      classes[classesLen] = newClass;
      courseClassIndexes[j] = classesLen;
      classesLen++;
    }

    //*** Handle remaining requests ***/
    
    // Can we add remaining requests to existing classes
    bool remainingFitsInExistingClasses = remaining <= classRunCount * (CLASS_CAP - MEDIAN);
    
    // Can we create a new class using only remaining requests
    bool remainingCanCreateNewClass = remaining >= MIN_REQ;
    
    // Can we create a new class if we borrow students from created classes to add to remaining requests to meet min req
    bool remainingPlusExtraFromExistingCanCreateNewClass = MIN_REQ - remaining < classRunCount * (MEDIAN - MIN_REQ);
    
    if (remainingFitsInExistingClasses) {
      // Simply add remaining to existing classes
      while (remaining > 0) {
        for (size_t j = 0; j < classRunCount; j++) {
          classes[courseClassIndexes[j]].numberOfStudents++;
          remaining--;
          if (remaining == 0) break;
        }
      }

    } else if (remainingCanCreateNewClass) {
      // Create new class
      CLASS newClass;
      strcpy(newClass.baseCrsNo, courses[index].crsNo);
      char courseID[MAX_COURSE_ID_LEN];
      strcpy(courseID, courses[index].crsNo);
      appendChar(courseID, hex[classRunCount]);
      strcpy(newClass.crsNo, courseID);
      strcpy(newClass.description, courses[index].description);
      newClass.numberOfStudents = remaining;

      // Insert class into empty classes array and update index
      classes[classesLen] = newClass;
      courseClassIndexes[classRunCount] = classesLen;
      classesLen++;

      // update class run count; if there is more than one class, equalize the class number of students
      classRunCount++;
      if (classRunCount >= 2) {
        uint8_t *numberOfStudentsArr = malloc(classRunCount * sizeof(uint8_t));
        for (size_t j = 0; j < classRunCount; j++)
          numberOfStudentsArr[j] = classes[courseClassIndexes[j]].numberOfStudents;

        numberOfStudentsArr = equal(numberOfStudentsArr, classRunCount);
        for (size_t j = 0; j < classRunCount; j++)
          classes[courseClassIndexes[j]].numberOfStudents = numberOfStudentsArr[j];

        free(numberOfStudentsArr);
      }

    } else if (remainingPlusExtraFromExistingCanCreateNewClass) {
      // Take 1 student from each existing class till min requirement is met
      while (remaining < MIN_REQ) {
        for (size_t j = 0; j < classRunCount; j++) {
          classes[courseClassIndexes[j]].numberOfStudents--;
          remaining++;
          if (remaining == MIN_REQ) break;
        }
      }

      // Create new class with remaining
      CLASS newClass;
      strcpy(newClass.baseCrsNo, courses[index].crsNo);
      char courseID[MAX_COURSE_ID_LEN];
      strcpy(courseID, courses[index].crsNo);
      appendChar(courseID, hex[classRunCount]);
      strcpy(newClass.crsNo, courseID);
      strcpy(newClass.description, courses[index].description);
      newClass.numberOfStudents = remaining;

      // Insert class into empty classes array and update index
      classes[classesLen] = newClass;
      courseClassIndexes[classRunCount] = classesLen;
      classesLen++;
      classRunCount++;

      // Equalize the class number of students
      uint8_t *numberOfStudentsArr = malloc(classRunCount * sizeof(uint8_t));
      for (size_t j = 0; j < classRunCount; j++)
        numberOfStudentsArr[j] = classes[courseClassIndexes[j]].numberOfStudents;

      numberOfStudentsArr = equal(numberOfStudentsArr, classRunCount);
      for (size_t j = 0; j < classRunCount; j++)
        classes[courseClassIndexes[j]].numberOfStudents = numberOfStudentsArr[j];

      free(numberOfStudentsArr);

    } else {
      /*
        If all above cannot handle remaining requests we will add as many of
        the remaining requests to the existing classes. Any number of requests
        that dont fit will be ignored so later they can be folded into their
        alternative choices
      */
      
      bool full = false;
      while (!full) {
        for (size_t j = 0; j < classRunCount; j++) {
          if (classes[courseClassIndexes[classRunCount - 1]].numberOfStudents == CLASS_CAP) {
            // If the last class in the array is at class_cap, all other classes must be at class cap and we are full
            full = true;
            break;
          }

          classes[courseClassIndexes[j]].numberOfStudents++;
          remaining--;
        }
      }
    }

    free(courseClassIndexes);
    allClassRunCounts[i] = classRunCount;
  }

  // realloc classes to correct size
  CLASS *tempclasses = malloc(classesLen * sizeof(CLASS));
  for (size_t i = 0; i < classesLen; i++)
    tempclasses[i] = classes[i];
  classes = realloc(classes, classesLen * sizeof(CLASS));
  memcpy(classes, tempclasses, classesLen * sizeof(CLASS));
  free(tempclasses);


  /*** STEP 3 - Insert students into empty classes ... yikes ***/
  STUDENT *tempStudents = malloc(size_students * sizeof(STUDENT));
  size_t size_tempStudents = size_students;
  memcpy(tempStudents, students, size_students * sizeof(STUDENT));

  uint8_t *currentInserted = malloc(classesLen * sizeof(uint8_t));
  for (size_t i = 0; i < classesLen; i++)
    currentInserted[i] = 0;

  while (size_tempStudents > 0) {
    // Choose student at random, to prevent success bias to students first in the array
    STUDENT student = tempStudents[rand() % size_tempStudents];
    // printf("pupilNum: %d\n", student.pupilNum);

    // Create an array of students alternates
    size_t numberOfAlts = 0;
    for (size_t i = 0; i < student.requestsLen; i++)
      if (student.requests[i].alternate)
        numberOfAlts++;  
 
    REQUEST *alternates = malloc(numberOfAlts * sizeof(REQUEST));
    size_t alternateIdx = 0;
    for (size_t i = 0; i < student.requestsLen; i++) {
      if (student.requests[i].alternate) {
        alternates[alternateIdx] = student.requests[i];
        alternateIdx++;
      }
    }

    // Search existing classes to insert student based off request
    for (size_t i = 0; i < student.requestsLen; i++) {
      if (student.requests[i].alternate) continue; // Ignore alternates
      char course[MAX_COURSE_NO_LEN] = {"\0"};
      strcpy(course, student.requests[i].crsNo);
      bool getAvailableCourse = true;
      bool isAlt = false;
      // printf("getting available course for %s...\n", course);
      while (getAvailableCourse) {
        // printf("searching...\n");
        for (size_t j = 0; j < classesLen; j++) {
          // Class exists in classes
          // printf("comparing target %s to %s at idx %ld\n", course, classes[j].crsNo, j);
          if (strcmp(classes[j].baseCrsNo, course) == 0) {
            // printf("course %s exists\n", course);
            // If this is an alternate, and there is room to expand, increase number of students to allow extra
            if (isAlt && classes[j].numberOfStudents < CLASS_CAP)
              classes[j].numberOfStudents++;

            // Class exists with room for student
            if (currentInserted[j] < classes[j].numberOfStudents) {
              classes[j].students[currentInserted[j]] = student.pupilNum;
              currentInserted[j]++;
              getAvailableCourse = false;
              break;

            } else if (currentInserted[j] == classes[j].numberOfStudents) {
              // If class is full, and there's no more classes available for that course, convert to alt
              if (j == classesLen - 1 || (j != classesLen - 1 && strcmp(classes[j + 1].baseCrsNo, course) != 0)) {
                if (numberOfAlts > 0) {
                  // Use alternate
                  strcpy(course, alternates[0].crsNo); // asign alternate to course and retry
                  // remove alternate from array of alts to retry same alt over and over
                  REQUEST *tempAlternates = malloc(numberOfAlts * sizeof(REQUEST));
                  memcpy(tempAlternates, alternates, numberOfAlts * sizeof(REQUEST));
                  numberOfAlts--;
                  alternates = realloc(alternates, numberOfAlts * sizeof(REQUEST));
                  for (size_t k = 1; k <= numberOfAlts; k++)
                    alternates[k - 1] = tempAlternates[k];
                  free(tempAlternates);
                  isAlt = true;
                  break;
                } else {
                  // Force break the loop, ignore as it cannot be resolved
                  // Allow administrator to handle error manually
                  getAvailableCourse = false;
                  break;
                }
              }
            }
            // this class does not exist, i.e not enough requests
          } else if (j == classesLen - 1) {
            // printf("course %s does not exist\n", course);
            if (numberOfAlts > 0) {
              // Use alternate
              strcpy(course, alternates[0].crsNo); // asign alternate to course and retry
              // remove alternate from array of alts to retry same alt over and over
              REQUEST *tempAlternates = malloc(numberOfAlts * sizeof(REQUEST));
              memcpy(tempAlternates, alternates, numberOfAlts * sizeof(REQUEST));
              numberOfAlts--;
              alternates = realloc(alternates, numberOfAlts * sizeof(REQUEST));
              for (size_t k = 1; k <= numberOfAlts; k++)
                alternates[k - 1] = tempAlternates[k];
              free(tempAlternates);
              isAlt = true;
              break;
            } else {
              // Force break the loop, ignore as it cannot be resolved
              // Allow administrator to handle error manually
              getAvailableCourse = false;
              break;
            }
          }
        }
      }
      // printf("got available course for %s...\n", course); 
    }

    // Asign remaining alternates to student
    for (size_t i = 0; i < size_students; i++) {
      if (students[i].pupilNum == student.pupilNum) {
        students[i].remainingAlts = realloc(students[i].remainingAlts, numberOfAlts * sizeof(REQUEST));
        memcpy(students[i].remainingAlts, alternates, numberOfAlts * sizeof(REQUEST));
        students[i].remainingAltsLen = numberOfAlts;
        break;
      }
    }
    free(alternates);

    // realloc tempStudents to be size_tempStudents - 1 without struct of student just processed
    size_tempStudents--;
    STUDENT *new_tempStudents = malloc(size_tempStudents * sizeof(STUDENT));
    size_t idx = 0;
    for (size_t i = 0; i <= size_tempStudents; i++) {
      if (tempStudents[i].pupilNum != student.pupilNum) {
        new_tempStudents[idx] = tempStudents[i];
        idx++;
      }
    }

    tempStudents = realloc(tempStudents, size_tempStudents * sizeof(STUDENT));
    memcpy(tempStudents, new_tempStudents, size_tempStudents * sizeof(STUDENT));
    free(new_tempStudents);
  }

  free(currentInserted);
  free(tempStudents);

  // DO MORE ALGORITHM

  free(classes);
  free(allClassRunCounts);

  return timetable;
}

/*** MAIN ***/

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

  free(students);
  free(courses);
  
  return 0;
}
