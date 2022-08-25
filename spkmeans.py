from os import remove
from os.path import exists

import numpy as np


def main():
    np.random.seed(0)
    args = get_program_args()
    if args is False:
            return 1
    [k, goal, file_name] = args
    if exists("c_output_file.txt"):
        remove("c_output_file.txt")
    result = mykmeanssp.fit(str(k), goal, file_name)  # todo: check module name is valid
    if (goal == "spk"):
        # call the kmeans ++ mechanism
        pass









def get_program_args():
    try:
        assert 5 <= len(sys.argv) <= 6
        k = int(sys.argv[1])
        filename1 = sys.argv[-2]
        filename2 = sys.argv[-1]
        eps = float(sys.argv[-3])
        if len(sys.argv) == 6:
            max_iter = int(sys.argv[2])
        else:
            max_iter = DEFAULT_MAX_ITER
    except:
        print_invalid_input()
        return False
    return [k, max_iter, eps, filename1, filename2]