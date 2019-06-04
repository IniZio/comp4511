#include <stdio.h>      /* C Standard IO library */
#include <stdlib.h>     /* C Standard library routines */
#include <string.h>     /* library for string processing */
#include <pwd.h>        /* library to query user information */
#include <dirent.h>     /* library for directory traversing */
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

int returnTrue(char* line) {return 1;}
int isLinuxVersion(char* line) {return strstr(line, "Linux version ") != NULL;}
int isMemoryTotal(char* line) {return strstr(line, "MemTotal") != NULL;}
int isModelName (char* line) {return strstr(line, "model name	: ") != NULL;}

char* uid;

char* substring(const char* str, size_t begin, size_t len)
{
  if (str == 0 || strlen(str) == 0 || strlen(str) < begin || strlen(str) < (begin+len))
    return 0;

  return strndup(str + begin, len == 0 ? strlen(str) - begin : len);
}

char* readfile(char* fileName, char* buffer, int (*match)(char*), int allOccurences) {
	char* line = (char *)malloc(200 * sizeof(char));
  size_t len = 0;
	FILE *fin = fopen(fileName, "r");

  while(getline(&line, &len, fin) > 0){
    if ((*match)(line)) {
      strcat(buffer, line);
      if (!allOccurences) {
        return buffer;
      }
    }
  }

  free(line);
	fclose(fin);

	return buffer;
}

void processdir(const struct dirent *dir)
{
    //  puts(dir->d_name);
    char* buffer = (char *)malloc(200 * sizeof(char));
    char* path = (char *)malloc(200 * sizeof(char));;

    snprintf(path, 200, "/proc/%s/status", dir->d_name);

    readfile(path, buffer, returnTrue, 0);

    printf("%s: %s", dir->d_name, substring(buffer, strlen("Name: "), 0));
}

int filter(const struct dirent *dir)
{
     uid_t user;
     struct stat dirinfo;
     int len = strlen(dir->d_name) + 7; 
     char path[len];

     strcpy(path, "/proc/");
     strcat(path, dir->d_name);
     if (stat(path, &dirinfo) < 0) {
      perror("processdir() ==> stat()");
      exit(EXIT_FAILURE);
     }
     return !fnmatch("[1-9]*", dir->d_name, 0) && uid == dirinfo.st_uid;
}

/* Header files should be placed here */
/* Implementation of different handler functions */
int main (int argc, char *argv[]) {
  char* option;
  
  /* Variables definition of main() */
  if (argc < 2) { /* print out the usage of myproc */
    printf(
      "%s\n%s\n%s\n%s\n%s\n%s\n",
      "myproc: [option] [parameter]",
      "Usage 1: $> ./myprocprocessor -display processor type",
      "Usage 2: $> ./myproc kernel_version -display kernel version",
      "Usage 3: $> ./myproc memory -display the amount of memory in kB",
      "Usage 4: $> ./myproc uptime -display up time since booted in D:H:M:S format",
      "Usage 5: $> ./myproc user_process [username] -display a list of processes owned by the given username"
    );

  } else {
    /* Note: argv[0] is always equal to "myproc" */
    option = argv[1];

    if (strcmp(option, "processor") == 0) {
      char* buffer = (char *)malloc(200 * sizeof(char));
      
      readfile("/proc/cpuinfo", buffer, isModelName, 0);

      printf(
        "%s\n",
        substring(buffer, strlen("model name  : ") - 1, 0)
      );
      free(buffer);
    } else if (strcmp(option, "kernel_version") == 0) {
      char* buffer = (char *)malloc(200 * sizeof(char));

      readfile("/proc/version", buffer, isLinuxVersion, 0);

      printf(
        "%s\n",
        substring(
          buffer,
          strlen("Linux version ") - 1,
          strstr(buffer, "(") - buffer- strlen("Linux version ") + 1
        )
      );
      free(buffer);
    } else if (strcmp(option, "memory") == 0) {
      char* buffer = (char *)malloc(200 * sizeof(char));

      readfile("/proc/meminfo", buffer, isMemoryTotal, 0);

      printf(
        "%s\n",
        substring(
          buffer,
          strlen("MemTotal:        ") - 1,
          strlen(buffer)- strlen("MemTotal:        ") + 1
        )
      );
      free(buffer);
    } else if (strcmp(option, "uptime") == 0) {
      char* buffer = (char *)malloc(200 * sizeof(char));

      readfile("/proc/uptime", buffer, returnTrue, 0);

      float up_seconds = atof(substring(buffer, 0, strstr(buffer, " ") - buffer));

      printf(
        "%02dD:%02dH:%02dM:%02dS\n",
        (int) up_seconds / (3600 * 24),
        (int) up_seconds / (3600),
        (int) (up_seconds / 60) % 60,
        (int) up_seconds % 60
      );
      free(buffer);
    } else if (strcmp(option, "uptime") == 0) {
      char* buffer = (char *)malloc(200 * sizeof(char));

      readfile("/proc/uptime", buffer, returnTrue, 0);

      float up_seconds = atof(substring(buffer, 0, strstr(buffer, " ") - buffer));

      printf(
        "%02dD:%02dH:%02dM:%02dS\n",
        (int) up_seconds / (3600 * 24),
        (int) up_seconds / (3600),
        (int) (up_seconds / 60) % 60,
        (int) up_seconds % 60
      );
      free(buffer);
    } else if (strcmp(option, "user_process") == 0) {
      struct dirent **namelist;
      int n;
      int i = 0;

      if (strlen(argv[2]) == 0) {
        printf("Error: Missing username\n");
        return 1;
      }

      if (getpwnam(argv[2]) == NULL) {
        printf("Error: Invalid username %s\n", argv[2]);
        return 1;
      }

      uid = getpwnam(argv[2])->pw_uid;

      n = scandir("/proc", &namelist, filter, 0);

      if (n < 0)
      perror("Not enough memory.");
      else {
      while(i < n) {
          processdir(namelist[i]);
          free(namelist[i]);
          i++;
      }
      free(namelist);
      }
    } else {
      printf("Error: Unknown option %s\n", option);
      return 1;
    }

    /* Branching statements to handle different option */}
    return 0;
}
