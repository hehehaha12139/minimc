# CIS 560 Final Project of Group adams_error_1281  
## [Mini Minecraft Milestone 1](https://www.youtube.com/watch?v=CwsMUc-_AGs&t=2s)  
- Xuecheng Sun implemented Procedural Terrain.  
- Shangru Li implemented Efficient Terrain Rendering and Chunking.  
- Qiaosen Chen implemented Game Engine Update Function and Player Physics.  

## [Mini Minecraft Milestone 2](https://youtu.be/9p_NY1altIs)  
- Xuecheng Sun L-System Rivers.  
- Shangru Li implemented Texturing and Texture Animation.
- **Qiaosen Chen implemented Swimming and Multithreaded Terrain Generation.**  
About the implementation of swimming, firstly I makes the player determine whether his feet (4 vertices in the down layer) are in the water or lava block or not, if yes, change the player's current motion state to SWIMMING. The SWIMMING motion state has its own velocity update function which achieves the effect of slow motion in the water. Also, I let the player determine whether his eye (the camera's eye position) is under water / lava or not, if yes, draw the overlay according to the current block where the camera is located.  
About the implementation of multithreaded terrain generation, at first I met a problem that some parts of terrain did not be drawn when I used another thread to generate new terrain. After some discussion with my teammate Shangru Li, we concluded that the main thread called the function GLDrawScene() to draw every chunk before the new thread finished its job to generate new terrain and update chunks' data. Therefore, I changed my multithread strategy: I create 16 threads to generate different parts of a new terrain, respectively, according to a given index. Wait for all threads' work done and then create another 16 threads to update the original 16 chunks' data (the strategy of generating new terrain of our group is that, in fact we do not generate new chunks, we just update chunks' data according to a given "offset" to implement the world-expanding effect, as we know, in the real game, chunks cannot be added or created infinitely), which is successful to prevent the gameplay from slowing down when new terrain is generated to expand the world.  
