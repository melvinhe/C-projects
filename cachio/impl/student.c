#define _GNU_SOURCE
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>
#include <assert.h>
#include <stdarg.h>

#include "../io300.h"



/*
    student.c
    Fill in the following stencils
*/

/*
    When starting, you might want to change this for testing on small files.
*/
#ifndef CACHE_SIZE
#define CACHE_SIZE  4096

#endif

#if(CACHE_SIZE < 4)
#error "internal cache size should not be below 4."
#error "if you changed this during testing, that is fine."
#error "when handing in, make sure it is reset to the provided value"
#error "if this is not done, the autograder will not run"
#endif

/*
   This macro enables/disables the dbg() function. Use it to silence your
   debugging info.
   Use the dbg() function instead of printf debugging if you don't want to
   hunt down 30 printfs when you want to hand in
*/
#define DEBUG_PRINT 0
#define DEBUG_STATISTICS 1

struct io300_file {
    /* read,write,seek all take a file descriptor as a parameter */
    int fd;
    /* this will serve as our cache */
    char *cache;


    // TODO: Your properties go here
    //int file_size;
    off_t file_head;
    off_t cache_head;
    size_t active_cache_size;
    int is_dirty;


    /* Used for debugging, keep track of which io300_file is which */
    char *description;
    /* To tell if we are getting the performance we are expecting */
    struct io300_statistics {
        int read_calls;
        int write_calls;
        int seeks;
    } stats;
};

/*
    Assert the properties that you would like your file to have at all times.
    Call this function frequently (like at the beginning of each function) to
    catch logical errors early on in development.
*/
static void check_invariants(struct io300_file *f) {
    assert(f != NULL);
    assert(f->cache != NULL);
    assert(f->fd >= 0);

    // TODO: Add more invariants
    assert(f->file_head >= 0);
    assert(f->cache_head >= 0);
    //assert(f->cache_head <= f->file_head + (off_t) f->active_cache_size);
    assert(f->is_dirty == 0 || f->is_dirty == 1);
}

/*
    Wrapper around printf that provides information about the
    given file. You can silence this function with the DEBUG_PRINT macro.
*/
static void dbg(struct io300_file *f, char *fmt, ...) {
    (void)f; (void)fmt;
#if(DEBUG_PRINT == 1)
    static char buff[300];
    size_t const size = sizeof(buff);
    int n = snprintf(
        buff,
        size,
        // TODO: Add the fields you want to print when debugging
        "{desc:%s, } -- ",
        f->description
    );
    int const bytes_left = size - n;
    va_list args;
    va_start(args, fmt);
    vsnprintf(&buff[n], bytes_left, fmt, args);
    va_end(args);
    printf("%s", buff);
#endif
}



struct io300_file *io300_open(const char *const path, char *description) {
    if (path == NULL) {
        fprintf(stderr, "error: null file path\n");
        return NULL;
    }

    int const fd = open(path, O_RDWR | O_CREAT, S_IRUSR | S_IWUSR);
    if (fd == -1) {
        fprintf(stderr, "error: could not open file: `%s`: %s\n", path, strerror(errno));
        return NULL;
    }

    struct io300_file *const ret = malloc(sizeof(*ret));
    if (ret == NULL) {
        fprintf(stderr, "error: could not allocate io300_file\n");
        return NULL;
    }

    ret->fd = fd;
    ret->cache = malloc(CACHE_SIZE);
    if (ret->cache == NULL) {
        fprintf(stderr, "error: could not allocate file cache\n");
        close(ret->fd);
        free(ret);
        return NULL;
    }
    ret->description = description;
    // TODO: Initialize your file
    ret->file_head = 0;
    ret->cache_head = 0;
    ret->active_cache_size = 0;
    ret->is_dirty = 0;
    ret->stats.read_calls = 0;
    ret->stats.write_calls = 0;
    ret->stats.seeks = 0;

    check_invariants(ret);
    dbg(ret, "Just finished initializing file from path: %s\n", path);
    return ret;
}

int io300_seek(struct io300_file *const f, off_t const pos) {
    check_invariants(f);
    // TODO: Implement this
    f->stats.seeks++;
    off_t new_pos = lseek(f->fd, pos, SEEK_SET);
    
    if (new_pos >= 0) {
        f->file_head = new_pos;
        if (new_pos < f->cache_head || new_pos >= f->cache_head + (off_t) f->active_cache_size) {
            io300_flush(f);
            f->cache_head = f->file_head;
            f->active_cache_size = read(f->fd, f->cache, CACHE_SIZE);
        }
    }
    return new_pos;
}

