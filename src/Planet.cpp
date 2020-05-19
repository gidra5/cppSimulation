#include "Planet.hpp"
#include "grid.hpp"

extern double dt;
extern glm::uvec2 screenCenter;
extern Grid grid;

Planet::Planet(): df(0), dp(0), pos(0), vel(0), acc(0), r(0), m(0) {
    pPos = glm::dvec2(pos);
}

Planet::Planet(glm::dvec2 _pos, double _r): df(0), dp(0), pos(_pos), vel(0), acc(0), r(_r), m(r * r) {
    pPos = glm::dvec2(_pos) - glm::dvec2(dt) * r;
}

Planet::Planet(glm::ivec3 _col): df(0), dp(0), pos(0), vel(0), acc(0), r(0), m(0), col(_col) {
    pPos = glm::dvec2(pos);
}

void Planet::draw() {
    ofSetColor(col.r, col.g, col.b);
    ofFill();
    ofDrawCircle(pos, r);
}

void Planet::update() {
    acc += (df + dp / dt) / m;

    // 1
    // vel += acc * dt;
    // pos += vel * dt + acc * dt * dt / 2;

    // 2
    // vel += acc * dt;
    // pos += vel * dt;

    // 3
    glm::dvec2 temp(pos);
    // vel = (pos - pPos) / dt+ acc * dt;
    // pos = pos + vel * dt;
    pos += pos - pPos + acc * dt * dt;
    pPos = temp;

    df = glm::dvec2(0);
    dp = glm::dvec2(0);

    if (!abs(pos.x - ofGetWidth() * .5f) < ofGetWidth() * .5f)
    {
        vel.x = -vel.x;
        pos.x = min(max(pos.x, 0.d), (double)ofGetWidth());
    }
    if (!abs(pos.y - ofGetHeight() * .5f) < ofGetHeight() * .5f)
    {
        vel.y = -vel.y;
        pos.y = min(max(pos.y, 0.d), (double)ofGetHeight());
    }

    if (floor(pos / grid.size) != floor(pPos / grid.size)) {
        grid.cells[floor(pPos / grid.size)].remove(*this);
        grid.cells[floor(pos / grid.size)].add(*this);
    } else grid.cells[floor(pos / grid.size)].update(*this);


    // vel = (pos - pPos) / dt;

    acc = glm::dvec2(0);
}