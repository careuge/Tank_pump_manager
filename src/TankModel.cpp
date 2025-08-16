#include "TankModel.h"
#include <math.h> // For acos(), sqrt(), and M_PI

#ifndef M_PI
// Define PI if it's not available in the math library
#define M_PI 3.14159265358979323846
#endif

namespace TankModel {

double calculateWaterVolume(double waterLevel, double tankRadius, double tankLength) {
    // Handle edge cases first for robustness and efficiency.
    if (waterLevel <= 0.0) {
        return 0.0; // Tank is empty
    }
    // The diameter is 2 * radius.
    if (waterLevel >= 2 * tankRadius) {
        // Tank is full. Volume is that of a cylinder: π * r^2 * L
        return M_PI * tankRadius * tankRadius * tankLength;
    }

    // Abbreviate variables for the formula for readability
    double r = tankRadius;
    double h = waterLevel;

    // This is the formula for the area of a circular segment:
    // A = r^2 * acos((r - h) / r) - (r - h) * sqrt(2*r*h - h^2)

    double term1 = r * r * acos((r - h) / r);
    double term2 = (r - h) * sqrt(2 * r * h - h * h);

    double segmentArea = term1 - term2;

    // Volume is the area of the water segment multiplied by the tank's length.
    return segmentArea * tankLength;
}

} // namespace TankModel