int io300_close(struct io300_file *const f) {
    check_invariants(f);

#if(DEBUG_STATISTICS == 1)
    printf("stats: {desc: %s, read_calls: %d, write_calls: %d, seeks: %d}\n",
            f->description, f->stats.read_calls, f->stats.write_calls, f->stats.seeks);
#endif
    // TODO: Implement this
    io300_flush(f);
    close(f->fd);
    free(f->cache);
    free(f);
    return 0;
}

off_t io300_filesize(struct io300_file *const f) {
    check_invariants(f);
    struct stat s;
    int const r = fstat(f->fd, &s);
    if (r >= 0 && S_ISREG(s.st_mode)) {
        return s.st_size;
    } else {
        return -1;
    }
}

int io300_readc(struct io300_file *const f) {
    check_invariants(f);
    // TODO: Implement this
    f->stats.read_calls++;
    if (f->file_head < f->cache_head || f->file_head >= f->cache_head + (off_t) f->active_cache_size) {
        io300_flush(f);
        f->cache_head = f->file_head;
        f->active_cache_size = read(f->fd, f->cache, CACHE_SIZE);
    }
    if (f->file_head >= f->cache_head && f->file_head < f->cache_head + (off_t) f->active_cache_size) {
        unsigned char c;
        c = f->cache[f->file_head - f->cache_head];
        f->file_head++;
        return (int) c;
    } else {
        return -1;
    }
}

int io300_writec(struct io300_file *f, int ch) {
    check_invariants(f);
    // TODO: Implement this
    f->stats.write_calls++;

    if (f->file_head < f->cache_head || f->file_head >= f->cache_head + CACHE_SIZE) {
        io300_flush(f);
        f->cache_head = f->file_head;
        f->active_cache_size = read(f->fd, f->cache, CACHE_SIZE);
    }
    f->cache[f->file_head - f->cache_head] = ch;
    f->is_dirty = 1;
    f->file_head++;
    f->active_cache_size = (f->active_cache_size < CACHE_SIZE) ? f->active_cache_size + 1 : CACHE_SIZE;
    return ch;
}

ssize_t io300_read(struct io300_file *const f, char *const buff, size_t const sz) {
    check_invariants(f);
    f->stats.read_calls++;
    size_t read_count = 0;
    while (read_count < sz) {
        if (f->file_head < f->cache_head || f->file_head >= f->cache_head + (off_t) f->active_cache_size) {
            io300_flush(f);
            f->cache_head = f->file_head;
            f->active_cache_size = read(f->fd, f->cache, CACHE_SIZE);
        }
        size_t cache_ptr = f->file_head - f->cache_head;
        size_t to_read = (sz - read_count < f->active_cache_size - cache_ptr) ? sz - read_count : f->active_cache_size - cache_ptr;
        memcpy(buff + read_count, f->cache + cache_ptr, to_read);
        f->file_head += to_read;
        read_count += to_read;

        if (to_read == 0) {
            break;
        }
    }
    return read_count;
}

ssize_t io300_write(struct io300_file *const f, const char *buff, size_t const sz) {
    check_invariants(f);   
    f->stats.write_calls++;

    size_t write_count = 0;

    while (write_count < sz) {
        if (f->file_head < f->cache_head || f->file_head >= f->cache_head + CACHE_SIZE) {
            io300_flush(f);
            f->cache_head = f->file_head;
            f->active_cache_size = read(f->fd, f->cache, CACHE_SIZE);
        }

        size_t cache_ptr = f->file_head - f->cache_head;
        size_t to_write = (sz - write_count < CACHE_SIZE - cache_ptr) ? sz - write_count : CACHE_SIZE - cache_ptr;
        memcpy(f->cache + cache_ptr, buff + write_count, to_write);
        f->is_dirty = 1;
        f->active_cache_size = (f->active_cache_size < CACHE_SIZE) ? f->active_cache_size + to_write : CACHE_SIZE;
        f->file_head += to_write;
        write_count += to_write;
        if (to_write == 0) {
            break;
        }
    }
    return write_count;
}

int io300_flush(struct io300_file *const f) {
    check_invariants(f);
    // TODO: Implement this
    if (f->is_dirty) {
        lseek(f->fd, f->cache_head, SEEK_SET);
        write(f->fd, f->cache, f->active_cache_size);
        f->is_dirty = 0;
    }
    return 0;
}
    