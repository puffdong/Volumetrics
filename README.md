Volumetrics features! 

Raymarcher marching some kind of smoke/clouds
It references a voxelgrid texture to determine where to render based on occupancy in the grid!
It has some sort of lighting albeit it looks rather bad atm (to be worked on now)

A cool feature however is that the voxelgrid texture also has a debug-view that renders out the grid cells that are != 0
I realized that this could be used to get some performance gains for the raymarcher so we have a dual purpose use for this debug-view!

We render it to capture the depth! 
This depth is used in the compositing step between the non-smoke stuff and the smoke
The depth also serves as a guide for where to start the marching or if we just skip it entirely! 

if you crank the marching parameters to really high step values you can see the optimization in action! Just compare between filling the entire screen with smoke vs just having a bit of it on screen! 


Been a while since I updated this readme... hiiii! :D


The voxelgrid texture is in uint8_t meaning each voxel stores a byte of info. 

Perlin noise is generated in like 50 milliseconds (128x128x128) on my macbook

sun color in shaders are glm::vec4 where .w is the intensity

todo: add back model matrix to the render command because it actually would solve the "have one shader for every darn object" problem. 

# Improvements could be made

## More detailed performance metrics
- One word: **"Flamegraphs"**. There is currently a general lack of clarity regarding what parts are slow/fast. Would it even be that hard to add the ability to capture how long the various passes take in **execute_pipeline()**?


## VoxelGrid  
- **update_voxel_data()** could be improved by only updating a subset of the buffer based on what has been changed instead of completely re-uploading all the data. Haven't caught wind of this being a big issue yet but I can't confirm that for certain. 
- Capture how long various parts take to complete. I know there are tools that can build nice flamegraphs, would be nice to get that to work at some point. 





# Other

code: 

vec3 point_light_contributions = vec3(0.0);
for (int l = 0; l < u_light_count; ++l) {
        Light point_light = u_lights[l];
        vec3 point_light_pos = point_light.position_radius.xyz;
        vec3 point_light_color = point_light.color_intensity.xyz;
        float point_light_intensity = point_light.color_intensity.w;
        float point_light_vol_multiplier = point_light.misc.x;

        vec3 to_light = point_light_pos - ray_pos;
        float distance_to_light = length(to_light);
        vec3 light_dir = to_light / distance_to_light;
        float attenuation = 1.0 / (distance_to_light * distance_to_light); // inverse square falloff
        float point_light_transmittance = do_point_light_march(ray_pos, light_dir);
        float point_phase = henyey_greenstein(dot(light_dir, ray_direction), u_anisotropy);
        vec3 point_light_contrib = point_light_color * point_light_intensity * attenuation * point_light_transmittance * point_phase * point_light_vol_multiplier;
        point_light_contributions += point_light_contrib;
}


