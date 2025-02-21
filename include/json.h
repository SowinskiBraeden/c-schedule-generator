#include "courses.h"
#include "students.h"

#ifndef JSON_H_INCLUDED
#define JSON_H_INCLUDED

int createDirectory(const char *dir_name);
int writeStudentsToJson(STUDENT *students, size_t numberOfStudents, char output_dir[]);
int writeCoursesToJson(COURSE *courses, size_t numberOfCourses, char output_dir[]);

#endif