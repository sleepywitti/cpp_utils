#pragma once

#include <cmath>        // std::remainder
#include <limits>       // std::numeric_limits
//#include <numeric>       // std::accumulate
namespace math
{
    const double pi = 3.14159265358979323846;
    
    template<class T>
    inline T radians(const T &deg) {
        static_assert(std::is_floating_point<T>::value, "unsupported type!");
        return std::remainder(deg * static_cast<T>(pi / 180.), static_cast<T>(2. * pi));
    }
    template<class T>
    inline T degrees(const T &rad) {
        static_assert(std::is_floating_point<T>::value, "unsupported type!");
        return std::remainder(rad * static_cast<T>(180. / pi), static_cast<T>(360.));
    }

    // credits go to http://en.cppreference.com/w/cpp/types/numeric_limits/epsilon
    // http://floating-point-gui.de/errors/comparison/
    template<class T>
    bool almost_equal(T x, T y, unsigned int ulp=1) {
        static_assert(std::is_floating_point<T>::value, "unsupported type!");

        // the machine epsilon has to be scaled to the magnitude of the values used
        // and multiplied by the desired precision in ULPs (units in the last place)
        return std::abs(x-y) < std::numeric_limits<T>::epsilon() * std::abs(x+y) * ulp
               // unless the result is subnormal
               || std::abs(x-y) < std::numeric_limits<T>::min();
    }

    template<class T>
    bool almost_zero(T x, unsigned int ulp=1) {
        static_assert(std::is_floating_point<T>::value, "unsupported type!");
        // the machine epsilon has to be scaled to the magnitude of the values used
        // and multiplied by the desired precision in ULPs (units in the last place)
        return std::abs(x) < std::numeric_limits<T>::epsilon() * static_cast<T>(ulp);
    }
} // namespace math

