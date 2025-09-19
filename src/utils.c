#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>

void log_debug(FILE *logfp, const char *str)
{
    if (!logfp || !str)
        return;

    fprintf(logfp, "Debug info: %s\n", str);
    fflush(logfp);
}

// trim spaces for char*
char *trim(char *str)
{
    char *end;

    // delete space at beginning
    while (isspace((unsigned char)*str))
        str++;

    if (*str == 0)
        return str;

    // delete space at last
    end = str + strlen(str) - 1;
    while (end > str && isspace((unsigned char)*end))
        end--;

    // add '\0' (for "strncpy" function)
    *(end + 1) = '\0';

    return str;
}

// todo: change to variable
#define MAX_COMMANDS 10

int splitStringBy(int argc, char *argv[], char *command[])
{
    // save log to debug.log
    FILE *logfp = fopen("debug.log", "w");
    char *logvar[1024];
    if (logfp == NULL)
    {
        perror("fopen failed");
        return 1;
    }
    sprintf(logvar, "Original string: [%s]\n", argv[0]);
    log_debug(logfp, logvar);
    if (argc < 2)
    {
        fprintf(stderr, "Usage: %s \"command string\"\n", argv[0]);
        return 1;
    }
    // test
    // char commandChar[] = "grep abc -> grep 123 -> awk '{print $1}'";
    char commandChar[1024];
    strncpy(commandChar, argv[0], sizeof(commandChar) - 1);
    commandChar[sizeof(commandChar) - 1] = '\0'; // 确保有结尾符

    sprintf(logvar, "Original string: [%s]\n", commandChar);
    log_debug(logfp, logvar);
    sprintf(logvar, "Original string: [%s]\n", argv[0]);
    log_debug(logfp, logvar);

    char *commands[MAX_COMMANDS];

    int count = 0;

    // split by "->"
    char *token = strtok(commandChar, argv[1]);

    while (token != NULL && count < MAX_COMMANDS)
    {
        token = trim(token);
        // solve malloc issue
        command[count] = malloc(strlen(token) + 1);
        strcpy(command[count], token);
        count++;
        token = strtok(NULL, "->");
    }

    for (int i = 0; i < count; i++)
    {
        // sprintf(logvar, "Command %d: [%s]\n", i, command[i]);
    }

    return count;
}

// test
//  void test_splitStringBy()
//  {
//      char *test[] = {"grep abc -> grep 123 -> awk '{print $1}'", "->"};
//      char *command[] = {"NULL"};

//     int n = splitStringBy(2, test, command);
//     printf("Total commands: %d\n", n);
//     printf("commands: %s\n", command[0]);
// }

// int main()
// {
//     test_splitStringBy();
//     return 0;
// }