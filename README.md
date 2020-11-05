# **VulkanApp**

This is a project I'm using to explore the Vulkan API and graphics theory. The core renderer is based on various Vulkan tutorials, samples and articles. 

![alt text](https://github.com/FergusBrown/VulkanApp/blob/master/Images/Title.png "Title Image")

## **Contents**

- [Features](#features)
    - [Application Creation](#application-creation)
    - [Model Loading](#model-loading)
    - [Mipmap Generation](#mipmap-generation)
    - [First Person Controls](#first-person-controls)
- [Draw Function Methodology](#draw-function-methodology)
    - [Multithreaded Command Recording](#multithreaded-command-recording)
- [Applications](#applications)
    - [Application Creation](#application-creation)
    - [Model Loading](#model-loading)
    - [Mipmap Generation](#mipmap-generation)
## **Features**

The core of this project is the [VulkanRenderer](https://github.com/FergusBrown/VulkanApp/blob/master/VulkanApp/Renderer/VulkanRenderer.cpp) abstract class which can be inherited to create applications. This class handles the setup of most of the fundamental Vulkan objects when creating an application such as Instance, Device . Furthermore, most Vulkan objects are abstracted to a class to simplify object creation. In many information is inferred from other abstracted classes during object creation. This greatly cuts down on the code that needs to be written for an application as most Vulkan CreateInfo structures are automatically handle by class constructors. 

Object destruction is also simplified since vkDestroy functions are called in the destructors of the associated classes. This means that object destruction order is automatically handled in the correct order reducing the chance of errors.

The sections below detail some features in the VulkanRenderer abstract class which are common across applications.

### Application Creation

What is handled by the VulkanRenderer class and what abstract functions must be implemented by applications is summaried below:

#### Initialisation



#### Per Frame

### Model Loading

Assimp is used to import meshes and materials, the implementation can be found in [ModelLoader.cpp](https://github.com/FergusBrown/VulkanApp/blob/master/VulkanApp/Renderer/ModelLoader.cpp). The VulkanRenderer class is setup to import meshes and the associated diffuse, normal and specular textures if they exist. Upon being imported, a descriptor with samplers for the material textures associated with each mesh is created. If a texture for a particular texture property does not exist a default black texture is bound in its place.

### Mipmap Generation

When a [Texture](https://github.com/FergusBrown/VulkanApp/blob/master/VulkanApp/Renderer/Texture.cpp) object is created, mipmaps are automatically created by blitting the texture image to smaller dimensions. 

A comparison of a scene with and without mipmaps is shown below.

![alt text](https://github.com/FergusBrown/VulkanApp/blob/master/Images/mipmap_compare.png "Mipmap Comparison")

### First Person Controls



## **Draw Function Methodology**

This section discusses the structure of the draw() function which is called every frame. A similar setup is used in each application. An overview of the draw procedure is summaried below:

list 

### Multithreaded Command Recording

This is not implemented in the VulkanRenderer class but the methodology used for recording commands is common across applications. TODO

## **Applications**

![alt text](https://github.com/FergusBrown/VulkanApp/blob/master/Images/SSAO_scene_360p.gif "Example Scene")

The following sections detail applications which each shade a scene using different techniques. Crytek's Sponza sample scene is used and has 3 point lights and a spotlight acting as a flashlight. The first person camera can be used to navigate the scene as shown in the GIF above.

### Forward Rendering

<img src="https://github.com/FergusBrown/VulkanApp/blob/master/Images/Forward.PNG" width="250">
(Note: the application actually also has a second subpass for post processing using depth buffer data which is not shown here)

This application uses the setup shown above with a single pass for shading geometry and calculating lighting. In this application lighting for fragments is recalculated after each new mesh is drawm, resulting in many obsolete calculations. This is compared to deferred rendering in the next section.

### Deferred Rendering

<img src="https://github.com/FergusBrown/VulkanApp/blob/master/Images/Deferred.png" width="500">

#### Performance Comparison



### SSAO

<img src="https://github.com/FergusBrown/VulkanApp/blob/master/Images/SSAO.PNG" width="1000">

![alt text](https://github.com/FergusBrown/VulkanApp/blob/master/Images/SSAO_compare.png "SSAO Comparison")
