// Fall 2018

#include <glm/ext.hpp>
#include <thread>
#include <mutex>
#include <random>
#include <iomanip>

#include "A5.hpp"
#include "GeometryNode.hpp"


void loading( float percent ) {
    float p = percent;
    std::cout << '[';
    for( int i = 0; i < 25; i++ ) {
        if( percent >= 4 ) {
            std::cout << '#';
            percent -= 4;
        } else if(percent >= 2) {
            std::cout << '=';
            percent -= 2;
        } else {
            std::cout << '-';
        }
    }

    std::cout << ']';
    std::cout.width(4); std::cout << std::right << p << '%' << '\r' << std::flush;
}

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
std::random_device rd;
std::mt19937 mt(rd());

glm::vec3 getColour( surface s, const std::list<Light *> & lights, SceneNode *root, glm::vec3 ambient ) {
    glm::vec3 Lout(0);
    float distRange = 1;
    std::uniform_real_distribution<double> dist( -distRange, distRange );

    ray lR;
    lR.E = s.intersect_pt;
    lR.origE = s.intersect_pt;
    for( auto *l : lights ) {
        // Check light ray intersection.
        glm::vec3 colAccumulator(0);
        uint numConts = 0;
        bool moveOn = ( SoftShad == 1 )? true : false;
        for( int i = 0; i < SoftShad; i++ ) {
            if( moveOn ) lR.P = l->position;
            else lR.P = l->position + glm::normalize( glm::vec3( dist(mt), dist(mt), dist(mt) ) );

            if(checkIntersect(*root, lR).intersected) {
                numConts++;
                continue;
            } else if(  numConts == 0 && i > SoftShad/4 ) {
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

            if( moveOn ) {
                Lout += col;
                break;
            }
        }
        if( !moveOn ) {
            colAccumulator /= SoftShad;
            Lout += colAccumulator;
        }
    }

    // Add ambient light
    Lout += s.mat->get_kd() * ambient;

    return Lout;
}


// Get the colour of the reflection cast from a surface. There will only be one reflective iteration.
glm::vec3 getReflection( glm::vec3 surfaceColour, glm::vec3 eye, surface s, SceneNode *root, const std::list<Light *> & lights, glm::vec3 ambient ) {
    glm::vec3 col;

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
        col = getColour( rS, lights, root, ambient );
        timesInt++;
    } else {
        col = surfaceColour;
    }
    if( GlossAmt != 1 ) {
        std::uniform_real_distribution<double> dist( -1.0f, 1.0f );
        for( int i = 1; i < GlossAmt; i++ ) {
            rR.P = ( s.intersect_pt + cReflect * 25 ) + glm::normalize( glm::vec3( dist(mt), dist(mt), dist(mt) ) );
            rS = checkIntersect(*root, rR);
            if (rS.intersected) {
                timesInt++;
                glm::vec3 temp = getColour( rS, lights, root, ambient );
                //std::cout << glm::to_string( rS.mat->get_kd() ) << glm::to_string(temp) << std::endl;
                col += temp;
            } else {
                col += surfaceColour;
            }
        }
        if( timesInt == 0 ) return glm::vec3(-1);

        col /= GlossAmt;
        //std::cout << glm::to_string( col ) << std::endl;

    }

    return col;
}

// Cast a ray from the eye to a projected x and y point relative to a screen denoted by topLeft.
glm::vec3 castRayTo(float x, float y,
                    size_t h, size_t w,
                    glm::vec3 eye, glm::vec3 topLeft, double pixelSize,
                    SceneNode *root, const std::list<Light *> & lights,
                    glm::vec3 ambient ) {
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
            glm::vec3 Lout2 = getReflection( Lout, eye, s, root, lights, ambient );
            if (Lout2.x != -1 && Lout2.y != -1 && Lout2.z != -1)
                Lout = (glm::vec3(1) - s.mat->get_ks()) * Lout + s.mat->get_ks() * Lout2;
        }
        return Lout;
    } else {
        glm::vec3 Lout;

        Lout.x = 0;
        Lout.y = (float) glm::max( 1 - ( y / h * 1.3 ), 0.2 );
        Lout.z = (float) glm::max( 1 - ( y / h * 1.3 ), 0.2 );

        //std::cout << y << " " << h << " " << superPoints[x][y][2] << std::endl;

        float invX = w - x;

        if ( invX * invX + y * y < (w / 5.0) * (w / 5.0) ) {
            Lout.x = 1;
            Lout.y = 1;
            Lout.z = 0;
        }

        return Lout;
    }
}

// Make the image.
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

            loading( y * ( (Adaptive == 1)? 100 : 50 ) / h );
        } else {
            mtx.unlock();
            return;
        }
        mtx.unlock();

        glm::vec3 Lout = castRayTo( x, y, h, w, eye, topLeft, pixelSize, root, lights, ambient );

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
float getColourDiff( Image &image, uint x1, uint y1, uint x2, uint y2 ) {
    if( x1 < 0 || x2 < 0 || y1 < 0 || y2 < 0 ) {
        std::cerr << "ERROR: Indices out of bounds when retrieving colour difference." << std::endl;
        exit(1);
    }

    return float( glm::sqrt( glm::pow( image(x1, y1, 0) - image(x2, y2, 0), 2 ) + glm::pow( image(x1, y1, 1) - image(x2, y2, 1), 2 ) + glm::pow( image(x1, y1, 2) - image(x2, y2, 2), 2 ) ) );
}


