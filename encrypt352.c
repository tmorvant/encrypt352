/*
 *  author: tmorvant@iastate.edu using template encrypt-module.h
 *  This file uses interface encrypt-module.h and implementation in encrypt-module.c
 *  to simultaneously iterate through an input file character by character,
 *  count number of occurences of each character, encrypt the character,
 *  count the occurences of the encrypted characters, and write the encrypted
 *  characters to an output file.
 */

#include "encrypt-module.h"
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>
#include <unistd.h>
#include <ctype.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <semaphore.h>

int *inbuffer;     //int array buffer to hold input
int *outbuffer;    //int array buffer to hold output

//counters for read/write and counters for buffers
int reader, incounter, encryptincounter, encryptoutcounter, outcounter, writer;

int in,out; //size of in/out buffers

//semaphores to signal when program is ready to read characters, count characters in input and output
//encrypt characters in the first buffer, place new characters in output buffer,
//and write them to output file
sem_t readsem;
sem_t countinsem;
sem_t encryptinsem;
sem_t encryptoutsem;
sem_t countoutsem;
sem_t writesem;

int resetting;

//not implemented
void reset_requested() {
    
    printf("Reset requested.\n");
    resetting = 1;
    printf("Total input count with current key is %d\n", get_input_total_count());
    for(int k = 'A'; k <= 'Z'; k++) {
        printf("%c:%d ", k, get_input_count(k));
    }
    printf("\n");
    printf("Total output count with current key is %d\n", get_output_total_count());
    for(int l = 'A'; l <= 'Z'; l++) {
        printf("%c:%d ", l, get_output_count(l));
    }
    printf("\n");
    
    return;

}

//not implemented
void reset_finished(){
    resetting = 0;
    printf("Reset finished\n\n");
}

//thread method to read each character in the input file as they buffer is ready to receive them,
//calls read_input from encrypt-module.h to iterate thorugh file and places each character in the inbuffer.
//signals that characters are ready to be counted
void *readFile(void *param) {

    // printf("readFile() says hello\n");

    int c;
    reader = 0;

    while ((c = read_input()) != EOF) {

        while(resetting == 1) {

        }
        sem_wait(&readsem);
        // printf("Reading\n");
        
        inbuffer[reader] = c;
        sem_post(&countinsem);
        // int value;
        // sem_getvalue(&countinsem, &value);
        // printf("%d\n", value);
        reader = (reader + 1) % in;

    }   
    inbuffer[reader] = EOF;
    sem_post(&countinsem);
    // printf("Reader done\n");
    
    pthread_exit(0);
}

//thread method to count each character in the inbuffer and add to total count 
//and character counts. after character is counted it signals it is ready to be encrypted
void *countInBuffer(void *param) {

    // printf("countInBuffer() says hello\n");

    incounter = 0;
    
    while (1) {

        while(resetting == 1) {

        }
        
        sem_wait(&countinsem);
        // printf("Counting in\n");
        if(inbuffer[incounter] == EOF) {
            // printf("count in done\n");
            sem_post(&encryptinsem);
            break;
        }
        
        count_input(inbuffer[incounter]);
        sem_post(&encryptinsem);
        incounter = (incounter + 1) % in;

    }
    
    pthread_exit(0);
}

//thread method that encrypts characters as they become available in the inbuffer
//writes encrypted character to outbuffer and signals that character is ready to be counter
//signals to reader that encrypted character can be overwritten through readFile()
//signals that encrypted character is ready to be counted
void *encrypt(void *param) {

    // printf("encrypt() says hello\n");

    encryptincounter = 0;
    encryptoutcounter = 0;
    while(1) {
        
        while(resetting == 1) {

        }

        sem_wait(&encryptinsem);
        // printf("Encrypting\n");
        if(inbuffer[encryptincounter] == EOF) {
            // printf("encrypt done\n");
            outbuffer[encryptoutcounter] = EOF;
            sem_post(&countoutsem); //potential error
            break;
        }
        sem_wait(&encryptoutsem);
        outbuffer[encryptoutcounter] = caesar_encrypt(inbuffer[encryptincounter]);
        sem_post(&readsem);
        sem_post(&countoutsem);
        // int value;
        // sem_getvalue(&countoutsem, &value);
        // printf("%d\n", value);
        encryptincounter = (encryptincounter + 1) % in;
        encryptoutcounter = (encryptoutcounter + 1) % out;

    }
    pthread_exit(0);
}

