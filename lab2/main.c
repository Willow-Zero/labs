#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <getopt.h>
#include <grp.h>
#include <pwd.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>
#include <dirent.h>
#include <unistd.h>

static int err_code;

/*
 * here are some function signatures and macros that may be helpful.
 */

void handle_error(char* fullname, char* action);
bool test_file(char* pathandname);
bool is_dir(char* pathandname);
const char* ftype_to_str(mode_t mode);
void list_file(char* pathandname, char* name, bool list_long);
void list_dir(char* dirname, bool list_long, bool list_all, bool recursive);

/*
 * You can use the NOT_YET_IMPLEMENTED macro to error out when you reach parts
 * of the code you have not yet finished implementing.
 */
#define NOT_YET_IMPLEMENTED(msg)                  \
    do {                                          \
        printf("Not yet implemented: " msg "\n"); \
        exit(255);                                \
    } while (0)

/*
 * PRINT_ERROR: This can be used to print the cause of an error returned by a
 * system call. It can help with debugging and reporting error causes to
 * the user. Example usage:
 *     if ( error_condition ) {
 *        PRINT_ERROR();
 *     }
 */
#define PRINT_ERROR(progname, what_happened, pathandname)               \
    do {                                                                \
        printf("%s: %s %s: %s\n", progname, what_happened, pathandname, \
               strerror(errno));                                        \
    } while (0)

/* PRINT_PERM_CHAR:
 *
 * This will be useful for -l permission printing.  It prints the given
 * 'ch' if the permission exists, or "-" otherwise.
 * Example usage:
 *     PRINT_PERM_CHAR(sb.st_mode, S_IRUSR, "r");
 */
#define PRINT_PERM_CHAR(mode, mask, ch) printf("%s", (mode & mask) ? ch : "-");

/*
 * Get username for uid. Return 1 on failure, 0 otherwise.
 */
static int uname_for_uid(uid_t uid, char* buf, size_t buflen) {
    struct passwd* p = getpwuid(uid);
    if (p == NULL) {
        return 1;
    }
    strncpy(buf, p->pw_name, buflen);
    return 0;
}

/*
 * Get group name for gid. Return 1 on failure, 0 otherwise.
 */
static int group_for_gid(gid_t gid, char* buf, size_t buflen) {
    struct group* g = getgrgid(gid);
    if (g == NULL) {
        return 1;
    }
    strncpy(buf, g->gr_name, buflen);
    return 0;
}

/*
 * Format the supplied `struct timespec` in `ts` (e.g., from `stat.st_mtime`) as a
 * string in `char *out`. Returns the length of the formatted string (see, `man
 * 3 strftime`).
 */
static size_t date_string(struct timespec * ts, char* out, size_t len) {
    struct timespec now;
    timespec_get(&now, TIME_UTC);
    time_t * seconds =  &ts->tv_sec;
    struct tm * t = localtime(seconds);
    if (now.tv_sec < ts->tv_sec) {
        // Future time, treat with care.
        return strftime(out, len, "%b %e %Y", t);
    } else {
        time_t difference = now.tv_sec - ts->tv_sec;
        if (difference < 31556952ull) {
            return strftime(out, len, "%b %e %H:%M", t);
        } else {
            return strftime(out, len, "%b %e %Y", t);
        }
    }
}

/*
 * Print help message and exit.
 */
static void help() {
    /* TODO: add to this */
    printf("ls: List files\n");
    printf("\t--help\t: Print this help\n");
    printf("\t-a\t: list all files, including hidden ones\n");
    printf("\t-n\t: list number of files (UNIMPLEMENTED)\n");
    printf("\t-R\t: recursively list all subdirs (UNIMPLEMENTED)\n");
    printf("\t-a\t: list all files, including hidden ones (UNIMPLEMENTED)\n");
    exit(0);
}

/*
 * call this when there's been an error.
 * The function should:
 * - print a suitable error message (this is already implemented)
 * - set appropriate bits in err_code
 */
