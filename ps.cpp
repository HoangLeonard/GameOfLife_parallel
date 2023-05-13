#include <iostream>
#include <omp.h>
#include <iostream>
#include <fstream>
#include <chrono>

int main() {

    int size = 100;
    while(size <= 1000000) {
        const int num_threads = 4;
        const int array_size = size;
        double start_time[num_threads];
        double end_time[num_threads];
        int array[array_size];

        // Initialize array with random values
        for (int i = 0; i < array_size; i++) {
            array[i] = 1;
        }

        // Parallel loop to compute sum of array elements
        #pragma omp parallel num_threads(num_threads)
        {
            int thread_num = omp_get_thread_num();
            int num_threads = omp_get_num_threads();
            int chunk_size = array_size / num_threads;
            int start_index = thread_num * chunk_size;
            int end_index = (thread_num == num_threads - 1) ? array_size : start_index + chunk_size;

            start_time[thread_num] = omp_get_wtime(); // Start timer

            int sum = 0;
            for (int i = start_index; i < end_index; i++) {
                sum += array[i];
            }

            end_time[thread_num] = omp_get_wtime(); // End timer

            // Print thread number and computation speed
            #pragma omp critical
            {
                std::cout << size << " Thread " << thread_num << ": "
                          << (double) (end_index - start_index) / (end_time[thread_num] - start_time[thread_num])
                          << " elements/s\n";
            }
        }

        std::ofstream outfile;
        outfile.open("ProcessingSpeed.txt");
        if (outfile.is_open()) {
            for (int i = 0; i < num_threads; i++) {
                outfile << size << " Thread " << i << ": "
                        << (double)array_size / ((end_time[i] - start_time[i]) * 1000000)
                        << " million elements/s\n";
            }
            outfile.close();
        } else {
            std::cerr << "Error: Unable to open file for writing\n";
        }

        size += 2000;
    }
    return 0;
}

