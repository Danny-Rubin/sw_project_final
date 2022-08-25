function run_wam() {
  echo "\nrunning: _wam\n"
  gcc spkmeans.c spkmeansmodule.c
  leaks --atExit --list -- ./a.out wam /Users/drubinov/Downloads/sw_project_final/inputs_hw1/example_jacobi.txt | grep "^Process"
  # cat outpudpudpud.txt
}

function run_ddg() {
  echo "\nrunning: ddg\n"
  gcc spkmeans.c spkmeansmodule.c
  leaks --atExit --list -- ./a.out ddg /Users/drubinov/Downloads/sw_project_final/inputs_hw1/example_jacobi.txt | grep "^Process"
  # cat outpudpudpud.txt
}

function run_lnorm() {
  echo "\nrunning: lnorm\n"
  gcc spkmeans.c spkmeansmodule.c
  leaks --atExit --list -- ./a.out lnorm /Users/drubinov/Downloads/sw_project_final/inputs_hw1/example_jacobi.txt | grep ".+leaks for"
  # cat outpudpudpud.txt
}

function run_jacobi() {
  echo "\nrunning: jacobi\n"
  gcc spkmeans.c spkmeansmodule.c
  leaks --atExit --list -- ./a.out lnorm /Users/drubinov/Downloads/sw_project_final/inputs_hw1/example_jacobi.txt | grep "^Process"
  # cat outpudpudpud.txt
}


function run_all_memcheck() {
  #export MallocStackLogging=1
   run_jacobi && run_wam && run_ddg && run_lnorm
}

function tsiben() {
    echo "tsiben: \n"
    gcc spkmeans.c spkmeansmodule.c
    ./a.out jacobi /Users/drubinov/Downloads/sw_project_final/TsibenTests/test_jacobi_lecture_13_input.txt > tsibout.txt
    diff -y tsibout.txt correct.txt
      # cat outpudpudpud.txt
}

#run_all_memcheck
tsiben
#run_jacobi