//method thread to count total and count each character in the outbuffer
//once counted, signals that the character is ready to be written to output file
void *countOutBuffer(void *param) {

    // printf("countOutBuffer() says hello\n");

    outcounter = 0;
    while(1) {

        while(resetting == 1) {

        }

        sem_wait(&countoutsem);
        // printf("Counting out\n");
        
        if(outbuffer[outcounter] == EOF) {
            // printf("count out done\n");
            sem_post(&writesem);
            break;
        }
        
        count_output(outbuffer[outcounter]);
        sem_post(&writesem);
        outcounter = (outcounter + 1) % out;

    }
    pthread_exit(0);
}

//method thread to write character to output file
//once character is written, signals encrypt thread that the output buffer is ready to 
//receive new characters
void *writeFile(void *param) {

    // printf("writeFile() says hello\n");
    writer = 0;
    while(1) {
        
        while(resetting == 1) {

        }

        sem_wait(&writesem);
        // printf("Writing\n");
        if(outbuffer[writer] == EOF) {
            // printf("writer done\n");
            break;
        }
        // printf("Writing\n");
        //sem_wait(&writesem);
        write_output(outbuffer[writer]);
        // printf("%c", outbuffer[writer]);
        sem_post(&encryptoutsem);
        writer = (writer + 1) % out;

    }
    pthread_exit(0);
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        printf("Incorrect number of arguments; enter an input and output file name after executable\n");
        exit(1);
    }
    
    init(argv[1], argv[2]);
    
    printf("Enter input buffer size:\n");
    scanf("%d", &in);
    if(in < 2) {
        printf("Input buffer size must be greater than one\n");
        exit(1);
    }
    printf("Enter output buffer size:\n");
    scanf("%d", &out);
    
    if(in < 2) {
        printf("Output buffer size must be greater than one\n");
        exit(1);
    }
    printf("\n");
    inbuffer = malloc(in * sizeof(int));
    outbuffer = malloc(out * sizeof(int));
    reader = 0;
    incounter = 0;
    encryptincounter = 0;
    encryptoutcounter = 0;
    outcounter = 0;
    writer = 0;

    resetting = 0;

    sem_init(&readsem, 0, in);
    sem_init(&countinsem, 0, 0);
    sem_init(&encryptinsem, 0, 0);
    sem_init(&encryptoutsem, 0, 1);
    sem_init(&countoutsem, 0, 0);
    sem_init(&writesem, 0, 0);

    

    pthread_t thread1, thread2, thread3, thread4, thread5;
    pthread_attr_t attr;

    pthread_attr_init(&attr);

    pthread_create(&thread1, &attr, &readFile, 0);
    pthread_create(&thread2, &attr, &countInBuffer, 0);
    pthread_create(&thread3, &attr, &encrypt, 0);
    pthread_create(&thread4, &attr, &countOutBuffer, 0);
    pthread_create(&thread5, &attr, &writeFile, 0);

    // while ((c = read_input()) != EOF) {
    //     count_input(c);
    //     c = caesar_encrypt(c);
    //     count_output(c); 
    //     write_output(c);
    // }
    

    pthread_join(thread1, NULL);
    pthread_join(thread2, NULL);
    pthread_join(thread3, NULL);
    pthread_join(thread4, NULL);
    pthread_join(thread5, NULL);

    sem_destroy(&readsem);
    sem_destroy(&countinsem);
    sem_destroy(&encryptinsem);
    sem_destroy(&encryptoutsem);
    sem_destroy(&countoutsem);
    sem_destroy(&writesem);

    
    printf("End of file reached\n");
    printf("Total input count with current key is %d\n", get_input_total_count());
    for(int i = 'A'; i <= 'Z'; i++) {
        printf("%c:%d ", i, get_input_count(i));
    }
    printf("\n");
    printf("Total output count with current key is %d\n", get_output_total_count());
    for(int j = 'A'; j <= 'Z'; j++) {
        printf("%c:%d ", j, get_output_count(j));
    }
    printf("\n\n");



    // printf("im out\n");
    
    free(inbuffer);
    free(outbuffer);
       
    return 0;
}