void adaptiveAA(    Image &aImage, Image &image,
                    size_t w, size_t h,
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

            loading( 50 + (y*50)/h );
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
            if( Adaptive == 2 ) {
                sampledPoints += castRayTo(x +  4.0f/16, y +  4.0f/16, h, w, eye, topLeft, pixelSize, root, lights, ambient);
                sampledPoints += castRayTo(x + -4.0f/16, y + -4.0f/16, h, w, eye, topLeft, pixelSize, root, lights, ambient);
                sampledPoints /= 2;
            } else if( Adaptive == 4 ) {
                sampledPoints += castRayTo(x + -2.0f/16, y + -6.0f/16, h, w, eye, topLeft, pixelSize, root, lights, ambient);
                sampledPoints += castRayTo(x +  6.0f/16, y + -2.0f/16, h, w, eye, topLeft, pixelSize, root, lights, ambient);
                sampledPoints += castRayTo(x + -6.0f/16, y +  2.0f/16, h, w, eye, topLeft, pixelSize, root, lights, ambient);
                sampledPoints += castRayTo(x +  2.0f/16, y +  6.0f/16, h, w, eye, topLeft, pixelSize, root, lights, ambient);
                sampledPoints /= 4;
            } else if( Adaptive == 8 ) {
                sampledPoints += castRayTo(x +  1.0f/16, y + -3.0f/16, h, w, eye, topLeft, pixelSize, root, lights, ambient);
                sampledPoints += castRayTo(x + -1.0f/16, y + -3.0f/16, h, w, eye, topLeft, pixelSize, root, lights, ambient);
                sampledPoints += castRayTo(x +  5.0f/16, y +  1.0f/16, h, w, eye, topLeft, pixelSize, root, lights, ambient);
                sampledPoints += castRayTo(x + -3.0f/16, y + -5.0f/16, h, w, eye, topLeft, pixelSize, root, lights, ambient);
                sampledPoints += castRayTo(x + -5.0f/16, y +  5.0f/16, h, w, eye, topLeft, pixelSize, root, lights, ambient);
                sampledPoints += castRayTo(x + -7.0f/16, y +  1.0f/16, h, w, eye, topLeft, pixelSize, root, lights, ambient);
                sampledPoints += castRayTo(x +  3.0f/16, y +  7.0f/16, h, w, eye, topLeft, pixelSize, root, lights, ambient);
                sampledPoints += castRayTo(x +  7.0f/16, y + -7.0f/16, h, w, eye, topLeft, pixelSize, root, lights, ambient);
                sampledPoints /= 8;
            } else if( Adaptive == 16 ) {
                sampledPoints += castRayTo(x +  1.0f/16, y +  1.0f/16, h, w, eye, topLeft, pixelSize, root, lights, ambient);
                sampledPoints += castRayTo(x + -1.0f/16, y + -3.0f/16, h, w, eye, topLeft, pixelSize, root, lights, ambient);
                sampledPoints += castRayTo(x + -3.0f/16, y +  2.0f/16, h, w, eye, topLeft, pixelSize, root, lights, ambient);
                sampledPoints += castRayTo(x +  4.0f/16, y + -1.0f/16, h, w, eye, topLeft, pixelSize, root, lights, ambient);
                sampledPoints += castRayTo(x + -5.0f/16, y + -2.0f/16, h, w, eye, topLeft, pixelSize, root, lights, ambient);
                sampledPoints += castRayTo(x +  2.0f/16, y +  5.0f/16, h, w, eye, topLeft, pixelSize, root, lights, ambient);
                sampledPoints += castRayTo(x +  5.0f/16, y +  3.0f/16, h, w, eye, topLeft, pixelSize, root, lights, ambient);
                sampledPoints += castRayTo(x +  3.0f/16, y + -5.0f/16, h, w, eye, topLeft, pixelSize, root, lights, ambient);
                sampledPoints += castRayTo(x + -2.0f/16, y +  6.0f/16, h, w, eye, topLeft, pixelSize, root, lights, ambient);
                sampledPoints += castRayTo(x +  0.0f/16, y + -7.0f/16, h, w, eye, topLeft, pixelSize, root, lights, ambient);
                sampledPoints += castRayTo(x + -4.0f/16, y + -6.0f/16, h, w, eye, topLeft, pixelSize, root, lights, ambient);
                sampledPoints += castRayTo(x + -6.0f/16, y +  4.0f/16, h, w, eye, topLeft, pixelSize, root, lights, ambient);
                sampledPoints += castRayTo(x + -8.0f/16, y +  0.0f/16, h, w, eye, topLeft, pixelSize, root, lights, ambient);
                sampledPoints += castRayTo(x +  7.0f/16, y + -4.0f/16, h, w, eye, topLeft, pixelSize, root, lights, ambient);
                sampledPoints += castRayTo(x +  6.0f/16, y +  7.0f/16, h, w, eye, topLeft, pixelSize, root, lights, ambient);
                sampledPoints += castRayTo(x + -7.0f/16, y + -8.0f/16, h, w, eye, topLeft, pixelSize, root, lights, ambient);
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
		  "\t" << "numthreads: " << ( mthread? std::thread::hardware_concurrency() : 1 ) << std::endl <<
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
	int numThreads = mthread? std::thread::hardware_concurrency() : 1;
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

    // concurrently do adaptive antialiased image
    if( Adaptive > 1 ) {
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

	if( super ) {
		// set width and height back to correct values
		h--;
		w--;
	}

    loading( 100 );

	// TODO: Delete superpts.

}
