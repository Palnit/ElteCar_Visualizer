#ifndef ELTECAR_VISUALIZER_INCLUDE_GENERAL_SHAREDMEMORY_THREADED_MULTI_READER_HANDLER_H
#define ELTECAR_VISUALIZER_INCLUDE_GENERAL_SHAREDMEMORY_THREADED_MULTI_READER_HANDLER_H

#include <future>
#include <string>
#include <thread>
#include <tuple>
#include "general/SharedMemory/bufferd_reader.h"
#include "general/SharedMemory/info.h"

namespace SharedMemory {

#if defined(WIN32) || defined(_WIN32) \
    || defined(__WIN32) && !defined(__CYGWIN__)

/// \class ThreadedMultiReaderHandler
/// \brief This class splits multiple buffered readers into separate threads
/// and syncs them tougether
/// @tparam T The return type of all the buffered readers
template<class T>
class ThreadedMultiReaderHandler {

public:

    /// Constructor
    /// @param bufferName Name of the threaded readers info buffer
    /// @param readerFunc The callback function that handels reading from the memory
    /// the function must be of type T(void*, int) replecated to all buffered readers
    ThreadedMultiReaderHandler(std::string bufferName,
                               std::function<T(void*, int)> readerFunc)
        : m_infoBufferName(bufferName),
          m_readerFunc(readerFunc) {}

    /// Deconstructor to handle windows unmaping
    ~ThreadedMultiReaderHandler() {
        UnmapViewOfFile(m_memoryInfo);
        CloseHandle(m_infoDescriptor);
        CloseHandle(m_infoSem);
    }

    /// Handels the initializations of all shared memory and semaphores
    /// associated whit this threaded reader handler and initializes all the
    /// buffered readers
    /// @return error check
    bool initalize() {

        m_infoDescriptor = OpenFileMapping(FILE_MAP_ALL_ACCESS, true,
                                           m_infoBufferName.c_str());

        if (m_infoDescriptor == nullptr) {
            std::cout << "Shared Memory Open of Name: " << m_infoBufferName
                      << " failed error: " << GetLastError() << std::endl;
            return false;
        }

        m_memoryInfo = (SharedMemory::ThreadedInfo*) MapViewOfFile(
            m_infoDescriptor, FILE_MAP_ALL_ACCESS, 0, 0,
            sizeof(SharedMemory::ThreadedInfo));
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

        switch (WaitForSingleObject(m_infoSem, 10)) {
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

        char* infoPointer = (char*) (m_memoryInfo + 1);

        for (int i = 0; i < m_memoryInfo->numberOfWriters; ++i) {
            std::string name(infoPointer);
            infoPointer = infoPointer + name.length() + 1;

            m_readers.push_back(
                SharedMemory::BufferedReader<T>(name, m_readerFunc));
        }
        if (!ReleaseSemaphore(m_infoSem, 1, nullptr)) {
            std::cout << "Semaphore post error: " << GetLastError();
            return false;
        }
        return true;
    }

    /// Splits into threads all the buffered readers and syncs their
    /// reading speeding up the shared memory handling while syncing mutliple
    /// data points of the same type
    /// @return a vector containing all the readers readMemory return values
    /// in order that was given when initializing the writer
    std::vector<T> ReadMultiMemory() {

        if (!m_initalized) {
            if ((m_initalized = initalize()) == false) {
                return std::vector<T>();
            }
        }

        switch (WaitForSingleObject(m_infoSem, 10)) {
            case WAIT_ABANDONED:
                std::cout << "Semaphore wait Abandoned: " << std::endl;
                return std::vector<T>();
            case WAIT_TIMEOUT:
                std::cout << "Semaphore wait Timeout: " << std::endl;
                return std::vector<T>();
            case WAIT_FAILED:
                std::cout << "Semaphore wait Failed: " << std::endl;
                return std::vector<T>();
        }

        std::vector<std::thread> threads;
        std::vector<std::future<std::tuple<T, bool> > > futures;

        for (SharedMemory::BufferedReader<T>& reader : m_readers) {
            std::promise<std::tuple<T, bool> > promise;
            futures.push_back(promise.get_future());
            std::thread thread(
                [](SharedMemory::BufferedReader<T>& reader,
                   std::promise<std::tuple<T, bool> > promise) {
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

        if (!ReleaseSemaphore(m_infoSem, 1, nullptr)) {
            std::cout << "Semaphore post error: " << GetLastError();
            return std::vector<T>();
        }
        if (failed) { return std::vector<T>(); }

        return output;
    }

private:
    std::function<T(void*, int)> m_readerFunc;
    std::vector<SharedMemory::BufferedReader<T> > m_readers;
    std::string m_infoBufferName;
    HANDLE m_infoSem = nullptr;
    HANDLE m_infoDescriptor = nullptr;
    SharedMemory::ThreadedInfo* m_memoryInfo;
    bool m_initalized = false;
};
#else

/// \class ThreadedMultiReaderHandler
/// \brief This class splits multiple buffered readers into separate threads
/// and syncs them tougether
/// @tparam T The return type of all the buffered readers
template<class T>
class ThreadedMultiReaderHandler {

public:
    /// Constructor
    /// @param bufferName Name of the threaded readers info buffer
    /// @param readerFunc The callback function that handels reading from the memory
    /// the function must be of type T(void*, int) replecated to all buffered readers
    ThreadedMultiReaderHandler(std::string bufferName,
                               std::function<T(void*, int)> readerFunc)
        : m_infoBufferName(bufferName),
          m_readerFunc(readerFunc) {}

    /// Handels the initializations of all shared memory and semaphores
    /// associated whit this threaded reader handler and initializes all the
    /// buffered readers
    /// @return error check
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

    /// Splits into threads all the buffered readers and syncs their
    /// reading speeding up the shared memory handling while syncing mutliple
    /// data points of the same type
    /// @return a vector containing all the readers readMemory return values
    /// in order that was given when initializing the writer
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
        std::vector<std::future<std::tuple<T, bool> > > futures;

        for (SharedMemory::BufferedReader<T>& reader : m_readers) {
            std::promise<std::tuple<T, bool> > promise;
            futures.push_back(promise.get_future());
            std::thread thread(
                [](SharedMemory::BufferedReader<T>& reader,
                   std::promise<std::tuple<T, bool> > promise) {
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
    std::vector<SharedMemory::BufferedReader<T> > m_readers;
    std::string m_infoBufferName;
    SharedMemory::ThreadedInfo* m_memoryInfo;
    bool m_initalized = false;
};
#endif

}// namespace SharedMemory
#endif// ELTECAR_VISUALIZER_INCLUDE_GENERAL_SHAREDMEMORY_THREADED_MULTI_READER_HANDLER_H
