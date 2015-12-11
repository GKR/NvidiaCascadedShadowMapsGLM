## Nvidia Cascaded Shadow Maps with shaders and GLM
Modification of the Nvidia Cascading Shadow Maps demo using a pure shader based approach over immediate mode. Read more about (cascaded) shadow mapping here: https://en.wikipedia.org/wiki/Shadow_mapping

![Screenshot](media/screenshot_001.png "Screenshot")

## Legal Disclamer
The original code was not written by me, and the original copyright notices have been kept in place where applicable.
The original whitepaper and code can be found here:
* http://developer.download.nvidia.com/SDK/10.5/opengl/screenshots/samples/cascaded_shadow_maps.html
* http://developer.download.nvidia.com/SDK/10.5/opengl/src/cascaded_shadow_maps/doc/cascaded_shadow_maps.pdf
* http://developer.download.nvidia.com/SDK/10.5/Samples/cascaded_shadow_maps.zip

## Dependencies
* GLUT (https://www.opengl.org/resources/libraries/glut/)
* libpng (http://www.libpng.org/pub/png/libpng.html)
* GLEW (http://glew.sourceforge.net/)
* gml (http://glm.g-truc.net/0.9.7/index.html)
 
## Building
The project uses CMake to generate build files for any platform (currently only tested on Linux and OS X). 

    cd NvidiaCascadedShadowMapsGLM
    mkdir build && cd build
    cmake -G "Unix Makefiles" .. # NOTE: Replace "Unix Makefiles" with your platform / build tool of choice
    make
    ./csm_demo_glm

