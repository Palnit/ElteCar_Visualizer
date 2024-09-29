#ifndef ELTECAR_VISUALIZER_INCLUDE_GENERAL_SHAREDMEMORY_BUFFERD_READER_H
#define ELTECAR_VISUALIZER_INCLUDE_GENERAL_SHAREDMEMORY_BUFFERD_READER_H

#include <fcntl.h>
#include <cstring>
#include <functional>
#include <iostream>
#include <string>
#include <tuple>
#include <vector>
#include "general/SharedMemory/info.h"

#if defined(WIN32) || defined(_WIN32) \
    || defined(__WIN32) && !defined(__CYGWIN__)
#include <windows.h>
#else
#include <semaphore.h>
#include <sys/mman.h>
#endif

namespace SharedMemory {

#if defined(WIN32) || defined(_WIN32) \
    || defined(__WIN32) && !defined(__CYGWIN__)

template<class T>
class BufferedReader {
public:
    BufferedReader(const std::string& infoBufferName,
                   std::function<T(void*, int)> readerFunc)
        : m_readerFunc(readerFunc),
          m_infoBufferName(infoBufferName) {}

    ~BufferedReader() {
        UnmapViewOfFile(m_memoryInfo);
        CloseHandle(m_infoDescriptor);
        CloseHandle(m_infoSem);
        for (auto& [descript, pointer, sem] : m_memoryBuffer) {
            UnmapViewOfFile(pointer);
            CloseHandle(descript);
            CloseHandle(sem);
        }
    }

    bool initalize() {

        m_infoDescriptor = OpenFileMapping(FILE_MAP_ALL_ACCESS, true,
                                           m_infoBufferName.c_str());

        if (m_infoDescriptor == nullptr) {
            std::cout << "Shared Memory Open of Name: " << m_infoBufferName
                      << " failed error: " << GetLastError() << std::endl;
            return false;
        }

        m_memoryInfo = (SharedMemory::Info*) MapViewOfFile(
            m_infoDescriptor, FILE_MAP_ALL_ACCESS, 0, 0,
            sizeof(SharedMemory::Info));
        if (m_memoryInfo == nullptr) {
            std::cout << "Shared Memory Mapping of name: " << m_infoBufferName
                      << " failed error: " << GetLastError() << std::endl;
            return false;
        }

        std::string infoSemName = m_infoBufferName + "_sem";
        m_infoSem =
            OpenSemaphore(SEMAPHORE_ALL_ACCESS, true, infoSemName.c_str());
        if (m_infoSem == nullptr) {
            std::cout
                << "Initalization of Semaphore for shared memory of name: "
                << m_infoBufferName << " failed errror: " << GetLastError()
                << std::endl;
            return false;
        }

        switch (WaitForSingleObject(m_infoSem, 1000)) {
            case WAIT_ABANDONED:
                std::cout << "Semaphore wait Abandoned: " << std::endl;
                return false;
            case WAIT_TIMEOUT:
                std::cout << "Semaphore wait Timeout: " << std::endl;
                return false;
            case WAIT_FAILED:
                std::cout << "Semaphore wait Failed: " << std::endl;
                return false;
        }

        m_size = m_memoryInfo->bufferSize;
        char* infoPointer = (char*) (m_memoryInfo + 1);

        for (int i = 0; i < m_memoryInfo->bufferNamesCount; i++) {
            std::string name(infoPointer);
            void* pointer;
            infoPointer = (char*) (infoPointer + name.length() + 1);

            std::string sem_name = name + "_sem";

            HANDLE sem =
                OpenSemaphore(SEMAPHORE_ALL_ACCESS, true, sem_name.c_str());
            if (sem == nullptr) {
                std::cout
                    << "Initalization of Semaphore for shared memory of name: "
                    << m_infoBufferName << " failed errror: " << GetLastError()
                    << std::endl;
                return false;
            }

            HANDLE descript =
                CreateFileMapping(INVALID_HANDLE_VALUE, nullptr, PAGE_READWRITE,
                                  0, m_size, name.c_str());

            if (descript == nullptr) {
                std::cout << "Shared Memory Open of Name: " << name
                          << " failed error: " << GetLastError() << std::endl;
                return false;
            }

            pointer =
                MapViewOfFile(descript, FILE_MAP_ALL_ACCESS, 0, 0, m_size);
            if (pointer == nullptr) {
                std::cout << "Shared Memory Mapping of name: " << name
                          << " failed error: " << GetLastError() << std::endl;
                return false;
            }

            m_memoryBuffer.push_back({descript, pointer, sem});
        }

        if (!ReleaseSemaphore(m_infoSem, 1, nullptr)) {
            std::cout << "Semaphore post error: " << GetLastError();
            return false;
        }
        return true;
    }

