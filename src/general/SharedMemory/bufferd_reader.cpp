#include "general/SharedMemory/bufferd_reader.h"
#include <fcntl.h>
#include <semaphore.h>
#include <sys/mman.h>
#include <unistd.h>
#include <cerrno>
#include <cstring>
#include <iostream>
#include "general/SharedMemory/info.h"

namespace SharedMemory {}// namespace SharedMemory
