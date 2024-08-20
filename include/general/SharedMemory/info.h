#ifndef ELTECAR_DATASERVER_INCLUDE_GENERAL_SHAREDMEMORY_INFO_H
#define ELTECAR_DATASERVER_INCLUDE_GENERAL_SHAREDMEMORY_INFO_H

#include <semaphore.h>

namespace SharedMemory {

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

}// namespace SharedMemory

#endif// ELTECAR_DATASERVER_INCLUDE_GENERAL_SHAREDMEMORY_INFO_H
