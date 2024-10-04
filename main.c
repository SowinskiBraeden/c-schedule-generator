#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <stdint.h>
#include <sys/stat.h>
#include <math.h>

// Import and define proper functions depending on OS
#ifdef _WIN32
#include <direct.h>
#define MKDIR(name) _mkdir(name)
#else
#include <unistd.h>
#define MKDIR(name) mkdir(name, 0777)
#endif

/*
  Using define instead of constants prevents
  "Variable modified error"
*/
// Important constants
#define MAX_PUPIL_NUM_LEN 8
#define MAX_COURSE_DES_LEN 50
#define MAX_COURSE_NO_LEN 21
#define MAX_COURSE_ID_LEN MAX_COURSE_NO_LEN + 3 // 3 includes the underscore and the 2 digit unique number to identify the course

#define TOTAL_BLOCKS 10 // the number of blocks between 2 semesters i.e 8 = 4 blocks per semester, 10 = 5 blocks per semester
#define MAX_REQUEST_ALTS 6
#define CLASSROOMS 40
#define MIN_REQ 18
#define CLASS_CAP 30
#define MAX_STUDENTS CLASS_CAP * CLASSROOMS
#define MAX_COURSES CLASSROOMS * TOTAL_BLOCKS

// 5 is the length of "FALSE" & 3 is the number of commas per line
#define MAX_CHAR MAX_PUPIL_NUM_LEN + MAX_COURSE_NO_LEN + MAX_COURSE_DES_LEN + 5 + 3

const char FLEX[2][11] = {"XAT--12A-S", "XAT--12B-S"};

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
    fputs("Failed to open CSV file\n", stderr);
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
    fputs("Failed to open CSV file\n", stderr);
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
  uint32_t students[CLASS_CAP * CLASSROOMS * TOTAL_BLOCKS]; // Will be large enough to handle all pupil numbers taking this course, even if it is highly requested.
  uint8_t globalNumberOfStudents;
} COURSE; // Course is the general information for selection and stuff?

typedef struct {
  char crsNo[MAX_COURSE_NO_LEN];
  char description[MAX_COURSE_DES_LEN];
  uint32_t students[CLASS_CAP];
  uint8_t numberOfStudents;
} CLASS; // A class is an actual active course, there can be many classes for 1 course, each with its own number of students.

/*** DEFINE GET FUNCTIONS FOR STUDENTS & COURSES FROM CSV ***/

bool intInArray(uint32_t val, uint32_t *arr, size_t size) {
  for (size_t i = 0; i < size; i++) 
    if (arr[i] == val)
      return true;
  return false;
}

/*
  TODO: find a way to make the arr paramter work with any size, i.e char **arr
  However, when I call this array I can't seem to be able to pass the string array
  I want without it throwing an error when using char **arr as the parameter
*/
bool strInArray(char *str, char arr[MAX_COURSES][MAX_COURSE_NO_LEN], size_t size) {
  for (size_t i = 0; i < size; i++)
    if (strcmp(arr[i], str) == 0)
      return true;
  return false;
}

typedef struct {
  uint32_t uniquePupilNumbers[MAX_STUDENTS];
  size_t numberOfStudents;
} UNIQUE_STUDENTS; 

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

typedef struct {
  char uniqueCrsNos[MAX_COURSES][MAX_COURSE_NO_LEN];
  uint16_t numberOfCourses;
} UNIQUE_COURSES;

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
        COURSE course = {"\0", 0, "\0", 4, 0}; // Ensure array of pupil numbers is null till we populate it later with the correct size
        strcpy(course.crsNo, lines[i].crsNo);
        strcpy(course.description, lines[i].description);
        courses[i] = course;
        break;
      }
    }
  }
  return courses;
}

// Yeah the above functions are not optimal, doing the same loop over and over in different places but oh well, works for me

/*** DEFINE JSON FUNCTION HANDLERS ***/

int createDirectory(const char *dir_name) {
  // check if directory already exists
  struct stat statbuf;
  if (stat(dir_name, &statbuf) == 0 && S_ISDIR(statbuf.st_mode))
    return 0; // Directory exists

  if (MKDIR(dir_name) == 0) {
    return 1; // Directory created
  } else {
    fputs("Failed to create directory", stderr);
    return -1;
  }
}

