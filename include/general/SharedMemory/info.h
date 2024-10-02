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

/// \class Info
/// \brief struct controling the information syncing trough shared memory
///
/// This class serves as the back bone of the shared memory every time the shared
/// memory wants to sync data its goes trough this class and provides reading and
/// writing saftes trough semaphores
struct Info {
    /// the current buffer number
    int bufferNumber = 0;
    /// the size of the info buffer
    int infoBufferSize;
    /// size of the shared memory buffer
    int bufferSize;
    /// size of the current data in the shared memory
    int dataSize;
    /// number of buffers the shared memory maneger has
    int bufferNamesCount;
    /// if the memory needs remaping TODO
    int remap;
    /// Constructor
    /// @param newbufferNumber the start buffer number
    /// @param newbufferSize the default size of the buffer
    /// @param newbufferNamesCount the number of buffers
    Info(int newbufferNumber, int newbufferSize, int newbufferNamesCount)
        : bufferNumber(newbufferNumber),
          bufferSize(newbufferSize),
          bufferNamesCount(newbufferNamesCount) {}
    /// Constructor
    /// @param newbufferSize the default size of the buffer
    Info(int newbufferSize) { bufferSize = newbufferSize;
    /// Default constructor
    Info()
        : bufferNumber(0),
          infoBufferSize(-1),
          bufferSize(-1),
          bufferNamesCount(-1) {}
};

/// \class ThreadedInfo
/// \brief info buffer for threaded shared memory
///
/// This struct is basicly the same as the Info struct only for the threaded
/// shared memory
struct ThreadedInfo {
    /// the number of writers/reader names associated whit this treaded memory
    int numberOfWriters;
    /// size of the info buffer
    int infoSize;
};
#else

/// \class Info
/// \brief struct controling the information syncing trough shared memory
///
/// This class serves as the back bone of the shared memory every time the shared
/// memory wants to sync data its goes trough this class and provides reading and
/// writing saftes trough semaphores
struct Info {
    /// the current buffer number
    int bufferNumber = 0;
    /// the size of the info buffer
    int infoBufferSize;
    /// size of the shared memory buffer
    int bufferSize;
    /// size of the current data in the shared memory
    int dataSize;
    /// number of buffers the shared memory maneger has
    int bufferNamesCount;
    /// semaphore of the info buffer
    sem_t semaphore;

    /// Constructor
    /// @param newbufferNumber the start buffer number
    /// @param newbufferSize the default size of the buffer
    /// @param newbufferNamesCount the number of buffers
    Info(int newbufferNumber, int newbufferSize, int newbufferNamesCount)
        : bufferNumber(newbufferNumber),
          bufferSize(newbufferSize),
          bufferNamesCount(newbufferNamesCount) {}

    /// Constructor
    /// @param newbufferSize the default size of the buffer
    Info(int newbufferSize) { bufferSize = newbufferSize; }

    /// Default constructor
    Info()
        : bufferNumber(0),
          infoBufferSize(-1),
          bufferSize(-1),
          bufferNamesCount(-1) {}
};

/// \class ThreadedInfo
/// \brief info buffer for threaded shared memory
///
/// This struct is basicly the same as the Info struct only for the threaded
/// shared memory
struct ThreadedInfo {
    /// the number of writers/reader names associated whit this treaded memory
    int numberOfWriters;
    /// size of the info buffer
    int infoSize;
    /// semaphore of the info buffer
    sem_t semaphore;
};

#endif
}// namespace SharedMemory

#endif// ELTECAR_DATASERVER_INCLUDE_GENERAL_SHAREDMEMORY_INFO_H
