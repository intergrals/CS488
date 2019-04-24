// Fall 2018

#include <glm/ext.hpp>
#include <thread>
#include <mutex>
#include <random>
#include <iomanip>
#include <stack>
#include <algorithm>
#include <queue>

#include "A5.hpp"
#include "GeometryNode.hpp"

std::mutex mtx;
std::vector<photon> pList;
TreeNode *pTree;
bool donePM = false;


void loading(float percent) {
    float p = percent;
    std::cout << '[';
    for (int i = 0; i < 25; i++) {
        if (percent >= 4) {
            std::cout << '#';
            percent -= 4;
        } else if (percent >= 2) {
            std::cout << '=';
            percent -= 2;
        } else {
            std::cout << '-';
        }
    }

    std::cout << ']';
    std::cout.width(4);
    std::cout << std::right << p << '%' << '\r' << std::flush;
}


// n-closest neighbours
std::vector<photon> getNClosest(glm::vec3 n) {
    auto cmp = [&n](photon a, photon b) { return glm::distance(n, a.pos) < glm::distance(n, b.pos); };
    std::priority_queue<photon, std::vector<photon>, decltype(cmp)> pq(cmp);

    for (auto p : pList) {
        pq.push(p);
        if (pq.size() > ClosestN) {
            pq.pop();
        }
    }

    std::vector<photon> ret;
    while (!pq.empty()) {
        ret.push_back(std::move(const_cast<photon &>(pq.top())));
        pq.pop();
    }

    return ret;
}


// KD TREE

// Make kd tree for photons
TreeNode *makeKD(char layer, uint start, uint end) {
    // tree depth value
    uint i;
    if (layer == 'x') i = 0;
    else if (layer == 'y') i = 1;
    else i = 2;

    char nextLayer;
    if (layer == 'z') nextLayer = 'x';
    else nextLayer = char(layer + 1);


    if (end - start == 0) {
        TreeNode *retNode = new TreeNode(pList.front(), nextLayer);
        retNode->min = retNode->p.pos;
        retNode->max = retNode->p.pos;

        return retNode;
    }

    uint mid = (end + start) / 2;

    std::nth_element(pList.begin() + start, pList.begin() + mid, pList.begin() + end,
                     [&i](photon a, photon b) { return a.pos[i] < b.pos[i]; });

    TreeNode *node = new TreeNode(pList[pList.size() / 2], layer);

    // Set bounds
    for (auto p = pList.begin() + start; p != pList.begin() + end; p++) {
        node->min.x = glm::min(node->min.x, p->pos.x);
        node->min.y = glm::min(node->min.y, p->pos.y);
        node->min.z = glm::min(node->min.z, p->pos.z);
        node->max.x = glm::max(node->max.x, p->pos.x);
        node->max.y = glm::max(node->max.y, p->pos.y);
        node->max.z = glm::max(node->max.z, p->pos.z);
    }

    std::cout << glm::to_string(node->p.pos) << glm::to_string(node->min) << glm::to_string(node->max) << std::endl;

    if (mid - start >= 1) {
        node->left = makeKD(nextLayer, start, mid - 1);
    }
    if (end - mid >= 1) {
        node->right = makeKD(nextLayer, mid + 1, end);
    }

    return node;
}

// Check every node for intersection.
surface checkIntersect(const SceneNode &node, ray r) {
    surface ret;

    r.E = glm::vec3(glm::inverse(node.trans) * glm::vec4(r.E, 1.0f));
    r.P = glm::vec3(glm::inverse(node.trans) * glm::vec4(r.P, 1.0f));
    r.trans = r.trans * node.trans;

    for (SceneNode *n: node.children) {
        // Recursive intersection check
        surface s;

        // Child intersection
        s = checkIntersect(*n, r);
        if (!s.intersected) {
        } else if (!ret.intersected) {
            ret = s;
        } else if (s.t < ret.t) {
            ret = s;
        }

        // Check intersection for geometry nodes only
        if (n->m_nodeType == NodeType::GeometryNode) {
            GeometryNode *gNode = static_cast<GeometryNode *>(n);
            s = gNode->intersection(r);
            if (!s.intersected) {
                continue;
            } else if (!ret.intersected) {
                ret = s;
            } else if (s.t < ret.t) {
                ret = s;
            }
        }
    }
    return ret;
}

/* Given parameters about the surface of an intersection point, the light sources, and a list of scene nodes, determine the colour of the intersection point.
 *  A list of scene nodes is required in order to determine whether a shadow is cast over the point.
 */
