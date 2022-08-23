function run() {
  echo "running: \n"
  gcc spkmeans.c spkmeansmodule.c
  ./a.out jacobi /Users/drubinov/Downloads/sw_project_final/inputs_hw1/smol.txt #> sm.txt
  # cat outpudpudpud.txt
}

run