int writeStudentsToJson(STUDENT *students, size_t numberOfStudents, char output_dir[]) {
  FILE *stream = fopen(output_dir, "w");
  if (!stream) {
    fputs("Failed to open students output file", stderr);
    return -1;
  }

  fprintf(stream, "[\n");

  for (size_t i = 0; i < numberOfStudents; i++) {
    fprintf(stream, "  {\n");
    fprintf(stream, "    \"pupilNum\": %d,\n", students[i].pupilNum);
    fprintf(stream, "    \"requests\": [\n");
    
    for (size_t j = 0; j < students[i].requestsLen; j++) {
      fprintf(stream, "      {\n");
      fprintf(stream, "        \"crsNo\": \"%s\",\n", students[i].requests[j].crsNo);
      fprintf(stream, "        \"description\": \"%s\",\n", students[i].requests[j].description);
      fprintf(stream, "        \"alternate\": %s\n", students[i].requests[j].alternate ? "true" : "false");
      fprintf(stream, "      }%s\n", j == students[i].requestsLen - 1 ? "" : ",");
    }
    fprintf(stream, "    ],\n");
    fprintf(stream, "    \"requestsLen\": %d,\n", students[i].requestsLen);
    
    if (students[i].classes > 0) {
      fprintf(stream, "    \"schedule\": [\n");
      for (size_t j = 0; j < students[i].classes; j++) {
        fprintf(stream, "      \"%s\"%s\n", students[i].schedule[j], j == students[i].classes - 1 ? "" : ",");
      }
      fprintf(stream, "    ],\n");
    } else {
      fprintf(stream, "    \"schedule\": [],\n");
    }

    fprintf(stream, "    \"expectedClasses\": %d,\n", students[i].expectedClasses);
    fprintf(stream, "    \"classes\": %d,\n", students[i].classes);
    
    if (students[i].remainingAltsLen > 0) {
      fprintf(stream, "    \"remainingAlts\": [\n");
      for (size_t j = 0; j < students[i].remainingAltsLen; j++) {
        fprintf(stream, "      {\n");
        fprintf(stream, "        \"crsNo\": \"%s\",\n", students[i].remainingAlts[j].crsNo);
        fprintf(stream, "        \"description\": \"%s\",\n", students[i].remainingAlts[j].description);
        fprintf(stream, "        \"alternate\": %s\n", students[i].remainingAlts[j].alternate ? "true" : "false");
        fprintf(stream, "      }%s\n", j == students[i].remainingAltsLen - 1 ? "" : ",");
      }
      fprintf(stream, "    ],\n");
    } else {\
      fprintf(stream, "    \"remainingAlts\": [],\n");
    }
    
    fprintf(stream, "    \"grade\": %d\n", students[i].grade);
    fprintf(stream, "  }%s\n", i == numberOfStudents - 1 ? "" : ",");
  }

  fprintf(stream, "]");
  fclose(stream);
  return 0;
}

int writeCoursesToJson(COURSE *courses, size_t numberOfCourses, char output_dir[]) {
  FILE *stream = fopen(output_dir, "w");
  if (!stream) {
    fputs("Failed to open courses output file", stderr);
    return -1;
  }
  
  fprintf(stream, "[\n");

  for (size_t i = 0; i < numberOfCourses; i++) {
    fprintf(stream, "  {\n");
    fprintf(stream, "    \"crsNo\": \"%s\",\n", courses[i].crsNo);
    fprintf(stream, "    \"requests\": %d,\n", courses[i].requests);
    fprintf(stream, "    \"description\": \"%s\",\n", courses[i].description);
    fprintf(stream, "    \"credits\": %d,\n", courses[i].credits);
    
    if (courses[i].globalNumberOfStudents > 0) {
      fprintf(stream, "    \"students\": [\n");
      for (size_t j = 0; j < courses[i].globalNumberOfStudents; j++) {
        fprintf(stream, "      \"%d\"%s\n", courses[i].students[j], j == courses[i].globalNumberOfStudents - 1 ? "" : ",");
      }
      fprintf(stream, "    ],\n");
    } else {
      fprintf(stream, "    \"students\": [],\n");
    }

    fprintf(stream, "    \"globalNumberOfStudents\": %d\n", courses[i].globalNumberOfStudents);
    fprintf(stream, "  }%s\n", i == numberOfCourses - 1 ? "" : ",");
  }

  fprintf(stream, "]");
  fclose(stream);
  return 0;
}