std::random_device rd;
std::mt19937 mt(rd());

glm::vec3 getColour(surface s, const std::list<Light *> &lights, SceneNode *root, glm::vec3 ambient) {
    glm::vec3 Lout(0);
    float distRange = 1;
    std::uniform_real_distribution<double> dist(-distRange, distRange);

    // Display photon mapping if enabled
    if (ShowPhotonMapping && donePM) {
        uint numWithin = 0;
        glm::vec3 pColour(0.0f);

        for (photon p : pList) {
            //std::cout << glm::distance( s.intersect_pt, p.pos ) << std::endl;
            if (glm::distance(s.intersect_pt, p.pos) < 0.01) {
                pColour += p.intensity;
                numWithin++;
            }
        }
        if (numWithin) {
            pColour /= numWithin;
            return pColour;
        }
    }

    ray lR;
    lR.E = s.intersect_pt;
    lR.origE = s.intersect_pt;
    for (auto *l : lights) {
        // Check light ray intersection.
        glm::vec3 colAccumulator(0);
        uint numConts = 0;
        bool moveOn = (SoftShad == 1) ? true : false;
        for (int i = 0; i < SoftShad; i++) {
            if (moveOn) lR.P = l->position;
            else lR.P = l->position + glm::normalize(glm::vec3(dist(mt), -glm::abs(dist(mt)), dist(mt))) / (1/distRange);

            if (checkIntersect(*root, lR).intersected) {
                numConts++;
                continue;
            } else if (numConts == 0 && i > SoftShad / 4) {
                // If no shadows are being hit, just move onto the next pixel
                moveOn = true;
            }

            glm::vec3 l_dir = glm::normalize(lR.P - lR.E);
            double light_dist = glm::length(lR.P - lR.E);

            glm::vec3 r = -l_dir + 2 * glm::dot(l_dir, s.n) * s.n;
            glm::vec3 col = s.mat->get_kd() * glm::dot(l_dir, s.n) * l->colour
                            + s.mat->get_ks() * pow(glm::dot(r, s.v), s.mat->get_shininess()) * l->colour;

            col /= (l->falloff[0] + l->falloff[1] * light_dist + l->falloff[2] * light_dist * light_dist);

            col[0] = glm::max(col[0], 0.0f);
            col[1] = glm::max(col[1], 0.0f);
            col[2] = glm::max(col[2], 0.0f);

            colAccumulator += col;

            if (moveOn) {
                Lout += col;
                break;
            }
        }
        if (!moveOn) {
            colAccumulator /= SoftShad;
            Lout += colAccumulator;
        }
    }

    // Add ambient light
    Lout += s.mat->get_kd() * ambient;

    // Photon colors
    if( MaxPhotons > 0 && !ShowPhotonMapping ) {
        std::vector<photon> pClosest = getNClosest(s.intersect_pt);
        glm::vec3 photonCol(0.0f);
        float maxDist = 0.0f;
        for (auto p: pClosest) {
            photonCol += (glm::dot( -p.dir, p.n ) * Lout * ( p.intensity /*/ pList.size()*/ ) );
            //std::cout << glm::to_string(p.dir) << glm::to_string(p.n) << glm::dot( -p.dir, p.n ) << std::endl;
            maxDist = glm::max(maxDist, glm::distance(p.pos, s.intersect_pt));
        }

        //std::cout << glm::to_string(Lout) << std::endl;
        Lout = photonCol / ClosestN;
    }

    return Lout;
}

// Given a ray, a surface normal, and two refractive indices n and nt, calculate refracted ray from n to nt
glm::vec3 getRefractedRay(glm::vec3 r, surface s, std::stack<double> &rIndexes) {
    double n;
    double nt;

    double angle = acos(glm::dot(glm::reflect(r, s.n), s.n));
    if (angle > glm::half_pi<double>()) {
        // case where ray goes out of object

        //std::cout << dot(rayDir, s.n) << " " << acos( glm::dot(rayDir, s.n) ) << std::endl;
        s.n = -s.n;

        n = rIndexes.top();
        rIndexes.pop();
        nt = rIndexes.top();

    } else {
        // case where ray goes into object
        n = rIndexes.top();
        nt = s.refractiveness;
        rIndexes.push(s.refractiveness);
    }

    // use refraction formula to get new ray direction
    glm::vec3 t = n * (r - s.n * dot(r, s.n)) / nt -
                  s.n * sqrt(1 - pow(n, 2) * (1 - pow(glm::dot(r, s.n), 2)) / pow(nt, 2));

    return t;
}


