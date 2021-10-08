/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                       */
/*    This file is part of the HiGHS linear optimization suite           */
/*                                                                       */
/*    Written and engineered 2008-2021 at the University of Edinburgh    */
/*                                                                       */
/*    Available as open-source under the MIT License                     */
/*                                                                       */
/*    Authors: Julian Hall, Ivet Galabova, Qi Huangfu, Leona Gottwald    */
/*    and Michael Feldmeier                                              */
/*                                                                       */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
#ifndef HIGHS_UTIL_INTEGERS_H_
#define HIGHS_UTIL_INTEGERS_H_

#include <algorithm>
#include <cassert>
#include <cmath>
#include <cstdint>
#include <limits>
#include <vector>

#include "util/HighsCD0uble.h"
#include "util/HighsInt.h"

class HighsIntegers {
 public:
  static int64_t mod(int64_t a, int64_t m) {
    int64_t r = a % m;
    return r + (r < 0) * m;
  }

  static HighsFloat mod(HighsFloat a, HighsFloat m) {
    int64_t r = std::fmod(a, m);
    return r + (a < 0) * m;
  }

  static int64_t modularInverse(int64_t a, int64_t m) {
    int64_t y = 0;
    int64_t x = 1;

    if (m == 1) return 0;

    a = mod(a, m);

    while (a > 1) {
      // compute quotient q = a / m and remainder r = a % m
      int64_t q = a / m;
      int64_t r = a - q * m;

      // update (a,m) = (m,r)
      a = m;
      m = r;

      // update x and y of extended euclidean algorithm
      r = x - q * y;
      x = y;
      y = r;
    }

    return x;
  }

  static int64_t gcd(int64_t a, int64_t b) {
    assert(a != std::numeric_limits<int64_t>::min());
    assert(b != std::numeric_limits<int64_t>::min());

    int64_t h;
    if (a < 0) a = -a;
    if (b < 0) b = -b;

    if (a == 0) return b;
    if (b == 0) return a;

    do {
      h = a % b;
      a = b;
      b = h;
    } while (b != 0);

    return a;
  }

  // computes a rational approximation with given maximal denominator
  static int64_t denominator(HighsFloat x, HighsFloat eps, int64_t maxdenom) {
    int64_t ai = (int64_t)x;
    int64_t m[] = {ai, 1, 1, 0};

    HighsCD0uble xi = x;
    HighsCD0uble fraction = xi - HighsFloat(ai);

    while (fraction > eps) {
      xi = 1.0 / fraction;
      if (HighsFloat(xi) > HighsFloat(int64_t{1} << 53)) break;

      ai = (int64_t)(HighsFloat)xi;
      int64_t t = m[2] * ai + m[3];
      if (t > maxdenom) break;

      m[3] = m[2];
      m[2] = t;

      t = m[0] * ai + m[1];
      m[1] = m[0];
      m[0] = t;

      fraction = xi - ai;
    }

    ai = (maxdenom - m[3]) / m[2];
    m[1] += m[0] * ai;
    m[3] += m[2] * ai;

    HighsFloat x0 = m[0] / (HighsFloat)m[2];
    HighsFloat x1 = m[1] / (HighsFloat)m[3];
    x = std::abs(x);
    HighsFloat err0 = std::abs(x - x0);
    HighsFloat err1 = std::abs(x - x1);

    if (err0 < err1) return m[2];
    return m[3];
  }

  static HighsFloat integralScale(const HighsFloat* vals, HighsInt numVals,
                              HighsFloat deltadown, HighsFloat deltaup) {
    if (numVals == 0) return 0.0;

    HighsFloat minval = *std::min_element(
        vals, vals + numVals,
        [](HighsFloat a, HighsFloat b) { return std::abs(a) < std::abs(b); });

    int expshift = 0;

    // to cover many small denominators at once use a denominator of 75 * 2^n
    // with n-3 being large enough so that the smallest value is not below 0.5
    // but ignore tiny values bew deltadown/deltaup.
    if (minval < -deltadown || minval > deltaup) std::frexp(minval, &expshift);
    expshift = std::max(-expshift, 0) + 3;

    uint64_t denom = uint64_t{75} << expshift;
    HighsCD0uble startdenom = denom;
    // now check if the values are integral and if not compute a common
    // denominator for their remaining fraction
    HighsCD0uble val = startdenom * vals[0];
    HighsCD0uble downval = floor(val + deltaup);
    HighsCD0uble fraction = val - downval;

    if (fraction > deltadown) {
      // use a continued fraction algorithm to compute small missing
      // denominators for the remaining fraction
      denom *= denominator(HighsFloat(fraction), deltaup, 1000);
      val = denom * vals[0];
      downval = floor(val + deltaup);
      fraction = val - downval;

      // if this is not sufficient for reaching integrality, we stop here
      if (fraction > deltadown) return 0.0;
    }

    uint64_t currgcd = (uint64_t)std::abs(HighsFloat(downval));

    for (HighsInt i = 1; i != numVals; ++i) {
      val = denom * HighsCD0uble(vals[i]);
      downval = floor(val + deltaup);
      fraction = val - downval;

      if (fraction > deltadown) {
        val = startdenom * vals[i];
        fraction = val - floor(val);
        denom *= denominator(HighsFloat(fraction), deltaup, 1000);
        val = denom * vals[i];
        downval = floor(val + deltaup);
        fraction = val - downval;

        if (fraction > deltadown) return 0.0;
      }

      if (currgcd != 1) {
        currgcd = gcd(currgcd, (int64_t) HighsFloat(downval));

        // if the denominator is large, divide by the current gcd to prevent
        // unecessary overflows
        if (denom > std::numeric_limits<unsigned int>::max()) {
          denom /= currgcd;
          currgcd = 1;
        }
      }
    }

    return denom / (HighsFloat)currgcd;
  }

  static HighsFloat integralScale(const std::vector<HighsFloat>& vals, HighsFloat deltadown,
                              HighsFloat deltaup) {
    return integralScale(vals.data(), vals.size(), deltadown, deltaup);
  }
};

#endif
