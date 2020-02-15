#include <dirent.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>

bool ispid(char name[]);

int main() {
  DIR *procdir;
  FILE *fptr;
  struct dirent *dirptr = (void *)0xDEC0DED;
  
  const char *basepath = "/proc/";
  char *line = NULL;
  char pname[64];
  char *token = NULL;
  char  uid[4];
  size_t len = 0;
  ssize_t read;

  procdir = opendir(basepath);
  sprintf(uid, "%lu", (unsigned long int)getuid()); 

  if (procdir == NULL) {
    printf("Can't open /proc");
    return 1;
  }

  printf("PID\tCMD\n");
  for(;;) {
    dirptr = readdir(procdir);
    if (dirptr == NULL) {
      break;
    }
   
    if (ispid(dirptr->d_name)) {
      char *path = malloc(strlen(basepath) + 1 + strlen(dirptr->d_name) + 7);
      strcpy(path, basepath);
      strcat(path, dirptr->d_name);
      strcat(path, "/status");
      
      fptr = fopen(path, "r");
      if (fptr == NULL) {
        printf("Couldn't read pid %s, skipping", dirptr->d_name);
        continue;
      }

      int i;
      while ((read = getline(&line, &len, fptr)) != -1) {
        if (strncmp("Name", line, 4) == 0) {
          token = strtok(line, "\t");
          for (i = 0; token != NULL; i++) {
            if (!i) {
              token = strtok(NULL, "\t");
              continue;
            }
            
            strcpy(pname, token);
            token = strtok(NULL, "\t");
          }
        }
        
        if (strncmp("Uid", line, 3) == 0) {
          token = strtok(line, "\t");
          for (i = 0; token != NULL; i++) {
            if (!i) {
              token = strtok(NULL, "\t");
              continue;
            }
         
            if (strncmp(uid, token, strlen(uid)) == 0) {
              printf("%s\t%s", dirptr->d_name, pname);
              break;
            } 
            token = strtok(NULL, "\t");
          }
        }
      }
       
      fclose(fptr);
    } 
  }

  closedir(procdir);
  return 0;
}

bool ispid(char name[]) {
  int i;
 
  for (i = 0; i < strlen(name); i++) {
    if (isdigit(name[i]) == 0) {
      return false;
    } 
  }

  return true;
}