glm::vec3 refraction(glm::vec3 surfaceColour, glm::vec3 rayDir, surface s, std::stack<double> &rIndexes,
                     const std::list<Light *> &lights, SceneNode *root, glm::vec3 ambient) {
    // get refractive indices

    // Check if normal is facing away. If it is, then flip it.
    glm::vec3 t = getRefractedRay(rayDir, s, rIndexes);

    // check if the new ray intersects any objects
    ray refR;
    refR.origE = s.intersect_pt;
    refR.E = s.intersect_pt;
    refR.P = refR.E + t;
    surface refS = checkIntersect(*root, refR);

    // If ray intersects an object with transparency, recursively call function
    if (refS.intersected && refS.transparency > 0) {
        glm::vec3 nextSurfaceColour = getColour(refS, lights, root, ambient);
        return (1 - s.transparency) * surfaceColour +
               s.transparency * refraction(nextSurfaceColour, t, refS, rIndexes, lights, root, ambient);
    } else if (refS.intersected) {
        // If ray intersects an object with no transparency, compute colour and return
        return getColour(refS, lights, root, ambient);
    } else {
        return glm::vec3(0);
    }

    /*mtx.lock();
    //std::cout << glm::dot( rayDir, s.n) << std::endl;
    //std::cout << glm::to_string( s.n ) << std::endl;
    mtx.unlock();*/
}


// Get the colour of the reflection cast from a surface. There will only be one reflective iteration.
glm::vec3
getReflection(glm::vec3 surfaceColour, glm::vec3 eye, surface s, SceneNode *root, const std::list<Light *> &lights,
              glm::vec3 ambient) {
    glm::vec3 col;

    // Get reflected eye ray
    glm::vec3 cReflect = glm::normalize(eye - s.intersect_pt);
    cReflect = -cReflect + 2 * glm::dot(cReflect, s.n) * s.n;

    // reflected ray
    ray rR;
    rR.E = s.intersect_pt;
    rR.origE = s.intersect_pt;
    rR.P = s.intersect_pt + cReflect;

    // check intersection of reflected intersection
    surface rS = checkIntersect(*root, rR);
    uint timesInt = 0;
    if (rS.intersected) {
        col = getColour(rS, lights, root, ambient);
        timesInt++;
    } else {
        col = glm::vec3(0.0f);
    }
    if (GlossAmt != 1) {
        std::uniform_real_distribution<double> dist(-1.0f, 1.0f);
        for (int i = 1; i < GlossAmt; i++) {
            rR.P = (s.intersect_pt + cReflect * 25) + glm::normalize(glm::vec3(dist(mt), dist(mt), dist(mt)));
            rS = checkIntersect(*root, rR);
            if (rS.intersected) {
                timesInt++;
                glm::vec3 temp = getColour(rS, lights, root, ambient);
                //std::cout << glm::to_string( rS.mat->get_kd() ) << glm::to_string(temp) << std::endl;
                col += temp;
            } else {
                col += surfaceColour;
            }
        }
        if (timesInt == 0) return glm::vec3(0);

        col /= GlossAmt;
        //std::cout << glm::to_string( col ) << std::endl;

    }

    if( s.metallic ) {
        col = glm::vec3( 0.2126 * col.r + 0.7152 * col.g + 0.0722 * col.b );
    }

    return col;
}

