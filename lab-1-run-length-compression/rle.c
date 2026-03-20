
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>
#include <sys/stat.h>

int main(int argc, char *argv[]){
    //step3: parse and validate arguments
    if (argc != 5){
        fprintf(stderr, "Usage: rle <input> <output> <k> <mode>\n");
        exit(-1);
    }

    char *input_file = argv[1];
    char *output_file = argv[2];

    // converting K from string to integer
    int K = atoi(argv[3]);
    if (K < 1) {
        fprintf(stderr, "Error: compression length K must be at least 1\n");
        exit(-1);
    }

    //Convert mode from string to integer
    int mode = atoi(argv[4]);
    if (mode != 0 && mode != 1){
        fprintf(stderr, "Error: mode must be 0 (compress) or 1 (decompress)\n");
        exit(-1);
    }

    //step4: open inputs and outputs
    int fd_in = open(input_file, O_RDONLY);
    if (fd_in == -1) {
        perror("Error opening input file");
        exit(-1);
    }
    //Open output file
    int fd_out = open(output_file, O_WRONLY | O_CREAT | O_TRUNC,
                      S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
    if (fd_out == -1) {
        perror("Error opening output file");
        close(fd_in);
        exit(-1);
    }

    if (mode == 0) {
        //step5: Compression logic (with k)
        unsigned char *pattern = malloc(K);
        unsigned char *next_pattern = malloc(K);
        if (!pattern || !next_pattern) {
            perror("malloc failed");
            close(fd_in);
            close(fd_out);
            exit(-1);
        }

        //read the first K bytes from input
        ssize_t n1 = read(fd_in, pattern, K);
        if (n1 == -1) {
            perror("read error");
            free(pattern); free(next_pattern);
            close(fd_in); close(fd_out);
            exit(-1);
        }

        // if file is empty, nothing to compress
        if (n1 == 0) {
            free(pattern); free(next_pattern);
            close(fd_in); close(fd_out);
            exit(0);
        }

        //If the file is shorter than K
        if (n1 < K) {
            unsigned char count = 1;
            if (write(fd_out, &count, 1) != 1) { perror("write"); exit(-1); }
            if (write(fd_out, pattern, n1) != n1) { perror("write"); exit(-1); }
            free(pattern); free(next_pattern);
            close(fd_in); close(fd_out);
            exit(0);
        }

        unsigned char count = 1;   // current run length

        while (1) {
            ssize_t n2 = read(fd_in, next_pattern, K);
            if (n2 == -1) {
                perror("read error");
                free(pattern); free(next_pattern);
                close(fd_in); close(fd_out);
                exit(-1);
            }

            // case1: End of file reached
            if (n2 == 0) {
                //write current run
                if (write(fd_out, &count, 1) != 1) { perror("write"); exit(-1); }
                if (write(fd_out, pattern, K) != K) { perror("write"); exit(-1); }
                break; //Done
            }

            //case2: partial final chunk
            if (n2 < K) {
                // First write the current full run
                if (write(fd_out, &count, 1) != 1) { perror("write"); exit(-1); }
                if (write(fd_out, pattern, K) != K) { perror("write"); exit(-1); }
                // Then write the partial as a new run with count 1
                unsigned char last_count = 1;
                if (write(fd_out, &last_count, 1) != 1) { perror("write"); exit(-1); }
                if (write(fd_out, next_pattern, n2) != n2) { perror("write"); exit(-1); }
                break;
            }

            // Case3: compare Full pattern with current pattern
            if (memcmp(pattern, next_pattern, K) == 0 && count < 255) {
                // Same pattern and we haven't hit the 255 limit
                count++;
            } else {
                // Different pattern or count reached 255: write the current run
                if (write(fd_out, &count, 1) != 1) { perror("write"); exit(-1); }
                if (write(fd_out, pattern, K) != K) { perror("write"); exit(-1); }
                // Start new run with next_pattern
                memcpy(pattern, next_pattern, K);
                count = 1;
            }
        }

        // free allocated memory
        free(pattern);
        free(next_pattern);

    } else {
        //step6: Decompression logic (with k)
        unsigned char count;
        unsigned char pattern[K];

        while (1) {
            // read count
            int r1 = read(fd_in, &count, 1);
            if (r1 == 0) break; // end of file
            if (r1 < 0) {
                perror("read count");
                exit(-1);
            }

            // read pattern
            int r2 = read(fd_in, pattern, K);
            if (r2 <= 0) {
                perror("read pattern");
                exit(-1);
            }

            // write pattern count times
            for (int i = 0; i < count; i++) {
                if (write(fd_out, pattern, r2) < 0) {
                    perror("write");
                    exit(-1);
                }
            }
            // if we read fewer than K bytes, that was the last chunk
            if (r2 < K) break;
        }
    }

    // close files and free memory
    close(fd_in);
    close(fd_out);
    return 0;
}






