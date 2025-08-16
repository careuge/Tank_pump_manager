#ifndef TANK_MODEL_H
#define TANK_MODEL_H

namespace TankModel {

/**
 * @brief Calculates the volume of water in a horizontal cylindrical tank.
 *
 * This function determines the volume of water based on the area of the
 * circular segment filled with water, multiplied by the length of the tank.
 *
 * @param waterLevel The height of the water from the bottom of the tank (in meters).
 * @param tankRadius The internal radius of the tank (in meters).
 * @param tankLength The internal length of the tank (in meters).
 * @return The volume of water in the tank in cubic meters (m^3).
 */
double calculateWaterVolume(double waterLevel, double tankRadius, double tankLength);

} // namespace TankModel

#endif // TANK_MODEL_H
