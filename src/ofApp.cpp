#include "ofApp.hpp"
#include "grid.hpp"
#define sensativity 0.1
#define count 2000

const double k = 1e+6;
int N = count;
double dt = 1e-4;
glm::dvec2 screenCenter;
double radius_min;
double radius_max;
const bool sparse = true;

Grid grid;

Planet planets[count];

glm::dvec2 gravity(Planet &, Planet &);
glm::dvec2 collision(Planet&, Planet&);
double random(const double, const double);

glm::dvec2 gravity(Planet& p1, Planet& p2) {
    glm::dvec2 d = p2.pos - p1.pos;
    double magSq = dot(d, d);

    if (magSq > (p2.r + p1.r) * (p2.r + p1.r))
        return d * k * p1.m * p2.m / (magSq * sqrt(magSq));

    return glm::dvec2(0);
}
glm::dvec2 collision(Planet& p1, Planet& p2) {
    glm::dvec2 d = p2.pos - p1.pos;
    double dist = sqrt(dot(d, d));
    double k = (p1.r + p2.r - dist) / 2;

    if(k >= 0) {
        d /= dist;
        glm::dvec2 c = d * k;
        p1.pos -= c;
        p2.pos += c;

        //glm::dvec2 dv = (p2.pos - p2.pPos - p1.pos + p1.pPos) / dt;
        glm::dvec2 dv2 = p2.vel - p1.vel;
        float dvX = 1.4142 * p1.m * p2.m * dot(dv2, d) / (p1.m + p2.m);

        // std::cout << dv.x << ", " << dv.y << std::endl;
        // std::cout << dv2.x << ", " << dv2.y << std::endl;
        // std::cout << std::endl;

        return glm::dvec2(-dvX * d.x, dvX * d.y);
    }

    return glm::dvec2(0,0);
}

double random(const double a, const double b) {
    auto rv = rand() / 256.d;

    return a + (b - a) * (rv - floor(rv));
}

glm::dvec2 random2D() {
    glm::dvec2 vec(random(-1, 1), random(-1, 1));

    return vec / sqrt(dot(vec, vec));
}

//--------------------------------------------------------------
void ofApp::setup() {
    font.load("Monsterrat-Light.ttf", 25);

    scaling = 1.0d;
    screenCenter = glm::dvec2(ofGetWidth() * .5f, ofGetHeight() * .5f);
    screenPos = glm::dvec2(screenCenter);
    pmousePos = glm::ivec2(0);

    radius_min = sqrt(50000 / N);
    radius_max = sqrt(450000 / N);

    planets[0] = Planet(glm::dvec2(random(0, 2.d * screenCenter.x),
                                   random(0, 2.d * screenCenter.y)),
                        random(radius_min, radius_max));

    struct spawnPoint {
        glm::dvec2 pos = glm::dvec2(0);
        double r = 0;
        spawnPoint(Planet& p) : pos(p.pos), r(p.r) {}
    };

    std::list<spawnPoint*> spawnPoints;
    spawnPoints.push_back(new spawnPoint(planets[0]));
    grid.size = radius_max * 1.4142;

    auto IsValid = [&grid, &planets, &screenCenter](glm::dvec2 candidate, float radius) -> bool {
        if (abs(candidate.x - ofGetWidth() * .5f) < ofGetWidth() * .5f && abs(candidate.y - ofGetHeight() * .5f) < ofGetHeight() * .5f)
        {
            glm::ivec2 cell = floor(candidate / grid.size);

            for (int i = -2; i <= 2; ++i) {
                for (int j = -2; j <= 2; ++j) {
                    glm::ivec2 pos(cell.x - i, cell.y - j);
                    if (grid.cells.find(pos) != grid.cells.end())
                    {
                        Cell nearCell = grid.cells[pos];

                        for (const Planet* planet : nearCell.planets_in_cell) {
                            double dstSq = dot(candidate - planet->pos, candidate - planet->pos);

                            if (dstSq < (radius + planet->r) * (radius + planet->r))
                                return false;
                        }
                    }
                }
            }
            return true;
        }
        else
            return false;
    };

    for (int i = 0; i < N; ++i)
    {
        bool candidateAccepted = false;
        if (!spawnPoints.empty())
        {
            spawnPoint *randomPoint = sparse ? spawnPoints.back() : spawnPoints.front();

            for (int j = 0; j < 50; ++j)
            {
                double radius = random(radius_min, radius_max);
                glm::dvec2 candidate = randomPoint->pos + 2 * random2D() * random(randomPoint->r + radius_max, 2 * (randomPoint->r + radius_max));
                if (IsValid(candidate, radius)) {
                    planets[i] = Planet(candidate, radius);
					spawnPoints.push_back(new spawnPoint(planets[i]));
					grid.add(planets[i]);
					candidateAccepted = true;
                    break;
				}
            }
            if(!candidateAccepted) {
                --i;
                spawnPoints.remove(randomPoint);
            }
        }
        else
        {
            for (int j = 0; j < 50; ++j)
            {
                double radius = random(radius_min, radius_max);
                glm::dvec2 candidate = glm::dvec2(random(0, 2.d * screenCenter.x),
                                            random(0, 2.d * screenCenter.y));
                if (IsValid(candidate, radius)) {
                    planets[i] = Planet(candidate, radius);
					spawnPoints.push_back(new spawnPoint(planets[i]));
					grid.add(planets[i]);
					candidateAccepted = true;
                    break;
				}
            }
            if(!candidateAccepted)
                --i;
        }
    }

    // ofSetFrameRate(2);
    ofSetCircleResolution(64);

    ofSetBackgroundAuto(true);
    ofBackground(110);
}

