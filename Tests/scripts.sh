function run_wam() {
  echo "\nrunning: _wam\n"
  gcc -o c_executable ../spkmeans.c ../spkmeansmodule.c
  leaks --atExit --list -- ./c_executable wam ./example_jacobi.txt | grep "^Process"
  # cat outpudpudpud.txt
}

function run_ddg() {
  echo "\nrunning: ddg\n"
  gcc -o c_executable ../spkmeans.c ../spkmeansmodule.c
  leaks --atExit --list -- ./c_executable ddg ./example_jacobi.txt | grep "^Process"
  # cat outpudpudpud.txt
}

function run_lnorm() {
  echo "\nrunning: lnorm\n"
  gcc -o c_executable ../spkmeans.c ../spkmeansmodule.c
  leaks --atExit --list -- ./c_executable lnorm ./example_jacobi.txt | grep "^Process"
  # cat outpudpudpud.txt
}

function run_jacobi() {
  echo "\nrunning: jacobi\n"
  gcc -o c_executable ../spkmeans.c ../spkmeansmodule.c
  leaks --atExit --list -- ./c_executable lnorm ./example_jacobi.txt | grep "^Process"
  # cat outpudpudpud.txt
}


function run_all_memcheck() {
  #export MallocStackLogging=1
   run_jacobi && run_wam && run_ddg && run_lnorm
}

function tsiben() {
    echo "tsiben: \n"
    gcc -o c_executable ../spkmeans.c ../spkmeansmodule.c
    ./c_executable jacobi ./test_jacobi_lecture_13_input.txt > tsibout.txt
    diff -y tsibout.txt correct.txt
      # cat outpudpudpud.txt
}

run_all_memcheck
tsiben
#run_jacobi