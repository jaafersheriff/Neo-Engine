
# God Rays app

App showcasing volumetric light scattering. Technique sourced from [GPU Gems](https://developer.nvidia.com/gpugems/GPUGems3/gpugems3_ch13.html).

How it works:
- Render sun to an offscreen FBO in white (GodRaySun)
- Render all models in the scene to the same FBO as black (GodRayOccluder)
- Radial blur the offscreen FBO (Blur)
- Render the scene normally to the back buffer
- Apply the blurred god rays as a post process affect (Combine)

I wanted to use the stencil buffer for the god ray FBO but I didn't. 

I _could_ collapse the combine shader into the phong shader, but the phong shader is one of the standard shaders provided by the engine, so I don't want to have to modify/recreate it specifically for this use case. Instead, the combine shader is a post process affect. 

TODO : readme image

<img src="res/readme.png">
