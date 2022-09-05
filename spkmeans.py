import numpy as np
import pandas as pd
import sys
from os import remove
from os.path import exists
import mykmeanssp
DEBUG = False

### utils ###
def print_error():
    print("An Error Has Occurred\n")


def print_invalid_input():
    print("Invalid Input!\n")


# @todo- check that this version works correctly !
def read_data(filename):
    try:
        df = pd.read_csv(filename, header=None)
        #print("shubidubi")
        #print(df)
        #print("shubidubi")
        return df.to_numpy()
    except:
        print_error()
        return False


def initialize_centroids(vectors: np.ndarray, k: int):
    try:
        N = vectors.shape[0]
        centroids_indices = np.zeros(k, dtype="int32")
        centroids_indices[0] = np.random.choice([i for i in range(N)])
        dists = np.zeros(N)
        probs = np.zeros(N)
        for i in range(1, k):
            for l in range(N):
                d = [np.square(np.linalg.norm(vectors[l] - vectors[centroids_indices[j]])) for j in range(i)]
                dists[l] = np.min(d)
            probs = dists / np.sum(dists)
            centroids_indices[i] = np.random.choice(N, p=probs)
        #print("centroids_indices:")
        #print(centroids_indices)
        return centroids_indices
    except:
        print_error()
        return False


def create_c_input(vectors: np.ndarray, centroids_indices: np.ndarray):
    """
    This function creates a list of vectors (initialized centorids first) and writes it to file
    :param vectors: np.ndarray
    :param centroids_indices:np.ndarray
    :return: None
    """
    N = vectors.shape[0]
    try:
        #print()
        #print(vectors)
        #print()
        sorted_vectors = [list(vectors[i]) for i in centroids_indices] + [list(vectors[i]) for i in range(N) if
                                                                          i not in centroids_indices]
        sorted_vectors = [[format(x, '.4f') for x in i] for i in sorted_vectors]
        if exists("c_input_file.txt"):
            remove("c_input_file.txt")
        #print("file written after pp:")
        with open("c_input_file.txt", 'w+') as c_input:
            for vec in sorted_vectors:
                c_input.write(",".join(vec) + "\n")
                #print(vec)



            #print("end file")

        return True
    except Exception as err:
        if DEBUG:
            print(err)
            print("problem in 'create c input'")
        print_error()
        return False


def print_results(centroids_indices):
    try:
        print(",".join([str(i) for i in centroids_indices]))
        with open("c_output_file.txt", 'r') as c_out:
            file_contents = c_out.read()
            print(file_contents)
        return True
    except:
        if (DEBUG):
            print("problem in 'print results'")
        print_error()
        return False


def finish_run():
    if exists("c_input_file.txt"):
        remove("c_input_file.txt")
    if exists("c_output_file.txt"):
        remove("c_output_file.txt")


def main():
    np.random.seed(0)
    args = get_program_args()
    if args is False:
            return 1
    [k, goal, file_name] = args
    if exists("c_output_file.txt"):
        remove("c_output_file.txt")
    result = mykmeanssp.pythonEntryPoint(k, goal, file_name, 1)  # todo: check module name is valid
    if (goal != "spk"):
        finish_run()
        return result
    # call the kmeans ++ mechanism
    # @todo : comment in!!!
    vectors = read_data("c_output_file.txt")  # get ndarray of input vectors, sorted by index
    #print("lollllllllllll")
    #with open('c_output_file.txt', 'r') as f:
        #print(f.read())

    #print("lollllllllllll")
    #print(vectors)
   # print()
    if vectors is False:
        return 1
    k = vectors.shape[1]

    centroids_indices = initialize_centroids(vectors, k)
    if centroids_indices is False:
        return 1
    if not create_c_input(vectors, centroids_indices):  # write to txt file list of vectors s.t first k vectors are init vectors
        return 1
    result = mykmeanssp.pythonEntryPoint(k, "spk", "c_input_file.txt", 2)
    if result == 0:
        print_results(centroids_indices)
    finish_run()
    return result


def get_program_args():
    try:
        assert len(sys.argv) == 4
        k = int(sys.argv[1])
        goal = sys.argv[2]
        file_name = sys.argv[3]
    except:
        print_invalid_input()
        return False
    return [k, goal, file_name]


if __name__ == '__main__':
    main()
