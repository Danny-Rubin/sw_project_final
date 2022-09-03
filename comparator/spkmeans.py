import math
import sys
import numpy as np
import pandas as pd
from mykmeanssp import *
from enum import Enum

np.random.seed(0)


class Goal(Enum):
    # enum for the wanted goal of the program with the specified options at hand
    SPK = "spk"
    WAM = "wam"
    DDG = "ddg"
    LNORM = "lnorm"
    JACOBI = "jacobi"


def sub(point1, point2):
    # this method subtracts vector point2 from vector point1 and returns the result (point1 - point2)
    subb = []
    # subb starts off as an empty array that's about to be filled with the values of the result
    for i in range(len(point1)):
        subb.append(point1[i] - point2[i])
        # each value i in subb is the subtraction result of the respective values of point1 and point2
    return subb


def euc_norm(x):
    # this method returns the euclidian norm of vector x, squared
    arg_sum = 0
    for arg in x:
        arg_sum += arg ** 2
    # calculation of the squared norm as the sum of all squared values of x
    return arg_sum


def initialize_centroids(data, k, n):
    # this method is only put to use in case the goal is spk
    # it initializes k centroids according to the input k from the data given
    cent_index = []
    # an empty array is initialized to hold the indices of all k centroids
    datapoints = data.values.tolist()
    m_index = np.random.choice(data.index)
    # the index of the first centroid is chosen randomly from the datapoints
    # this index is added right away to a list of indices
    cent_index.append(m_index)
    for i in range(1, k):
        distance = [0] * n
        probability = [0] * n
        all_dist = 0
        # initializes the distance values and probability for each datapoint, and the sum of all
        for l in range(n):
            if (i > 0):
                distance[l] = min(euc_norm(sub(datapoints[l], datapoints[cent_index[j]])) for j in range(i))
                # the distance is calculated according to the specified in ex2
                all_dist += distance[l]
                # the sum of all distances is calculated in order to determine the probability to choose each datapoint
        for l in range(n):
            probability[l] = distance[l] / all_dist
            # for each datapoint, the probability to choose it according to its relative distance from
            # existing centroids is calculated, in order to choose the best next centroid possible
        m_index = np.random.choice(data.index, p=probability)
        # the next index is chosen randomly, but according to the probability function calculated above
        cent_index.append(m_index)
        # the index of the newly added centroid is added to the list of centroid indices
    return cent_index
    # after filling cent_index with k indices of k centroids
    # all centroids have been chosen and the list of their indices is ready to be used


def kmp(k, goal, max_iter, epsilon, file):
    data = pd.read_csv(file, header=None)
    datapoints = data.values.tolist()
    # each row is a datapoint
    n = data.shape[0]
    d = data.shape[1]
    # the number of rows and columns, which is the number of datapoints and the size of each, respectively

    # the program proceeds to call methods from the C interface according to the goal chosen
    # the options are specified and taken care of according to the optional goals at hand

    # i
    # the Normalized Spectral Clustering (spk) option
    if goal == Goal.SPK:
        if k < 0 or k >= n:
            # there can't be less than 0 clusters or more than n clusters
            # so if k's value isn't within range, it's invalid
            invalid()
        try:
            T_mat = np.array(fit_spk(datapoints, n, d, k))
            # T is calculated using the C interface
        except:
            error()
            # in case something went wrong
        k = T_mat.shape[1]
        d = k
        # now both k and d are the number of columns in T
        # since each row is a point, the number of columns is the size of each point
        data = pd.DataFrame(T_mat)
        datapoints = data.values.tolist()
        # arrangement of the datapoints from matrix T
        cent_index = initialize_centroids(data, k, n)
        # print("centroids_indices:")
        # print(cent_index)
        # cent_index is now a list of indices from data such that for each index i in cent_index
        # the ith point in data is one of the initialized centroids
        first_centroids = [list(data.iloc[cent_index[i]]) for i in range(k)]
        # print("first_centroids:")
        # print(first_centroids)
        # first_centroids uses the indices of cent_index to make a list of the initialized centroids
        try:
            last_centroids = np.array(fit_kms(datapoints, max_iter, epsilon, n, d, k, first_centroids))
            # the final centroids are calculated using the C interface, using the initialized ones
            # and the algorithm specified in ex2
        except:
            error()
            # in case something went wrong
        print_centroids(cent_index, last_centroids)
        # the final centroids are printed as specified in the chosen goal


    # ii
    # the Weighted Adjacency Matrix (wam) option
    elif goal == Goal.WAM:
        try:
            W_mat = np.array(fit_wam(datapoints, n, d))
            # W is calculated using the C interface
        except:
            error()
            # in case something went wrong
        print_mat(W_mat)
        # the wam is printed as specified in the chosen goal


    # iii
    # the Diagonal Degree Matrix (ddg) option
    elif goal == Goal.DDG:
        try:
            D_mat = np.array(fit_ddg(datapoints, n, d))
            # D is calculated using the C interface
        except:
            error()
            # in case something went wrong
        print_mat(D_mat)
        # the ddg is printed as specified in the chosen goal


    # iv
    # the Normalized Graph Laplacian (lnorm) option
    elif goal == Goal.LNORM:
        try:
            L_mat = np.array(fit_lnorm(datapoints, n, d))
            # L is calculated using the C interface
        except:
            error()
            # in case something went wrong
        print_mat(L_mat)
        # the lnorm is printed as specified in the chosen goal


    # v
    # the Jacobi algorithm option
    elif goal == Goal.JACOBI:
        if n != d:
            invalid()
            # this goal is only optional if the n == d, because the input matrix must be symmetric
            # so the number of rows and columns must be the same
        try:
            J_mat = fit_jacobi(datapoints, n, d)
            # J is calculated using the C interface
        except:
            error()
            # in case something went wrong
        print_values_as_line(J_mat[0])
        print_mat(J_mat[1])
        # the eigenvalues and eigenvectors are printed as specified in the chosen goal


