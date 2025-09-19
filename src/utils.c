#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>

// todo: change to variable
#define MAX_COMMANDS 10

// log debug info to file
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

//
/*
    function: split string by separator, save to resultCommands[]
    argv[0]: original command string (e.g. "grep abc -> grep 123 -> awk '{print $1}'")
    argv[1]: separator string (e.g. "->")
    resultCommands[]: array to save split commands (e.g. "grep abc")
    return: command count
*/
// todo: move resultCommands to argv
int splitStringBy(int argc, char *argv[], char *resultCommands[])
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

    // oriCommandStr (e.g. "grep abc -> grep 123 -> awk '{print $1}'");
    char oriCommandStr[1024];
    // copy argv[0] to oriCommandStr
    strncpy(oriCommandStr, argv[0], sizeof(oriCommandStr) - 1);
    // ensure null-termination
    oriCommandStr[sizeof(oriCommandStr) - 1] = '\0';

    sprintf(logvar, "original command string: [%s]\n", oriCommandStr);
    log_debug(logfp, logvar);

    // define commands array to save split result(e.g."grep abc")
    char *commands[MAX_COMMANDS];

    // define count to save command count
    int count = 0;

    char *seperater = argv[1];
    // split by seperater
    char *token = strtok(oriCommandStr, seperater);
    while (token != NULL && count < MAX_COMMANDS)
    {
        // trim space
        token = trim(token);
        // solve malloc issue
        resultCommands[count] = malloc(strlen(token) + 1);
        strcpy(resultCommands[count], token);
        count++;
        // NULL means get next token
        token = strtok(NULL, seperater);

        sprintf(logvar, "Splited Command %d: [%s]\n", count, resultCommands[count]);
        log_debug(logfp, logvar);
    }

    // fclose(logfp);

    return count;
}