// Cast a ray from the eye to a projected x and y point relative to a screen denoted by topLeft.
glm::vec3 castRayTo(float x, float y,
                    size_t h, size_t w,
                    glm::vec3 eye, glm::vec3 topLeft, double pixelSize,
                    SceneNode *root, const std::list<Light *> &lights,
                    glm::vec3 ambient) {
    ray R;
    glm::vec3 P(topLeft.x + pixelSize * x, topLeft.y - pixelSize * y, topLeft.z);
    R.origE = eye;
    R.E = eye;
    R.P = P;
    // Check if ray intersects object
    surface s = checkIntersect(*root, R);
    // if it intersects, calculate the colour through lighting
    if (s.intersected) {

        // Get surface colour
        glm::vec3 Lout = getColour(s, lights, root, ambient);

        // Do reflection
        if (reflect && s.mat->get_ks().x != 0 && s.mat->get_ks().y != 0 && s.mat->get_ks().z != 0) {
            glm::vec3 Lout2 = getReflection(Lout, eye, s, root, lights, ambient);
            if (Lout2.x != -1 && Lout2.y != -1 && Lout2.z != -1)
                Lout = (1 - s.reflectiveness) * Lout + s.reflectiveness * Lout2;
        }

        // Do refraction
        if (s.transparency > 0) {
            std::stack<double> rIndexes;
            rIndexes.push(1.0);
            Lout = refraction(Lout, glm::normalize(R.P - R.E), s, rIndexes, lights, root, ambient);
        }

        /*for( int i = 0; i < 1; i++ ) {
            double n = 1;
            double nt = 1.2;
            glm::vec3 t = refraction(glm::normalize(R.P - R.E), s, n, nt, root);

            ray refR;
            refR.origE = s.intersect_pt;
            refR.E = s.intersect_pt;
            refR.P = refR.E + t*//* s.intersect_pt + (R.P - R.E)*//*;
            surface refS = checkIntersect( *root, refR );

            t = refraction( t, refS, nt, n, root );

            mtx.lock();
            //std::cout << glm::to_string( t );
            //std::cout << glm::to_string( glm::normalize( R.P - R.E ) ) << std::endl;
            mtx.unlock();

            refR.E = refS.intersect_pt;
            refR.origE = refS.intersect_pt;
            refR.P = refR.E + t*//* refS.intersect_pt + (R.P - R.E)*//*;
            refS = checkIntersect( *root, refR );

            if( refS.intersected ) {
                glm::vec3 Lout3 = getColour( refS, lights, root, ambient );
                if( refractMap ) Lout = glm::vec3(1, 0, 0);
                else Lout = *//*0.5 * Lout + 0.5 **//* Lout3;
            } else {
                if( refractMap ) Lout = glm::vec3(0, 0, 1);
                else Lout = glm::vec3(0);
            }
        }*/

        return Lout;
    } else {
        glm::vec3 Lout;

        Lout.x = 0;
        Lout.y = (float) glm::max(1 - (y / h * 1.3), 0.2);
        Lout.z = (float) glm::max(1 - (y / h * 1.3), 0.2);

        //std::cout << y << " " << h << " " << superPoints[x][y][2] << std::endl;

        float invX = w - x;

        if (invX * invX + y * y < (w / 5.0) * (w / 5.0)) {
            Lout.x = 1;
            Lout.y = 1;
            Lout.z = 0;
        }

        return Lout;
    }
}

/*********************************** Photon Mapping (First Pass) **********************************/

// Decide photon path via russian roulette until it is absorbed or lost.
photon tracePhoton(photon p, SceneNode *root, std::stack<double> &rIndexes, uint bounces) {
    std::uniform_real_distribution<double> dist(0.0f, 1.0f);

    // If photon bounced 10 times, absorb
    if (bounces <= 0) {
        return p;
    }

    ray r(p.pos, p.pos + p.dir);
    surface s = checkIntersect(*root, r);

    // return if photon fails to intersect
    if (!s.intersected) {
        p.lost = true;
        return p;
    }

    // probabilities of each action

    // Diffuse reflection
    double Pd = glm::max(s.mat->get_kd().r * p.intensity.r,
                         glm::max(s.mat->get_kd().g * p.intensity.g, s.mat->get_kd().b * p.intensity.b)) /
                glm::max(p.intensity.r, glm::max(p.intensity.g, p.intensity.b));
    // Specular reflection
    double Ps = glm::max(s.mat->get_ks().r * p.intensity.r,
                         glm::max(s.mat->get_ks().g * p.intensity.g, s.mat->get_ks().b * p.intensity.b)) /
                glm::max(p.intensity.r, glm::max(p.intensity.g, p.intensity.b));

    // Refraction
    double Pr = glm::max(s.transparency * p.intensity.r,
                         glm::max(s.transparency * p.intensity.g, s.transparency * p.intensity.b)) /
                glm::max(p.intensity.r, glm::max(p.intensity.g, p.intensity.b));

    // TEST
    //std::cout << Pd << " + " << Ps << " + " << Pr << " = " << Pd+Ps+Pr << std::endl;

    // Choose action
    double Xi = dist(mt); // random double between 0 and 1.

    uint action;
    if (Xi <= Pd) {
        // Diffuse reflection chosen

        // Adjust photon intensity
        p.intensity = p.intensity * (s.mat->get_kd() / Pd);
        p.pos = s.intersect_pt;
        p.n = s.n;
        p.dir = glm::reflect(p.dir, s.n);

        // Add photon to ulist
        mtx.lock();
        pList.push_back(p);
        mtx.unlock();

        return tracePhoton(p, root, rIndexes, bounces - 1);
    } else if (Xi <= Ps + Pd) {
        // Specular reflection chosen

        // Adjust photon intensity
        p.intensity = p.intensity * (s.mat->get_ks() / Ps);
        p.pos = s.intersect_pt;
        p.n = s.n;
        p.dir = glm::reflect(p.dir, s.n);

        // Add photon to ulist
        mtx.lock();
        pList.push_back(p);
        mtx.unlock();

        return tracePhoton(p, root, rIndexes, bounces - 1);
    } else if (Xi <= Ps + Pd + Pr) {
        // Refraction chosen

        // Adjust photon intensity
        p.intensity = p.intensity * (s.transparency / Pr);
        p.pos = s.intersect_pt;
        p.n = s.n;
        p.dir = getRefractedRay(p.dir, s, rIndexes);

        // Add photon to ulist
        mtx.lock();
        pList.push_back(p);
        mtx.unlock();

        return tracePhoton(p, root, rIndexes, bounces - 1);
    } else if (Xi <= 1) {
        // Absorption chosen
        p.pos = s.intersect_pt;
        p.n = s.n;
        p.dir = glm::reflect( p.dir, s.n );
        return p;
    } else {
        // Error case
        std::cerr << "How can this be? Why is Xi = " << Xi << "?" << std::endl;
        exit(1);
    }
}

