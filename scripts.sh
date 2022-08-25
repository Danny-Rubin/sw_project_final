function run_wam() {
  echo "\nrunning: _wam\n"
  gcc spkmeans.c spkmeansmodule.c
  ./a.out wam /Users/drubinov/Downloads/sw_project_final/inputs_hw1/smol.txt #> sm.txt
  # cat outpudpudpud.txt
}

function run_ddg() {
  echo "\nrunning: ddg\n"
  gcc spkmeans.c spkmeansmodule.c
  ./a.out ddg /Users/drubinov/Downloads/sw_project_final/inputs_hw1/smol.txt #> sm.txt
  # cat outpudpudpud.txt
}

function run_lnorm() {
  echo "\nrunning: lnorm\n"
  gcc spkmeans.c spkmeansmodule.c
  ./a.out lnorm /Users/drubinov/Downloads/sw_project_final/inputs_hw1/smol.txt #> sm.txt
  # cat outpudpudpud.txt
}

function run_jacobi() {
  echo "\nrunning: jacobi\n"
  gcc spkmeans.c spkmeansmodule.c
  ./a.out jacobi /Users/drubinov/Downloads/sw_project_final/example_jacobi.txt #> sm.txt
  # cat outpudpudpud.txt
}


function run_all() {
  run_wam && run_ddg && run_lnorm && run_jacobi
}

run_all