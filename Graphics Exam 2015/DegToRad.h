#pragma once
#include <glm/gtc/matrix_transform.hpp>

inline float degToRad(float angleInDeg)
{
    float angleInRad = angleInDeg * glm::pi<float>() / 180;
    return angleInRad;
}