# **VulkanApp**

This is a project I'm using to explore the Vulkan API and graphics theory. The core renderer is based on various tutorials, samples and articles. 

![alt text](https://github.com/FergusBrown/VulkanApp/blob/master/Images/SSAO_scene_360p.gif "Title Image")

## **Summary**

This is a project I have used to familiarise myself with the Vulkan API and explore different graphical techniques. I created an abstract class to implement core renderer features, such as model loading, which can be inherited to create applications. Application creation is simplified as many Vulkan objects are abstracted to classes which handle their creation and lifetime.


To demo the project I created several applications which shade a scene using Phong lighting and can be traversed with a first person camera. The applications compare forward and deferred rendering approaches, implement screen space ambient occlusion and explore the use of multithreaded rendering. RenderDoc was used to debug the applications by inspecting the SPIR-V disassembly for shaders.

## **Contents**

- [Core Features](#core-features)
    - [Model Loading](#model-loading)
    - [Mipmap Generation](#mipmap-generation)
- [Additional Features and Techniques](#additional-features-and-techniques)
    - [First Person Controls](#first-person-controls)
    - [Multithreaded Command Recording](#multithreaded-command-recording)
- [Applications](#applications)
    - [Forward Rendering](#forward-rendering)
    - [Deferred Rendering](#deferred-rendering)
    - [Screen Space Ambient Occlusion](#ssao)
    
## **Core Features**

The core of this project is the [VulkanRenderer](https://github.com/FergusBrown/VulkanApp/blob/master/VulkanApp/Renderer/VulkanRenderer.cpp) abstract class which can be inherited to create applications. This class handles the setup of most of the fundamental Vulkan objects when creating an application such as Instance, Device and Swapchain. Furthermore, most Vulkan objects are abstracted to a class to simplify object creation. For example, with many of these objects information is inferred from other abstracted classes during object creation. Object destruction is also simplified since vkDestroy functions are called in the destructors of the abstracted classes. This means that object destruction order is automatically handled and reducing the chance of errors. Overall, this greatly cuts down on the code that needs to be written for an application.

The sections below detail some features in the VulkanRenderer abstract class which are common across applications.

### **Model Loading**

Assimp is used to import meshes and materials, the implementation can be found in [ModelLoader.cpp](https://github.com/FergusBrown/VulkanApp/blob/master/VulkanApp/Renderer/ModelLoader.cpp). The VulkanRenderer class is setup to import meshes and the associated diffuse, normal and specular textures if they exist. Upon being imported, a descriptor with samplers for the material textures associated with each mesh is created. If a texture for a particular texture property does not exist a default black texture is bound in its place.

### **Mipmap Generation**

When a [Texture](https://github.com/FergusBrown/VulkanApp/blob/master/VulkanApp/Renderer/Texture.cpp) object is created, mipmaps are automatically created by blitting the texture image to smaller dimensions. 

A comparison of a scene with and without mipmaps is shown below.

![alt text](https://github.com/FergusBrown/VulkanApp/blob/master/Images/mipmap_compare.png "Mipmap Comparison")


## **Additional Features and Techniques**

This section discusses additional features and techniques which are not part of the core VulkanRenderer class but which are used in each of the example applications.

### **First Person Controls**

First person controls are implemented by the [InputHandlerMouse](https://github.com/FergusBrown/VulkanApp/blob/master/VulkanApp/InputHandlerMouse.h) and [Pawn](https://github.com/FergusBrown/VulkanApp/blob/master/VulkanApp/Pawn.h) classes. The pawn class stores position, direction, euler angles and other camera related parameters. A pawn can be controlled through functions such as moveForwardBy(float) and rotate(vec3). The InputHandlerMouse class captures mouse and keyboard input and uses the command desgin pattern to create a list of commands which can be executed to update the camera parameters stored in a Pawn object. This system is setup so that the mouse can be used to rotate the Pawn and the keyboard can be used to move the Pawn's position.

### **Multithreaded Command Recording**

Vulkan allows multithreading of command recording through the use of Secondary Command Buffers. These can be recorded in parallel then executed using a primary command buffer. An example of how this can be used can be found in any of the application classes such as [DeferredApp](https://github.com/FergusBrown/VulkanApp/blob/master/VulkanApp/Applications/DeferredApp.h). All meshes which need to be rendered are divided equally between threads. Then, for each thread, draw commands for each of the meshes assigned to a thread can be recorded to a secondary command buffer in parallel. In the example applications the recordSecondaryCommandBuffers() function is submitted to a threadpool for execution allowing for multithreaded command recording.

This method has the limitation that it can only be used for recording draw commands for opaque objects. Transparent objects need to be drawn in the correct order from furthest to closest. Splitting meshes between threads and recording draw commands in parallel does not ensure this so draw commands for transparent objects should be recorded in serial.

## **Applications**

The following sections details the renderpass setup and functionality applications which each shade a scene using different techniques. The associated files for each application can be found in the [Applications](https://github.com/FergusBrown/VulkanApp/tree/master/VulkanApp/Applications) and [Shaders](https://github.com/FergusBrown/VulkanApp/tree/master/VulkanApp/Shaders) directories.

Crytek's Sponza sample scene is used and has 3 point lights (each in different positions and of different colours) and a spotlight acting as a flashlight. The first person camera can be used to navigate the scene as shown in the GIF below.

![alt text](https://github.com/FergusBrown/VulkanApp/blob/master/Images/SSAO_scene_360p.gif "Example Scene")


### **Forward Rendering**

This application uses the setup shown below with a single subpass for shading geometry and calculating lighting. Lighting is calculated in tangent space using information from diffuse, specular and normal textures.

<img src="https://github.com/FergusBrown/VulkanApp/blob/master/Images/Forward.PNG" width="250">

### **Deferred Rendering**

The deferred rendering application first writes diffuse, specular, normal and position data to textures. This data is loaded in subpass 1's fragment shader using  the subpassLoad function and is used to calculate lighting as shown below. In this application the lighting was calculated in world space.

<img src="https://github.com/FergusBrown/VulkanApp/blob/master/Images/Deferred.png" width="500">

#### **Performance Comparison**

Compared to forward rendering, the deferred approach should remove obsolete fragment shader runs as lighting per fragment should only be calculated once. Running the application with RenderDoc showed that the forward app hovered around a frame time of 1ms while the deferred app was around 1.25ms. Using RenderDoc's event browser, draw call durations can be compared with captures from the applications. Note: the timings seem to vary greatly each capture so these numbers may be a bit innaccurate.

| Subpass        | Forward  (μs)         | Deferred (μs)  |
| ------------- |-------------| -----|
| 0      | 1016 | 1312 |
| 1      | -      |   180 |

While the duration for lighting calculations is very short in the deferred setup, the first subpass which writes out to textures takes longer than the entirety of the forward app's single subpass. This indicates that for this particular scene the deferred app's performance is limited by the first subpass writing to textures. Perhaps if there were many more lights in the scene or lighting calculations were more complex then the deferred app might have a slight edge.

### **SSAO**

The SSAO app has a similar approach to the deferred rendering setup but with 2 extra subpasses to implement screen space ambient occlusion. This was done with the guidance of [this](http://john-chapman-graphics.blogspot.com/2013/01/ssao-tutorial.html) tutorial by John Chapman. Additionally, in this application, rather than writing position to a texture, which requires the high precision VK\_FORMAT\_R32G32B32A32_SFLOAT format, position is instead reconstructed using depth buffer data. This was based on [this](https://therealmjp.github.io/posts/position-from-depth-3/) tutorial. This should retain the same precision as writing position to texture while saving memory. Also, fragment shader calculations were all performed in view space in this application which should provide some small performance benefits over the deferred app which used world space.

<img src="https://github.com/FergusBrown/VulkanApp/blob/master/Images/SSAO.PNG" width="1000">

A comparison of the scene with and without SSAO is shown below.

![alt text](https://github.com/FergusBrown/VulkanApp/blob/master/Images/SSAO_compare.png "SSAO Comparison")

This effect works by generating a sample kernel in a hemishpere. For each fragment the hemisphere is normal oriented and the position is taken for each sample. For each sample the depth buffer is sampled using the corresponding screens space coordinate. If the sample position is behind the sampled depth then that sample is occluded and contributes to an occlusion factor which is used to control the amount of ambient light contributing to a fragment's colour. This process is summarised below.

#### **Setup**

The resources required for the SSAO is created in the createSSAOResources() function. This randomly generates N sample positions within a hemishpere. Additionally, a 4 by 4 noise texture is generated to hold random rotation vectors. These rotation vectors are used to rotate the sample kernel, effectively increasing sample count and minimising banding artefacts.

#### **Subpass 1 SSAO Fragment Shader**

The shader used to generate the SSAO can be found [here](https://github.com/FergusBrown/VulkanApp/blob/master/VulkanApp/Shaders/SSAOApp/ssao.frag). In this shader the noise texture is sampled and an orthogonal basis is created between the rotation vector and the fragment normal. Then, for each sample the sample position is transformed to clip space, perspective divide is performed and the resulting xy coordinates are used to sample the depth buffer. The sampled depth is then compared to the sample position. If the depth sample is less than sample position then the sample is occluded. If occluded a 0 is output and otherwise a 1 is output. This is averaged for all the samples to create the occlusion factor meacing the factor will be larger if more samples are occluded. 

The occlusion value is also modified by a range check which lessens the contribution to the occlusion factor if the sample exists outside a defined radius. The final occlusion factor is subtracted from 1 and written to texture. This results in a darker fragment the higher the occlusion factor.

#### **Subpass 2 Blur Fragment Shader**

The blur fragment shader can be found [here](https://github.com/FergusBrown/VulkanApp/blob/master/VulkanApp/Shaders/SSAOApp/blur.frag). This shader uses a simple box blur kernel to mask the noise created by the SSAO pass. 

#### **Subpass 3 Lighting**

The SSAO effect is applied in the lighting fragment shader by simply multiplying the ambient light value by the value sampled from the blur texture.
