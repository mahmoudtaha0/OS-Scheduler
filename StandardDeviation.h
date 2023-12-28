#include <stdio.h>
#include <math.h>

float calculateStandardDeviation(float* WTA_values, int WTA_count) {
    float mean = 0.0, variance = 0.0, std_deviation = 0.0;

    // Calculate the mean of WTA values
    for (int i = 0; i < WTA_count; ++i) {
        mean += WTA_values[i];
    }
    mean /= WTA_count;

    // Calculate the variance
    for (int i = 0; i < WTA_count; ++i) {
        variance += pow(WTA_values[i] - mean, 2);
    }
    variance /= WTA_count;

    // Calculate the standard deviation
    std_deviation = sqrt(variance);

    return std_deviation;
}