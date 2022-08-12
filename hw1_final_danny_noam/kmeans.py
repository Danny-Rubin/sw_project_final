import sys

EPSILON = 0.001
DEFAULT_MAX_ITER = 100

class Vector:
    """
    This class represents a vector from the given input
    """
    def __init__(self, value):
        self.value = value      # value = list of floating point numbers
        self.len = len(value)   # len = integer that represents the size of the vecotr

    def l2_squared(self, other_vec):
        """
        This function calculates and returns the L2 norm squared
        :param other_vec: another vector to compute distance from
        :return: distance (int)
        """
        return sum([(self.value[i]- other_vec.value[i])**2 for i in range(self.len)])

    def add(self, other_vec):
        return Vector([self.value[i] + other_vec.value[i] for i in range(self.len)])

    def round_vector(self):
        return [format(i, '.4f') for i in self.value]



class Cluster:
    def __init__(self, centroid):
        self.size = 0
        self.sum = Vector([0] * centroid.len)   # sum of of vectors currently in the cluster
        self.centroid = centroid                    # element of Vector type


    def add_vec(self, other_vec):
        """
        This method takes another vector and adds it to the cluster
        :param other_vec: Vector object
        :return: None
        """
        self.size+=1
        self.sum = self.sum.add(other_vec)

    def calc_centroid(self):
        # return a centroid to be updated
        return Vector([x / self.size for x in self.sum.value])

    def is_converged(self, new_centroid):
        return (self.centroid.l2_squared(new_centroid) < EPSILON ** 2)

    def set_centroid(self, vector):
        self.centroid = vector

    def reset(self):
        self.size = 0
        self.sum = Vector([0] * self.centroid.len)

def print_error():
    print("An Error Has Occurred")


def print_invalid_input():
    print("Invalid Input!")

def get_program_args():
    try:
        assert 3 < len(sys.argv) < 6
        k = int(sys.argv[1])
        if len(sys.argv) == 5:
            max_iter = int(sys.argv[2])
            in_file = sys.argv[3]
            out_file = sys.argv[4]
        else:
            in_file = sys.argv[2]
            out_file = sys.argv[3]
    except:
        print_invalid_input()
        return False
    if len(sys.argv) == 4:
        return [k, in_file, out_file]
    return [k, max_iter, in_file, out_file]



def read_data(in_file: str):
    """
    this function gets an input file name and returns a list of the vectors contained in it
    :param in_file: string of the filename
    :return: list of Vector objects
    """
    try:
        with open(in_file, 'r') as data_file:
            lines = data_file.readlines()
            # create a list of vectors from data:
            vectors =[Vector([float(i) for i in l.split(",")]) for l in lines]
            return vectors
    except:
        print_error()
        return False



def write_result(result : list, out_file : str):
    """
    this function takes the result of the program and writes it into the given filename
    :param result: list of vectors values as strs, rounded to 4 decimal points
    :param out_file: str of output filename
    :return: True if writing succeeded, else False
    """
    try:
        with open(out_file, 'w') as write_file:
            for vec in result:
                write_file.write(",".join(vec) + "\n")
    except:
        print_error()


def get_closest_clust(vector, clusters):
    min_dist = float("inf")
    closest_clust = None
    for clust in clusters:
        dist = vector.l2_squared(clust.centroid)
        if dist < min_dist:
            min_dist = dist
            closest_clust = clust
    return closest_clust


def main():
    # change to actual parameter!
    args = get_program_args()
    if not args:
        return 1
    # defining varaibles from command line arguments
    k, in_file, out_file = args[0], args[-2], args[-1]
    max_iter = args[1] if len(args) == 4 else DEFAULT_MAX_ITER
    vectors = read_data(in_file)
    if not vectors:
        return 1
    N = len(vectors)
    if not ( 1 < k < N):
        print_invalid_input()
        return 1
    clusters = [Cluster(vectors[i]) for i in range(k)]
    converged = False # becomes True when all centroid converge
    for i in range(max_iter):
        if converged:
            break
        converged = True
        for vec in vectors:
            clust = get_closest_clust(vec, clusters)
            clust.add_vec(vec)
        for clust in clusters:
            new_centroid = clust.calc_centroid()
            if not clust.is_converged(new_centroid):
                converged = False
            clust.set_centroid(new_centroid)
            clust.reset()
    results = [clust.centroid.round_vector() for clust in clusters]
    write_result(results, out_file)












if __name__ == '__main__':
    main()