// Cast photons from light source & trace until it is absorbed.
void castPhoton(Light l, SceneNode *root, uint &numPhotons) {
    std::uniform_real_distribution<double> dist(-1.0f, 1.0f);

    // Cast photon in random direction from light
    while (numPhotons < MaxPhotons) {
        photon p;
        // generate direction TODO: should it be uniform or not (pixar 51)
        p.dir = glm::normalize(glm::vec3(dist(mt), dist(mt), dist(mt)));

        p.pos = l.position;
        p.intensity = l.colour;

        std::stack<double> rIndexes;
        rIndexes.push(1.0f);

        // Trace photon to a termination point (or until lost)
        p = tracePhoton(p, root, rIndexes, 10);

        // continue without incrementing count if photon has been lost with a 50% chance
        if (p.lost) {
            if (dist(mt) < 0) numPhotons++;
            continue;
        }
        std::cout << pList.size() << '\r' << std::flush;

        mtx.lock();
        // Add photon to ulist
        pList.push_back(p);

        numPhotons++;
        mtx.unlock();
    }
    std::cout << std::endl;
    // TODO: Scale photon powers
}

/**************************************************************************************************/

// Make the image.
void makeImage(glm::vec3 **superPoints, Image &image,
               size_t h, size_t w,
               uint &globalx, uint &globaly,
               glm::vec3 eye, glm::vec3 topLeft, double pixelSize,
               SceneNode *root, const std::list<Light *> &lights,
               glm::vec3 ambient) {

    while (1) {
        // set x and y
        uint x, y;
        mtx.lock();
        x = globalx;
        y = globaly;

        if (x < w - 1) {
            globalx++;
        } else if (y < h - 1) {
            globalx = 0;
            globaly++;

            loading(y * ((Adaptive == 1) ? 100 : 50) / h);
        } else {
            mtx.unlock();
            return;
        }
        mtx.unlock();

        glm::vec3 Lout = castRayTo(x, y, h, w, eye, topLeft, pixelSize, root, lights, ambient);

        if (super) {
            superPoints[x][y][0] = Lout.x;
            superPoints[x][y][1] = Lout.y;
            superPoints[x][y][2] = Lout.z;
        } else {
            image(x, y, 0) = Lout.x;
            image(x, y, 1) = Lout.y;
            image(x, y, 2) = Lout.z;
        }
    }
}

// Get colour difference of two pixels.
float getColourDiff(Image &image, uint x1, uint y1, uint x2, uint y2) {
    if (x1 < 0 || x2 < 0 || y1 < 0 || y2 < 0) {
        std::cerr << "ERROR: Indices out of bounds when retrieving colour difference." << std::endl;
        exit(1);
    }

    return float(glm::sqrt(
            glm::pow(image(x1, y1, 0) - image(x2, y2, 0), 2) + glm::pow(image(x1, y1, 1) - image(x2, y2, 1), 2) +
            glm::pow(image(x1, y1, 2) - image(x2, y2, 2), 2)));
}


