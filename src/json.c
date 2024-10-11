#include <stdio.h>
#include <sys/stat.h>
#include <stdint.h>
#include "../include/students.h"
#include "../include/courses.h"

// Import and define proper functions depending on OS
#ifdef _WIN32
#include <direct.h>
#define MKDIR(name) _mkdir(name)
#else
#include <unistd.h>
#define MKDIR(name) mkdir(name, 0777)
#endif

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
