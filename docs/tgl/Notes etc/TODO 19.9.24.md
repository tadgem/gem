We need to do the following things:
1. Add normals to the voxel structure, this will 
2. Rework voxel texture mip generation, we need more information from the previous mip, e.g. we need to a 2x2x2 texture lookup on mip n-1 and average these values for the current mip.
3. Rethink how we trace a cone vs tracing a ray into this texture; As the cone trace's distance gets further from the source point, the angle of the cone increases, meaning that the sampled light has lower and lower resolution, meaning we can  traverse _up_ the mipchain instead of sampling and traversing the more expensive lower resolution mips. We are currently tracing a ray, where we do the opposite, we traverse _down_ the mip chain to find the nearest point in the lowest level mip of the 3D texture.
4. TLDR: Use the cone tracing method for diffuse indirect illlumination, use the existing ray traversal function for Specular reflections only.
   