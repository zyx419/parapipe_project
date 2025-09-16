#include <stdio.h>
#include <string.h>
#include <ctype.h>

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
    printf("Original string: [%s]\n", argv[0]);
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
    printf("Original string: [%s]\n", commandChar);
    printf("Original string: [%s]\n", argv[0]);

    char *commands[MAX_COMMANDS];

    int count = 0;

    // split by "->"
    char *token = strtok(commandChar, "->");

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
        printf("Command %d: [%s]\n", i, command[i]);
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