#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>

#include <openssl/sha.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/io.h>

// #define TMP_KEY_PATH "/tmp/filebox_crypto_key"
#define CONFIG_PATH "/etc/filebox.cnf"
#define PASSWD_MAX_LENGTH 21
#define CMD_MAX_LENGTH 80
#define PATH_MAX_LENGTH 256

char PASSWD_FILE[PATH_MAX_LENGTH];

void sha256(char *src, char *result)
{
    unsigned char md[33] = {0};
    char tmp[3] = {0};
    SHA256(src, strlen(src), md);
    for (int i = 0; i < 32; i++)
    {
        sprintf(tmp, "%02x", md[i]);
        strcat(result, tmp);
    }
}

void set_password()
{
    char verify_pwd[PASSWD_MAX_LENGTH] = {0};
    char pwd[PASSWD_MAX_LENGTH] = {0};
    while (1)
    {
        printf("Please set your password (length <= %d):\n", PASSWD_MAX_LENGTH - 1);
        system("stty -echo");
        scanf("%s", pwd);
        printf("\n");
        printf("Please verify your password:\n");
        scanf("%s", verify_pwd);
        printf("\n");
        system("stty echo");
        if (strcmp(pwd, verify_pwd) != 0)
        {
            printf("Verification failed! Please try again!\n");
        }
        else
        {
            break;
        }
    };
    char *sha256_pwd = malloc(65);
    memset(sha256_pwd, 0, 65);
    sha256(pwd, sha256_pwd);

    FILE *file = fopen(PASSWD_FILE, "w");
    fputs(sha256_pwd, file);
    fclose(file);
    free(sha256_pwd);
}

int check_password()
{
    char buf[65];
    char pwd[PASSWD_MAX_LENGTH] = {0};

    FILE *file = fopen(PASSWD_FILE, "r");
    char *read_sha256 = fgets(buf, 65, file);
    fclose(file);

    char *sha256_pwd = malloc(65);

    printf("Please input your password to login the filebox: \n");

    system("stty -echo");
    for (int i = 5; i >= 1; i--)
    {
        memset(sha256_pwd, 0, 65);
        scanf("%s", pwd);
        printf("\n");
        sha256(pwd, sha256_pwd);
        if (strcmp(read_sha256, sha256_pwd) == 0)
        {
            system("stty echo");
            printf("Login Successfully!\n");
            free(sha256_pwd);
            return 1;
        }
        else if (i > 1)
        {
            printf("Login Failed! You have %d chance left to try again!\n", i - 1);
            printf("Please input your password to login the filebox: \n");
        }
    }
    system("stty echo");
    printf("Login Failed!\n");
    free(sha256_pwd);
    return 0;
}

// produce cipher key for file encryption/decryption
// use sha256(passwd) to produce random num
void create_key(unsigned char *key)
{
    FILE *file;
    char *sha256_pwd;
    unsigned int seed = 0;

    file = fopen(PASSWD_FILE, "r");
    sha256_pwd = fgets(buf, 65, file);
    fclose(file);

    for (int i = 0; i < 64; i++)
        seed += sha256_pwd[i];
    seed *= 2;

    srand(seed);
    for (int i = 0; i < 16; i++)
    {
        *(key + i) = (unsigned char)(rand() % 256);
    }
}

