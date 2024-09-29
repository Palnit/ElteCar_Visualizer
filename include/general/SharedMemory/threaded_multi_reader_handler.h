#ifndef ELTECAR_VISUALIZER_INCLUDE_GENERAL_SHAREDMEMORY_THREADED_MULTI_READER_HANDLER_H
#define ELTECAR_VISUALIZER_INCLUDE_GENERAL_SHAREDMEMORY_THREADED_MULTI_READER_HANDLER_H

#include <future>
#include <string>
#include <thread>
#include "general/SharedMemory/bufferd_reader.h"

template<class T>
class ThreadedMultiReaderHandler {

public:
    ThreadedMultiReaderHandler(std::string bufferName,
                               std::function<T(void*, int)> readerFunc)
        : m_infoBufferName(bufferName),
          m_readerFunc(readerFunc) {}

    bool initalize();

    std::vector<T> ReadMultiMemory() {
        std::vector<std::thread> threads;
        std::vector<std::future<T>> futures;

        for (SharedMemory::BufferedReader<T> reader : m_readers) {
            std::promise<T> promise;
            futures.push_back(promise.get_future());
            std::thread thread(
                [](SharedMemory::BufferedReader<T> reader,
                   std::promise<T> promise) {
                    promise.set_value(reader.readData());
                },
                std::ref(reader), std::move(promise));

            threads.push_back(std::move(thread));
        }
        std::vector<T> output;
        for (auto& future : futures) { output.push_back(future.get()); }
        for (auto& thread : threads) { thread.join(); }
        return output;
    }

private:
    std::function<T(void*, int)> m_readerFunc;
    std::vector<SharedMemory::BufferedReader<T>> m_readers;
    std::string m_infoBufferName;
    SharedMemory::ThreadedInfo* m_memoryInfo;
    bool m_initalized = false;
};

#endif// ELTECAR_VISUALIZER_INCLUDE_GENERAL_SHAREDMEMORY_THREADED_MULTI_READER_HANDLER_H
