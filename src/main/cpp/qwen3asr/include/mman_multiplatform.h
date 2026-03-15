#ifndef MMAN_MULTIPLATFORM_H
#define MMAN_MULTIPLATFORM_H

#ifdef _WIN32

namespace qwen3_asr {
#include <windows.h>
#include <io.h>

#define PROT_READ  0x01
#define PROT_WRITE 0x02
#define MAP_PRIVATE 0x02
#define MAP_SHARED  0x01
#define MAP_FAILED ((void*)-1)

    // Windows 下 mmap 实现（真实内存映射，支持超过2GB大文件）
    static void* mmap(void* /*ptr*/, size_t size, int prot, int flags, int fd, size_t offset) {
        // 校验参数：仅支持读、私有映射，文件句柄有效
        if (fd < 0 || !(prot & PROT_READ) || !(flags & MAP_PRIVATE)) {
            SetLastError(ERROR_INVALID_PARAMETER);
            return MAP_FAILED;
        }

        // 校验大小参数
        if (size == 0) {
            SetLastError(ERROR_INVALID_PARAMETER);
            return MAP_FAILED;
        }

        // 将 C 运行时文件句柄转换为 Windows 内核句柄
        HANDLE win_fd = (HANDLE)_get_osfhandle(fd);
        if (win_fd == INVALID_HANDLE_VALUE) {
            SetLastError(ERROR_INVALID_HANDLE);
            return MAP_FAILED;
        }

        // 检查文件大小
        LARGE_INTEGER fileSize;
        if (!GetFileSizeEx(win_fd, &fileSize)) {
            DWORD error = GetLastError();
            SetLastError(error);
            return MAP_FAILED;
        }

        // 确保请求的映射范围在文件大小内
        if ((offset + size) > (size_t)fileSize.QuadPart) {
            SetLastError(ERROR_INVALID_PARAMETER);
            return MAP_FAILED;
        }

        // 计算文件大小的高32位和低32位
        DWORD sizeLow = (DWORD)(size & 0xFFFFFFFF);
        DWORD sizeHigh = (DWORD)((size >> 32) & 0xFFFFFFFF);

        // 创建文件映射对象
        HANDLE mapping = CreateFileMappingW(
            win_fd,
            nullptr,
            PAGE_READONLY,
            sizeHigh,
            sizeLow,
            nullptr
        );
        if (mapping == nullptr) {
            DWORD error = GetLastError();
            SetLastError(error);
            return MAP_FAILED;
        }

        // 计算偏移量的高32位和低32位
        DWORD offsetLow = (DWORD)(offset & 0xFFFFFFFF);
        DWORD offsetHigh = (DWORD)((offset >> 32) & 0xFFFFFFFF);

        // 映射文件视图
        void* mapped_ptr = MapViewOfFile(
            mapping,
            FILE_MAP_READ,
            offsetHigh,
            offsetLow,
            size
        );
        CloseHandle(mapping);

        if (mapped_ptr == nullptr) {
            DWORD error = GetLastError();
            SetLastError(error);
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

}  // namespace qwen3_asr
#else
    #include <sys/mman.h>
#endif  // _WIN32

#endif // MMAN_MULTIPLATFORM_H