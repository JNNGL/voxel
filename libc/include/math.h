/*
 * math.h - mathematical declarations
 * https://pubs.opengroup.org/onlinepubs/9699919799/basedefs/math.h.html
 */

#ifndef _MATH_H
#define _MATH_H 1

#ifdef __cplusplus
extern "C" {
#endif

#if FLT_EVAL_METHOD == 1
typedef double float_t;
typedef double double_t;
#elif FLT_EVAL_METHOD == 2
typedef long double float_t;
typedef long double double_t;
#else
typedef float float_t;
typedef double double_t;
#endif

#define M_E        2.7182818
#define M_LOG2E    1.4426950
#define M_LOG10E   0.43429448
#define M_LN2      0.69314718
#define M_LN10     2.3025851
#define M_PI       3.1415927
#define M_PI_2     1.5707963
#define M_PI_4     0.78539816
#define M_1_PI     0.31830989
#define M_2_PI     0.63661977
#define M_2_SQRTPI 1.1283792
#define M_SQRT2    1.4142136
#define M_SQRT1_2  0.70710678

#define NAN       (__builtin_nanf(""))
#define INFINITY  (__builtin_inff(""))
#define HUGE_VAL  (__builtin_huge_val())
#define HUGE_VALF (__builtin_huge_valf())
#define HUGE_VALL (__builtin_huge_vall())

#define FP_NAN       1
#define FP_INFINITE  2
#define FP_ZERO      3
#define FP_NORMAL    4
#define FP_SUBNORMAL 5

#define MATH_ERRNO     1
#define MATH_ERREXCEPT 2

// int fpclassify(double);

#define isfinite(x) (fpclassify(x) != FP_NAN && fpclassify(x) != FP_INFINITE)
#define isnormal(x) (fpclassify(x) == FP_NORMAL)
#define isnan(x)    (fpclassify(x) == FP_NAN)
#define isinf(x)    (fpclassify(x) == FP_INFINITE)

double acos(double); // TODO: Implementation
// float acosf(float);
// double acosh(double);
// float acoshf(float);
// long double acoshl(long double);
// long double acosl(long double);
double asin(double); // TODO: Implementation
// float asinf(float);
// double asinh(double);
// float asinhf(float);
// long double asinhl(long double);
// long double asinl(long double);
double atan(double); // TODO: Implementation
double atan2(double, double); // TODO: Implementation
// float atan2f(float, float);
// long double atan2l(long double, long double);
// float atanf(float);
// double atanh(double);
// float atanhf(float);
// long double atanhl(long double);
// long double atanl(long double);
// double cbrt(double);
// float cbrtf(float);
// long double cbrtl(long double);
double ceil(double); // TODO: Implementation
// float ceilf(float);
// long double ceill(long double);
// double copysign(double, double);
// float copysignf(float, float);
// long double copysignl(long double, long double);
double cos(double); // TODO: Implementation
// float cosf(float);
double cosh(double); // TODO: Implementation
// float coshf(float);
// long double coshl(long double);
// long double cosl(long double);
// double erf(double);
// double erfc(double);
// float erfcf(float);
// long double erfcl(long double);
// float erff(float);
// long double erfl(long double);
double exp(double); // TODO: Implementation
// double exp2(double);
// float exp2f(float);
// long double exp2l(long double);
// float expf(float);
// long double expl(long double);
// double expm1(double);
// float expm1f(float);
// long double expm1l(long double);
double fabs(double); // TODO: Implementation
// float fabsf(float);
// long double fabsl(long double);
// double fdim(double, double);
// float fdimf(float, float);
// long double fdiml(long double, long double);
double floor(double); // TODO: Implementation
// float floorf(float);
// long double floorl(long double);
// double fma(double, double, double);
// float fmaf(float, float, float);
// long double fmal(long double, long double, long double);
// double fmax(double, double);
// float fmaxf(float, float);
// long double fmaxl(long double, long double);
// double fmin(double, double);
// float fminf(float, float);
// long double fminl(long double, long double);
double fmod(double, double); // TODO: Implementation
// float fmodf(float, float);
// long double fmodl(long double, long double);
double frexp(double, int*); // TODO: Implementation
// float frexpf(float, int*);
// long double frexpl(long double, int*);
// double hypot(double, double);
// float hypotf(float, float);
// long double hypotl(long double, long double);
// int ilogb(double);
// int ilogbf(float);
// int ilogbl(long double);
// double j0(double);
// double j1(double);
// double jn(int, double);
double ldexp(double, int); // TODO: Implementation
// float ldexpf(float, int);
// long double ldexpl(long double, int);
// double lgamma(double);
// float lgammaf(float);
// long double lgammal(long double);
// long long llrint(double);
// long long llrintf(float);
// long long llrintl(long double);
// long long llround(double);
// long long llroundf(float);
// long long llroundl(long double);
double log(double); // TODO: Implementation
double log10(double); // TODO: Implementation
// float log10f(float);
// long double log10l(long double);
// double log1p(double);
// float log1pf(float);
// long double log1pl(long double);
// double log2(double);
// float log2f(float);
// long double log2l(long double);
// double logb(double);
// float logbf(float);
// long double logbl(long double);
// float logf(float);
// long double logl(long double);
// long lrint(double);
// long lrintf(float);
// long lrintl(long double);
// long lround(double);
// long lroundf(float);
// long lroundl(long double);
double modf(double, double*); // TODO: Implementation
// float modff(float, float *);
// long double modfl(long double, long double *);
// double nan(const char *);
// float nanf(const char *);
// long double nanl(const char *);
// double nearbyint(double);
// float nearbyintf(float);
// long double nearbyintl(long double);
// double nextafter(double, double);
// float nextafterf(float, float);
// long double nextafterl(long double, long double);
// double nexttoward(double, long double);
// float nexttowardf(float, long double);
// long double nexttowardl(long double, long double);
double pow(double, double); // TODO: Implementation
// float powf(float, float);
// long double powl(long double, long double);
// double remainder(double, double);
// float remainderf(float, float);
// long double remainderl(long double, long double);
// double remquo(double, double, int *);
// float remquof(float, float, int *);
// long double remquol(long double, long double, int *);
// double rint(double);
// float rintf(float);
// long double rintl(long double);
// double round(double);
// float roundf(float);
// long double roundl(long double);
// double scalbln(double, long);
// float scalblnf(float, long);
// long double scalblnl(long double, long);
// double scalbn(double, int);
// float scalbnf(float, int);
// long double scalbnl(long double, int);
double sin(double); // TODO: Implementation
// float sinf(float);
double sinh(double); // TODO: Implementation
// float sinhf(float);
// long double sinhl(long double);
// long double sinl(long double);
double sqrt(double); // TODO: Implementation
// float sqrtf(float);
// long double sqrtl(long double);
double tan(double); // TODO: Implementation
// float tanf(float);
double tanh(double); // TODO: Implementation
// float tanhf(float);
// long double tanhl(long double);
// long double tanl(long double);
// double tgamma(double);
// float tgammaf(float);
// long double tgammal(long double);
// double trunc(double);
// float truncf(float);
// long double truncl(long double);
// double y0(double);
// double y1(double);
// double yn(int, double);

extern int signgam;

#ifdef __cplusplus
}
#endif

#endif