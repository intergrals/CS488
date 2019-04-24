// Fall 2018

#pragma once

#include <glm/glm.hpp>

#include "SceneNode.hpp"
#include "Light.hpp"
#include "Image.hpp"
#include "PhongMaterial.hpp"

void A5_Render(
		// What to render
		SceneNode * root,

		// Image to write to, set to a given width and height
		Image & image,

		// Viewing parameters
		const glm::vec3 & eye,
		const glm::vec3 & view,
		const glm::vec3 & up,
		double fovy,

		// Lighting parameters
		const glm::vec3 & ambient,
		const std::list<Light *> & lights
);

static const double Epsilon = 0.01;
static const bool super = false;
static const bool bound = false;
static const bool ShowPhotonMapping = true;
static const bool refractMap = false;
static const bool reflect = true;
static const bool mthread = true;
static const bool showAdaptive = false;
static const uint Adaptive = 1;
static const uint SoftShad = 1;
static const uint GlossAmt = 1;
static const uint MaxPhotons = 0;
static const uint ClosestN = 500;


class surface {
public:
    //Primitive &shape;
    bool intersected = false;
    double t;
    glm::vec3 intersect_pt;
    glm::vec3 n;
    glm::vec3 v;
    PhongMaterial *mat = nullptr;

    bool metallic;
    double reflectiveness;
    double refractiveness;
    double transparency;
    //glm::mat4 trans;
};

class ray {
public:
    ray() {}
	ray( glm::vec3 E, glm::vec3 P ) : E(E), P(P) { origE = E; }
    glm::vec3 origE;                        // Original eye position
    glm::vec3 E;                            // Eye / starting point
    glm::vec3 P;			                // Screen point
    glm::vec3 C;                            // Ray direction
    double tmin = 0;                        // minimum t
    double tmax = INFINITY;                 // maximum t
    glm::mat4 trans = glm::mat4(1.0f);      // transformations applied to ray
};

class photon {
public:
    bool lost = false;                      // whether the photon has been lost (cast into a space with no intersection)
	glm::vec3 pos;							// photon position
	glm::vec3 n;							// surface normal
	glm::vec3 dir;							// direction of travel
	glm::vec3 intensity = glm::vec3(1.0f);	// colour intensity

	uint sortIn = 0;
};

class TreeNode {
public:
    // Constructor overload
    TreeNode() {
        min = glm::vec3(INFINITY);
        max = glm::vec3(-INFINITY);

        left = nullptr;
        right = nullptr;
    }
    TreeNode( photon p ) : TreeNode() { TreeNode::p = p; }
    TreeNode( photon p, char layer ) : TreeNode() {
        TreeNode::p = p;
        TreeNode::layer = layer;
    }
    // Destructor
    ~TreeNode() {
        delete left;
        delete right;
    }

    photon p;
    char layer;
    glm::vec3 min;
    glm::vec3 max;
    TreeNode *left;
    TreeNode *right;
};
