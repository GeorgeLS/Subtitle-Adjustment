/* sub_sync.c --- 
 * 
 * Filename: sub_sync.c
 * Author: George Liontos
 * Created: Sat Feb  3 04:10:49 2018 (+0200)
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or (at
 * your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with GNU Emacs.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <locale.h>

typedef struct {
  int hours;
  int minutes;
  double seconds;  
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

int is_id(void) {
  int flag = 1;
  char* aux = buffer;
  while (*aux != '\r' && *aux != '\n') {
    if (!isdigit(*aux++)) {
      flag = 0;
      break;
    }
  }
  return(flag);
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
  from.minutes = time;
  ++aux;
  double dtime = 0.0;  
  while (*aux != ',') {
    dtime = (dtime*10.0) + *aux++ - '0'; 
  }
  from.seconds = dtime;
  dtime = 0;
  ++aux;
  while (*aux != ' ') {
    dtime = (dtime*10.0) + *aux++ - '0';
  }
  dtime /= 1000.0;
  from.seconds += dtime;
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
  to.minutes= time;
  ++aux;
  while (*aux != ',') {
    dtime = (dtime*10.0) + *aux++ - '0'; 
  }
  to.seconds = dtime;
  dtime = 0;
  ++aux;
  while (*aux != '\n' && *aux != '\r') {
    dtime = (dtime*10.0) + *aux++ - '0';
  }
  dtime /= 1000.0;
  to.seconds += dtime;
}

void modify_timestamp(void) {
  from.hours += user_timestamp.hours;
  from.hours = (from.hours < 0) ? 0 : from.hours;
  from.minutes += user_timestamp.minutes;
  from.minutes = (from.minutes < 0) ? 0 : from.minutes;
  from.seconds += user_timestamp.seconds;
  from.seconds = (from.seconds < 0.0) ? 0.0 : from.seconds;
  to.hours += user_timestamp.hours;
  to.hours = (to.hours < 0) ? 0 : to.hours;
  to.minutes += user_timestamp.minutes;
  to.minutes = (to.minutes < 0) ? 0 : to.minutes;
  to.seconds += user_timestamp.seconds;
  to.seconds = (to.seconds < 0.0) ? 0.0 : to.seconds;
}

void reset_timestamp(void) {
  from.hours = to.hours = 0;
  from.minutes = to.minutes = 0;
  from.seconds = to.seconds = 0.0;
}

void update_timestamp_buffer(void) {
  char* old_locale = setlocale(LC_NUMERIC, NULL);
  setlocale(LC_NUMERIC, "de_DE.UTF-8");
  sprintf(buffer, "%02d:%02d:%06.3f --> %02d:%02d:%06.3f\r\n",
	  from.hours, from.minutes, from.seconds,
	  to.hours, to.minutes, to.seconds);
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
  int minutes;
  double seconds;
  printf("Please enter the number of hours you want to forward/delay: ");
  scanf("%d", &hours);
  printf("Please enter the number of minutes you want to forward/delay: ");
  scanf("%d", &minutes);    
  printf("Please enter the number of seconds you want to forward/delay: ");
  scanf("%lf", &seconds);
  user_timestamp.hours = hours;
  user_timestamp.minutes = minutes;
  user_timestamp.seconds = seconds;
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
    printf("Modifying file: %s\n", argv[i]);
    read_user_timestamp();
    write();
    free(new_sub_name);
    fclose(new_sub_file);
  }
  return EXIT_SUCCESS;
}
