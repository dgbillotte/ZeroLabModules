#ifndef UTIL_HPP
#define UTIL_HPP

template <typename T>
struct Stats {
    T min = 10.f;
    T max = -10.f;
    T sum = 0.f;
    int count = 0;

    void sample(T sample) {
        sum += sample;
        if(sample > max)
            max = sample;
        if(sample < min)
            min = sample;
    }
    void reset() {
        min = 10.f;
        max = -10.f;
        sum = 0.f;
        count = 0;
    }

    T mean() { return sum/(T)((count > 0) ? count : 0.00001f); }
};
typedef Stats<float> StatsF;

#endif