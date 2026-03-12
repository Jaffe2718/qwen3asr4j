#ifndef MMAN_MULTIPLATFORM_H
#define MMAN_MULTIPLATFORM_H

#ifdef _WIN32

#include <windows.h>
#include <io.h>

#define PROT_READ  0x01
#define PROT_WRITE 0x02
#define MAP_PRIVATE 0x02
#define MAP_SHARED  0x01
#define MAP_FAILED ((void*)-1)

// Windows 下 mmap 实现（真实内存映射）
static void* mmap(void* /*ptr*/, size_t size, int prot, int flags, int fd, size_t offset) {
    // 校验参数：仅支持读、私有映射，文件句柄有效，偏移量对齐
    if (fd < 0 || !(prot & PROT_READ) || !(flags & MAP_PRIVATE)) {
        SetLastError(ERROR_INVALID_PARAMETER);
        return MAP_FAILED;
    }

    // 将 C 运行时文件句柄转换为 Windows 内核句柄
    HANDLE win_fd = (HANDLE)_get_osfhandle(fd);
    if (win_fd == INVALID_HANDLE_VALUE) {
        SetLastError(ERROR_INVALID_HANDLE);
        return MAP_FAILED;
    }

    HANDLE mapping = CreateFileMappingW(
        win_fd,
        nullptr,
        PAGE_READONLY,
        0,
        size,
        nullptr
    );
    if (mapping == nullptr) {
        return MAP_FAILED;
    }
    void* mapped_ptr = MapViewOfFile(
        mapping,
        FILE_MAP_READ,
        0,
        (DWORD)offset,
        size
    );
    CloseHandle(mapping);

    if (mapped_ptr == nullptr) {
        SetLastError(GetLastError());
        return MAP_FAILED;
    }

    return mapped_ptr;
}

static int munmap(void* ptr, size_t size) {
    (void)size;

    if (ptr == MAP_FAILED || ptr == nullptr) {
        SetLastError(ERROR_INVALID_PARAMETER);
        return -1;
    }

    // 解除视图映射
    if (!UnmapViewOfFile(ptr)) {
        SetLastError(GetLastError());
        return -1;
    }

    return 0;
}

#else
    #include <sys/mman.h>
#endif

#endif // MMAN_MULTIPLATFORM_H