void adaptiveAA(Image &aImage, Image &image,
                size_t w, size_t h,
                uint &globalx, uint &globaly,
                glm::vec3 eye, glm::vec3 topLeft, double pixelSize,
                SceneNode *root, const std::list<Light *> &lights,
                glm::vec3 ambient) {
    while (1) {
        // set x and y
        uint x, y;
        mtx.lock();
        x = globalx;
        y = globaly;

        if (x < w - 1) {
            globalx++;
        } else if (y < h - 1) {
            globalx = 0;
            globaly++;

            loading(50 + (y * 50) / h);
        } else {
            mtx.unlock();
            return;
        }
        mtx.unlock();

        float colourDiff = 0;
        // cmp with left
        if (x != 0) {
            colourDiff = glm::max(colourDiff, getColourDiff(image, x, y, x - 1, y));
        }
        // cmp with right
        if (x != w - 1) {
            colourDiff = glm::max(colourDiff, getColourDiff(image, x, y, x + 1, y));
        }
        // cmp with bottom
        if (y != 0) {
            colourDiff = glm::max(colourDiff, getColourDiff(image, x, y, x, y - 1));
        }
        // cmp with top
        if (y != h - 1) {
            colourDiff = glm::max(colourDiff, getColourDiff(image, x, y, x, y + 1));
        }

        if (colourDiff > 0.1) {
            if (showAdaptive) {
                aImage(x, y, 0) = 0;
                aImage(x, y, 1) = 1;
                aImage(x, y, 2) = 0;
            }

            // Sample points and take average. Point locations taken from (https://docs.microsoft.com/en-us/windows/desktop/api/d3d11/ne-d3d11-d3d11_standard_multisample_quality_levels)
            glm::vec3 sampledPoints(0);
            if (Adaptive == 2) {
                sampledPoints += castRayTo(x + 4.0f / 16, y + 4.0f / 16, h, w, eye, topLeft, pixelSize, root, lights,
                                           ambient);
                sampledPoints += castRayTo(x + -4.0f / 16, y + -4.0f / 16, h, w, eye, topLeft, pixelSize, root, lights,
                                           ambient);
                sampledPoints /= 2;
            } else if (Adaptive == 4) {
                sampledPoints += castRayTo(x + -2.0f / 16, y + -6.0f / 16, h, w, eye, topLeft, pixelSize, root, lights,
                                           ambient);
                sampledPoints += castRayTo(x + 6.0f / 16, y + -2.0f / 16, h, w, eye, topLeft, pixelSize, root, lights,
                                           ambient);
                sampledPoints += castRayTo(x + -6.0f / 16, y + 2.0f / 16, h, w, eye, topLeft, pixelSize, root, lights,
                                           ambient);
                sampledPoints += castRayTo(x + 2.0f / 16, y + 6.0f / 16, h, w, eye, topLeft, pixelSize, root, lights,
                                           ambient);
                sampledPoints /= 4;
            } else if (Adaptive == 8) {
                sampledPoints += castRayTo(x + 1.0f / 16, y + -3.0f / 16, h, w, eye, topLeft, pixelSize, root, lights,
                                           ambient);
                sampledPoints += castRayTo(x + -1.0f / 16, y + -3.0f / 16, h, w, eye, topLeft, pixelSize, root, lights,
                                           ambient);
                sampledPoints += castRayTo(x + 5.0f / 16, y + 1.0f / 16, h, w, eye, topLeft, pixelSize, root, lights,
                                           ambient);
                sampledPoints += castRayTo(x + -3.0f / 16, y + -5.0f / 16, h, w, eye, topLeft, pixelSize, root, lights,
                                           ambient);
                sampledPoints += castRayTo(x + -5.0f / 16, y + 5.0f / 16, h, w, eye, topLeft, pixelSize, root, lights,
                                           ambient);
                sampledPoints += castRayTo(x + -7.0f / 16, y + 1.0f / 16, h, w, eye, topLeft, pixelSize, root, lights,
                                           ambient);
                sampledPoints += castRayTo(x + 3.0f / 16, y + 7.0f / 16, h, w, eye, topLeft, pixelSize, root, lights,
                                           ambient);
                sampledPoints += castRayTo(x + 7.0f / 16, y + -7.0f / 16, h, w, eye, topLeft, pixelSize, root, lights,
                                           ambient);
                sampledPoints /= 8;
            } else if (Adaptive == 16) {
                sampledPoints += castRayTo(x + 1.0f / 16, y + 1.0f / 16, h, w, eye, topLeft, pixelSize, root, lights,
                                           ambient);
                sampledPoints += castRayTo(x + -1.0f / 16, y + -3.0f / 16, h, w, eye, topLeft, pixelSize, root, lights,
                                           ambient);
                sampledPoints += castRayTo(x + -3.0f / 16, y + 2.0f / 16, h, w, eye, topLeft, pixelSize, root, lights,
                                           ambient);
                sampledPoints += castRayTo(x + 4.0f / 16, y + -1.0f / 16, h, w, eye, topLeft, pixelSize, root, lights,
                                           ambient);
                sampledPoints += castRayTo(x + -5.0f / 16, y + -2.0f / 16, h, w, eye, topLeft, pixelSize, root, lights,
                                           ambient);
                sampledPoints += castRayTo(x + 2.0f / 16, y + 5.0f / 16, h, w, eye, topLeft, pixelSize, root, lights,
                                           ambient);
                sampledPoints += castRayTo(x + 5.0f / 16, y + 3.0f / 16, h, w, eye, topLeft, pixelSize, root, lights,
                                           ambient);
                sampledPoints += castRayTo(x + 3.0f / 16, y + -5.0f / 16, h, w, eye, topLeft, pixelSize, root, lights,
                                           ambient);
                sampledPoints += castRayTo(x + -2.0f / 16, y + 6.0f / 16, h, w, eye, topLeft, pixelSize, root, lights,
                                           ambient);
                sampledPoints += castRayTo(x + 0.0f / 16, y + -7.0f / 16, h, w, eye, topLeft, pixelSize, root, lights,
                                           ambient);
                sampledPoints += castRayTo(x + -4.0f / 16, y + -6.0f / 16, h, w, eye, topLeft, pixelSize, root, lights,
                                           ambient);
                sampledPoints += castRayTo(x + -6.0f / 16, y + 4.0f / 16, h, w, eye, topLeft, pixelSize, root, lights,
                                           ambient);
                sampledPoints += castRayTo(x + -8.0f / 16, y + 0.0f / 16, h, w, eye, topLeft, pixelSize, root, lights,
                                           ambient);
                sampledPoints += castRayTo(x + 7.0f / 16, y + -4.0f / 16, h, w, eye, topLeft, pixelSize, root, lights,
                                           ambient);
                sampledPoints += castRayTo(x + 6.0f / 16, y + 7.0f / 16, h, w, eye, topLeft, pixelSize, root, lights,
                                           ambient);
                sampledPoints += castRayTo(x + -7.0f / 16, y + -8.0f / 16, h, w, eye, topLeft, pixelSize, root, lights,
                                           ambient);
                sampledPoints /= 16;
            } else {
                std::cerr << "ERROR: Antialiasing must be one of {1, 2, 4, 8, 16}." << std::endl;
            }

            aImage(x, y, 0) = sampledPoints.r;
            aImage(x, y, 1) = sampledPoints.g;
            aImage(x, y, 2) = sampledPoints.b;

        }
    }
}


