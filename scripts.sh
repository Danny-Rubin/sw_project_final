function run() {
  echo "running: \n"
  gcc spkmeans.c spkmeansmodule.c
  ./a.out ddg /Users/drubinov/Downloads/sw_project_final/inputs_hw1/smol.txt
}

run