//--------------------------------------------------------------
void ofApp::update(){
    // for(Planet& p1 : planets) {
    //     for(Planet& p2 : planets) {
    //         if (&p1 != &p2) {
    //             glm::dvec2 force = gravity(p1, p2); // + collision(p1, p2) / dt;

    //             p1.df += force;
    //         }
    //     }
    // }

    for (auto& c1 : grid.cells) {
        for (Planet* p1 : c1.second.planets_in_cell) {
            glm::dvec2 outerForce(0);
            for (auto& c2 : grid.cells)
            {
                if(c1.first == c2.first) continue;
                // if(c2.second.center.x != c2.second.center.x || c2.second.center.y != c2.second.center.y)
                //     continue;

                glm::dvec2 d = c2.second.center - p1->pos;
                double magSq = dot(d, d);

                outerForce += d * k * p1->m * c2.second.m / (magSq * sqrt(magSq));
            }
            p1->df += outerForce;

            for (Planet* p2 : c1.second.planets_in_cell) {
                if (p1 != p2) {
                    glm::dvec2 force = gravity(*p1, *p2);

                    p1->df += force;
                }
            }

            for (int i = -1; i <= 1; ++i) {
                for (int j = -1; j <= 1; ++j) {
                    if (grid.cells.find(c1.first - glm::ivec2(i, j)) != grid.cells.end())
                    {
                        Cell& nearCell = grid.cells[c1.first - glm::ivec2(i, j)];

                        for (Planet *p2 : nearCell.planets_in_cell)
                            if(p2 != p1) p1->df += collision(*p1, *p2) / dt;
                    }
                }
            }
        }
    }

    for (Planet &p : planets)
    {
        p.update();
    }
    grid.removeInvalid();
}

//--------------------------------------------------------------
void ofApp::draw(){
    ofTranslate(screenCenter);
    ofScale(scaling);
    ofTranslate(screenPos - 2.d * screenCenter);

    for (Planet& p : planets)
        p.draw();

    ofTranslate(2.d * screenCenter - screenPos);
    ofScale(1.f/scaling);
    ofTranslate(-screenCenter);

    ofSetColor(255, 255, 255);
    ofFill();
    // font.drawString(ofGetFrameRate(), 50, 50);
    // font.drawString(1.0d / dt, 50, 100);
    ofDrawBitmapString(ofGetFrameRate(), 50, 50);
    ofDrawBitmapString(1 / dt, 50, 100);

    // for (float i = 0; i < ofGetWidth(); i += grid.size)
    //     ofDrawLine(i, 0, i, ofGetHeight());
    // for (float i = 0; i < ofGetHeight(); i += grid.size)
    //     ofDrawLine(0, i, ofGetWidth(), i);
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key){

}

//--------------------------------------------------------------
void ofApp::keyReleased(int key){

}

//--------------------------------------------------------------
void ofApp::mouseMoved(int x, int y ){
    pmousePos = mousePos;
    mousePos = glm::ivec2(x, y);
}

//--------------------------------------------------------------
void ofApp::mouseDragged(int x, int y, int button){
    if (button != 0)
        return;
    screenPos += (mousePos - pmousePos) / scaling;

    pmousePos = mousePos;
    mousePos = glm::ivec2(x, y);
}

//--------------------------------------------------------------
void ofApp::mouseScrolled(int x, int y, float scrollX, float scrollY){
    scaling *= 1 + scrollY * sensativity;
}

//--------------------------------------------------------------
void ofApp::mousePressed(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mouseReleased(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mouseEntered(int x, int y){

}

//--------------------------------------------------------------
void ofApp::mouseExited(int x, int y){

}

//--------------------------------------------------------------
void ofApp::windowResized(int w, int h){

}

//--------------------------------------------------------------
void ofApp::gotMessage(ofMessage msg){

}

//--------------------------------------------------------------
void ofApp::dragEvent(ofDragInfo dragInfo){

}