// TEST multithreading
void test(uint &x, uint &y) {
    while (1) {
        //std::cout << x << " " << y << " " << std::this_thread::get_id() << std::endl;

        for (int i = 0; i < 1000; i++) {
            std::pow(std::log(x * y), y);
        }

        mtx.lock();
        if (x < 256) {
            x++;
        } else if (y < 256) {
            y++;
            x = 0;
        } else {
            mtx.unlock();
            return;
        }
        mtx.unlock();
    }

}

void A5_Render(
        // What to render
        SceneNode *root,

        // Image to write to, set to a given width and height
        Image &image,

        // Viewing parameters
        const glm::vec3 &eye,
        const glm::vec3 &view,
        const glm::vec3 &up,
        double fovy,

        // Lighting parameters
        const glm::vec3 &ambient,
        const std::list<Light *> &lights
) {

    // Fill in raytracing code here...
    double aspect = (double) image.width() / image.height();
    glm::vec3 nView = glm::normalize(view);
    const glm::vec3 &right = glm::cross(view, up) / glm::length(glm::cross(view, up));

    // get virtual screen parameters
    double screenDist = 1;
    double vHeight = 2 * tan(glm::radians(fovy) / 2) * screenDist;
    double vWidth = vHeight * aspect;

    //std::cout << vHeight << std::endl;

    double pixelSize = vWidth / image.width();

    //glm::vec3 topLeft = eye + view - vWidth/2 * right + vHeight/2 * up + pixelSize / 2 * right + pixelSize / 2 * up;
    glm::vec3 topLeft = eye + nView - vWidth / 2 * right + vHeight / 2 * up;

    if (!super) {
        topLeft += pixelSize / 2 * right + pixelSize / 2 * up;
    }

    std::cout << "Calling A5_Render(\n" <<
              "\t" << *root <<
              "\t" << "numthreads: " << (mthread ? std::thread::hardware_concurrency() : 1) << std::endl <<
              "\t" << "Supersampling: " << (super ? "ON" : "OFF") << std::endl <<
              "\t" << "Image(width:" << image.width() << ", height:" << image.height() << ")\n"
                                                                                          "\t" << "eye:  "
              << glm::to_string(eye) << std::endl <<
              "\t" << "view: " << glm::to_string(view) << std::endl <<
              "\t" << "up:   " << glm::to_string(up) << std::endl <<
              "\t" << "right:" << glm::to_string(right) << std::endl <<
              "\t" << "fovy: " << fovy << std::endl <<
              "\t" << "ambient: " << glm::to_string(ambient) << std::endl <<
              "\t" << "lights{" << std::endl;

    for (const Light *light : lights) {
        std::cout << "\t\t" << *light << std::endl;
    }
    std::cout << "\t}" << std::endl;
    std::cout << ")" << std::endl;

    size_t h = image.height();
    size_t w = image.width();

    // constructing superPoints array
    glm::vec3 **superPoints/*[w+1][h+1]*/;
    superPoints = new glm::vec3 *[w + 1];
    for (int i = 0; i <= w; i++) {
        superPoints[i] = new glm::vec3[h + 1];
    }

    if (super) {
        h++;
        w++;
    }


    // Make an array of threads
    int numThreads = mthread ? std::thread::hardware_concurrency() : 1;
    std::thread renderThread[numThreads];

    if (MaxPhotons > 0) {
        for (auto *l : lights) {
            uint numPhotons = 0;
            // Photon mapping (first pass)
            for (int i = 0; i < numThreads; i++) {
                renderThread[i] = std::thread(castPhoton, *l, root, std::ref(numPhotons));
            }
            for (int i = 0; i < numThreads; i++) {
                renderThread[i].join();
            }
        }
    }
    donePM = true;

    // Sort photons in KD tree
    //pTree = makeKD( 'x', 0, (uint)pList.size() );

    // TEST Priority Queue (HEAP)
    /*std::vector<photon> closest = getNClosest(glm::vec3(0.0f));
    for (photon p: closest) {
        std::cout << glm::to_string(p.pos) << glm::distance(glm::vec3(0.0f), p.pos) << std::endl;
    }*/


    uint globalx = 0;
    uint globaly = 0;
    // Render each pixel concurrently
    for (int i = 0; i < numThreads; i++) {
        //Test: renderThread[i] = std::thread(test, std::ref(x), std::ref(y));
        renderThread[i] = std::thread(makeImage, superPoints, std::ref(image), h, w, std::ref(globalx),
                                      std::ref(globaly), eye, topLeft, pixelSize, root, lights, ambient);
    }
    for (int i = 0; i < numThreads; i++) {
        renderThread[i].join();
    }

    // handle supersampling
    if (super) {
        for (uint y = 0; y < h; y++) {
            for (uint x = 0; x < w; x++) {
                if (x >= 1 && y >= 1) {
                    for (uint i = 0; i < 3; i++) {
                        image(x - 1, y - 1, i) = (superPoints[x - 1][y - 1][i]
                                                  + superPoints[x - 1][y][i]
                                                  + superPoints[x][y - 1][i]
                                                  + superPoints[x][y][i]) / 4;
                        //std::cout << image( x-1, y-1, i ) << " ";
                    }
                    //std::cout << std::endl;
                }
            }
        }
    }

    // concurrently do adaptive antialiased image
    if (Adaptive > 1) {
        Image aImage = image;
        globalx = 0;
        globaly = 0;
        for (int i = 0; i < numThreads; i++) {
            //Test: renderThread[i] = std::thread(test, std::ref(x), std::ref(y));
            renderThread[i] = std::thread(adaptiveAA, std::ref(aImage), std::ref(image), w, h, std::ref(globalx),
                                          std::ref(globaly), eye, topLeft, pixelSize, root, lights, ambient);
        }
        for (int i = 0; i < numThreads; i++) {
            renderThread[i].join();
        }
        image = aImage;
    }

    if (super) {
        // set width and height back to correct values
        h--;
        w--;
    }

    loading(100);

    // TODO: Delete superpts.

}
