#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <stdint.h>

/*
  Using define instead of constants prevents
  "Variable modified error"
*/
// Important constants
#define MAX_PUPIL_NUM_LEN 8
#define MAX_COURSE_DES_LEN 50
#define MAX_COURSE_NO_LEN 20
#define MAX_COURSE_ID_LEN MAX_COURSE_NO_LEN + 4 // 4 includes the underscore and the 3 digit unique number to identify the course

#define TOTAL_BLOCKS 10 // the number of blocks between 2 semesters i.e 8 = 4 blocks per semester, 10 = 5 blocks per semester
#define MAX_REQUEST_ALTS 6
#define CLASSROOMS 40
#define MIN_REQ 18
#define CLASS_CAP 30
#define MAX_STUDENTS CLASS_CAP * CLASSROOMS

// 6 is the length of "FALSE" + a null character & 3 is the number of commas per line
#define MAX_CHAR MAX_PUPIL_NUM_LEN + MAX_COURSE_NO_LEN + MAX_COURSE_DES_LEN + 5 + 3

/*** DEFINE CSV READER FUNCTIONS & DATASTRUCTURES ***/

typedef struct {
  uint32_t pupilNum;
  char crsNo[MAX_COURSE_NO_LEN];
  char description[MAX_COURSE_DES_LEN];
  bool alternate;
} CSV_LINE;

// Dumbass me set this to size_t earlier, forgot that size_t is unsigned and I can't return -1 on failure
int count_lines(char data_dir[]) {
  FILE *stream = fopen(data_dir, "r");
  if (!stream) {
    fputs("Failed to open CSV file", stderr);
    return -1;
  }
  char buff[MAX_CHAR];
  size_t counter = 1;
  while (fgets(buff, sizeof(buff), stream) != NULL) {
    if (strchr(buff, '\n') != NULL) counter++;
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
  while (fgets(buff, sizeof(buff), stream)) {
    CSV_LINE csv_line;
    lines[i] = csv_line;

    // Tokenize the line to extract data
    char *token = strtok(buff, ",");
    if (token)
      lines[i].pupilNum = atoi(token); // convert string to uint32_t
    
    token = strtok(NULL, ",");
    if (token)
      strncpy(lines[i].crsNo, token, MAX_COURSE_NO_LEN);

    token = strtok(NULL, ",");
    if (token)
      strncpy(lines[i].description, token, MAX_COURSE_DES_LEN);

    token = strtok(NULL, ",");
    if (token) {
      char alternate[5];
      strncpy(alternate, token, 5);
      alternate[strcspn(alternate, "\n")] = 0; // Remove newline
      lines[i].alternate = !strcmp(alternate, "TRUE") ? true : false;
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
  bool alternate;
} REQUEST;

typedef struct {
  uint32_t pupilNum;
  REQUEST *requests;
  uint8_t requestsLen;
  char **schedule;
  uint8_t expectedClasses;
  uint8_t classes;
  REQUEST *remainingAlts;
  uint8_t remainingAltsLen;
  uint8_t grade;
} STUDENT;

typedef struct {
  char crsNo[MAX_COURSE_NO_LEN];
  uint16_t requests;
  char description[MAX_COURSE_DES_LEN];
  uint8_t credits;
  char *students;
} COURSE;

/*** DEFINE GET FUNCTIONS FOR STUDENTS & COURSES FROM CSV ***/

bool valueInArray(uint32_t val, uint32_t *arr, size_t n) {
  for (size_t i = 0; i < n; i++) 
    if (arr[i] == val)
      return true;
  return false;
}

typedef struct {
  uint32_t uniquePupilNumbers[MAX_STUDENTS];
  size_t numberOfStudents;
} STUDENT_ARR_INFO; 

STUDENT_ARR_INFO getNumberOfStudents(CSV_LINE *lines, size_t lines_len) {
  STUDENT_ARR_INFO students_info = {{0}, 0};
  for (size_t i = 0; i < lines_len; i++) {
    bool exists = valueInArray(lines[i].pupilNum, students_info.uniquePupilNumbers, students_info.numberOfStudents);
    if (sizeof(students_info.uniquePupilNumbers) == 0 || !exists) {
      students_info.uniquePupilNumbers[students_info.numberOfStudents] = lines[i].pupilNum;
      students_info.numberOfStudents++;
    }
  }
  return students_info;
}

STUDENT *getStudents(CSV_LINE *lines, size_t lines_len, int total_blocks, STUDENT_ARR_INFO students_info) {
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

COURSE *getCourses(CSV_LINE *lines) {
  // TODO: same shit as above but just for the courses
  return NULL;
}

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

  STUDENT_ARR_INFO students_info = getNumberOfStudents(lines, num_lines);

  STUDENT *students = getStudents(lines, num_lines, TOTAL_BLOCKS, students_info);
  if (students == NULL) return -1;

  free(lines);

  /*
    Below code used for debugging csvReader & getStudents function
  */
  for (int i = 0; i < students_info.numberOfStudents; i++) {
    printf("----------\n");
    printf("Pupil Num: %d\n", students[i].pupilNum);
    for (int j = 0; j < students[i].requestsLen; j++) {
      printf("Request: %s\n", students[i].requests[j].description);
    }
  }
 
  /*
    TODO: create writeStudentsToJson function to write the student structs to a json for debugging
    and final output data
  */

  // if (writeStudentsToJson(students, num_students, "students.json") == -1) return -1;

  // Algorithm
  free(students);
  
  return 0;
}