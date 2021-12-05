#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>

#include <openssl/sha.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/io.h>

#define CONFIG_PATH "/proc/filevault_config"
#define PASSWD_MAX_LENGTH 21
#define CMD_MAX_LENGTH 80
#define PATH_MAX_LENGTH 256

char PASSWD_FILE[PATH_MAX_LENGTH] = {0};

void sha256(const char* src, char* result) {
  unsigned char md[33] = {0};
  char tmp[3] = {0};
  SHA256(src, strlen(src), md);
  for (int i = 0; i < 32; i++) {
    sprintf(tmp, "%02x", md[i]);
    strcat(result, tmp);
  }
}

void set_password() {
  char pwd[PASSWD_MAX_LENGTH] = {0};
  char verify_pwd[PASSWD_MAX_LENGTH] = {0};

  while (1) {
    printf("Please set your password (length <= %d): ", PASSWD_MAX_LENGTH - 1);
    system("stty -echo");
    fgets(pwd, PASSWD_MAX_LENGTH-1, stdin);
    printf("\n");

    printf("Please verify your password: ");
    fgets(verify_pwd, PASSWD_MAX_LENGTH-1, stdin);
    printf("\n");
    system("stty echo");

    if (strcmp(pwd, verify_pwd) != 0) printf("Password does not match. Please try again!\n");
    else break;
  }

  char pwd_hash[65] = {0};
  sha256(pwd, pwd_hash);
  FILE* pwd_file = fopen(PASSWD_FILE, "w");
  fputs(pwd_hash, pwd_file);
  fclose(pwd_file);
  printf("Password is saved.\n\n");
}

int login() {
  char saved_pwd_hash[65] = {0};
  FILE* pwd_file = fopen(PASSWD_FILE, "r");
  fgets(saved_pwd_hash, 65, pwd_file);
  fclose(pwd_file);

  char input_pwd[PASSWD_MAX_LENGTH] = {0};
  char input_pwd_hash[65] = {0};
  system("stty -echo");

  for (int i = 5; i > 0; i--) {
    printf("Please input your password to login the filebox: ");
    memset(input_pwd, 0, PASSWD_MAX_LENGTH);
    memset(input_pwd_hash, 0, 65);
    fgets(input_pwd, PASSWD_MAX_LENGTH-1, stdin);
    printf("\n");

    sha256(input_pwd, input_pwd_hash);
    if (strcmp(saved_pwd_hash, input_pwd_hash) == 0) {
      system("stty echo");
      printf("Login successful!\n");
      return 1;
    }
    else if (i > 1) printf("Login failed. You have %d chance left to try again!\n", i-1);
  }

  system("stty echo");
  printf("Login failed!\n\n");
  return 0;
}

int set_protected_path() {
  char path[PATH_MAX_LENGTH];
  printf("Please input a path for filevault (leave blank to disable all protection): ");
  fgets(path, PATH_MAX_LENGTH-1, stdin);
  path[strcspn(path, "\r\n")] = '\0';

  if (access(CONFIG_PATH, W_OK) == 0) {
    FILE* config_file = fopen(CONFIG_PATH, "w");
    fputs(path, config_file);
    printf("New path for filevault is %s\n", path);
    fclose(config_file);
    return 0;
  }
  else {
    printf("Failed to modify the config file. Consider run this program with \"sudo\".\n");
    return -1;
  }
}

void banner();
void help();

int main(int argc, char* argv[]) {
  // prepare pwd file
  strcpy(PASSWD_FILE, getenv("HOME"));
  strcat(PASSWD_FILE, "/.filevault_key");
  banner();

  // put settings outside of main routine
  if (argc > 1) {
    // set password
    if (strcmp(argv[1], "setpwd") == 0) {
      if (access(PASSWD_FILE, F_OK) != 0) set_password();
      else {
        if (login() == 0) {
          printf("You must login to modify the password.\n");
          return -1;
        }
        else set_password();
      }
      return 0;
    }
    
    // set protected path
    else if (strcmp(argv[1], "setpath") == 0) {
      if (access(PASSWD_FILE, F_OK) != 0) {
        printf("First time user please use \"filevault setpwd\" to set a password and try again.\n");
        return -1;
      }
      else {
        if (login() == 0) {
          printf("You must login to modify the path for filevault.\n");
          return -1;
        }
        else return set_protected_path();
      }
    }

    else if (strcmp(argv[1], "help") == 0) {
      help();
      return 0;
    }

    else {
      printf(
        "Unrecognizable argument, possible arguments are \"help, setpwd, setpath\".\n"
        "If you wish to enter the filevault environment, use filevault without any arguments.\n"
      );
      return -1;
    }
  }
  else {
    if (login() == 0) {
      printf("You must login to use the filevault environment.\n");
      return -1;
    }
    // create a simulated command-line environment
    system("clear");
    printf("(This is the filevault environment, press Ctrl+C to exit.)\n\n");
    while (1) {
      char current_path[PATH_MAX_LENGTH];
      getcwd(current_path, PATH_MAX_LENGTH);
      printf("filevault:%s > ", current_path);

      char cmd[CMD_MAX_LENGTH];
      fgets(cmd, CMD_MAX_LENGTH, stdin);
      cmd[strcspn(cmd, "\r\n")] = '\0';
      if (strncmp(cmd, "cd", 2) == 0) {
        char tmp[CMD_MAX_LENGTH];
        strncpy(tmp, cmd, CMD_MAX_LENGTH);
        char* p = strtok(tmp, " ");
        p = strtok(NULL, " ");
        if (p) chdir(p);
      }
      else system(cmd);
    }
  }
}

void banner() {
  printf(
    " ______  _______  _       ______  _     _   _____   _     _  _    _______ \n"
    "(______)(_______)(_)     (______)(_)   (_) (_____) (_)   (_)(_)  (__ _ __)\n"
    "(_)__      (_)   (_)     (_)__   (_)   (_)(_)___(_)(_)   (_)(_)     (_)   \n"
    "(____)     (_)   (_)     (____)  (_)   (_)(_______)(_)   (_)(_)     (_)   \n"
    "(_)      __(_)__ (_)____ (_)____  (_)_(_) (_)   (_)(_)___(_)(_)____ (_)   \n"
    "(_)     (_______)(______)(______)  (___)  (_)   (_) (_____) (______)(_)   \n"
    "                                                                          \n"
  );
}

void help() {
  printf(
    "Command line Arguments:\n"
    "(no argument)    enter filevault command line environment\n"
    "help             print this help\n"
    "setpwd           set password\n"
    "setpath          set path for filevault\n"
  );
}
