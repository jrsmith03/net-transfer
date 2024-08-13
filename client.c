#include <stdio.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <stdlib.h>
#include <netinet/ip.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <sys/stat.h>
#include <errno.h>
#include <string.h>
#include <dirent.h>
#include <sys/mman.h>
#include <stdbool.h>
#include <fcntl.h>

#include "util.c"

void copy_file(int port, char* ip, char* file_name, char* original_name);
void traverse_directory(char* path_to_start, DIR* dir_open, int sockfd, char* ip);


int main(int argc, char** argv) {
    char file_name[32]; 
    printf("Enter file name or directory to copy...\n");
    scanf("%s", file_name);
    
    // Attempt opendir to see if the provided argument is a directory or just a file.
    DIR* root_dir = opendir(file_name);

    if (root_dir) {
        // We want to recursively open any subdirectories
        traverse_directory(file_name, root_dir, atoi(argv[2]), argv[1]);
    } else {
        copy_file(atoi(argv[2]), argv[1], file_name, NULL);
    }

    return 0;
}

void copy_file(int port, char* ip, char* file_name, char* original_name) {
     // Establish an IPV4 socket file descriptor using TCP
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    // First argument is the address for which to bind the socket to.
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = port;
    struct in_addr sin_addr;
    sin_addr.s_addr = inet_addr(ip); 
    addr.sin_addr = sin_addr; 

    if (connect(sockfd, &addr, sizeof(addr)) == -1) {
        printf("Connect failed! Are you running the server on port %d? Error: %s\n", port, strerror(errno));
        return -1;
    } else {

        struct stat fileinfo;
        int copyfd = open(file_name, 0);

        if (copyfd != -1) {
            stat(file_name, &fileinfo);
            // Give the server a file name
            if (original_name) {
                write(sockfd, original_name, 32);
            } else {
                write(sockfd, file_name, 32);
            }
            // Give the server the file size for transfer
            char buf_size[32];
            itoa(fileinfo.st_size, buf_size, digits(fileinfo.st_size));
            write(sockfd,buf_size, 32);

            // Read the file into memory
            char *file_mem;
            file_mem = mmap(NULL, fileinfo.st_size, PROT_READ, MAP_PRIVATE, copyfd, 0);
            if (file_mem == MAP_FAILED) {
                printf("fatal error- Unable to memory map cache for file; error %s\n", strerror(errno));
            }

            printf("File %s's size: %d bytes\n", file_name, fileinfo.st_size);


            // Write will send an arbitrary number of bytes. For larger files,
            // we might need to make multiple calls to get all the data across.
            size_t written = write(sockfd, file_mem, fileinfo.st_size);
            while (written != fileinfo.st_size) {
                printf("Transfer in progress.\n");
                int writnew = write(sockfd, file_mem + written, fileinfo.st_size - written);
                if (writnew == -1) {
                    printf("fatal error- write failed; error %s\n", strerror(errno));
                    return;
                }
                written += writnew;
            }
            
            printf("Transfer of %s complete.\n", file_name);
            munmap(file_mem, fileinfo.st_size);
            close(copyfd);
        } else {
            printf("The file %s does not exist.\n", file_name);
        }
        close(sockfd);
    }
}

// Modified from my media-sort project on GitHub
// Recursive method to open each file in a directory for transfer.
void traverse_directory(char* path_to_start, DIR* dir_open, int port, char* ip) {
    if (dir_open) {
        struct dirent *main_dir;
        while(main_dir = readdir(dir_open)) {
            DIR *child_dir;
            bool is_dir = false;
            bool not_implicit = (strcmp(main_dir->d_name, ".") != 0) && (strcmp(main_dir->d_name, "..") != 0);
            char *my_path = (char*) malloc(1000);
            strcpy(my_path, path_to_start);
            strcat(my_path, "/\0");

            strcat(my_path, main_dir->d_name);

            // Is the current file we are processing a directory entry? If so, recursively traverse that as well.
            child_dir = opendir(my_path);
   
            if (not_implicit && child_dir) {
                is_dir = true;
                traverse_directory(my_path, child_dir, port, ip);
                // closedir(my_path);
            }
            
            // Given that it is not a directory, copy the file to server.
            if (not_implicit) {
                printf("File to write: %s\n", main_dir->d_name);
                copy_file(port, ip, my_path, main_dir->d_name);           
            }
        } 
    }
}