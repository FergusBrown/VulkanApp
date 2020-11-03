# VulkanApp

This is a project I'm using to explore the Vulkan API and graphics theory. The core renderer is based on various Vulkan tutorials, samples and articles. 

## Contents

- [Features](#features)
- [Applications](#applications)

## Features

The core of this project is the [VulkanRenderer](https://github.com/FergusBrown/VulkanApp/blob/master/VulkanApp/Renderer/VulkanRenderer.cpp) abstract class. This class handles the setup of most of the fundamental Vulkan objects when creating an application such as Instance, Device . Furthermore, most Vulkan objects are abstracted to a class to simplify object creation. In many information is inferred from other abstracted classes during object creation. This greatly cuts down on the code that needs to be written for an application as most Vulkan CreateInfo structures are automatically handle by class constructors. 

Object destruction is also simplified since vkDestroy functions are called in the destructors of the associated classes. This means that object destruction order is automatically handled in the correct order reducing the chance of errors.

The sections below detail some features in the VulkanRenderer abstract class which are common across applications.

### Application Creation
TODO


### Model Loading

Assimp is used to import meshes and materials, the implementation can be found in [ModelLoader.cpp](https://github.com/FergusBrown/VulkanApp/blob/master/VulkanApp/Renderer/ModelLoader.cpp). The VulkanRenderer class is setup to import meshes and the associated diffuse, normal and specular textures if they exist. Upon being imported, a descriptor with samplers for the material textures associated with each mesh is created. If a texture for a particular texture property does not exist a default black texture is bound in its place.

### Mipmap Generation

When a [Texture](https://github.com/FergusBrown/VulkanApp/blob/master/VulkanApp/Renderer/Texture.cpp) object is created, mipmaps are automatically created by blitting the texture image to smaller dimensions. 

A comparison of a scene with and without mipmaps is shown below.

![alt text](https://github.com/FergusBrown/VulkanApp/blob/master/Images/mipmap_compare.png "Mipmap Comparison")
## Applications

The following applications each implement the above scene using different techniques. 

### Setup

### Forward Rendering

### Deferred Rendering

### SSAO
