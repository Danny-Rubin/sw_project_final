int kmeans(Matrix vectors, int k) {
    int closest_clust_index = 0;
    int res = 0;
    int i = 0;
    int j = 0;
    int converged = False;


    double **vectors = NULL;
    double **centroids = NULL;
    double **clust_sums = NULL;
    int *clust_sizes = NULL;
    double *new_centroid = NULL;
    char ***centroids_strs = NULL;




    res = getProgramArgs(argc, argv, &k, &max_iter, &in_file, &out_file);
    if (!res){
        return 1;
    }

    if(readData(in_file, &vectors, &N, &d) == 0){
        if(vectors != NULL){
            free_memory_arr(vectors, N);
        }
        return 1;
    }
    if(k < 2 || k >= N){
        print_invalid_input();
        free_memory_arr(vectors, N);
        return 1;
    }


    centroids= (double**)calloc(k, (sizeof (double*)));
    if(centroids == NULL){
        free_memory_arr(vectors, N);
        print_error();
        return 1;
    }
    for (i = 0; i < k; i++){
        centroids[i] = (double*)calloc(d, (sizeof (double )));
        if(centroids[i] == NULL){
            free_memory_arr(vectors, N);
            free_memory_arr(centroids, i-1);
            print_error();
            return 1;
        }
    }
    for (i = 0; i < k; i++){
        for (j = 0; j < d; j++){
            centroids[i][j] = vectors[i][j];
        }
    }
    clust_sums= (double**)calloc(k,(sizeof (double*)));
    if(clust_sums == NULL){
        free_memory_arr(vectors, N);
        free_memory_arr(centroids, k);

        print_error();
        return 1;
    }

    for (i = 0; i < k; i++){
        clust_sums[i] = (double*)calloc(d, (sizeof (double )));
        if(clust_sums[i] == NULL){
            free_memory_arr(vectors, N);
            free_memory_arr(centroids, k);
            free_memory_arr(clust_sums, i-1);
            print_error();
            return 1;
        }
    }
    clust_sizes= (int*)calloc(k, (sizeof (int)));
    if(clust_sizes == NULL){
        free_memory_arr(vectors, N);
        free_memory_arr(centroids, k);
        free_memory_arr(clust_sums, k);
        print_error();
        return 1;
    }
    if (vectors == NULL || centroids == NULL || clust_sums == NULL || clust_sizes == NULL){
        print_error();
        return 1;
    }

    /* main loop: */
    for(i = 0; (i < max_iter) && !converged; i++){
        converged = True;
        for (j = 0; j < N; j++){
            closest_clust_index = closest_clust(vectors[j], centroids, k, d);
            assign_vec(vectors[j], clust_sums, clust_sizes, closest_clust_index, d);
        }

        for (j = 0; j < k; j++){
            new_centroid = calc_mean(clust_sums[j], clust_sizes[j], d);

            if(new_centroid == NULL){
                free_memory_arr(vectors, N);
                free_memory_arr(centroids, k);
                free_memory_arr(clust_sums, k);
                free(clust_sizes);
                return 1;
            }

            if(!is_converged(new_centroid, centroids[j], d, eps)){
                converged = False;
            }
            centroids[j] = new_centroid;
        }

        reset(clust_sums, clust_sizes, k, d);
    }

    centroids_strs = doubleVecsToStr(centroids, d,k);
    if(centroids_strs == NULL){
        free_memory_arr(vectors, N);
        free_memory_arr(centroids, k);
        free_memory_arr(clust_sums, k);
        free(clust_sizes);
        return 1;
    }

    writeData(out_file, centroids_strs, d, k);

    for (i = 0; i < k; i++){
        free(centroids[i]);
        free(clust_sums[i]);
    }
    free(centroids);
    free(clust_sums);
    free(clust_sizes);
    for (i = 0; i < N; i++){
        free(vectors[i]);
    }
    free(vectors);
    return 0;
}