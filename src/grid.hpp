#pragma once

#include "Planet.hpp"
#include <unordered_map>
#include <list>

struct Grid;

struct KeyFuncs
{
    size_t operator()(const glm::ivec2& k)const
    {
        return std::hash<int>()(k.x) ^ (std::hash<int>()(k.y) << 1);
    }

    bool operator()(const glm::ivec2& a, const glm::ivec2& b)const
    {
        return a.x == b.x && a.y == b.y;
    }
};

struct Cell {
    Grid* internalGrid;
    std::list<Planet*> planets_in_cell;
    glm::dvec2 center;
    double m = 0;
    void update(const Planet& p) {
        bool includes = false;
        for (const Planet *p1 : planets_in_cell)
            if (p1 == &p) { includes = true; break; }

        if (!includes) return;

        center += p.m * (p.pos - p.pPos) / m;
    }
    void add(Planet& p) {
        if ( p.pos.x != p.pos.x || p.pos.y != p.pos.y)
            return;

        for (const Planet *p1 : planets_in_cell)
            if (p1 == &p) { center += p.m * (p.pos - p.pPos) / m; return; }

        planets_in_cell.push_front(&p);
        center *= m;
        m += p.m;
        if (m != 0)
            center = (center + p.m * p.pos) / m;
        else center = glm::dvec2(0);
    }
    void remove(Planet& p) {
        bool includes = false;
        for (const Planet *p1 : planets_in_cell)
            if (p1 == &p) { includes = true; break; }

        if (!includes) return;

        planets_in_cell.remove(&p);
        center *= m;
        m -= p.m;
        center = (center - p.m * p.pos) / m;
    }
};

struct Grid {
    double size = 1;
    std::unordered_map<glm::ivec2, Cell, KeyFuncs, KeyFuncs> cells;

    Cell get(const glm::ivec2& pos) {
        return cells[pos];
    }

    void add(Planet& p) {
        glm::ivec2 pos(p.pos / size);

        if ( p.pos.x != p.pos.x || p.pos.y != p.pos.y)
            return;

        if(cells.find(pos) == cells.end())
            cells[pos] = Cell();
        cells[pos].add(p);
    }

    void removeInvalid() {
        list<std::unordered_map<glm::ivec2, Cell, KeyFuncs, KeyFuncs>::const_iterator> empty;
        for (auto it = cells.cbegin(); it != cells.cend(); ++it)
            if(it->second.planets_in_cell.empty()) empty.push_back(it);

        for(auto pos : empty)
            cells.erase(pos);
    }
};