    T readData() {
        if (!m_initalized) {
            if ((m_initalized = initalize()) == false) {
                std::cout << "failed to intialize reader" << std::endl;
            }
        }

        switch (WaitForSingleObject(m_infoSem, 1000)) {
            case WAIT_ABANDONED:
                std::cout << "Semaphore wait Abandoned: " << std::endl;
                return false;
            case WAIT_TIMEOUT:
                std::cout << "Semaphore wait Timeout: " << std::endl;
                return false;
            case WAIT_FAILED:
                std::cout << "Semaphore wait Failed: " << std::endl;
                return false;
        }

        auto& [descript, pointer, sem] =
            m_memoryBuffer[m_memoryInfo->bufferNumber];
        switch (WaitForSingleObject(sem, 1000)) {
            case WAIT_ABANDONED:
                std::cout << "Semaphore wait Abandoned: " << std::endl;
                return false;
            case WAIT_TIMEOUT:
                std::cout << "Semaphore wait Timeout: " << std::endl;
                return false;
            case WAIT_FAILED:
                std::cout << "Semaphore wait Fail: " << std::endl;
                return false;
        }
        int size = m_memoryInfo->dataSize;

        T value = m_readerFunc(pointer, size);
        if (!ReleaseSemaphore(sem, 1, nullptr)) {
            std::cout << "Semaphore post error: " << GetLastError();
            return false;
        }
        if (!ReleaseSemaphore(m_infoSem, 1, nullptr)) {
            std::cout << "Semaphore post error: " << GetLastError();
            return false;
        }
        return value;
    }

private:
    std::function<T(void*, int)> m_readerFunc;
    std::vector<std::tuple<HANDLE, void*, HANDLE>> m_memoryBuffer;
    SharedMemory::Info* m_memoryInfo;
    std::string m_infoBufferName;
    HANDLE m_infoSem = nullptr;
    HANDLE m_infoDescriptor = nullptr;
    int m_size;
    bool m_initalized = false;
};

#else
template<class T>
class BufferedReader {
public:
    BufferedReader(const std::string& infoBufferName,
                   std::function<T(void*, int)> readerFunc)
        : m_readerFunc(readerFunc),
          m_infoBufferName(infoBufferName) {}

    bool initalize() {

        int infoDescriptor = shm_open(m_infoBufferName.c_str(), O_RDWR, 0666);

        if (infoDescriptor == -1) {
            std::cout << "Shared Memory Open of Name: " << m_infoBufferName
                      << " failed error: " << strerror(errno) << std::endl;
            return false;
        }

        m_memoryInfo = (SharedMemory::Info*) mmap(
            NULL, sizeof(SharedMemory::Info), PROT_READ | PROT_WRITE,
            MAP_SHARED, infoDescriptor, 0);
        if (m_memoryInfo == MAP_FAILED) {
            std::cout << "Shared Memory Mapping of name: " << m_infoBufferName
                      << " failed error: " << strerror(errno) << std::endl;
            return false;
        }

        if (sem_wait(&m_memoryInfo->semaphore) == -1) {
            std::cout << "Semaphore wait error: " << strerror(errno);
            return false;
        }

        m_memoryInfo = (SharedMemory::Info*) mmap(
            NULL, m_memoryInfo->infoBufferSize, PROT_READ | PROT_WRITE,
            MAP_SHARED, infoDescriptor, 0);

        if (m_memoryInfo == MAP_FAILED) {
            std::cout << "Shared Memory Mapping of name: " << m_infoBufferName
                      << " failed error: " << strerror(errno) << std::endl;
            return false;
        }

        m_size = m_memoryInfo->bufferSize;
        char* infoPointer = (char*) (m_memoryInfo + 1);

        for (int i = 0; i < m_memoryInfo->bufferNamesCount; i++) {
            std::string name(infoPointer);
            void* pointer;
            sem_t* sem = (sem_t*) (infoPointer + name.length() + 1);

            int descript = shm_open(name.c_str(), O_CREAT | O_RDWR, 0666);

            if (descript == -1) {
                std::cout << "Shared Memory Open of Name: " << name
                          << " failed error: " << strerror(errno) << std::endl;
                return false;
            }

            pointer = mmap(NULL, m_memoryInfo->bufferSize,
                           PROT_READ | PROT_WRITE, MAP_SHARED, descript, 0);

            if (pointer == MAP_FAILED) {
                std::cout << "Shared Memory Mapping of name: " << name
                          << " failed error: " << strerror(errno) << std::endl;
                return false;
            }

            m_memoryBuffer.push_back({descript, pointer, sem});

            infoPointer = (char*) (sem + 1);
        }

        if (sem_post(&m_memoryInfo->semaphore) == -1) {
            std::cout << "Semaphore post error: " << strerror(errno);
            return false;
        }
        return true;
    }

    T readData(bool& failed) {
        if (!m_initalized) {
            if ((m_initalized = initalize()) == false) {}
            failed = true;
            return T();
        }

        if (sem_wait(&m_memoryInfo->semaphore) == -1) {
            std::cout << "Semaphore wait error: " << strerror(errno);
            failed = true;
            return T();
        }
        auto& [descript, pointer, sem] =
            m_memoryBuffer[m_memoryInfo->bufferNumber];
        if (sem_wait(sem) == -1) {
            std::cout << "Semaphore wait error: " << strerror(errno);
            failed = true;
            return T();
        }
        int size = m_memoryInfo->dataSize;

        T value = m_readerFunc(pointer, size);
        if (sem_post(sem) == -1) {
            std::cout << "Semaphore post error: " << strerror(errno);
            failed = true;
            return T();
        }
        if (sem_post(&m_memoryInfo->semaphore) == -1) {
            std::cout << "Semaphore post error: " << strerror(errno);
            failed = true;
            return T();
        }
        failed = false;
        return value;
    }

private:
    std::function<T(void*, int)> m_readerFunc;
    std::vector<std::tuple<int, void*, sem_t*>> m_memoryBuffer;
    SharedMemory::Info* m_memoryInfo;
    std::string m_infoBufferName;
    int m_size;
    bool m_initalized;
};
#endif
}// namespace SharedMemory

#endif// ELTECAR_VISUALIZER_INCLUDE_GENERAL_SHAREDMEMORY_BUFFERD_READER_H