def print_mat(mat):
    # this method prints the matrix mat
    # it prints all values of mat, line by line
    for line in mat:
        for i in range(len(line)):
            if i < len(line) - 1:
                # if this isn't the last value in the line, it needs a comma to separate it from the next value
                # so the printed format of this value should be 4 digits after the decimal point
                # and a comma afterwards
                printed_value = "%.4f," % (line[i])
            else:
                # in this case, this value is the last one in the line
                # it doesn't need a comma to separate it from the next value
                # because the next value is not in the same line
                # so there's a new line after it, instead
                # the printed format of this value should be 4 digits after the decimal points
                # and a new line afterwards
                printed_value = "%.4f\n" % (line[i])
            print(printed_value, end="")
            # the value is printed, and the loop moves on to the next one until it prints the whole matrix


def print_values_as_line(mat):
    # this method prints the 1-dim matrix mat as a line
    for i in range(len(mat)):
        if i < len(mat) - 1:
            # if this isn't the last value in the line, it needs a comma to separate it from the next value
            # so the printed format of this value should be 4 digits after the decimal point
            # and a comma afterwards
            printed_value = "%.4f," % (mat[i][0])
        else:
            # in this case, this value is the last one in the line
            # it doesn't need a comma to separate it from the next value
            # because the next value is not in the same line
            # so there's a new line after it, instead
            # the printed format of this value should be 4 digits after the decimal points
            # and a new line afterwards
            printed_value = "%.4f\n" % (mat[i][0])
        print(printed_value, end="")
        # the value is printed, and the loop moves on to the next one until it prints the whole matrix


def print_centroids(cent_index, last_centroids):
    # this method indices of the final centroids, and the final centroids as well
    indices = ",".join([str(index) for index in cent_index])
    print(indices)
    print_mat(last_centroids)


def invalid():
    # this method is called upon only when invalid input is detected
    # it notifies the user and closes the program
    print("Invalid Input!")
    sys.exit()


def error():
    # this method is called upon only when an error occurs
    # it notifies the user and closes the program
    print("An Error Has Occurred")
    sys.exit()


def main():
    if len(sys.argv) != 4:
        # if true, there's either a missing argument or an extra one
        # thus the input is invalid and the program will end its run
        invalid()
    # if we got this far there is a valid number of arguments in the input
    k = sys.argv[1]
    # k is the argument that's located first
    goal_name = sys.argv[2]
    # the goal is the argument that's located second
    max_iter = 300
    epsilon = 0
    # both max_iter and epsilon are as specified in the instructions
    infile = sys.argv[3]
    # the name of the file is the argument that's located third
    if not k.isnumeric():
        # k is the number of clusters required, so it must be numeric
        # if true k isn't numeric thus the input is invalid
        invalid()
    # if we got this far k is numeric so can be cast into an integer
    k = int(k)
    try:
        # this makes sure that the goal name from the input is a valid option
        # if goal_name is indeed a valid goal, it's translated to its respective enum
        goal = Goal(goal_name)
    except:
        # in that case, the input is invalid
        invalid()
    kmp(k, goal, max_iter, epsilon, infile)


if __name__ == "__main__":
    main()
