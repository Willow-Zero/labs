#include <stdio.h>
#include <string.h>

int main(int argc, char** argv){
    char* firstArg;
    char* secondArg;

    if (argc != 3){
        printf("usage:  %s <arg1> <arg2>\n   Returns first three characters of each argument.\n", argv[0]);
        return 0;
    }
    firstArg = argv[1];
    secondArg = argv[2];

    if (strlen(firstArg)<3 || strlen(secondArg)<3){
        printf("Error: Both arguments must be 3 characters or longer.\n");
        return 0;
    }
    char firstFirstThree[4];
    char secondFirstThree[4];

    strncpy(firstFirstThree, firstArg, 3);
    strncpy(secondFirstThree, secondArg, 3);

    firstFirstThree[3] = '\0'; 
    secondFirstThree[3] = '\0';
    
    printf("First three chars: %s, %s\n", firstFirstThree, secondFirstThree);

    return 0;
}