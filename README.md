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
