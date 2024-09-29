#ifndef ELTECAR_VISUALIZER_INCLUDE_GENERAL_SHAREDMEMORY_THREADED_MULTI_READER_HANDLER_H
#define ELTECAR_VISUALIZER_INCLUDE_GENERAL_SHAREDMEMORY_THREADED_MULTI_READER_HANDLER_H

#include <future>
#include <string>
#include <thread>
#include <tuple>
#include "general/SharedMemory/bufferd_reader.h"
#include "general/SharedMemory/info.h"

namespace SharedMemory {

template<class T>
class ThreadedMultiReaderHandler {

public:
    ThreadedMultiReaderHandler(std::string bufferName,
                               std::function<T(void*, int)> readerFunc)
        : m_infoBufferName(bufferName),
          m_readerFunc(readerFunc) {}

    bool initalize() {

        int infoDescriptor = shm_open(m_infoBufferName.c_str(), O_RDWR, 0666);

        if (infoDescriptor == -1) {
            std::cout << "Shared Memory Open of Name: " << m_infoBufferName
                      << " failed error: " << strerror(errno) << std::endl;
            return false;
        }

        m_memoryInfo = (SharedMemory::ThreadedInfo*) mmap(
            NULL, sizeof(SharedMemory::ThreadedInfo), PROT_READ | PROT_WRITE,
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

        m_memoryInfo = (SharedMemory::ThreadedInfo*) mmap(
            NULL, m_memoryInfo->infoSize, PROT_READ | PROT_WRITE, MAP_SHARED,
            infoDescriptor, 0);

        if (m_memoryInfo == MAP_FAILED) {
            std::cout << "Shared Memory Mapping of name: " << m_infoBufferName
                      << " failed error: " << strerror(errno) << std::endl;
            return false;
        }

        char* infoPointer = (char*) (m_memoryInfo + 1);

        for (int i = 0; i < m_memoryInfo->numberOfWriters; ++i) {
            std::string name(infoPointer);
            infoPointer = infoPointer + name.length() + 1;

            m_readers.push_back(
                SharedMemory::BufferedReader<T>(name, m_readerFunc));
        }
        if (sem_post(&m_memoryInfo->semaphore) == -1) {
            std::cout << "Semaphore post error: " << strerror(errno);
            return false;
        }
        return true;
    }

    std::vector<T> ReadMultiMemory() {

        if (!m_initalized) {
            if ((m_initalized = initalize()) == false) {
                return std::vector<T>();
            }
        }

        if (sem_wait(&m_memoryInfo->semaphore) == -1) {
            std::cout << "Semaphore wait error: " << strerror(errno);
            return std::vector<T>();
        }

        std::vector<std::thread> threads;
        std::vector<std::future<std::tuple<T, bool>>> futures;

        for (SharedMemory::BufferedReader<T>& reader : m_readers) {
            std::promise<std::tuple<T, bool>> promise;
            futures.push_back(promise.get_future());
            std::thread thread(
                [](SharedMemory::BufferedReader<T>& reader,
                   std::promise<std::tuple<T, bool>> promise) {
                    bool error;
                    T data = reader.readData(error);
                    promise.set_value(std::make_tuple(data, error));
                },
                std::ref(reader), std::move(promise));

            threads.push_back(std::move(thread));
        }
        std::vector<T> output;
        bool failed = false;
        for (auto& future : futures) {
            auto [data, error] = future.get();
            if (error) { failed = true; }
            output.push_back(data);
        }
        for (auto& thread : threads) { thread.join(); }

        if (sem_post(&m_memoryInfo->semaphore) == -1) {
            std::cout << "Semaphore post error: " << strerror(errno);
            return std::vector<T>();
        }
        if (failed) { return std::vector<T>(); }

        return output;
    }

private:
    std::function<T(void*, int)> m_readerFunc;
    std::vector<SharedMemory::BufferedReader<T>> m_readers;
    std::string m_infoBufferName;
    SharedMemory::ThreadedInfo* m_memoryInfo;
    bool m_initalized = false;
};

}// namespace SharedMemory
#endif// ELTECAR_VISUALIZER_INCLUDE_GENERAL_SHAREDMEMORY_THREADED_MULTI_READER_HANDLER_H
