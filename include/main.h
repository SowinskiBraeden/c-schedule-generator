#ifndef MAIN_H_INCLUDED
#define MAIN_H_INCLUDED

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
#define MAX_COURSE_ID_LEN MAX_COURSE_NO_LEN + 1

#define TOTAL_BLOCKS 10 // the number of blocks between 2 semesters i.e 8 = 4 blocks per semester, 10 = 5 blocks per semester
#define MAX_REQUEST_ALTS 6
#define CLASSROOMS 40
#define MIN_REQ 18
#define CLASS_CAP 30
#define MAX_STUDENTS CLASS_CAP * CLASSROOMS
#define MAX_CLASSES CLASSROOMS * TOTAL_BLOCKS

// 5 is the length of "FALSE" & 3 is the number of commas per line
#define MAX_CHAR MAX_PUPIL_NUM_LEN + MAX_COURSE_NO_LEN + MAX_COURSE_DES_LEN + 5 + 3

static const char FLEX[][11] = {"XAT--12A-S", "XAT--12B-S"};

void handle(void* mem, char name[]);

#endif