# Voxel Project
_Author: Tiberiu Rociu_  

![Landscape1](https://github.com/user-attachments/assets/42c30144-9324-42fd-b330-5c8fa351340b)
![Landscape2](https://github.com/user-attachments/assets/41863d41-d1df-4fcc-a2bc-b357703a2c78)
![NPC](https://github.com/user-attachments/assets/e91494d3-ba5b-4f1a-8bc2-b3278a8c0abd)

# Table of Contents  
- [Showcase](#showcase)
- [Packaged project](#packaged-project)
    - [Basic Controls](#basic-controls)
- [Design and Methodology](#design-and-methodology)
  - [Requirements and Constraints](#requirements-and-constraints)
  - [Tools, Technologies, and Hardware](#tools-technologies-and-hardware)
  - [Algorithms and Techniques](#algorithms-and-techniques)
    - [Chunking System](#chunking-system)
    - [Binary Greedy Meshing](#binary-greedy-meshing)
    - [Binary Face Culling](#binary-face-culling)
    - [LOD System](#lod-system)
    - [A* Pathfinding Algorithm](#a-pathfinding-algorithm)
- [Implementation](#implementation)
  - [Terrain Generation Optimisations](#terrain-generation-optimisations)
    - [Binary Face Culling](#binary-face-culling-1)
    - [Binary Greedy Meshing](#binary-greedy-meshing-1)
    - [LOD System – Collision](#lod-system-collision)
    - [LOD System – Spawning/Despawning](#lod-system-spawningdespawning)
  - [NPC Ecosystem Optimisations](#npc-ecosystem-optimisations)
    - [Custom Pathfinding using A*](#custom-pathfinding-using-a)
    - [Custom Collision Checks](#custom-collision-checks)
    - [Vision Spheres Checks](#vision-spheres-checks)
    - [LOD System – Spawning/Despawning](#lod-system-spawningdespawning-1)
- [Testing and Evaluation](#testing-and-evaluation)
  - [Terrain Generation Evaluation](#terrain-generation-evaluation)
  - [NPC Ecosystem Evaluation](#npc-ecosystem-evaluation)
  - [Self-Reflection](#self-reflection)
- [Conclusion and Future Work](#conclusion-and-future-work)
- [References](#references)
  
# Showcase  

Showing the runtime world generation when spawning in the world. 

https://github.com/user-attachments/assets/cc541bbc-ccfd-45f1-befa-a0838b605c29

Walking around and observing chasing behaviours and others. 

https://github.com/user-attachments/assets/414bc3c1-19fa-4677-8322-feedc5b7e7b2

Flying to show the terrain, NPC, and vegetation LOD spawning and despawning systems. 

https://github.com/user-attachments/assets/63bf557f-5366-436b-a85d-b899c1a7f3fb  

Showing the world size, including the LOD systems of spawning objects only around the player. 

https://github.com/user-attachments/assets/695b64fc-2464-49f3-983b-8d9f891bf768  

Showing the greedy meshing and face culling algorithm, which merges adjacent faces and removes any geometry that would be hidden from view.. 

https://github.com/user-attachments/assets/854a68fc-e102-4bf3-9ce0-fbf2904e2231  

Showing the LOD collision system, where terrain and trees have their collision updated as the player moves in the world. 

https://github.com/user-attachments/assets/c9bb3c24-d6c3-4843-a357-e25e97f7e5f9  

Test environment meant to show two NPCs communicating, in both scenarios one of the tigers notifies the other tiger of a food source, and the other one accepts and tries to also reach and consume said food source. This is meant to improve survival by notifying allies of food sources and predators.    

https://github.com/user-attachments/assets/086308c4-e94d-48ba-9779-d1374615af06  

Dynamic NPCs interacting with the environment and with each other, chasing, fleeing, eating, roaming, and resting based on their attributes and relationships.  

https://github.com/user-attachments/assets/fbde7d1b-5b18-4392-b0ee-dab75c0b37e1    

# Packaged project  

Packaged project link (Windows only): [CM4105 Honours Project - Packaged project build](https://liverguac-my.sharepoint.com/my?id=%2Fpersonal%2Ft%5Frociu%5Frgu%5Fac%5Fuk%2FDocuments%2FCM4105%20Honours%20Project%20%2D%20Packaged%20project%20build&ga=1)  
### Basic Controls  
| Key                | Action                            |
| ------------------ | --------------------------------- |
| `W`, `A`, `S`, `D` | Move the player                   |
| `Space`            | Jump                              |
| `F`                | Toggle flying on/off              |
| `O`                | Respawn the world with a new seed |

<h1 id="design-and-methodology">Design and Methodology</h1>
<p>Since the project doesn’t follow a clear existing design, the
implementation steps and required technologies had to be initially
explored. An agile methodology was employed, breaking down the project
into three main tasks: technology stack, terrain generation, and NPC
ecosystem, which were then further subdivided into more digestible
subtasks.</p>
<h2 id="requirements-and-constraints">Requirements and Constraints</h2>
<p>To achieve the performance and optimisations needed for the project,
with a smooth minimum 60 frames per second (FPS), C++ is the programming
language selected, as it provides low‑level control over memory
allocation and footprint that other languages typically abstract
away.</p>
<p>As for the rendering side, although the Vulkan graphics API was
initially considered for its performance and granular control of
resources, it was ultimately passed over due to its steep learning curve
and the project’s tight schedule. Instead, Unreal Engine 5 was selected,
as it supports C++ and reduces the overhead of handling the GPU pipeline
and shader management, among other things.</p>
<p>Thus, the requirements for the project are:</p>
<ul>
<li><p>Implement the project entirely in C++, instead of Unreal’s
Blueprint system, to reduce any unnecessary overhead.</p></li>
<li><p>Implement a chunk system that divides a large object into
multiple blocks, computing each chunk’s mesh on demand instead of
generating one massive mesh for the entire object.</p></li>
<li><p>Implement an optimised mesh generation algorithm that can be used
for the landscape and foliage.</p></li>
<li><p>Use multi-threading to handle the terrain mesh generation and
other location calculations for spawning and despawning
objects.</p></li>
<li><p>Use mesh instancing for foliage objects that share the same mesh
data.</p></li>
<li><p>Implement an LOD system for collision and different draw
distances for terrain, types of foliage, and NPCs.</p></li>
<li><p>Spawn large numbers of various types of NPCs in the world and
visually display their most important attributes.</p></li>
<li><p>Implement a custom pathfinding system for NPCs using A* and a
single multi-threaded Pathfinding Manager to handle requests.</p></li>
<li><p>Implement a custom collision check for all NPCs and animate their
position to any location.</p></li>
<li><p>Implement a single Decision System that all NPCs can use,
determining their actions based on their different base
parameters.</p></li>
</ul>
<h2 id="tools-technologies-and-hardware">Tools, Technologies, and
Hardware</h2>
<p>As mentioned, the main technology stack consists of C++ for the
project's logic, with Unreal Engine 5.4 primarily used just for
rendering. Visual Studio was used to integrate Unreal Engine with the
C++ implementation. There are also two libraries used for this project,
one is an A* implementation written by (Hui, 2023) in Java and
translated into C++ by the author of this project. The heuristic
function, however, is not present in the original library and was
implemented separately. The other library is FastNoiseLite by (Peck,
2021), which provides a highly optimised C++ implementation of various
noise types.</p>
<p>The NPC models and their animations used in this project were
purchased from (Omabuarts Studio, 2023) and are used with appropriate
licensing.</p>
<p>The hardware used for the implementation and, more importantly, the
evaluation tests for this project consists of an NVIDIA RTX 3080 GPU (12
GB VRAM) and an Intel i9-12900K CPU (24 cores).</p>
<h2 id="algorithms-and-techniques">Algorithms and Techniques</h2>
<p>This project implements various algorithms and techniques aimed at
reducing computations, memory usage, and render times, ensuring the
real‐time feasibility of runtime mesh generation for terrain and
foliage, even with a large and actively simulated NPC ecosystem.</p>
<h3 id="chunking-system">Chunking System</h3>
<p>The chunking system helps break down a larger object into smaller
blocks, creating meshes and rendering them on demand, rather than
generating and rendering one large one. This can be seen in Figure 1,
where the landscape is subdivided into smaller chunks that can be added
as needed, instead of rendering a single mesh for the entire landscape
in one draw call.</p>
<table>
<colgroup>
<col style="width: 100%" />
</colgroup>
<thead>
<tr>
<th style="text-align: center;"><img src="https://github.com/user-attachments/assets/65ef534e-4216-4c28-a0cd-37c3d1bcc0be"
style="width:4.9248in;height:2.60631in" /></th>
</tr>
</thead>
<tbody>
<tr>
<td style="text-align: center;">Highlighted terrain chunks showing the
Chunking System</td>
</tr>
<tr>
<td style="text-align: center;">Figure 1</td>
</tr>
</tbody>
</table>
<h3 id="binary-greedy-meshing">Binary Greedy Meshing</h3>
<p>The Greedy Meshing algorithm reduces the geometry and voxel data by
combining faces that are coplanar and adjacent into larger rectangular
polygons. As seen in Figure 2, each voxel has its own quad that is
formed of two triangles, and through Greedy Meshing, the quads are
combined and the vertex count and storage requirements are significantly
reduced.</p>
<table>
<colgroup>
<col style="width: 50%" />
<col style="width: 50%" />
</colgroup>
<thead>
<tr>
<th style="text-align: center;"><img src="https://github.com/user-attachments/assets/37b231fc-a0f2-4c3e-b76e-0c5f8bd2e806"
style="width:2.3622in;height:1.89071in" /></th>
<th style="text-align: center;"><img src="https://github.com/user-attachments/assets/893a4fbe-83f2-45d2-88d9-8eeb3a776787"
style="width:2.3622in;height:1.89071in" /></th>
</tr>
</thead>
<tbody>
<tr>
<td style="text-align: center;">One quad per each voxel</td>
<td style="text-align: center;">One quad for multiple voxels</td>
</tr>
<tr>
<td colspan="2" style="text-align: center;">Greedy Meshing example on a
top down voxel terrain</td>
</tr>
<tr>
<td colspan="2" style="text-align: center;">Figure 2</td>
</tr>
</tbody>
</table>
<p>Commonly, this technique is implemented by looping and checking the
bounds into which a quad can expand into. This can become
computationally expensive, considering that the algorithm is applied on
all six sides of a chunk, going up and down on each axis to cover all
the possible faces.</p>
<p>An improved version of this algorithm is implemented for this
project, as inspired by (Tantan, 2024), using bitwise operations
instead. 2D slices of the 3D chunk are built, as exemplified in Figure 3
and Figure 4.</p>
<table>
<colgroup>
<col style="width: 100%" />
</colgroup>
<thead>
<tr>
<th style="text-align: center;"><img src="https://github.com/user-attachments/assets/7bd20d47-8ddf-4b7b-b147-dc655b809a88"
style="width:4.9248in;height:2.25308in" /></th>
</tr>
</thead>
<tbody>
<tr>
<td style="text-align: center;">Example of building 2D planes of a 3D
chunk on all axis</td>
</tr>
<tr>
<td style="text-align: center;">Figure 3</td>
</tr>
</tbody>
</table>
<p>Each column in the Figure 4 example would be a 64-bit integer, where
each 1 represents a solid voxel, and 0 represents empty air. As a
simplified example, to combine the quads starting from the integer at
position x10, we use C++’s method std::countr_one() to get the number of
1s, meaning the height of the quad, then a mask is created, containing
the same number of 1s as our height. Using the mask, we can try to
expand into the next integer, x9, by using the bitwise operation AND
(&amp;). While this is true, we can expand up until x4, create the quad,
and discard all the 1s used and continue the process until all the quads
in the 2D slice are combined. For this algorithm to work,
std::countr_zero() is also used to jump any empty gaps in the terrain
slice. The final result will be similar to the one seen in Figure 2.</p>
<table>
<colgroup>
<col style="width: 100%" />
</colgroup>
<thead>
<tr>
<th style="text-align: center;"><img src="https://github.com/user-attachments/assets/5d8a0e48-2abe-4857-b3c3-3de5bee60010"
style="width:4.38969in;height:3.58505in" /></th>
</tr>
</thead>
<tbody>
<tr>
<td style="text-align: center;">Example of a 2D plane used in the binary
greedy meshing algorithm</td>
</tr>
<tr>
<td style="text-align: center;">Figure 4</td>
</tr>
</tbody>
</table>
<h3 id="binary-face-culling">Binary Face Culling</h3>
<p>The face culling algorithm reduces the geometry even further, by only
computing and drawing the meshes that are visible to the player. As seen
in Figure 5, a side view slice of some terrain, only the outer faces are
displayed, with any covered voxel face being discarded and its mesh
never computed in the first place.</p>
<p>Similar to Greedy Meshing, this algorithm is commonly implemented by
looping and checking each voxel’s neighbour voxels to determine if each
of its faces are surrounded by air or solid blocks. This results in
keeping only the outer layer of an object, drawing only the necessary
faces and discarding any that’s being hidden.</p>
<table>
<colgroup>
<col style="width: 100%" />
</colgroup>
<thead>
<tr>
<th style="text-align: center;"><img src="https://github.com/user-attachments/assets/b1473a98-ee04-42c1-9803-3eb1180ec470"
style="width:4.38969in;height:3.52226in" /></th>
</tr>
</thead>
<tbody>
<tr>
<td style="text-align: center;">Side view example of face culled
terrain, where only the outer voxel blue faces are drawn and the inside
faces are empty</td>
</tr>
<tr>
<td style="text-align: center;">Figure 5</td>
</tr>
</tbody>
</table>
<p>For this reason, a more optimised solution was implemented, as
inspired by (Tantan, 2024), using a Binary Face Culling algorithm where
bitwise operations replace most of the computationally heavy loops.</p>
<p>The same 2D slices of a 3D chunk used for the Binary Greedy Meshing
algorithm, as described in Figure 3, will be used to face cull the voxel
faces on all 3 axes, ascending and descending. This results in 6 sets of
planes, each set with 64 planes when a 64-bit integer is used. As the
outer edge of a plane is used for padding, to ensure faces are not drawn
in between chunks, this results in a 62x62 chunk.</p>
<p>The algorithm will calculate the left and right faces of each column.
In this case, each column is represented by a single 64-bit integer
within each of the 2D planes. The faces of each 64 voxel column will
then be computed in just a few bitwise operations.</p>
<p>In Figure 6 and 7, there is a breakdown of the 3 operations needed to
compute the left or right faces of a single column in a plane. The only
difference between the right or left faces computation is how we
bitshift in the first step.</p>
<table>
<colgroup>
<col style="width: 100%" />
</colgroup>
<thead>
<tr>
<th style="text-align: center;"><img src="https://github.com/user-attachments/assets/f3cd66eb-ac9c-4c62-8348-abe3fe0cac0e"
style="width:4.9621in;height:6.10377in" /></th>
</tr>
</thead>
<tbody>
<tr>
<td style="text-align: center;">Face culling for left faces in a 4x4
chunk with padding</td>
</tr>
<tr>
<td style="text-align: center;">Figure 6</td>
</tr>
</tbody>
</table>
<p>For these examples, a 4x4 chunk is used, with a 1x1 outer padding.
Meaning that any faces outside of the red square will be discarded, as
they’re part of a different chunk. This ensures that any outer faces of
our current chunk that will be hidden by an adjacent chunk will not be
drawn.</p>
<p>The y2 column is used as an example, where the bits are shifted right
by 1, negated, and compared to the original y2 bits in an AND operation.
These final bits, when overlaid over the terrain, show that each
positive bit is a left face. Finally, in this example, the left-most
face is discarded, as it is part of the padding area, and only the
middle face is kept.</p>
<table>
<colgroup>
<col style="width: 100%" />
</colgroup>
<thead>
<tr>
<th style="text-align: center;"></th>
</tr>
</thead>
<tbody>
<tr>
<td style="text-align: center;"><img src="https://github.com/user-attachments/assets/928e3dcd-4627-42e9-a5ae-81fc099c6745"
style="width:4.99712in;height:6.14685in" /></td>
</tr>
<tr>
<td style="text-align: center;">Face culling for right faces in a 4x4
chunk with padding</td>
</tr>
<tr>
<td style="text-align: center;">Figure 7</td>
</tr>
</tbody>
</table>
<p>The Figure 7 example is similar to the Figure 6 explanation, but the
first step is to shift the original bits of the y2 column to the left
instead of right, thus getting in the end the right faces for the 64-bit
occupancy column.</p>
<h3 id="lod-system">LOD System</h3>
<p>The LOD system can be applied to various aspects of the game, such as
terrain and foliage draw distance, foliage density, and NPC spawn
distance. The idea is to reduce the strain on the GPU and CPU by
reducing computations or draw calls for objects that are too far from
the player and wouldn’t normally be visible. These objects would only be
loaded into the scene as the player moves in the world.</p>
<table>
<colgroup>
<col style="width: 100%" />
</colgroup>
<thead>
<tr>
<th style="text-align: center;"><img src="https://github.com/user-attachments/assets/5358ec5b-e73d-41c1-942d-3370661b4efa"
style="width:5.94114in;height:3.95833in" /></th>
</tr>
</thead>
<tbody>
<tr>
<td style="text-align: center;">Top down view of terrain, showing
multiple draw distances for the LOD system around the player</td>
</tr>
<tr>
<td style="text-align: center;">Figure 8</td>
</tr>
</tbody>
</table>
<p>As seen in the simplified diagram of Figure 8, there can be multiple
LOD systems implemented at the same time, each handling the various
aspects mentioned. It is advantageous to employ multiple draw distances,
because larger objects visible to the player should extend further than
other smaller objects. This is to maintain the immersion and reduce the
visibility of bigger objects popping in when the player gets close.</p>
<h3 id="a-pathfinding-algorithm">A* Pathfinding Algorithm</h3>
<p>A* is a pathfinding algorithm that finds the shortest path between
two points using a cost function:</p>
<p>f(n) = g(n) + h(n)</p>
<p>where g(n) is the actual cost from the start to node n, and h(n) is a
heuristic estimate from n to the goal.</p>
<p>It is efficient because it combines the accuracy of Dijkstra's
algorithm, meaning it always finds the best path, with the speed of
greedy best-first search, guiding the search toward the goal. In Figure
9, the nodes explored by the algorithm can be visualised, along with the
final shortest path to the target, while avoiding any collisions, as
guided by the cost function and the successor() function.</p>
<table>
<colgroup>
<col style="width: 100%" />
</colgroup>
<thead>
<tr>
<th style="text-align: center;"><img src="https://github.com/user-attachments/assets/2a55512d-5df8-4603-a503-cd02dc294c2b"
style="width:5.53746in;height:3.95833in" /></th>
</tr>
</thead>
<tbody>
<tr>
<td style="text-align: center;">Showing the locations explored and the
shortest path found by the A* algorithm</td>
</tr>
<tr>
<td style="text-align: center;">(Patel, 1997)</td>
</tr>
<tr>
<td style="text-align: center;">Figure 9</td>
</tr>
</tbody>
</table>
<h1 id="implementation">Implementation</h1>
<p>The project implementation will, as before, be broken down into the
two main parts of the project: one focusing on terrain generation, and
the other on the NPC ecosystem.</p>
<h2 id="terrain-generation-optimisations">Terrain Generation
Optimisations</h2>
<h3 id="binary-face-culling-1">Binary Face Culling</h3>
<p>For simplicity, building the binary columns for the orientation of
each axis based on the noise function is abstracted away from the full
terrain algorithm explanation. Its full implementation can be found in
the CreateBinarySolidColumnsYXZ() function instead. The important part
is that the FaceCullingBinaryColumnsYXZ() function works on these
created binary columns to create the culled face masks.</p>
<p>As discussed in Section 1.3.3., the algorithm has to run the face
culling algorithm on each column or 64-bit integer of the 3D chunk. In
Figure 10, there are 6 vectors that will store all the faces’ data of a
3D chunk as 64-bit integer columns, each traversing a different axis in
ascending or descending manner, as also shown in the Figure 3 diagram.
Furthermore, to prevent resizing with each insertion, the memory of each
vector is pre-allocated, since the dimensions of each chunk are known
beforehand. Lastly, to increase data retrieval performance, the usual 3D
array used to store the chunk data is flattened into a 1D array
instead.</p>
<table>
<colgroup>
<col style="width: 100%" />
</colgroup>
<thead>
<tr>
<th style="text-align: center;"><img src="https://github.com/user-attachments/assets/40d26697-1664-429e-a4ac-1336ddb250e6"
style="width:5.7208in;height:2.16972in" /></th>
</tr>
</thead>
<tbody>
<tr>
<td style="text-align: center;">Code from CreateTerrainMeshesData(),
showing the 2D planes being built and face culled</td>
</tr>
<tr>
<td style="text-align: center;">Figure 10</td>
</tr>
</tbody>
</table>
<p>The columns created in columnFaceMasks will store just the face
culled data, discarding all the previous voxels that are hidden. In
Figure 10, a code snippet from FaceCullingBinaryColumnsYXZ(), we can see
the same algorithm described in Figure 6 and 7, showing how the left and
right faces for each column are computed.</p>
<table>
<colgroup>
<col style="width: 100%" />
</colgroup>
<thead>
<tr>
<th style="text-align: center;"><img src="https://github.com/user-attachments/assets/b729bcad-fff7-43cf-886d-68c5bd0e3cba"
style="width:5.7208in;height:0.9759in" /></th>
</tr>
</thead>
<tbody>
<tr>
<td style="text-align: center;">Code from FaceCullingBinaryColumnsYXZ(),
showing the face culling algorithm on a column from the 2D plane</td>
</tr>
<tr>
<td style="text-align: center;">Figure 11</td>
</tr>
</tbody>
</table>
<p>The result can be seen in the in-game screenshots of Figure 12, where
there are no meshes underneath each surface voxel or between the
highlighted chunks themselves.</p>
<table>
<colgroup>
<col style="width: 100%" />
</colgroup>
<thead>
<tr>
<th style="text-align: center;"><img src="https://github.com/user-attachments/assets/276a30e2-c21c-4761-8fd4-26fc57a690d0"
style="width:5.94114in;height:3.2305in" /></th>
</tr>
</thead>
<tbody>
<tr>
<td style="text-align: center;"><p><img src="https://github.com/user-attachments/assets/0970bbd8-c8cc-44b4-9e73-281fe56bbf5b"
style="width:5.94114in;height:3.2305in" /></p>
<p>In-game screenshots showing the terrain face culling
algorithm</p></td>
</tr>
<tr>
<td style="text-align: center;">Figure 12</td>
</tr>
</tbody>
</table>
<h3 id="binary-greedy-meshing-1">Binary Greedy Meshing</h3>
<p>As discussed in Section 1.3.2., the algorithm has to combine all
possible quads that are on the same elevation and are facing the same
way. This means that from the columnFaceMasks created in the previous
part, Section 2.1.1., 2D planes have to be created, as shown in Figure
3. columnFaceMasks is used as a base for the 2D planes because it
ensures only the visible quads will be combined.</p>
<table>
<colgroup>
<col style="width: 100%" />
</colgroup>
<thead>
<tr>
<th style="text-align: center;"><img src="https://github.com/user-attachments/assets/96e3693e-30f2-4633-af2e-5ef017c24115"
style="width:5.70411in;height:3.17361in" /></th>
</tr>
</thead>
<tbody>
<tr>
<td style="text-align: center;">Code from CreateTerrainMeshesData(),
showing the 2D planes being built and greedy meshed</td>
</tr>
<tr>
<td style="text-align: center;">Figure 13</td>
</tr>
</tbody>
</table>
<p>In the code snippet of Figure 13, each axis’ planes are stored as
before in std::vectors with pre-allocated memory, they get their bits
allocated by BuildBinaryPlanes(), and then each plane will have its
quads merged inside of GreedyMeshingBinaryPlane().</p>
<p>The explanation of getting the height of a quad from Section 1.3.2.
can be seen in the code implementation in Figure 14, where
std::countr_zero() gets the starting point of a quad, and
std::countr_one() will get the height. A bit height mask is created and
this will then be further compared to adjacent rows in the plane,
expanding the width of the quad with each true operation.</p>
<table>
<colgroup>
<col style="width: 100%" />
</colgroup>
<thead>
<tr>
<th style="text-align: center;"><img src="https://github.com/user-attachments/assets/00b453d2-4aca-47d7-9bbb-293a2c684112"
style="width:5.7827in;height:2.48892in" /></th>
</tr>
</thead>
<tbody>
<tr>
<td style="text-align: center;">Code from GreedyMeshingBinaryPlane(),
showing how a quad’s height is computed</td>
</tr>
<tr>
<td style="text-align: center;">Figure 14</td>
</tr>
</tbody>
</table>
<p>The merged quads can be seen in Figure 15, an in-game screenshot,
where the normal view and the wireframe view are overlaid over the same
terrain, to show the various quads’ dimensions.</p>
<table>
<colgroup>
<col style="width: 100%" />
</colgroup>
<thead>
<tr>
<th style="text-align: center;"><img src="https://github.com/user-attachments/assets/6c9d0662-fb0b-48f6-9fa5-0a2b617ebd6f"
style="width:6.12444in;height:2.47826in" /></th>
</tr>
</thead>
<tbody>
<tr>
<td style="text-align: center;">In-game greedy meshing example, showing
combined quads</td>
</tr>
<tr>
<td style="text-align: center;">Figure 15</td>
</tr>
</tbody>
</table>
<h3 id="lod-system-collision">LOD System – Collision</h3>
<p>Similar to the diagram shown in Figure 8, where depending on distance
various attributes can be changed, the result can be seen in the in-game
implementation in Figure 16, where collision is updated only around the
player. The collision will continuously update as the player moves in
the world, making no difference from their perspective that further
terrain chunks or trees don’t have collision.</p>
<p>This technique was implemented because collision can be expensive to
compute, even with Unreal’s optimisations such as spatial partitioning,
where objects are grouped and far objects are discarded for certain
collision checks. It is still more efficient for objects to not have the
collision in the first place, if one is not needed.</p>
<table>
<colgroup>
<col style="width: 100%" />
</colgroup>
<thead>
<tr>
<th style="text-align: center;"><img src="https://github.com/user-attachments/assets/af1acbaf-f326-4dc1-9b0b-7f50d3819337"
style="width:6.15495in;height:3.28324in" /></th>
</tr>
</thead>
<tbody>
<tr>
<td style="text-align: center;">In-game screenshot, displaying the LOD
collision system for terrain and trees</td>
</tr>
<tr>
<td style="text-align: center;">Figure 16</td>
</tr>
</tbody>
</table>
<p>In the code implementation in Figure 17, multiple function calls are
made to update various lists of objects that will eventually be updated
over many frames, as also seen in Figure 18. This functionality of
updating lists is made on a separate thread instead of the main game
thread, to ensure that all the distance calculations and list updates
are not introducing frame drops. Lastly, in Figure 19,
UpdateChunksCollision() is called every frame in the main game thread,
and two chunks get their collision updated, if any are waiting in the
update list populated by the separate thread.</p>
<p>Chunk collision updates are staggered instead of handling all the
waiting chunks at once, to prevent too many chunks from receiving
updates in a single frame, creating a temporary freeze. This way, the
updates are not noticeable, and a stable frame rate is kept.</p>
<table>
<colgroup>
<col style="width: 100%" />
</colgroup>
<thead>
<tr>
<th style="text-align: center;"><img src="https://github.com/user-attachments/assets/f5ddc7ce-388d-402a-a4bd-9bbf045b3a66"
style="width:4.29861in;height:3.4356in" /></th>
</tr>
</thead>
<tbody>
<tr>
<td style="text-align: center;"><p>Function calls, in a separate thread,
updating objects lists for</p>
<p>spawning and despawning, and for collision updates.</p></td>
</tr>
<tr>
<td style="text-align: center;">Figure 17</td>
</tr>
</tbody>
</table>
<table>
<colgroup>
<col style="width: 100%" />
</colgroup>
<thead>
<tr>
<th style="text-align: center;"><img src="https://github.com/user-attachments/assets/1823ee04-0efa-404e-9db4-f17e15c5d1ee"
style="width:4.17016in;height:4.73611in" /></th>
</tr>
</thead>
<tbody>
<tr>
<td style="text-align: center;"><p>Adding chunk pointers to lists for
collision updates,</p>
<p>based on the player’s position</p></td>
</tr>
<tr>
<td style="text-align: center;">Figure 18</td>
</tr>
</tbody>
</table>
<table>
<colgroup>
<col style="width: 100%" />
</colgroup>
<thead>
<tr>
<th style="text-align: center;"><img src="https://github.com/user-attachments/assets/2e80bce3-b321-4550-98cd-42a8b9aa0738"
style="width:5.4917in;height:1.87571in" /></th>
</tr>
</thead>
<tbody>
<tr>
<td style="text-align: center;">Code for adding and removing collision
for a chunk, if one is waiting in the update list</td>
</tr>
<tr>
<td style="text-align: center;">Figure 19</td>
</tr>
</tbody>
</table>
<h3 id="lod-system-spawningdespawning">LOD System –
Spawning/Despawning</h3>
<p>Similar to Section 2.1.3., all the spawn and despawn positions are
handled in a separate thread for the reasons previously mentioned. In
Figure 17, besides the list updates for collision, the list updates for
spawning and despawning are also made. In Figure 20, there is a code
snippet showing how the grass and flower objects associated with the
chunks too far from the player will be removed by adding them to a list.
Their mesh pointers added to the list will be removed over multiple
frames in the game thread, and a similar process is true for spawning
the objects.</p>
<table>
<colgroup>
<col style="width: 100%" />
</colgroup>
<thead>
<tr>
<th style="text-align: center;"><img src="https://github.com/user-attachments/assets/f8c0e46f-8c13-4119-a4ad-2a76cd2fc7a0"
style="width:5.87011in;height:2.00694in" /></th>
</tr>
</thead>
<tbody>
<tr>
<td style="text-align: center;"><p>Code snippet from
UpdateSpawnPoints(), showing how the spawning and despawning</p>
<p>of grass and flower objects is handled</p></td>
</tr>
<tr>
<td style="text-align: center;">Figure 20</td>
</tr>
</tbody>
</table>
<p>In the in-game screenshot of Figure 21, the implementation of
multiple draw distances through the LOD system can be seen, showing that
the terrain chunks span further than other objects, and that tree
objects span around the player at a reduced distance than the terrain.
This implementation drastically reduces the scene memory consumption and
also reduces the number of draw calls to the GPU.</p>
<table>
<colgroup>
<col style="width: 100%" />
</colgroup>
<thead>
<tr>
<th style="text-align: center;"><img src="https://github.com/user-attachments/assets/6614e9f4-be3f-4736-a285-b040a26be55f"
style="width:6.18315in;height:3.29828in" /></th>
</tr>
</thead>
<tbody>
<tr>
<td style="text-align: center;">In-game screenshot, displaying the LOD
system for spawning objects around the player</td>
</tr>
<tr>
<td style="text-align: center;">Figure 21</td>
</tr>
</tbody>
</table>
<h2 id="npc-ecosystem-optimisations">NPC Ecosystem Optimisations</h2>
<h3 id="custom-pathfinding-using-a">Custom Pathfinding using A*</h3>
<p>As mentioned in Section 1.3.5., A* is a powerful algorithm for
pathfinding, which even Unreal Engine uses behind the scenes for its
Navmesh pathfinding. The issue with Unreal’s implementation was that its
built-in Navmesh couldn’t be as detailed as needed, since the project’s
terrain is very fractal and Unreal wouldn’t compute the path from one
voxel to another.</p>
<p>For this reason, a custom implementation of A* was needed. Using the
A* library from (Hui, 2023) as a starting point, it was adapted from
Java to C++, and integrated with a Pathfinding Manager, a thread pool
that can receive pathfinding requests from the NPC class and allow each
thread to notify the NPC when a path to the goal was found (see Figure
22, 23, 24).</p>
<table>
<colgroup>
<col style="width: 100%" />
</colgroup>
<thead>
<tr>
<th style="text-align: center;"><img src="https://github.com/user-attachments/assets/b7b0b888-b2da-4ddc-85f1-e0ca19bccb72"
style="width:6.01679in;height:1.60448in" /></th>
</tr>
</thead>
<tbody>
<tr>
<td style="text-align: center;">Adding a pathfinding task to the
Pathfinding Manager from the NPC class</td>
</tr>
<tr>
<td style="text-align: center;">Figure 22</td>
</tr>
</tbody>
</table>
<table>
<colgroup>
<col style="width: 100%" />
</colgroup>
<thead>
<tr>
<th style="text-align: center;"><img src="https://github.com/user-attachments/assets/446e803c-018f-478e-b7f7-28de14804ab3"
style="width:4.53731in;height:2.77749in" /></th>
</tr>
</thead>
<tbody>
<tr>
<td style="text-align: center;"><p>Thread from Pathfinding Manager,
finding the path to the goal location</p>
<p>and notifying the NPC when a path is found</p></td>
</tr>
<tr>
<td style="text-align: center;">Figure 23</td>
</tr>
<tr>
<td style="text-align: center;"></td>
</tr>
<tr>
<td style="text-align: center;"><img src="https://github.com/user-attachments/assets/3f2ada00-d5d6-4508-8374-e26e1191a7d6"
style="width:5.80597in;height:2.77439in" /></td>
</tr>
<tr>
<td style="text-align: center;"><p>The Pathfinding Task creating a
VoxelSearchProblem and returning</p>
<p>the path to the goal location</p></td>
</tr>
<tr>
<td style="text-align: center;">Figure 24</td>
</tr>
</tbody>
</table>
<p>Assuming that there will be large amounts of pathfinding tasks, since
each NPC’s actions require moving to a certain location, and that there
is large number of NPCs simulated at once, moving the computations to a
separate thread was crucial to maintain a stable frame rate and not
introduce stutters. A thread pool was also needed, as it removes the
overhead of creating and destroying a thread each time a task is
completed, maintaining thus better performance by reusing threads.</p>
<table>
<colgroup>
<col style="width: 100%" />
</colgroup>
<thead>
<tr>
<th style="text-align: center;"><img src="https://github.com/user-attachments/assets/e8c6985d-eb76-4765-9443-aed7eba12721"
style="width:5.83827in;height:1.59701in" /></th>
</tr>
</thead>
<tbody>
<tr>
<td style="text-align: center;">Manhattan distance used a heuristic for
the A* search</td>
</tr>
<tr>
<td style="text-align: center;">Figure 25</td>
</tr>
</tbody>
</table>
<p>Lastly, as seen in Figure 25, the heuristic used for the pathfinding
search is a simple Manhattan distance between the current and goal
position.</p>
<h3 id="custom-collision-checks">Custom Collision Checks</h3>
<p>As the decision for custom pathfinding implementation was taken, the
algorithm has to take into consideration obstacles, since the NPC
movement is handled completely by the project’s implementation and not
by Unreal Engine. Thus, NPCs have their collision turned off from Unreal
Engine’s perspective, and their collision is instead handled
internally.</p>
<p>The NPCs have to avoid solid objects such as trees and other NPCs.
For this, as shown in Figure 26, maps are used to store the occupied
voxels, and the voxels that should be avoided. Because of their hashing
table implementation, maps offer a great average data retrieval time
complexity of O(1), which makes them an efficient custom solution for
collision checking.</p>
<table>
<colgroup>
<col style="width: 100%" />
</colgroup>
<thead>
<tr>
<th style="text-align: center;"><img src="https://github.com/user-attachments/assets/aa1e6e8c-48ac-4512-afa2-c8c69ab987dc"
style="width:5.52985in;height:1.37503in" /></th>
</tr>
</thead>
<tbody>
<tr>
<td style="text-align: center;"><p>Maps and the custom fair semaphore
used to store the surface voxel</p>
<p>data and their occupancy</p></td>
</tr>
<tr>
<td style="text-align: center;">Figure 26</td>
</tr>
</tbody>
</table>
<p>In Figure 27, the function IsSurfacePointValid() is used by a thread
from the Pathfinding Manager to validate locations during pathfinding.
This allows the search to return a path that avoids solid static objects
such as trees, and dynamic objects such as NPCs.</p>
<table>
<colgroup>
<col style="width: 100%" />
</colgroup>
<thead>
<tr>
<th style="text-align: center;"><img src="https://github.com/user-attachments/assets/bfcb6631-d520-4abb-973d-e7039a01a421"
style="width:5.35075in;height:3.33864in" /></th>
</tr>
</thead>
<tbody>
<tr>
<td style="text-align: center;">Function for checking if a pathfinding
point is valid</td>
</tr>
<tr>
<td style="text-align: center;">Figure 27</td>
</tr>
</tbody>
</table>
<p>In Figure 28, the first IsLocationOccupied() function is used by the
NPCs during their transition to one voxel to another, giving them a
check for occupancy and allowing them to momentarily pause if a voxel
they’re trying to reach is occupied. Otherwise, if the voxel is free,
the NPC will take ownership of that position up until they move to a new
location.</p>
<table>
<colgroup>
<col style="width: 100%" />
</colgroup>
<thead>
<tr>
<th style="text-align: center;"><img src="https://github.com/user-attachments/assets/91614808-ae3f-4d40-84dd-37a9fff77e6e"
style="width:5.54478in;height:2.05471in" /></th>
</tr>
</thead>
<tbody>
<tr>
<td style="text-align: center;"><p>Overloaded functions for checking if
a position is occupied or</p>
<p>for checking a position and assigning the NPC requesting it if it’s
not</p></td>
</tr>
<tr>
<td style="text-align: center;">Figure 28</td>
</tr>
</tbody>
</table>
<h3 id="vision-spheres-checks">Vision Spheres Checks</h3>
<p>Each NPC’s mental model of the world is encapsulated in their vision
sphere, as seen in Figure 29. All the objects of interest, such as
flowers and grass for food, and other NPCs as allies, threats, or prey,
are updated constantly as the NPC moves in the world.</p>
<table>
<colgroup>
<col style="width: 100%" />
</colgroup>
<thead>
<tr>
<th style="text-align: center;"><img src="https://github.com/user-attachments/assets/811c467c-0526-4ac9-b035-24879b8fe8bf"
style="width:5.92897in;height:3.22388in" /></th>
</tr>
</thead>
<tbody>
<tr>
<td style="text-align: center;">In-game NPC with its vision sphere shown
in debug mode</td>
</tr>
<tr>
<td style="text-align: center;">Figure 29</td>
</tr>
</tbody>
</table>
<p>The vision sphere is an Unreal Engine tool that provides an
OnOverlapBegin() and OnOverlapEnd() event-driven functions that are
triggered every time an object collides with the sphere. This allows a
developer to implement their own logic to handle the overlap event.</p>
<table>
<colgroup>
<col style="width: 100%" />
</colgroup>
<thead>
<tr>
<th style="text-align: center;"><img src="https://github.com/user-attachments/assets/2c5ddb08-1a33-4a70-875b-4f3e3c07c2b0"
style="width:2.6791in;height:2.29456in" /></th>
</tr>
</thead>
<tbody>
<tr>
<td style="text-align: center;"><p>Storing animal types as positive bits
in a 16-bit integer,</p>
<p>and a struct for storing an NPC’s relationships</p></td>
</tr>
<tr>
<td style="text-align: center;">Figure 30</td>
</tr>
</tbody>
</table>
<p>Because this function is triggered extremely often by large amounts
of NPCs, as each NPC has their own vision sphere with a different radius
depending on their type, this function had to be efficient. Thus, to
ensure quick checks for all the different relationship types between
NPCs, an approach inspired by the implementation of Binary Greedy
Meshing and Binary Face Culling was used, storing relationships as
positive bits in an 16-bit integer, as seen in Figure 30 and 45.</p>
<table>
<colgroup>
<col style="width: 100%" />
</colgroup>
<thead>
<tr>
<th style="text-align: center;"><img src="https://github.com/user-attachments/assets/4e73424e-8109-4fe9-b7d5-1daac8584901"
style="width:5.88692in;height:1.46269in" /></th>
</tr>
</thead>
<tbody>
<tr>
<td style="text-align: center;">Code example of some of the NPC
relationships, stored as a 16-bit integer</td>
</tr>
<tr>
<td style="text-align: center;">Figure 31</td>
</tr>
</tbody>
</table>
<p>When an NPC enters the vision sphere, fast bit AND operations are
made to determine the relationship type with that NPC and adding its
pointer to a vision array, as shown in the Figure 32. These vision
arrays, besides other attributes, will be used to determine an NPC’s
next action.</p>
<table>
<colgroup>
<col style="width: 100%" />
</colgroup>
<thead>
<tr>
<th style="text-align: center;"><img src="https://github.com/user-attachments/assets/7964f600-8012-4167-96df-bbfd447bcef0"
style="width:5.8248in;height:3.46269in" /></th>
</tr>
</thead>
<tbody>
<tr>
<td style="text-align: center;">In-game NPC with its vision sphere shown
in debug mode</td>
</tr>
<tr>
<td style="text-align: center;">Figure 32</td>
</tr>
</tbody>
</table>
<p>A simpler process is used for adding vegetation as food sources to a
vision array, by using a custom procedural mesh component that adds a
bit more functionality, like storing the mesh type. This can be seen in
Figure 33, where the mesh type of the mesh component is validated and
added to an NPC’s vision array.</p>
<table>
<colgroup>
<col style="width: 100%" />
</colgroup>
<thead>
<tr>
<th style="text-align: center;"><img src="https://github.com/user-attachments/assets/92a0087a-2168-45a2-ae99-6b654a736143"
style="width:5.84449in;height:0.98104in" /></th>
</tr>
</thead>
<tbody>
<tr>
<td style="text-align: center;">Code for adding food to an NPC’s vision
array</td>
</tr>
<tr>
<td style="text-align: center;">Figure 33</td>
</tr>
</tbody>
</table>
<h3 id="lod-system-spawningdespawning-1">LOD System –
Spawning/Despawning</h3>
<p>Similarly to the system detailed in Section 2.1.3. and 2.1.4., NPCs
are spawned based on a draw distance from the player’s position, as seen
in Figure 34. Using the control variables from Figure 35, NPCs have the
lowest draw distance out of all the environment objects, because they’re
computationally demanding and simulating them when they’re too far for
the player to see introduces redundant computations and a lower frame
rate.</p>
<table>
<colgroup>
<col style="width: 100%" />
</colgroup>
<thead>
<tr>
<th style="text-align: center;"><img src="https://github.com/user-attachments/assets/030caea5-d20f-41f8-9517-501286a8e325"
style="width:6.15053in;height:2.70783in" /></th>
</tr>
</thead>
<tbody>
<tr>
<td style="text-align: center;">In-game estimated draw distances
diagram</td>
</tr>
<tr>
<td style="text-align: center;">Figure 34</td>
</tr>
</tbody>
</table>
<p>Even with a lower draw distance, the NPC numbers with the current
spawn chance control variable value still result in an average 200+
concurrently simulated NPCs, depending on the terrain.</p>
<table>
<colgroup>
<col style="width: 100%" />
</colgroup>
<thead>
<tr>
<th style="text-align: center;"><img src="https://github.com/user-attachments/assets/a07ebf83-b19c-49ea-9547-f2d29c907b8d"
style="width:4.5789in;height:1.81614in" /></th>
</tr>
</thead>
<tbody>
<tr>
<td style="text-align: center;">Control variables determining the LOD
draw distances and spawn chances</td>
</tr>
<tr>
<td style="text-align: center;">Figure 35</td>
</tr>
</tbody>
</table>
<h1 id="testing-and-evaluation">Testing and Evaluation</h1>
<p>This chapter will focus on evaluating the implementation of the
terrain and the NPC ecosystem primarily from an optimisation
perspective, highlighting its overall performance. The chapter’s end
will include a short self-reflection on the project’s development
process.</p>
<h2 id="terrain-generation-evaluation">Terrain Generation
Evaluation</h2>
<p>In Figure 36, a comparison between mesh generation algorithms is
made, showing an average mesh compute and render time, and memory needed
to store a single chunk’s data.</p>
<p>It is clear that not applying any technique to reduce a chunk’s
geometry is not an option, as computing and drawing so many quads
results in an unplayable simulation, with seconds long stutters. Another
issue would be running out of RAM during play time, as storing so much
data becomes not feasible.</p>
<p>Lastly, a more fair comparison is made between a traditional
implementation of face culling and greedy meshing using loops for solid
voxel checks, and the project’s more advanced implementation of these
algorithms storing solid voxels as bit flags and using bitwise
operations instead. Since both implementations should result in almost
the same geometry, the mesh render time and memory per chunk are almost
identical. The difference is in the mesh compute time, with the binary
version of these algorithms being more than 9 times faster than the
traditional implementation.</p>
<table>
<colgroup>
<col style="width: 100%" />
</colgroup>
<thead>
<tr>
<th style="text-align: center;"><img src="https://github.com/user-attachments/assets/5c6fb5d2-d4e3-4cab-9ffb-810bc7b33551"
style="width:6.12671in;height:3.67532in" /></th>
</tr>
</thead>
<tbody>
<tr>
<td style="text-align: center;">(Note: Graph is using a logarithmic
scale)</td>
</tr>
<tr>
<td style="text-align: center;">Figure 36</td>
</tr>
</tbody>
</table>
<p>These algorithms, besides the other techniques described in Section
2.1., allow for a much larger landscape and number of foliage objects,
while still maintaining a good frame rate. In Figure 37, for testing
purposes, a large landscape is simulated with less than 10GB RAM used
and with a stable 60 frames per second. This would normally be
unnecessary, as from the player’s point of view, the horizon would be
filled with terrain even at half the render distance.</p>
<table>
<colgroup>
<col style="width: 100%" />
</colgroup>
<thead>
<tr>
<th style="text-align: center;"><img src="https://github.com/user-attachments/assets/1c26340e-1a4c-4133-a1d5-332531d33750"
style="width:6.1581in;height:3.34847in" /></th>
</tr>
</thead>
<tbody>
<tr>
<td style="text-align: center;">In-game screenshot showing a larger
landscape</td>
</tr>
<tr>
<td style="text-align: center;">Figure 37</td>
</tr>
</tbody>
</table>
<h2 id="npc-ecosystem-evaluation">NPC Ecosystem Evaluation</h2>
<p>In the table from Figure 38, there are some metrics showing the
average computations per minute when 150 NPCs are being simulated.
Considering that these results are recorded while running the project at
60 frames per second in the Unreal Editor, which runs slower than a
packaged project, the results are very favourable. It shows that the
implemented system is efficient and can handle large numbers of
pathfinding tasks, actions request, communication between NPCs, and an
even larger number of vision sphere updates for each NPC.</p>
<table>
<colgroup>
<col style="width: 50%" />
<col style="width: 50%" />
</colgroup>
<thead>
<tr>
<th style="text-align: center;">Pathfinding Tasks</th>
<th style="text-align: center;">1,705</th>
</tr>
</thead>
<tbody>
<tr>
<td style="text-align: center;">Actions Requested</td>
<td style="text-align: center;">1,501</td>
</tr>
<tr>
<td style="text-align: center;">Notifications Sent</td>
<td style="text-align: center;">1,086</td>
</tr>
<tr>
<td style="text-align: center;">Vision Sphere Updates</td>
<td style="text-align: center;">30,759</td>
</tr>
<tr>
<td colspan="2" style="text-align: center;">Computation Averages for 150
NPCs Over 60 Seconds</td>
</tr>
<tr>
<td colspan="2" style="text-align: center;">Figure 38</td>
</tr>
</tbody>
</table>
<p>In Figure 39, the dynamic voxel attributes above each NPC’s head can
be observed. These are helpful to discern important attributes such as
stamina (blue), hunger (yellow), health (orange), stored food (purple),
and communication voxel (grey – nothing, blue – notifying, green –
accepted notification, and red – discarded notification).</p>
<table>
<colgroup>
<col style="width: 100%" />
</colgroup>
<thead>
<tr>
<th style="text-align: center;"><img src="https://github.com/user-attachments/assets/96d36345-4e73-4dac-8947-9141c1ca1a45"
style="width:6.15809in;height:3.34846in" /></th>
</tr>
</thead>
<tbody>
<tr>
<td style="text-align: center;">In-game screenshot showing NPC’s dynamic
voxel attributes</td>
</tr>
<tr>
<td style="text-align: center;">Figure 39</td>
</tr>
</tbody>
</table>
<p>The in-game screenshots from Figure 40 and 41 are an attempt to show
the dynamic behaviours of the different types of NPCs. Particularly in
Figure 41, where multiple Tiger NPCs are attempting to attack the same
Peacock NPC prey target.</p>
<table>
<colgroup>
<col style="width: 100%" />
</colgroup>
<thead>
<tr>
<th style="text-align: center;"><img src="https://github.com/user-attachments/assets/9d520c4c-2ce8-46ee-a71b-11d9cf77685d"
style="width:6.1581in;height:3.34847in" /></th>
</tr>
</thead>
<tbody>
<tr>
<td style="text-align: center;">In-game screenshot showing NPC
interacting</td>
</tr>
<tr>
<td style="text-align: center;">Figure 40</td>
</tr>
</tbody>
</table>
<table>
<colgroup>
<col style="width: 100%" />
</colgroup>
<thead>
<tr>
<th style="text-align: center;"><img src="https://github.com/user-attachments/assets/cccc160a-1dfc-4bb6-b5a5-f37f61a6fc1d"
style="width:6.1581in;height:3.34846in" /></th>
</tr>
</thead>
<tbody>
<tr>
<td style="text-align: center;">In-game screenshot showing multiple
Tiger NPCs trying to attack a single Peacock NPC</td>
</tr>
<tr>
<td style="text-align: center;">Figure 41</td>
</tr>
</tbody>
</table>
<h2 id="section"></h2>
<h2 id="self-reflection">Self-Reflection</h2>
<p>The project was a huge personal undertaking, because with no prior
experience in C++ and Unreal Engine and limited knowledge of the
optimisation algorithms used, learning and developing the project within
the time limit was a significant and demanding task, although a
rewarding one. Visualising the data that was created and needed for the
bitwise operations was particularly challenging and took a significant
time to get the calculations right.</p>
<h1 id="conclusion-and-future-work">Conclusion and Future Work</h1>
<p>In conclusion, the project successfully achieves most of the
requirements initially set, with great optimisation results and with a
large landscape and NPC ecosystem simulation. The dynamic NPC actions
also result in interesting and almost realistic, natural
interactions.</p>
<p>As for the project’s shortcomings, because implementing a way to
fully visualise an NPC’s full list of attributes required significant
work to make it fit the current design, the NPC attributes were not
assigned based on Perlin noise, only their world position. Using noise
for manipulating attributes would’ve also increased the testing phase.
Furthermore, a more complex decision system could have been developed,
adding even more possible actions that could further increase the
interaction’s complexity. Lastly, there are a couple of pointer
mismanagement issues in the LOD system that introduce a few bugs that
can lead to a program crash. This could have been avoided through more
testing and the use of smart pointers in some cases.</p>
<p>In terms of future work, focusing on solving the current project’s
shortcomings, increasing the AI complexity while also maintaining
performance, and improving the foliage diversity and terrain biomes,
would drastically improve the simulation’s realism.</p>
<h1 id="references">References</h1>
<p>Hui, D. K.-Y., 2023. AIPS Search CM3038 Artificial Intelligence for
Problem Solving. Aberdeen: RGU.</p>
<p>Omabuarts Studio, 2023. Jungle Animals Vol 1 - Quirky Series.
[Online]</p>
<p>Available at:
https://www.omabuarts.com/product/jungle-animals-quirky-series/</p>
<p>[Accessed 4 May 2025].</p>
<p>Patel, A., 1997. Introduction to A*, theory.stanford.edu.
[Online]</p>
<p>Available at:
https://theory.stanford.edu/~amitp/GameProgramming/AStarComparison.html</p>
<p>[Accessed 4 May 2025].</p>
<p>Peck, J., 2021. FastNoiseLite. [Online]</p>
<p>Available at: https://github.com/Auburn/FastNoiseLite</p>
<p>[Accessed 2024].</p>
<p>Tantan, 2024. Blazingly Fast Greedy Mesher - Voxel Engine
Optimizations. [Online]</p>
<p>Available at: https://www.youtube.com/watch?v=qnGoGq7DWM</p>
<p>[Accessed 4 May 2025].</p>
