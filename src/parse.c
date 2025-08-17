#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>

#include "common.h"
#include "parse.h"

// void list_employees(struct dbheader_t *dbhdr, struct employee_t *employees) {

// }

int add_employee(struct dbheader_t *dbhdr, struct employee_t **employeesUgh, char *addstring) {   
    if(!employeesUgh || !dbhdr) {
        printf("Invaild argument, dbhdr or employees NULL");
        return STATUS_ERROR;
    }
    
    char *name = strtok(addstring, ",");
    char *addr = strtok(NULL, ",");
    char *hours = strtok(NULL, ",");

    dbhdr->count++;

    //struct employee_t* employees = realloc(*employeesUgh, dbhdr->count);
    // printf("Now %p / %p\n", employees, *employeesUgh);

    // strncpy(employees[dbhdr->count - 1].name, name, sizeof(employees[dbhdr->count].name));
    // strncpy(employees[dbhdr->count - 1].address, addr, sizeof(employees[dbhdr->count].address));
    // employees[dbhdr->count - 1].hours = atoi(hours);

    dbhdr->filesize += sizeof(struct employee_t);
    //*employeesUgh = employees;
    //printf("Now %p / %p\n", employees, *employeesUgh);

    return STATUS_SUCCESS;
}

int read_employees(int fd, struct dbheader_t *dbhdr, struct employee_t **employeesOut) {
    if(fd < 0) {
        printf("Got a bad FD from the user\n");
        return STATUS_ERROR;
    }

    int count = dbhdr->count;
    struct employee_t *employees = calloc(count, sizeof(struct employee_t));
    if(!employees) {
        printf("Malloc failed to employee array\n");
        return STATUS_ERROR;
    }

    read(fd, employees, count * sizeof(struct employee_t));
    for(int i = 0; i < count; i++) {
        employees[i].hours = ntohl(employees[i].hours);
    }

    *employeesOut = employees;
    return STATUS_SUCCESS;
}

int output_file(int fd, struct dbheader_t *dbhdr, struct employee_t *employees) {
    if(fd < 0) {
        printf("Got a bad FD from the user\n");
        return STATUS_ERROR;
    }

    if(!dbhdr) {
        printf("Invalid argument: dbhdr NULL");
        return STATUS_ERROR;
    }

    int employee_count = dbhdr->count;

    dbhdr->magic = htonl(dbhdr->magic);
    dbhdr->filesize = htonl(dbhdr->filesize);
    dbhdr->count = htons(dbhdr->count);
    dbhdr->version = htons(dbhdr->version);

    lseek(fd, 0, SEEK_SET);
    write(fd, dbhdr, sizeof(struct dbheader_t));

    if(!employees || employee_count == 0) {
        return STATUS_SUCCESS;
    }

    for(int i = 0; i < employee_count; i++) {
        employees[i].hours = htonl(employees[i].hours);
    }

    write(fd, employees, sizeof(struct employee_t) * employee_count);

    return STATUS_SUCCESS;
}	

int validate_db_header(int fd, struct dbheader_t **headerOut) {
    if(fd < 0) {
        printf("Got a bad FD from the user\n");
        return STATUS_ERROR;
    }

    struct dbheader_t* header = calloc(1, sizeof(struct dbheader_t));
    if(!header) {
        printf("Malloc failed to create db header\n");
        return STATUS_ERROR;
    }

    if(read(fd, header, sizeof(struct dbheader_t)) != sizeof(struct dbheader_t)) {
        perror("read");
        free(header);
        return STATUS_ERROR;
    }

    header->version = ntohs(header->version);
    header->count = ntohs(header->count);
    header->magic = ntohl(header->magic);
    header->filesize = ntohl(header->filesize);
    if(header->magic != HEADER_MAGIC) {
        printf("Improper header magic\n");
        free(header);
        return -1;
    }

    if(header->version != 1) {
        printf("Improper header version\n");
        free(header);
        return -1;
    }

    struct stat dbstat = {0};
    fstat(fd, &dbstat);
    if(header->filesize != dbstat.st_size) {
        printf("Corrupted database");
        free(header);
        return STATUS_ERROR;
    }

    *headerOut = header;
    return STATUS_SUCCESS;
}

int create_db_header(struct dbheader_t **headerOut) {
    if(!headerOut) {
        printf("Invalid NULL parameter headerOut");
        return STATUS_ERROR;
    }

    struct dbheader_t* header = calloc(1, sizeof(struct dbheader_t));
    if(!header) {
        printf("Malloc failed to create db header\n");
        return STATUS_ERROR;
    }

    header->magic = HEADER_MAGIC;
    header->version = 1;
    header->filesize = sizeof(struct dbheader_t);
    *headerOut = header;
    return STATUS_SUCCESS;
}


