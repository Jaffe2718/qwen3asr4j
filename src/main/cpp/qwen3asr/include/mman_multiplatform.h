#ifndef MMAN_MULTIPLATFORM_H
#define MMAN_MULTIPLATFORM_H

#ifdef _WIN32

#include <windows.h>
#include <io.h>

#define PROT_READ  0x1
#define MAP_PRIVATE 0x2
#define MAP_FAILED ((void*)-1)

static void* mmap(void* /*ptr*/, size_t size, int prot, int /*flags*/, int fd, size_t /*offset*/) {
    if (fd < 0 || (prot & PROT_READ) != PROT_READ) {
        SetLastError(ERROR_INVALID_PARAMETER);
        return MAP_FAILED;
    }
    void* buffer = _aligned_malloc(size, 32);
    if (!buffer) {
        SetLastError(ERROR_NOT_ENOUGH_MEMORY);
        return MAP_FAILED;
    }

    // read from fd to buffer
    size_t total_read = 0;
    char* buf_ptr = (char*)buffer;
    const size_t BUF_CHUNK = 4096;
    while (total_read < size) {
        size_t read_size = BUF_CHUNK < size - total_read ? BUF_CHUNK : size - total_read;
        size_t bytes_read = _read(fd, buf_ptr + total_read, read_size);
        total_read += bytes_read;

        if (bytes_read < 0) {
            free(buffer);
            SetLastError(ERROR_READ_FAULT);
            return MAP_FAILED;
        }
        if (bytes_read == 0) {
            free(buffer);
            SetLastError(ERROR_HANDLE_EOF);
            return MAP_FAILED;
        }
    }
    return buffer;
}

static int munmap(void* ptr, size_t /*size*/) {
    if (ptr == MAP_FAILED || ptr == nullptr) {
        return -1;
    }
    _aligned_free(ptr);
    return 0;
}

#define MAP_FAILED ((void*)-1)

#else
    #include <sys/mman.h>
#endif

#endif // MMAN_MULTIPLATFORM_H