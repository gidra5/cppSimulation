#pragma once

#include "ofMain.h"

class Planet {
    glm::dvec2 acc;

public:
    glm::ivec3 col = glm::ivec3(10, 255, 10);
    Planet(glm::dvec2, double);
    Planet(glm::ivec3 color);
    glm::dvec2 pPos;
    glm::dvec2 pos;
    glm::dvec2 vel;
    glm::dvec2 dp;
    glm::dvec2 df;
    void update();
    void draw();
    Planet();
    double r;
    double m;
};