/*** ALGORITHM ***/

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
  TIMETABLE timetable = {{{{0}}}, 0};
  
  uint8_t MEDIAN = floor((MIN_REQ + CLASS_CAP) / 2);
  uint8_t BLOCKS_PER_SEMESTER = TOTAL_BLOCKS / 2;


  // Step 1 - Tally requests to check which courses are eligable to run
  char activeCourses[MAX_COURSES][MAX_COURSE_NO_LEN] = {{"\0"}};
  size_t activeCoursesIndexes[MAX_COURSES] = {0};
  uint16_t activeCourseIndex = 0; // also acts as the length of activeCourses
  for (size_t i = 0; i < size_students; i++) {
    for (size_t j = 0; j < students[i].requestsLen; j++) {
      if (students[i].requests[j].alternate) continue;
      if (strcmp(students[i].requests[j].crsNo, FLEX[0]) == 0) continue;
      if (strcmp(students[i].requests[j].crsNo, FLEX[1]) == 0) continue;

      for (size_t k = 0; k < size_courses; k++) {
        if (strcmp(courses[k].crsNo, students[i].requests[j].crsNo) == 0) {
          courses[k].requests++;
          if (courses[k].requests >= MIN_REQ) {
            if (activeCourseIndex == 0) {
              strcpy(activeCourses[activeCourseIndex], courses[k].crsNo);
              activeCoursesIndexes[activeCourseIndex] = k;
              activeCourseIndex++;
            } else {
              bool exists = false;
              for (size_t l = 0; l < activeCourseIndex; l++) {
                if (strcmp(activeCourses[l], courses[k].crsNo) == 0) {
                  exists = true;
                  break;
                }
              }

              if (!exists) {
                strcpy(activeCourses[activeCourseIndex], courses[k].crsNo);
                activeCoursesIndexes[activeCourseIndex] = k;
                activeCourseIndex++;
              }
            }
          }
          break;
        }
      }
    }  
  }


  // Step 2 - Generate classes with no students, but calculate the number of expected students per class

  char hex[] = "0123456789abcdefABCDEF"; // Used to get a unique character to identify different classes of the same course

  CLASS *emptyClasses = malloc(CLASSROOMS * TOTAL_BLOCKS * sizeof(CLASS)); // max this out to total number of classrooms available between both semesters
  size_t currentIndex = 0;
  for (size_t i = 0; i < activeCourseIndex; i++) {
    uint16_t index = activeCoursesIndexes[i];
    uint8_t classRunCount = floor(courses[index].requests / MEDIAN);
    uint8_t remaining = courses[index].requests % MEDIAN;

    size_t *courseClassIndexes = malloc((classRunCount + 1) * sizeof(size_t)); // add 1 to classRunCount in case we need to create an extra class with remaining
    for (size_t j = 0; j < classRunCount; j++) {
      CLASS newClass;
      char courseID[MAX_COURSE_NO_LEN + 3]; // +3 = "_n\0" where n is the ID
      strcpy(courseID, courses[index].crsNo);
      appendChar(courseID, hex[j]);
      strcpy(newClass.crsNo, courseID);
      strcpy(newClass.description, courses[index].description);
      newClass.numberOfStudents = MEDIAN; // The expected number of students in this class

      // CLASS_HASH emptyClass;
      // strcpy(emptyClass.key, courses[index].crsNo);
      // emptyClass.class = newClass;
      // emptyClasses[currentIndex] = emptyClass;
      emptyClasses[currentIndex] = newClass;
      courseClassIndexes[j] = currentIndex;
      currentIndex++;
    }

    // Handle remaining requests
    
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
          emptyClasses[courseClassIndexes[j]].numberOfStudents++;
          remaining--;
        }
      }

    } else if (remainingCanCreateNewClass) {
      // Create new class
      CLASS newClass;
      char courseID[MAX_COURSE_NO_LEN + 3]; // +3 = "_n\0" where n is the ID
      strcpy(courseID, courses[index].crsNo);
      appendChar(courseID, hex[classRunCount]);
      strcpy(newClass.crsNo, courseID);
      strcpy(newClass.description, courses[index].description);
      newClass.numberOfStudents = remaining;

      // Insert class into empty classes array and update index
      emptyClasses[currentIndex] = newClass;
      courseClassIndexes[classRunCount] = currentIndex;
      currentIndex++;

      // update class run count; if there is more than one class, equalize the class number of students
      classRunCount++;
      if (classRunCount >= 2) {
        uint8_t *numberOfStudentsArr = malloc(classRunCount * sizeof(uint8_t));
        for (size_t j = 0; j < classRunCount; j++)
          numberOfStudentsArr[j] = emptyClasses[courseClassIndexes[j]].numberOfStudents;

        numberOfStudentsArr = equal(numberOfStudentsArr, classRunCount);
        for (size_t j = 0; j < classRunCount; j++)
          emptyClasses[courseClassIndexes[j]].numberOfStudents = numberOfStudentsArr[j];

        free(numberOfStudentsArr);
      }

    } else if (remainingPlusExtraFromExistingCanCreateNewClass) {
      // TODO handle this logic

    } else {
      /*
        If all above cannot handle remaining requests we will add as many of
        the remaining requests to the existing classes. Any number of requests
        that dont fit will be ignored so later they can be folded into their
        alternative choices
      */

      // TODO: handle this logic too
    }

    free(courseClassIndexes);
  }

  // DO MORE ALGORITHM

  free(emptyClasses);

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
    fputs("Failed to read number of lines in the givengiven CSV file!\n", stderr);
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

  // Write data to json for output
  if (createDirectory("output") == -1) return -1;
  if (writeStudentsToJson(students, students_info.numberOfStudents, "output/students.json") == -1) return -1;
  if (writeCoursesToJson(courses, courses_info.numberOfCourses, "output/courses.json") == -1) return -1;

  free(students);
  free(courses);
  
  return 0;
}
