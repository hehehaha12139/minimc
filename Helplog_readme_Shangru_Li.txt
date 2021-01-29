I am Shangru Li of the team adams_error_1281 implementing "Efficient Terrain Rendering and Chunking".

In my implementation, the Terrain owns an array of unique pointers to 16 Chunks. Each chunk will fill its data to a single interleaved vector, and the vector data is passed to the GPU side.

To make the process easier, I first implemented a version in which a terrain owns 4 chunks and a chunk is 2x2x2 instead of 64x256x64. During this process I met the first tricky problem, that the camera is not correctly positioned after I set these things up. I found the reason is that I changed the dimension of the terrain, and camera position is based on the dimension, which is changed in resizeGL(). So for this temporary version I set camera looking at (0, 0, 0) and rendered the mini-terrain at the original point. 

Then I replaced the original 3 buffers for positions, normals and colors with 1 buffer, and read those data with an offset. As in previous homeworks, I used keyboardAllStars to make keyboard control silky smooth, and I also enabled the camera to move upside-down, not using world normal. And since I only need to rendering 4 2x2x2 chunks temporarily, it gets really smooth.

Then the next step is to not render invisible faces. I give every chunk the pointer to their master terrain so that every block can determine whether its neighbor block is empty even if this block is in another chunk. Now the chunks are seamlessly sticked to each other.

In above mentioned procedures I have written a lot of static functions to do coordinate calculation so that every block can know the linear and non-linear indices relative to its chunk or the whole terrain.

At last I carefully expanded the terrain to be 16 chunks, each with the size of 16x256x16. Then comes another big trouble. The faces between chunks are not rendered correctly, even though they were correct in the 2x2x2 version. It turned out to be that my Y-coordinate transformation from local linear index to global index is incorrect. This function is only used for one purpose, and in the 2x2x2 case, y is always 0 or 1, so the problem was not easily revealed.

In the map extension problem, I suggested that we should exchange chunks, and when we get out of the middle 4 chunks, chunks get updated. While in Xuecheng Sun's implementation he filled out chunk data again when the map is extended, which will be more convenient since chunk indices do not need to be changed.

And when we merge code, Xuecheng Sun's terrain can not be rendered correctly with chunks when the map is extended. I suggested that we should not mess up with internal calculation of the chunk, which is hard-coded with lots of coordinate numbers, and we should only change the model matrix of the chunk when the map is extended. And the problem is solved.