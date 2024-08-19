# Mini Minecraft
## Milestone 1
### Procedural Terrain: Andrew Antenberg
I used fractal brownian motion on perlin noise functions with different smoothstep boundaries and frequencies to create the different "biomes" within the terrain. Then, I used another perlin noise function with a smaller "zoom" coefficient to interpolate the height between biomes. I used a cutoff value on this interpolation coefficient to decide which biome's block type to choose for a given x/z coordinate.

### Efficient Terrain Rendering and Chunking: Aarushi Singh
I made the Cube class inherit Drawable and wrote a function to write position, normal, and color information for each visible face (i.e. face that borders an empty block) to an interleaved VBO. I also wrote a function in shadeprogram.cpp to draw chunks based on this interleaved VBO, using the Lambertian shader. I also adapted the Terrain class to load and display chunks based on this new Chunk implementation. The initial scene loads all blocks within LOAD_DISTANCE of the player position. At every tick, the program checks the user's current position and loads in all terrain zones within LOAD_DISTANCE.

### Game Engine Tick Function and Engine Physics: Vishaal Kumar

- I created a switch statement to case on the key pressed and released events. Depending on the key pressed, I would update the player's relevant acceleration vector and use the player's velocity to update the player's position. 
- Kept track of the mouse position and used it to update the camera's orientation. After each movement, I would center the mouse back to the center of the screen for a better camera experience.
- I used gridmarching for both player collision and adding/removing blocks.
- Added additional logic to keep track of player state - whether the player is in the air or on the ground. This is used to determine if the player can jump or not and if collision detection should be activated. This also prevented the player from being able to jump infinitely.
- Added gravity to the player's acceleration vector to simulate the player falling when not on the ground and added friction and drag to the player's velocity vector by multiplying it by a constant decay factor.




## Milestone 2
### Cave Systems: Vishaal Kumar
- I used a 3D Perlin noise function to generate a cave system. I used a threshold value to determine if a block should be empty or not and used the guidelines from the milestone to determine where to place lava blocks.
- I used grid marching to determine if the player is inside water or lava. If the player is in either, I reduced the velocity of the player. Additionally, I added a player state variable that keeps track of whether the player is in water or lava. This is used to determine if the player should be slowed down and also is useful for the post processing shader.
- I created a post processing frame buffer object so that I could apply a post processing shader to the scene. I used the player state variable to determine if the player is in water or lava and applied a blue or red tint to the scene respectively.


### Texturing and Texture Animation: Aarushi Singh
I implemented this by following the OpenGL calls gone over in lecture and tracing through the HW4/5 code in detail to follow the chain of calls. I manually created a map of block types to UV coords in the texture atlas, and added in extra logic around what happens when blocks are adjacent to opaque vs transparent blocks (split the existing interleaved VBOs into opaque and transparent VBOs, and adjusted subsequent shaderprogram.cpp drawing functions). I also implemented animation of blocks by passing a time variable to the fragment shaders and doing some mod math. Besides the required tasks for this milestone, I also went back and refactored the way chunks and their VBOs are buffered and displayed, which will hopefully help clean things up for milestone 3.

### Multithreaded Terrain Generation: Andrew Antenberg
- I implemented two worker thread types: one to generate the block types within a chunk, and the other to generate the VBO data for the newly created chunks.
- I used QMutexes to ensure multiple threads did not modify the same piece of data at the same time
- Each call to the `tick()` function, any chunks that need to be newly loaded spawn BlockTypeWorker threads, which will add chunks to the list of chunks that need VBO data when their work is complete. Additionally, each tick, any chunks that are in the list of chunks that need VBO data spawn VBOWorker threads to generate the VBO data for the chunks. That vbo data is then sent to the buffers for shaders in the main thread.



## Milestone 3

### Inventory: Vishaal Kumar
- Created an inventory where the player can place and store blocks. The inventory is revelead and hidden using the 'I' button. I implemented the inventory by creating a new ui widget that dynamically hovers over the main mygl widget. 
- The player can collect blocks by breaking them, and can only place blocks when they have a block in their inventory. There are numbers next to every block to indicate how many blocks of that type a player has.
- The player can select what block to place by using the keys 1-7 and the selected block has a red border to indicate that it is the currently selected block.

### Water Waves: Andrew Antenberg
- Added y-displacement to vertex shader for water and lava blocks.
- Calculated normals based on the derivative of the sinusoidal function used for the displacement.
- Added blinn-phong shading to increase the visibility of the effect of the waves (parts of the wave that are more in line with the reflection of the light from your perspective look lighter.

### Post-process Camera Overlay: Andrew Antenberg
- Added an effect to a post-processing shader that applies a time-relative displacement to the UV coordinates sampled to create a wavy effect when the player is under water or lava.

### Distance Fog: Andrew Antenberg
- Used a smoothstep function on the distance between a vertex and the player in a vertex shader to create an interpolation value
- Interpolate the color of the vertex after existing shading with the fog color (equivalent to the skybox color) for the calculated interpolation value.
- The interpolation value changes dynamically based on the render distance. If the render distance is increased, the fog distance will also increase.

### Cross Pointer: Vishaal Kumar (Extra Credit)
- Added a cross pointer for the player just like the actual Minecraft so that placing and destroying blocks is easier. 
- Implemented this by creating a pointer object that extends Drawable and is drawn using the flat shader just like the world axes (however it uses different view matrices so that the cursor moves with the camera/player). 

### NPCs: Aarushi Singh
- Created a creeper NPC that moves around the world randomly.
- Used a .obj file found on a free website online (https://www.cgtrader.com/free-3d-models/character/child/minecraft-creeper-low-poly-3d-model-free-with-rigged). Modified my half-edge mesh code from a prior homework to read in the obj file and buffer VBO data. 
- Colored the creeper by randomly generating a color for each face (the color is interpolated randomly from a green color scheme). Created a new shader program to render a non-textured object using lambertian reflection.
- Created an NPC class that inherits from entity. Modified the player collision detection/movement code to fit the creeper.
- Random movement is generated by a thread running in the background that every few seconds randomly flips one of the fields in the NPC's m_inputs struct and sleeps for a random number of seconds.
- The user can press "C" to toggle creeper highlighting (to make the creeper easier to find by rendering it on top of the block geometry), "V" to transition the creeper from flight mode and start its random motion (once the world has been loaded in), and "B" to teleport the creeper to the player's location.
