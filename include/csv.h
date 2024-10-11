#include <stdbool.h>
#include <stdint.h>
#include "main.h"

#ifndef CSV_H_INCLUDED
#define CSV_H_INCLUDED

typedef struct {
  uint32_t pupilNum;
  char crsNo[MAX_COURSE_NO_LEN];
  char description[MAX_COURSE_DES_LEN];
  bool alternate;
} CSV_LINE;

int count_lines(char data_dir[]);
CSV_LINE *csvReader(char data_dir[], size_t size);

#endif