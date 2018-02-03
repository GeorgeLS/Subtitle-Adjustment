#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <locale.h>

typedef struct {
  int hours;
  int seconds;
  double milliseconds;  
} timestamp_t;

timestamp_t from;
timestamp_t to;
timestamp_t user_timestamp;
FILE* sub_file;
FILE* new_sub_file;
char buffer[1024];

int is_timestamp(void) {
  if (strstr(buffer, "-->") != NULL) return 1;
  return 0;
}

void extract_timestamp(void) {
  char* aux = buffer;
  int time = 0;  
  while (*aux != ':') {
    time = (time*10) + *aux++ - '0';    
  }
  from.hours = time;
  time = 0;
  ++aux;
  while (*aux != ':') {
    time = (time*10) + *aux++ - '0';    
  }
  from.seconds = time;
  ++aux;
  double dtime = 0.0;  
  while (*aux != ',') {
    dtime = (dtime*10.0) + *aux++ - '0'; 
  }
  from.milliseconds = dtime;
  dtime = 0;
  ++aux;
  while (*aux != ' ') {
    dtime = (dtime*10.0) + *aux++ - '0';
  }
  dtime /= 1000.0;
  from.milliseconds += dtime;
  time = 0;
  dtime = 0.0;  
  while (!isdigit(*aux)) ++aux;  
  while (*aux != ':') {
    time = (time*10) + *aux++ - '0';    
  }
  to.hours = time;  
  time = 0;
  ++aux;
  while (*aux != ':') {
    time = (time*10) + *aux++ - '0';    
  }
  to.seconds = time;
  ++aux;
  while (*aux != ',') {
    dtime = (dtime*10.0) + *aux++ - '0'; 
  }
  to.milliseconds = dtime;
  dtime = 0;
  ++aux;
  while (*aux != '\n' && *aux != '\r') {
    dtime = (dtime*10.0) + *aux++ - '0';
  }
  dtime /= 1000.0;
  to.milliseconds += dtime;
}

void modify_timestamp(void) {
  from.hours += user_timestamp.hours;
  from.seconds += user_timestamp.seconds;
  from.milliseconds += user_timestamp.milliseconds;
  to.hours += user_timestamp.hours;
  to.seconds += user_timestamp.seconds;
  to.milliseconds += user_timestamp.milliseconds;
}

void reset_timestamp(void) {
  from.hours = to.hours = 0;
  from.seconds = to.seconds = 0;
  from.milliseconds = to.milliseconds = 0.0;
}

void update_timestamp_buffer(void) {
  char* old_locale = setlocale(LC_NUMERIC, NULL);
  setlocale(LC_NUMERIC, "de_DE.UTF-8");
  sprintf(buffer, "%02d:%02d:%06.3f --> %02d:%02d:%06.3f\r\n",
	  from.hours, from.seconds, from.milliseconds,
	  to.hours, to.seconds, to.milliseconds);
  setlocale(LC_NUMERIC, old_locale);
}

void write(void) {
  while (fgets(buffer, sizeof(buffer), sub_file) != NULL) {
    if (is_timestamp()) {
      extract_timestamp();
      modify_timestamp();
      update_timestamp_buffer();
      fprintf(new_sub_file, "%s", buffer);
      reset_timestamp();      
    } else fprintf(new_sub_file, "%s", buffer);
  }
}

void read_user_timestamp(void) {
  int hours;
  int seconds;
  double milliseconds;
  printf("Please enter the number of hours you want to forward/delay: ");
  scanf("%d", &hours);
  printf("Please enter the number of seconds you want to forward/delay: ");
  scanf("%d", &seconds);    
  printf("Please enter the number of milliseconds you want to forward/delay: ");
  scanf("%lf", &milliseconds);
  user_timestamp.hours = hours;
  user_timestamp.seconds = seconds;
  user_timestamp.milliseconds = milliseconds;
}

int main(int argc, char* argv[]) {
  for (size_t i = 1U; i != argc; ++i) {    
    char* new_sub_name = (char*) malloc((strlen(argv[i]) + 2)*sizeof(char));
    strncpy(new_sub_name, argv[i], strlen(argv[i]));
    new_sub_name[strlen(new_sub_name) - 4] = '\0';
    strcat(new_sub_name, "2.srt");
    new_sub_file = fopen(new_sub_name, "wb");
    if (new_sub_file == NULL) {
      fprintf(stderr, "Could not create new fail. Abort...\n");
      exit(EXIT_FAILURE);
    }
    sub_file = fopen(argv[i], "rb");
    if (sub_file == NULL) {
      fprintf(stderr, "Could not open given subtitle file. Abort...\n");
      exit(EXIT_FAILURE);
    }
    read_user_timestamp();
    write();
    free(new_sub_name);
    fclose(new_sub_file);
  }
  return EXIT_SUCCESS;
}
