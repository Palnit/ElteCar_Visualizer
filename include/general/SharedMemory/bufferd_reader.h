#ifndef ELTECAR_VISUALIZER_INCLUDE_GENERAL_SHAREDMEMORY_BUFFERD_READER_H
#define ELTECAR_VISUALIZER_INCLUDE_GENERAL_SHAREDMEMORY_BUFFERD_READER_H

#include <fcntl.h>
#include <semaphore.h>
#include <sys/mman.h>
#include <cstring>
#include <functional>
#include <iostream>
#include <string>
#include <tuple>
#include <vector>
#include "general/SharedMemory/info.h"
namespace SharedMemory {
template<class T>
class BufferedReader {
public:
    BufferedReader(const std::string& infoBufferName,
                   std::function<T(void*, int)> readerFunc)
        : m_readerFunc(readerFunc),
          m_infoBufferName(infoBufferName) {}

    bool initalize() {

        int infoDescriptor =
            shm_open(m_infoBufferName.c_str(), O_CREAT | O_RDWR, 0666);

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

    T readData() {
        if (!m_initalized) {
            if ((m_initalized = initalize()) == false) {
                std::cout << "failed to intialize reader" << std::endl;
            }
        }

        if (sem_wait(&m_memoryInfo->semaphore) == -1) {
            std::cout << "Semaphore wait error: " << strerror(errno);
        }
        auto& [descript, pointer, sem] =
            m_memoryBuffer[m_memoryInfo->bufferNumber];
        if (sem_wait(sem) == -1) {
            std::cout << "Semaphore wait error: " << strerror(errno);
        }
        int size = m_memoryInfo->dataSize;

        T value = m_readerFunc(pointer, size);
        if (sem_post(sem) == -1) {
            std::cout << "Semaphore post error: " << strerror(errno);
        }
        if (sem_post(&m_memoryInfo->semaphore) == -1) {
            std::cout << "Semaphore post error: " << strerror(errno);
        }
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

}// namespace SharedMemory

#endif// ELTECAR_VISUALIZER_INCLUDE_GENERAL_SHAREDMEMORY_BUFFERD_READER_H
