#include <math.h>
#include "Util.hpp"

/*-----------------------------------------------------------------------------
 These functions provide audio like tapers for x:[0..1], y[0..1]
*/
// slow near 0, fast to 1
float audioTaperExp(float x) { return exp(x * 5.f - 5.f); }
float audioTaperX2(float x) { return x*x; }
float audioTaperX3(float x) { return x*x*x; }
float audioTaperX4(float x) { float tmp = x*x; return tmp*tmp; }

// fast near 0, slow to 1
float audioTaperLog(float x) { return (log(x + 0.1f) + 1.f) / 1.0414f; }
float audioTaperX2Inv(float x) { return -(x-1)*(x-1) + 1;}
float audioTaperX3Inv(float x) { return -(x-1)*(x-1)*(x-1) + 1;}
float audioTaperX4Inv(float x) { float tmp = (x-1)*(x-1); return -tmp*tmp + 1;}

// this is x[0,2] where [0,1] is log and [1,2] is exp
// this makes is slow at the ends and fast in the middle
// doh! these are wrong, need to piece together a sigmoid-like function
float audioTaperSlowEnds2(float x) { return (x <= 1.f) ? x * x : (x-2) * (x-2); }
float audioTaperSlowEnds3(float x) { return (x <= 1.f) ? x * x * x : -(x-2) * (x-2) * (x-2); }
float audioTaperSlowEnds4(float x) { float tmp = (x-2)*(x-2); return (x <= 1.f) ? tmp * tmp : (tmp-2) * (tmp-2); }
