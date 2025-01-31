#include <stdio.h>

int main(int argc, char** argv){

    if (argc != 2){
        printf("usage:  %s <arg1> \n   Returns number of ascii 'a' characters in provided string.\n", argv[0]);
        return 0;
    }   // provides neat error if argument count is wrong

    char* argument = argv[1];
    int inc = 0; //increment variable
    int aCount = 0; // counts a characters
    while (argument[inc] != '\0'){
        if (argument[inc] == 'a'){
            aCount++;
        }
        inc++;
    }
    printf("%d\n",aCount);

}