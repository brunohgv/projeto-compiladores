#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void readFileToString(char *fileName, char *string) {
  FILE *fp = fopen(fileName, "r");
  if (fp == NULL) {
    printf("Error opening file %s\n", fileName);
    exit(1);
  }

  int string_length = 0;
  char c;
  while ((c = fgetc(fp)) != EOF) {
    string[string_length++] = c;
  }

  string[string_length] = '\0';

  fclose(fp);
}
