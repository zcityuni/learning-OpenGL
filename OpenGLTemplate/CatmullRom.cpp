#include "CatmullRom.h"

CCatmullRom::CCatmullRom() {}
CCatmullRom::~CCatmullRom() {}

glm::vec3 CCatmullRom::Interpolate(glm::vec3& p0, glm::vec3& p1, glm::vec3& p2, glm::vec3& p3, float t)
{
    glm::vec3 a = p1;
    glm::vec3 b = 0.5f * (-p0 + p2);
    glm::vec3 c = 0.5f * (2.0f * p0 - 5.0f * p1 + 4.0f * p2 - p3);
    glm::vec3 d = 0.5f * (-p0 + 3.0f * p1 - 3.0f * p2 + p3);

    glm::vec3 result = (a + b * t + c * t * t + d * t * t * t);
    return result;
}