int main(int argc, char *argv[])
{
    printf("Welcome to the filebox!\n");
    printf("    ______ _____ _      ______ ____   ______   __     \n"
           "   |  ____|_   _| |    |  ____|  _ \\ / __ \\ \\ / /  \n"
           "   | |__    | | | |    | |__  | |_) | |  | \\ V /     \n"
           "   |  __|   | | | |    |  __| |  _ <| |  | |> <       \n"
           "   | |     _| |_| |____| |____| |_) | |__| / . \\     \n"
           "   |_|    |_____|______|______|____/ \\____/_/ \\_\\  \n"
           "\n"
           "Commands:\n\n"
                   "rpwd            reset your password\n"
                   "rpth <path>     set or reset filebox path\n"
                   "spth            show filebox path\n"
                   "cd <path>       change current working directory to <path>\n"
                   "h               show usage help\n"
                   "e               exit this program\n"
                   "\n"
                   "Shell commands (ls, cat, cp, etc..) are also available.\n");

    strcpy(PASSWD_FILE, getenv("HOME"));
    strcat(PASSWD_FILE, "/.filebox_key");

    if (access(PASSWD_FILE, F_OK))
    {
        printf("You are new to the filebox.\n");
        printf("Please register by the following steps.\n");
        set_password();
    }

    int is_login = check_password();
    if (is_login == 0)
        return 0;

    // create cipher key and save it to TMP_KEY_FILE
    unsigned char *key = (unsigned char *)malloc(16);
    memset(key, 0, 16);
    create_key(key);

    FILE *file = fopen(TMP_KEY_PATH, "w");
    fwrite(key, 1, 16, file);
    fclose(file);

    char *path = malloc(PATH_MAX_LENGTH);
    getcwd(path, PATH_MAX_LENGTH);

    setbuf(stdin, NULL);
    char cmd[CMD_MAX_LENGTH];
    char tmp[CMD_MAX_LENGTH];
    while (1)
    {
        printf("> %s: ", path);
        fgets(cmd, CMD_MAX_LENGTH, stdin);
        cmd[strlen(cmd) - 1] = '\0'; //消除行末的换行符
        if (strcmp(cmd, "e") == 0)
        {
            if (path)
                free(path);
            memset(cmd, 0, CMD_MAX_LENGTH);
            strcpy(cmd, "rm ");
            strcat(cmd, TMP_KEY_PATH);
            system(cmd);
            break;
        }
        else if (strcmp(cmd, "h") == 0)
        {
            printf("Commands:\n\n"
                   "rpwd            reset your password\n"
                   "rpth <path>     set or reset filebox path\n"
                   "spth            show filebox path\n"
                   "cd <path>       change current working directory to <path>\n"
                   "h               show usage help\n"
                   "e               exit this program\n"
                   "\n"
                   "Shell commands (ls, cat, cp, etc..) are also available.\n");
        }
        else if (strcmp(cmd, "rpwd") == 0)
        {
            set_password();
            setbuf(stdin, NULL);
        }
        else if (strcmp(cmd, "spth") == 0)
        {
            if (access(CONFIG_PATH, F_OK) != 0)
            {
                printf("Config file does not exits! Need to use \'rpth <path>\' to set a filebox path.\n");
            }
            else
            {
                FILE *file = fopen(CONFIG_PATH, "r");
                char filebox[PATH_MAX_LENGTH] = {0};
                fgets(filebox, PATH_MAX_LENGTH, file);
                printf("Filebox path is %s\n", filebox);
                fclose(file);
            }
        }
        else if (strncmp(cmd, "rpth", 6) == 0)
        {
            strcpy(tmp, cmd);
            int len = strlen(tmp);
            if (tmp[len - 1] != '/')
                tmp[len - 1] = '/'; //目录以/结尾
            char *p = strtok(tmp, " ");
            if (strcmp(p, "rpth") == 0)
            {
                p = strtok(NULL, " ");
                if (p != NULL)
                {
                    if (access(CONFIG_PATH, W_OK) == 0)
                    {
                        FILE *file = fopen(CONFIG_PATH, "w");
                        fputs(p, file);
                        printf("New filebox path is %s\n", p);
                        printf("Please reload the kernel module for the new path to take effect.\n");
                        fclose(file);
                    }
                    else
                    {
                        printf("Failed to modify config file. May try to run this program with \'sudo\'\n");
                    }
                }
            }
        }
        else if (strncmp(cmd, "cd", 2) == 0)
        {
            strcpy(tmp, cmd);
            char *p = strtok(tmp, " ");
            if (strcmp(p, "cd") == 0)
            {
                p = strtok(NULL, " ");
                if (p != NULL)
                {
                    chdir(p);
                    getcwd(path, PATH_MAX_LENGTH);
                }
            }
        }
        else
        {
            system(cmd);
        }
    }
    return 0;
}
