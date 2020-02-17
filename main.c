#include <dirent.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>

bool ispid(char name[]);

struct process {
  char *name;
  char *pid;
};

int main() {
  DIR *procdir;
  FILE *fptr;
  struct dirent *dirptr = (void *)0xAAAAAAAA;
  struct process proc;

  const char *basepath = "/proc/";
  char *line = NULL;
  char ppidstr[6]; // max 32768 + null term
  char *token = NULL;
  pid_t ppid;
  size_t len = 0;
  ssize_t read;

  procdir = opendir(basepath);
  ppid = getppid();
  sprintf(ppidstr, "%d", ppid);

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
      char *path = malloc(strlen(basepath) + strlen(dirptr->d_name) + 6);
      strcpy(path, basepath);
      strcat(path, dirptr->d_name);
      strcat(path, "/stat\0");
      
      fptr = fopen(path, "r");
      if (fptr == NULL) {
        printf("Couldn't read pid %s, skipping", dirptr->d_name);
        continue;
      }

      int i;
      bool filter;
      while ((read = getline(&line, &len, fptr)) != -1) {
        token = strtok(line, " ");
        filter = false;
        for (i = 0; token != NULL; i++) {
          switch(i) {
            case 0:
              proc.pid = token;
              if (strcmp(token, ppidstr) == 0) {
                filter = true;
              }
              break;
            case 1:
              proc.name = token;
              break;
            case 3:
              if (strcmp(token, ppidstr) == 0) {
                filter = true;
              }
              break;
            default:
              break;
          }
          token = strtok(NULL, " ");
        }
      }

      if (filter) {
        printf("%s\t%s\n", proc.pid, proc.name);
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
