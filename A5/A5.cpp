// Fall 2018

#include <glm/ext.hpp>
#include <thread>
#include <mutex>

#include "A5.hpp"
#include "GeometryNode.hpp"

// Check every node for intersection.
surface checkIntersect( const SceneNode &node, ray r ) {
	surface ret;

    r.E = glm::vec3( glm::inverse( node.trans ) * glm::vec4( r.E, 1.0f ) );
    r.P = glm::vec3( glm::inverse( node.trans ) * glm::vec4( r.P, 1.0f ) );
    r.trans = r.trans * node.trans;

	for( SceneNode *n: node.children ) {
		// Recursive intersection check
        surface s;

        // Child intersection
		s = checkIntersect( *n, r );
        if( !s.intersected ) {
        } else if( !ret.intersected ) {
            ret = s;
        } else if( s.t < ret.t ) {
            ret = s;
        }

		// Check intersection for geometry nodes only
		if( n->m_nodeType == NodeType::GeometryNode ) {
			GeometryNode *gNode =  static_cast<GeometryNode *>(n);
			s = gNode->intersection( r );
			if( !s.intersected ) {
				continue;
			} else if( !ret.intersected ) {
				ret = s;
			} else if( s.t < ret.t ) {
				ret = s;
			}
		}
	}
	return ret;
}

/* Given parameters about the surface of an intersection point, the light sources, and a list of scene nodes, determine the colour of the intersection point.
 *  A list of scene nodes is required in order to determine whether a shadow is cast over the point.
 */
glm::vec3 getColour( surface s, const std::list<Light *> & lights, SceneNode *root, glm::vec3 ambient ) {
    glm::vec3 Lout(0);

    for( auto *l : lights ) {
        // Check light ray intersection.
        ray lR;
        lR.E = s.intersect_pt;
        lR.origE = s.intersect_pt;
        //std::cout << glm::to_string( s.intersect_pt ) << std::endl;
        lR.P = l->position;
        //lR.C = l_dir;
        if (checkIntersect(*root, lR).intersected) continue;

        glm::vec3 l_dir = glm::normalize(lR.P - lR.E);
        double light_dist = glm::length(lR.P - lR.E);

        glm::vec3 r = -l_dir + 2 * glm::dot(l_dir, s.n) * s.n;
        glm::vec3 col = s.mat->get_kd() * glm::dot(l_dir, s.n) * l->colour
                         + s.mat->get_ks() * pow(glm::dot(r, s.v), s.mat->get_shininess()) * l->colour;

        col /= (l->falloff[0] + l->falloff[1] * light_dist + l->falloff[2] * light_dist * light_dist);

        col[0] = glm::max(col[0], 0.0f);
        col[1] = glm::max(col[1], 0.0f);
        col[2] = glm::max(col[2], 0.0f);

        Lout += col;
    }

    // Add ambient light
    Lout += s.mat->get_kd() * ambient;

    return Lout;
}


std::mutex mtx;

void makeImage( glm::vec3 **superPoints, Image &image,
                size_t h, size_t w,
                uint &globalx, uint &globaly,
                glm::vec3 eye, glm::vec3 topLeft, double pixelSize,
                SceneNode *root, const std::list<Light *> & lights,
                glm::vec3 ambient ) {

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

            std::cout << globaly << std::endl;
        } else {
            mtx.unlock();
            return;
        }
        mtx.unlock();

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

            // get reflected eye unit vector
            glm::vec3 cReflect = glm::normalize(eye - s.intersect_pt);
            cReflect = -cReflect + 2 * glm::dot(cReflect, s.n) * s.n;

            // reflected ray
            ray rR;
            rR.E = s.intersect_pt;
            rR.origE = s.intersect_pt;
            rR.P = s.intersect_pt + cReflect;

            // check intersection of reflected intersection
            surface rS = checkIntersect(*root, rR);
            if (rS.intersected) {
                glm::vec3 Lout2 = getColour(rS, lights, root, ambient);

                // Add reflection colour
                Lout = (glm::vec3(1) - s.mat->get_ks()) * Lout + s.mat->get_ks() * Lout2;
            }

            if (super) {
                superPoints[x][y][0] = Lout.x;
                superPoints[x][y][1] = Lout.y;
                superPoints[x][y][2] = Lout.z;
            } else {
                image(x, y, 0) = Lout.x;
                image(x, y, 1) = Lout.y;
                image(x, y, 2) = Lout.z;
            }
        } else {
            if (super) {
                superPoints[x][y][0] = 0;
                superPoints[x][y][1] = glm::max(1 - ((float) y / h * 1.3), 0.2);
                superPoints[x][y][2] = glm::max(1 - ((float) y / h * 1.3), 0.2);

                //std::cout << y << " " << h << " " << superPoints[x][y][2] << std::endl;

                size_t invX = w - x;

                if (invX * invX + y * y < (w / 5) * (w / 5)) {
                    superPoints[x][y][0] = 1;
                    superPoints[x][y][1] = 1;
                    superPoints[x][y][2] = 0;
                }
            } else {
                image(x, y, 0) = 0;
                image(x, y, 1) = glm::max(1 - ((float) y / h * 1.3), 0.2);
                image(x, y, 2) = glm::max(1 - ((float) y / h * 1.3), 0.2);

                size_t invX = w - x;

                if (invX * invX + y * y < (w / 5) * (w / 5)) {
                    image(x, y, 0) = 1;
                    image(x, y, 1) = 1;
                    image(x, y, 2) = 0;
                }
            }
        }
    }
}


