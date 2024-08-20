//
// Created by Palnit on 2024. 01. 17.
//

#ifndef GPGPU_EDGE_DETECTOR_INCLUDE_GENERAL_GENERIC_STRUCTS_H_
#define GPGPU_EDGE_DETECTOR_INCLUDE_GENERAL_GENERIC_STRUCTS_H_

#include <cstdint>
/*!
 * A simple RGBA struct to easily get the memory data from a uint32_t into the 4
 * channels if reinterpret casted or c style casted
 */
typedef struct RGBA {
    uint8_t r;
    uint8_t g;
    uint8_t b;
    uint8_t a;
} RGBA;

/*!
 * Used to get the memory data into an RGBA struct doesn't require casting
 */
typedef union Color {
    uint32_t raw;
    RGBA channels;
} Color;

/*!
 * \class Time
 * \brief Used to get time data
 *
 * A time class from witch statically we can get time data about the application
 * from anywhere in the program
 */
class Time {
public:
    Time() = delete;
    ~Time() = delete;
    static inline uint64_t ElapsedTime = 0;
    static inline uint64_t DeltaTime = 0;
    static inline double FPS = 0;
    static inline double Ms = 0;
};

/*!
 * Namespace containing different kinds of functions for error handling
 */
namespace ErrorHandling {
/*!
 * A simple function to display an SDL error if happened
 * \param type The errors type
 */
void HandelSDLError(const char* type);

}
#endif //GPGPU_EDGE_DETECTOR_INCLUDE_GENERAL_GENERIC_STRUCTS_H_
