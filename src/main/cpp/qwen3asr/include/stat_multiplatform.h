#ifndef STAT_MULTIPLATFORM_H
#define STAT_MULTIPLATFORM_H

namespace qwen3_asr {
#ifdef _WIN32
#define fstat64 _fstat64
#define stat64  _stat64
    typedef struct _stat64 stat64_t;
#else
    typedef struct stat64 stat64_t;
#endif // _WIN32
}

#endif // STAT_MULTIPLATFORM_H
