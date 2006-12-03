#ifndef MATH_UTILS_HEADER
#define MATH_UTILS_HEADER
#include <cmath>

/** Math utilities scrounged from parts of 2geom as well as some code found via Krugle -
 *  Game Boy Xport's mathutils.h .
 */

/** Sign function - indicates the sign of a numeric type.  -1 indicates negative, 1 indicates
 *  positive, and 0 indicates, well, 0.  Mathsy people will know this is basically the derivative
 *  of abs, except for the fact that it is defined on 0.
 */
template <class T> inline int sgn(const T& x) {return (x < 0 ? -1 : (x > 0 ? 1 : 0) );}

/** Square function - sqr(x) is equivalent to x * x. */
template <class T> inline int sqr(const T& x) {return x * x;}

/** Cube function - cube(x) is equivalent to x * x * x. */
template <class T> inline int cube(const T& x) {return x * x * x;}

/** Maximum function - returns the highest of two numeric types. They must have the same type.*/
template <class T> inline const T& max (const T& a, const T& b) {return (a > b) ? a : b;}

/** Minimum function - returns the lowest of two numeric types. They must have the same type.*/
template <class T> inline const T& min (const T& a, const T& b) {return (a < b) ? a : b;}

/** Between function - returns true if a number x is within a range. The values delimiting the
 *  range, as well as the number must have the same type.
 */
template <class T> inline const T& between (const T& min, const T& max, const T& x)
    { return min < x && max > x; }

/** Inverse Square-Root function - equivalent to 1 / sqrt(x), however much faster. Gleaned from the
 *  Quake 3 source, this function is very very magic in its working, actually converting a float to
 *  integer form and back during the process.
 */
float invSqrt (float x);

/** Returns x rounded to the nearest integer.  It is unspecified what happens
 *  if x is half way between two integers: we may in future use rint/round
 *  on platforms that have them.
 */
inline double round(double const x) { return std::floor(x + .5); }

/** Returns x rounded to the nearest \a places decimal places.

    Implemented in terms of round, i.e. we make no guarantees as to what happens if x is
    half way between two rounded numbers.
    
    Note: places is the number of decimal places without using scientific (e) notation, not the
    number of significant figures.  This function may not be suitable for values of x whose
    magnitude is so far from 1 that one would want to use scientific (e) notation.

    places may be negative: e.g. places = -2 means rounding to a multiple of .01
**/
inline double decimal_round(double const x, int const places) {
    //TODO: possibly implement with modulus instead?
    double const multiplier = std::pow(10.0, places);
    return round( x * multiplier ) / multiplier;
}

#endif