void test( uint &x, uint &y ) {
    while(1) {
        //std::cout << x << " " << y << " " << std::this_thread::get_id() << std::endl;

        for( int i = 0; i < 1000; i++ ) {
            std::pow(std::log(x*y), y);
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
) {

  // Fill in raytracing code here...
  double aspect = (double)image.width() / image.height();
  glm::vec3 nView = glm::normalize( view );
  const glm::vec3 &right = glm::cross( view, up ) / glm::length( glm::cross( view, up ) );

  // get virtual screen parameters
  double screenDist = 1;
  double vHeight = 2 * tan(glm::radians(fovy)/2) * screenDist;
  double vWidth = vHeight * aspect;

  //std::cout << vHeight << std::endl;

  double pixelSize = vWidth / image.width();

  //glm::vec3 topLeft = eye + view - vWidth/2 * right + vHeight/2 * up + pixelSize / 2 * right + pixelSize / 2 * up;
  glm::vec3 topLeft = eye + nView - vWidth/2 * right + vHeight/2 * up;

  if( !super ) {
      topLeft += pixelSize / 2 * right + pixelSize / 2 * up;
  }

  std::cout << "Calling A5_Render(\n" <<
		  "\t" << *root <<
		  "\t" << "Supersampling: " << ( super? "ON" : "OFF" ) << std::endl <<
          "\t" << "Image(width:" << image.width() << ", height:" << image.height() << ")\n"
          "\t" << "eye:  " << glm::to_string(eye) << std::endl <<
		  "\t" << "view: " << glm::to_string(view) << std::endl <<
		  "\t" << "up:   " << glm::to_string(up) << std::endl <<
          "\t" << "right:" << glm::to_string(right) << std::endl <<
		  "\t" << "fovy: " << fovy << std::endl <<
          "\t" << "ambient: " << glm::to_string(ambient) << std::endl <<
		  "\t" << "lights{" << std::endl;

	for(const Light * light : lights) {
		std::cout << "\t\t" <<  *light << std::endl;
	}
	std::cout << "\t}" << std::endl;
	std:: cout <<")" << std::endl;

	std::cout << std::endl << "num threads: " << std::thread::hardware_concurrency() << std::endl;

	size_t h = image.height();
	size_t w = image.width();

	// constructing superPoints array
    glm::vec3 **superPoints/*[w+1][h+1]*/;
    superPoints = new glm::vec3 *[w+1];
    for( int i = 0; i <= w; i++ ) {
        superPoints[i] = new glm::vec3[h+1];
    }

	if ( super ) {
        h++;
        w++;
	}

	// Make an array of threads
	int numThreads = std::thread::hardware_concurrency();
	//numThreads = 1;
    std::thread renderThread[numThreads];

    uint globalx = 0;
    uint globaly = 0;

    // Render each pixel concurrently
    for( int i = 0; i < numThreads; i++ ) {
        //Test: renderThread[i] = std::thread(test, std::ref(x), std::ref(y));
        renderThread[i] = std::thread( makeImage, superPoints, std::ref(image), h, w, std::ref(globalx), std::ref(globaly), eye, topLeft, pixelSize, root, lights, ambient );
    }
    for( int i = 0; i < numThreads; i++ ) {
        renderThread[i].join();
    }

    // handle supersampling
    if( super ) {
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



	if( super ) {
		// set width and height back to correct values
		h--;
		w--;
	}

}