void handle_error(char* what_happened, char* fullname) {
    PRINT_ERROR("ls", what_happened, fullname);

    // TODO: your code here: inspect errno and set err_code accordingly.
    return;
}

/*
 * test_file():
 * test whether stat() returns successfully and if not, handle error.
 * Use this to test for whether a file or dir exists
 */
bool test_file(char* pathandname) {
    struct stat sb;
    if (stat(pathandname, &sb)) {
        handle_error("cannot access", pathandname);
        return false;
    }
    return true;
}

/*
 * is_dir(): tests whether the argument refers to a directory.
 * precondition: test_file() returns true. that is, call this function
 * only if test_file(pathandname) returned true.
 */
bool is_dir(char* pathandname) {
    /* TODO: fillin */
    DIR * test = NULL;
    if ((test = opendir(pathandname)) != NULL){ //tests for dir by trying to open it. on null, the path doesnt describe a directory.
        closedir(test);
        return true;
    }
    return false;
}

/* convert the mode field in a struct stat to a file type, for -l printing */
const char* ftype_to_str(mode_t mode) {
    if (S_ISREG(mode)) {
        return "-";
    }
    else if (S_ISDIR(mode)) {
        return "d";
    }
    return "?";
}
const char* modeToPerms(mode_t mode,char*permString){
    char stringified[7];
    sprintf(stringified,"%o",mode);
    int i = 2;          //starts loop at 3rd element, in the case of test, the dir i made to test this, thats the 7 in 40755
    if (stringified[0] == '1'){
       i++;        //if the oct has overflowed, adds one to start at the right int 
    }
    int j = 0;
    while (j<3){
        int val = (stringified[i]-'0');
        switch (val){
            case(0):
                strcat(permString,"---");
                break;
            case(1):
                strcat(permString,"--x");
                break;
            case(2):
                strcat(permString,"-w-");
                break;
            case(3):
                strcat(permString,"-wx");
                break;
            case(4):
                strcat(permString,"r--");
                break;
            case(5):
                strcat(permString,"r-x");
                break;
            case(6):
                strcat(permString,"rw-");
                break;
            case(7):
                strcat(permString,"rwx");
                break;
        }
        j++;i++;
    }
    return permString;

   //        S_IRWXO     00007   others  (not  in  group) have read, write, and
    //                           execute permission
   //        S_IROTH     00004   others have read permission
   //        S_IWOTH     00002   others have write permission
   //        S_IXOTH     00001   others have execute permission
}
/* list_file():
 * implement the logic for listing a single file.
 * This function takes:
 *   - pathandname: the directory name plus the file name.
 *   - name: just the name "component".
 *   - list_long: a flag indicated whether the printout should be in
 *   long mode.
 *
 *   The reason for this signature is convenience: some of the file-outputting
 *   logic requires the full pathandname (specifically, testing for a directory
 *   so you can print a '/' and outputting in long mode), and some of it
 *   requires only the 'name' part. So we pass in both. An alternative
 *   implementation would pass in pathandname and parse out 'name'.
 */
void list_file(char* pathandname, char* name, bool list_long) {
    /* TODO: fill in*/
    if (!list_long){ //if not in long mode
        printf((name)); //starts by printing filename
        if (is_dir(pathandname)){//then lists / if its a directory
            printf("/");
        }
        printf("\n");   //then gives a /n to separate
    }
    else{ //if in long mode
        if (test_file(pathandname)==-1){
            exit(err_code);
        }
        struct stat fileStat;
        stat(pathandname, &fileStat);
        //DONE: derive links number
        //DONE: derive owner and group
        //DONE: derive size ", statbuf.st_nlink
        //TODO: derive date
        //TODO: derive perm string
        int linksNum = fileStat.st_nlink;
        char groupName[32];
        char userName[32];
        group_for_gid(fileStat.st_gid, groupName, 32);
        uname_for_uid(fileStat.st_uid, userName, 32);
        char * fileType = ftype_to_str(fileStat.st_mode);
        char permString[32];
        modeToPerms(fileStat.st_mode,permString);
        printf("%c%s %d %10s %10s %10ld %10s %s\n",*fileType,permString,linksNum,userName,groupName,fileStat.st_size,"DATETIME",name);
    }
}

