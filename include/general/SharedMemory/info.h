#ifndef ELTECAR_DATASERVER_INCLUDE_GENERAL_SHAREDMEMORY_INFO_H
#define ELTECAR_DATASERVER_INCLUDE_GENERAL_SHAREDMEMORY_INFO_H

#if defined(WIN32) || defined(_WIN32) \
    || defined(__WIN32) && !defined(__CYGWIN__)
#include <windows.h>
#else
#include <semaphore.h>
#endif

namespace SharedMemory {

#if defined(WIN32) || defined(_WIN32) \
    || defined(__WIN32) && !defined(__CYGWIN__)
struct Info {
    int bufferNumber = 0;
    int infoBufferSize;
    int bufferSize;
    int dataSize;
    int bufferNamesCount;
    int remap;
    Info(int newbufferNumber, int newbufferSize, int newbufferNamesCount)
        : bufferNumber(newbufferNumber),
          bufferSize(newbufferSize),
          bufferNamesCount(newbufferNamesCount) {}

    Info(int newbufferSize) { bufferSize = newbufferSize; }
    Info()
        : bufferNumber(0),
          infoBufferSize(-1),
          bufferSize(-1),
          bufferNamesCount(-1) {}
};
struct ThreadedInfo {
    int numberOfWriters;
    int infoSize;
};
#else
struct Info {
    int bufferNumber = 0;
    int infoBufferSize;
    int bufferSize;
    int dataSize;
    int bufferNamesCount;
    sem_t semaphore;
    Info(int newbufferNumber, int newbufferSize, int newbufferNamesCount)
        : bufferNumber(newbufferNumber),
          bufferSize(newbufferSize),
          bufferNamesCount(newbufferNamesCount) {}

    Info(int newbufferSize) { bufferSize = newbufferSize; }
    Info()
        : bufferNumber(0),
          infoBufferSize(-1),
          bufferSize(-1),
          bufferNamesCount(-1) {}
};

struct ThreadedInfo {
    int numberOfWriters;
    int infoSize;
    sem_t semaphore;
};

#endif
}// namespace SharedMemory

#endif// ELTECAR_DATASERVER_INCLUDE_GENERAL_SHAREDMEMORY_INFO_H
