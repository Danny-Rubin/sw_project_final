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


function pythonInterface() {
    echo "run all python interface: \n"
    echo "remove shitty old build... \n"
    rm -rf build && rm mykmeanssp.cpython*
    echo "building extenstion... \n"
    python3 setup.py build_ext --inplace
    echo "run spk python input 1: \n"
    python3 spkmeans.py 3 spk /Users/drubinov/Downloads/sw_project_final/Tests/input1.txt
    echo "run spk python input 2: \n"
    python3 spkmeans.py 3 spk /Users/drubinov/Downloads/sw_project_final/Tests/input2.txt
    echo "run spk python input 3: \n"
    python3 spkmeans.py 3 spk /Users/drubinov/Downloads/sw_project_final/Tests/input3.txt
    echo "run spk python input 4: \n"
    python3 spkmeans.py 3 spk /Users/drubinov/Downloads/sw_project_final/Tests/input4.txt
    echo "run spk python input 5: \n"
    python3 spkmeans.py 3 spk /Users/drubinov/Downloads/sw_project_final/Tests/input5.txt
    echo "run spk python input 6: \n"
    python3 spkmeans.py 3 spk /Users/drubinov/Downloads/sw_project_final/Tests/input6.txt
    echo "run spk python input 7: \n"
    python3 spkmeans.py 3 spk /Users/drubinov/Downloads/sw_project_final/Tests/input7.txt
    echo "#################################################"
    echo "run spk python input 2 for k = 0: \n"
    python3 spkmeans.py 0 spk /Users/drubinov/Downloads/sw_project_final/Tests/input2.txt
    echo "run spk python input 7 for k = 0: \n"
    python3 spkmeans.py 0 spk /Users/drubinov/Downloads/sw_project_final/Tests/input7.txt
    echo "run spk python NOA GABI for k = 3: \n"
    python3 spkmeans.py 3 spk /Users/drubinov/Downloads/sw_project_final/Tests/input7.txt



#    gcc -o c_executable ../spkmeans.c ../spkmeansmodule.c
#    ./c_executable jacobi ./test_jacobi_lecture_13_input.txt > tsibout.txt
#    diff -y tsibout.txt correct.txt
      # cat outpudpudpud.txt
}
echo "test run on: " > output.txt
date "+%Y-%m-%d %H:%M:%S" >> output.txt
pythonInterface >> output.txt