/* list_dir():
 * implement the logic for listing a directory.
 * This function takes:
 *    - dirname: the name of the directory
 *    - list_long: should the directory be listed in long mode?
 *    - list_all: are we in "-a" mode?
 *    - recursive: are we supposed to list sub-directories?
 */
void list_dir(char* dirname, bool list_long, bool list_all, bool recursive) {
    /* TODO: fill in
     *   You'll probably want to make use of:
     *       opendir()
     *       readdir()
     *       list_file()
     *       snprintf() [to make the 'pathandname' argument to
     *          list_file(). that requires concatenating 'dirname' and
     *          the 'd_name' portion of the dirents]
     *       closedir()
     *   See the lab description for further hints
     */
     if (!test_file(dirname)){
        printf("ERROR: Not a file");
     }
    if (!is_dir(dirname)){
        list_file (dirname,dirname,list_long);
        return;
    }
    DIR * targetDir = opendir(dirname); //opens direcroty as targetdir
    struct dirent * nextItem = readdir(targetDir); // creats a struct for the next item
    while (nextItem != NULL){
        char* name = nextItem->d_name; //assigns name to the proper struct element
        char pathandname[64] = "";     //new string for the pathandname, could be more efficient but im doing this quick and dirty and i can make it pretty later
        strcat(pathandname,dirname);    //appends dirname and name to the path
        strcat(pathandname,"/");
        strcat(pathandname,name);
        if (list_all){
            list_file(pathandname,name,list_long);  //lists file
        }
        else if (name[0] != '.'){
            list_file(pathandname,name,list_long); 
        }
        nextItem = readdir(targetDir); //advances the directory pointer
    }  
    closedir(targetDir); //closes the directory because its best to be responsible with memory usage.

}

int main(int argc, char* argv[]) {
    // This needs to be int since C does not specify whether char is signed or
    // unsigned.
    int opt;
    err_code = 0;
    bool list_long = false, list_all = false, recursive = false,number = false;
    // We make use of getopt_long for argument parsing, and this
    // (single-element) array is used as input to that function. The `struct
    // option` helps us parse arguments of the form `--FOO`. Refer to `man 3
    // getopt_long` for more information.
    struct option opts[] = {
        {.name = "help", .has_arg = 0, .flag = NULL, .val = '\a'}};

    // This loop is used for argument parsing. Refer to `man 3 getopt_long` to
    // better understand what is going on here.
    while ((opt = getopt_long(argc, argv, "1lnRa", opts, NULL)) != -1) {
        switch (opt) {
            case '\a':
                // Handle the case that the user passed in `--help`. (In the
                // long argument array above, we used '\a' to indicate this
                // case.)
                help();
                break;
            case '1':
                // Safe to ignore since this is default behavior for our version
                // of ls.
                break;
            case 'a':
                list_all = true;
                break;
                // TODO: you will need to add items here to handle the
                // cases that the user enters "-l" or "-R"
            case 'R':
                recursive = true;
                NOT_YET_IMPLEMENTED("RECURSIVE");
            case 'n':
                number = true;
                NOT_YET_IMPLEMENTED("NUMBER MODE");
            case 'l':
                list_long = true;
            default:
                printf("Unimplemented flag %d\n", opt);
                break;
        }
    }
    // TODO: Replace this.

    for (int i = optind; i < argc; i++) {
        list_dir(argv[i],list_long,list_all,recursive);
    }
    if (optind == argc) {
        char cwd[128] = "";
        getcwd(cwd,128);
        list_dir(cwd,list_long,list_all,recursive);
    }
    //NOT_YET_IMPLEMENTED("file ordering");
    
    //DONE: test if is directory
    //TODO: if file, but not direcroty, just return filename
    //TODO: if directory, retrieve directory pointer
    //TODO: iter through directory and list files
    //TODO: implement flags
 
    //exit(err_code);
}
