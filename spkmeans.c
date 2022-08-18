// c interface file

#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "spkmeansmodule.h"

#define DEBUG 1
#define DEFAULT_MAX_ITER 300
#define EPSILON 0.00001
#define True 1
#define False 0



int validateProgramArgs(int argc);


int main(int argc, char* argv[]){
    // read argv
    // validate cmd args
    // call c entry point with args
    // capture return value
    // return 0 on success and 1 on error
    if(!validateProgramArgs(argc)){
        return 1;
    }

    return cEntryPoint(-1, argv[1], argv[2]);

}




int validateProgramArgs(int argc){
    if (argc != 3){
        print_invalid_input();
        return False;
    }
    return True;
}