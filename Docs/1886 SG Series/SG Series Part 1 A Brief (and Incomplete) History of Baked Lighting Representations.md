---
title: "SG Series Part 1: A Brief (and Incomplete) History of Baked Lighting Representations"
source: https://mynameismjp.wordpress.com/2016/10/09/sg-series-part-1-a-brief-and-incomplete-history-of-baked-lighting-representations/
author:
  - "[[MJP]]"
published: 2016-10-10
created: 2026-06-14
description: "You can find an ad-free static site version of this post here: https://therealmjp.github.io/posts/sg-series-part-1-a-brief-and-incomplete-history-of-baked-lighting-representations/ This is part 1 of a series on Spherical Gaussians and their applications for pre-computed lighting. You can find the other articles here: Part 1 - A Brief (and Incomplete) History of Baked Lighting Representations Part 2 - Spherical Gaussians 101 Part 3 - Diffuse…"
tags:
  - Clippings review
updated: 2026-06-14T17:14
---
**You can find an ad-free static site version of this post here: [https://therealmjp.github.io/posts/sg-series-part-1-a-brief-and-incomplete-history-of-baked-lighting-representations/](https://therealmjp.github.io/posts/sg-series-part-1-a-brief-and-incomplete-history-of-baked-lighting-representations/)**

*This is part 1 of a series on Spherical Gaussians and their applications for pre-computed lighting. You can find the other articles here:*

Part 1 – [A Brief (and Incomplete) History of Baked Lighting Representations](https://mynameismjp.wordpress.com/2016/10/09/sg-series-part-1-a-brief-and-incomplete-history-of-baked-lighting-representations/)  
Part 2 – [Spherical Gaussians 101](https://mynameismjp.wordpress.com/2016/10/09/sg-series-part-2-spherical-gaussians-101/)  
Part 3 – [Diffuse Lighting From an SG Light Source](https://mynameismjp.wordpress.com/2016/10/09/sg-series-part-3-diffuse-lighting-from-an-sg-light-source/)  
Part 4 – [Specular Lighting From an SG Light Source](https://mynameismjp.wordpress.com/2016/10/09/sg-series-part-4-specular-lighting-from-an-sg-light-source/)  
Part 5 – [Approximating Radiance and Irradiance With SG’s](https://mynameismjp.wordpress.com/2016/10/09/sg-series-part-5-approximating-radiance-and-irradiance-with-sgs/)  
Part 6 – [Step Into The Baking Lab](https://mynameismjp.wordpress.com/2016/10/09/sg-series-part-6-step-into-the-baking-lab/)

For part 1 of this series, I’m going to provide some background material for our research into Spherical Gaussians. The main purpose is cover some of the alternatives to the approach we used for The Order: 1886, and also to help you understand why we decided to persue Spherical Gaussians. The main empahasis is going to be on discussing what exactly we store in pre-baked lightmaps and probes, and how that data is used to compute diffuse or specular lighting. If you’re already familiar with the concepts of pre-computing radiance or irradiance and approximating them using basis functions like the HL2 basis or Spherical Harmonics, then you will probably want to skip to the next article.

Before we get started, here’s a quick glossary of the terms I use the formulas:

- $L_{o}$ – the outgoing radiance (lighting) towards the viewer
- $L_{i}$ – the incoming radiance (lighting) hitting the surface
- $\mathbf{o}$  – the direction pointing towards the viewer (often denoted as “V” in shader code dealing with lighting)
- $\mathbf{i}$  – the direction pointing towards the incoming radiance hitting the surface (often denoted as “L” in shader code dealing with lighting)
- $\mathbf{n}$  – the direction of the surface normal
- $\mathbf{x}$  – the 3D location of the surface point
- $\int_{\Omega}$  – integral about the hemisphere
- $\theta_{i}$ – the angle between the surface normal and the incoming radiance direction
- $\theta_{o}$ – the angle between the surface normal and the outgoing direction towards the viewer
- $f()$ – the BRDF of the surface

### The Olden Days – Storing Irradiance

Games have used pre-computed lightmaps for almost as long as they have been using shaded 3D graphics, and they’re still quite popular in 2016. The idea is simple: pre-compute a lighting value for every texel, then sample those lighting values at runtime to determine the final appearance of a surface. It’s a simple concept to grasp, but there are some details you might not think about if you’re just learning how they work. For instance, what exactly does it mean to store “lighting” in a texture? What exact value are we computing, anyway? In the early days the value fetched from the lightmap was simply multiplied with the material’s diffuse albedo color (typically done with fixed-function texture stages), and then directly output to the screen. Ignoring the issue of gamma correction and sRGB transfer functions for the moment, we can work backwards from this simple description to describe this old-school approach in terms of the rendering equation. This might seem like a bit of a pointless exercise, but I think it helps build a solid base that we can use to discuss more advanced techniques.

So we know that our lightmap contains a single fixed color per-texel, and we apply it the same way regardless of the viewing direction for a given pixel. This implies that we’re using a simple Lambertian diffuse BRDF, since it lacks any sort of view-dependence. Recall that we compute the outgoing radiance for a single point using the following integral:

$$
L_{o}(\mathbf{o}, \mathbf{x}) = \int_{\Omega}f(\mathbf{i}, \mathbf{o}, \mathbf{x}) \cdot L_{i}(\mathbf{i}, \mathbf{x}) \cdot cos(\theta_{i}) \cdot d\Omega
$$

If we substitute the standard diffuse BRDF of $\frac{C_{diffuse}}{\pi}$  for our BRDF (where C <sub>diffuse</sub> is the diffuse albedo of the surface), then we get the following:

$$
L_{o}(\mathbf{o}, \mathbf{x}) = \int_{\Omega} \frac{C_{diffuse}}{\pi} \cdot L_{i}(\mathbf{i}, \mathbf{x}) \cdot cos(\theta_{i}) \cdot d\Omega
$$
  

$$
= \frac{C_{diffuse}}{\pi} \int_{\Omega} L_{i}(\mathbf{i}, \mathbf{x}) \cdot cos(\theta_{i}) \cdot d\Omega
$$
 

On the right side we see that we can pull the constant terms out the integral (the constant term is actually the entire BRDF!), and what we’re left with lines up nicely with how we handle lightmaps: the expensive integral part is pre-computed per-texel, and then the constant term is applied at runtime per-pixel. The “integral part” is actually computing the incident irradiance, which lets us finally identify the quantity being stored in the lightmap: it’s irradiance! In practice however most games would not apply the 1 / π term at runtime, since it would have been impractical to do so. Instead, let’s assume that the 1 / π was “baked” into the lightmap, since it’s constant for all surfaces (unlike the diffuse albedo, which we consider to be *spatially varying*). In that case, we’re actually storing a reflectance value that takes the BRDF into account. So if we wanted to be precise, we would say that it contains “the diffuse reflectance of a surface with C <sub>diffuse</sub> = 1.0″, AKA the maximum possible outgoing radiance for a surface with a diffuse BRDF.

### Light Map: Meet Normal Map

One of the key concepts of lightmapping is the idea of reconstructing the final surface appearance using data that’s stored at different rates in the spatial domain. Or in simpler words, we store lightmaps using one texel density while combining it with albedo maps that have a different (usually higher) density. This lets us retain the appearance of high-frequency details without actually computing irradiance integrals per-pixel. But what if we want to take this concept a step further? What it we also want the irradiance itself to vary in response to texture maps, and not just the diffuse albedo? By the early 2000’s normal maps were starting to see common use for this purpose, however they were generally only used when computing the contribution from punctual light sources. Normal maps were no help with light maps that only stored a single (scaled) irradiance value, which meant that that pure ambient lighting would look very flat compared to areas using dynamic lighting:*Areas in direct lighting (on the right) have a varying appearance due to a normal map, but areas in shadow (on the left) have no variation due to being lit by a baked lightmap containing only a single irradiance value.  
由于使用了法线贴图，直接光照区域（右侧）的外观会有所不同，但阴影区域（左侧）由于使用了烘焙光照贴图，只包含一个辐照度值，因此没有变化。*

To make lightmaps work with normal mapping, we need to stop storing a single value and instead somehow store a *distribution* of irradiance values for every texel. Normal maps contain a range of normal directions, where those directions are generally restricted to the hemisphere around a point’s surface normal. So if we want our lightmap to store irradiance values for all possible normal map values, then it must contain a distribution of irradiance that’s defined for that same hemisphere. One of the earliest and simplest examples of such a distribution was used by [Half-Life 2](http://www.valvesoftware.com/publications/2006/SIGGRAPH06_Course_ShadingInValvesSourceEngine.pdf) \[1\], and was referred to as [Radiosity Normal Mapping](http://www2.ati.com/developer/gdc/D3DTutorial10_Half-Life2_Shading.pdf) \[2\]:  
为了使光照贴图与法线贴图协同工作，我们需要停止存储单个值，而是以某种方式存储每个纹素的辐照度值 *分布* 。法线贴图包含一系列法线方向，这些方向通常限制在以点表面法线为中心的半球内。因此，如果我们希望光照贴图存储所有可能的法线贴图值的辐照度值，那么它必须包含一个定义在该半球内的辐照度分布。Half [\-Life 2](http://www.valvesoftware.com/publications/2006/SIGGRAPH06_Course_ShadingInValvesSourceEngine.pdf) \[1\] 中使用了这种分布的最早也是最简单的示例之一，它被称为 [辐射度法线贴图](http://www2.ati.com/developer/gdc/D3DTutorial10_Half-Life2_Shading.pdf) \[2\]：

![HL2_Basis](https://mynameismjp.wordpress.com/wp-content/uploads/2016/06/hl2_basis.png)

*Image from “Shading in Valve’s Source Engine “, SIGGRAPH 2006  
图片来自“Valve Source 引擎中的着色”，SIGGRAPH 2006*

Valve essentially modified their lightmap baker to compute 3 values instead of 1, with each value computed by projecting the irradiance signal onto one of the corresponding orthogonal [basis vectors](https://en.wikipedia.org/wiki/Basis_\(linear_algebra\)) in the above image. At runtime, the irradiance value used for shading would be computed by blending the 3 lightmap values based on the cosine of the angle between the normal map direction and the 3 basis directions (which is cheaply computed using a dot product). This allowed them to effectively vary the irradiance based on the normal map direction, thus avoiding the “flat ambient” problem described above.  
Valve 对其光照贴图烘焙器进行了改进，使其计算三个值而非一个。每个值都是通过将辐照度信号投影到上图中对应的三个正交 [基向量](https://en.wikipedia.org/wiki/Basis_\(linear_algebra\)) 之一上计算得到的。在运行时，用于着色的辐照度值将根据法线贴图方向与三个基向量方向之间夹角的余弦值（使用点积计算，计算成本很低）混合这三个光照贴图值来计算。这使得他们能够根据法线贴图方向有效地改变辐照度，从而避免了上述“平坦环境光”问题。

While this worked for their static geometry, there still remained the issue of applying pre-computed lighting to dynamic objects and characters. Some early games (such as the original Quake) used tricks like sampling the lightmap value at a character’s feet, and using that value to compute ambient lighting for the entire mesh. Other games didn’t even do that much, and would just apply dynamic lights combined with a global ambient term. Valve decided to take a more sophisticated approach that extended their hemispherical lightmap basis into a full spherical basis formed by 6 orthogonal basis vectors:  
虽然这种方法对静态几何体有效，但如何将预先计算的光照应用于动态物体和角色仍然是个问题。一些早期游戏（例如初代《雷神之锤》）使用了一些技巧，例如在角色脚部采样光照贴图值，并使用该值来计算整个网格的环境光照。其他游戏甚至没有进行如此复杂的处理，只是简单地应用动态光照并结合全局环境光项。Valve 决定采用一种更复杂的方法，将他们原有的半球形光照贴图基扩展到由 6 个正交基向量构成的完整球形基：

[![ambientcube](https://mynameismjp.wordpress.com/wp-content/uploads/2016/10/ambientcube.png)](https://mynameismjp.wordpress.com/wp-content/uploads/2016/10/ambientcube.png)

*Image from “Shading in Valve’s Source Engine “, SIGGRAPH 2006  
图片来自“Valve Source 引擎中的着色”，SIGGRAPH 2006*

The basis vectors coincided with the 6 face directions of a unit cube, which led Valve to call this basis the “Ambient Cube”. By projecting irradiance in all directions around a point in space (instead of a hemisphere surrounding a surface normal) onto their basis functions, a dynamic mesh could sample irradiance for any normal direction and use it to compute diffuse lighting. This type of representation is often referred to as a *lighting probe*, or often just “probe” for short.  
基向量与单位立方体的六个面方向重合，因此 Valve 将此基称为“环境立方体”。通过将空间中某一点周围所有方向的辐照度（而非围绕表面法线的半球）投影到基函数上，动态网格可以对任意法线方向的辐照度进行采样，并用它来计算漫反射光照。这种表示方法通常被称为 *光照探针* ，或简称“探针”。

### Going Specular 精彩纷呈

With Valve’s basis we can combine normal maps and light maps to get diffuse lighting that can vary in response to high-frequency normal maps. So what’s next? For added realism we would ideally like to support more complex BRDF’s, including view-dependent specular BRDF’s. Half-Life 2 handled environment specular by pre-generating cubemaps at hand-placed probe locations, which is still a common approach used by modern games (albeit with the addition of [pre-filtering](http://blog.selfshadow.com/publications/s2013-shading-course/karis/s2013_pbs_epic_notes_v2.pdf) \[3\] used to approximate the response from a microfacet BRDF). However the large memory footprint of cubemaps limits the practical density of specular probes, which can naturally lead to issues caused by incorrect parallax or disocclusion.  
借助 Valve 的基础技术，我们可以将法线贴图和光照贴图结合起来，获得能够响应高频法线贴图而变化的漫反射光照。那么下一步是什么呢？为了增强真实感，我们理想情况下希望支持更复杂的 BRDF，包括与视角相关的镜面反射 BRDF。Half-Life 2 通过在手动放置的探针位置预先生成立方体贴图来处理环境镜面反射，这仍然是现代游戏常用的方法（尽管现在会添加 [预过滤](http://blog.selfshadow.com/publications/s2013-shading-course/karis/s2013_pbs_epic_notes_v2.pdf) \[3\] 来近似微面 BRDF 的响应）。然而，立方体贴图占用大量内存，限制了镜面反射探针的实际密度，这自然会导致视差或遮挡错误等问题。

[![EnvMap_Disocclusion](https://mynameismjp.wordpress.com/wp-content/uploads/2016/06/envmap_disocclusion.png)](https://mynameismjp.wordpress.com/wp-content/uploads/2016/06/envmap_disocclusion.png)

*A combination of incorrect parallax and disocclusion when using a pre-filtered environment as a source for environment specular. Notice the bright edges on the sphere, which are actually caused by the sphere reflecting itself!  
使用预过滤环境作为环境镜面反射源时，会出现视差和遮挡错误同时出现的问题。请注意球体上的亮边，这些亮边实际上是球体自身反射造成的！*

With that in mind it would nice to be able to get some sort of specular response out of our lightmaps, even if only for a subset of materials. But if that is our goal, then our approach of storing an irradiance distribution starts to become a hinderance. Recall from earlier that with a diffuse BRDF we were able to completely pull the BRDF out of the irradiance integral, since the Lambertian diffuse BRDF is just a constant term. This is no longer the case even with a simple specular BRDF, whose value varies depending on both the viewing direction as well as the incident lighting direction.  
考虑到这一点，如果能从光照贴图中获得某种镜面反射响应，即使只针对部分材质，也是非常理想的。但如果这就是我们的目标，那么我们存储辐照度分布的方法就会成为一种阻碍。回想一下，之前使用漫反射 BRDF 时，我们可以完全从辐照度积分中提取 BRDF，因为朗伯漫反射 BRDF 只是一个常数项。但即使是简单的镜面 BRDF，情况也不再如此，因为它的值会随着观察方向和入射光方向的变化而变化。

If you’re working with the Half-Life 2 basis (or something similar), a tempting option might be to compute a specular term as if the 3 basis directions were directional lights. If you think about what this means, it’s basically what you get if you decide to say “screw it” and pull the specular BRDF out of the irradiance integral. So instead of Integrate(BRDF \* Lighting \* cos(theta)), you’re doing BRDF \* Integrate(Lighting \* cos(theta)). This will definitely give you *something,* and it’s perhaps a lot better than nothing. But you’ll also effectively lose out on a ton of your specular response, since you’ll only get specular when your viewing direction appropriately lines up with your basis directions according the the BRDF slice. To show you what I mean by this, here’s a comparison:  
如果你使用的是 Half-Life 2 的基准模型（或类似模型），一个看似诱人的方法是，将三个基准方向视为平行光，并计算镜面反射项。仔细想想，这其实就相当于你直接把镜面反射 BRDF 从辐照度积分中分离出来。也就是说，你不再使用 \`Integrate(BRDF \* Lighting \* cos(theta))\`，而是使用 \`BRDF \* Integrate(Lighting \* cos(theta))\`。这样做肯定能得到 *一些结果，* 总比没有强。但你也会损失大量的镜面反射信息，因为只有当你的视角方向与基准方向根据 BRDF 切片正确对齐时，才能获得镜面反射。为了更清楚地说明我的意思，这里有一个对比：

[![irradiance_specular_comparison](https://mynameismjp.wordpress.com/wp-content/uploads/2016/09/irradiance_specular_comparison1.png?w=1084)](https://mynameismjp.wordpress.com/wp-content/uploads/2016/09/irradiance_specular_comparison1.png)

*The top image shows a path-traced rendering of a green wall being lit by direct sun lighting. The middle image shows the indirect specular component of the top image, with exposure increased by 4x. The bottom image shows the resulting specular from treating the HL2 basis directions as directional lights.  
上图显示的是阳光直射下绿色墙体的路径追踪渲染结果。中间图显示的是上图的间接镜面反射分量，曝光度提高了 4 倍。下图显示的是将 HL2 基方向视为平行光后得到的镜面反射结果。*

Hopefully these images clearly show the problem that I’m describing. In the bottom image, you get specular reflections that look just like they came from a few point lights, since that’s effectively what you’re simulating. Meanwhile in the middle image with proper environment reflections, you can see that the the entire green wall effectively acts as an area light, and you get a very broad specular reflections across the entire floor. In general the problem tends to be less noticeable though as roughness increases, since higher roughness naturally results in broader, less-defined reflections that are harder to notice.  
希望这些图片能清晰地展示我描述的问题。在底部的图片中，你会看到镜面反射，看起来就像是由几个点光源产生的，因为你模拟的实际上就是点光源。而在中间的图片中，由于环境反射正确，你可以看到整面绿墙实际上充当了面光源，导致整个地面都出现了非常宽广的镜面反射。一般来说，随着粗糙度的增加，这个问题会变得不那么明显，因为更高的粗糙度自然会导致更宽广、更模糊的反射，从而更难被注意到。

### Let’s Try Spherical Harmonics我们来试试球谐函数

If we want to do better, we must instead find a way to store a radiance distribution and then efficiently integrate it against our BRDF. It’s at this point that we turn to spherical harmonics. Spherical harmonics (SH for short) have become a popular tool for real-time graphics, typically as a way to store an approximation of indirect lighting at discrete probe locations. I’m not going to go into the full specifics of SH since that could easily fill an [entire article](http://www.research.scea.com/gdc2003/spherical-harmonic-lighting.pdf) \[4\] on its own. If you have no experience with SH, the key thing to know about them is that they basically let you approximate a function defined on a sphere using a handful of coefficients (typically either 4 or 9 floats per RGB channel). It’s sort-of as if you had a compact cubemap, where you can take a direction vector and get back a value associated with that direction. The big catch is that you can only represent very low-frequency (fuzzy) signals with lower-order SH, which can limit what sort of things you can do with it. You can project detailed, high-frequency signals onto SH if you want to, but the resulting projection will be very blurry. Here’s an example showing what an HDR environment map looks like projected onto L2 SH, which requires 27 coefficients for RGB:  
如果我们想要做得更好，就必须找到一种方法来存储辐射分布，然后高效地将其与我们的双向反射分布函数 (BRDF) 进行积分。这时，我们就需要用到球谐函数了。球谐函数（简称 SH）已成为实时图形领域的一种常用工具，通常用于存储离散探针位置处间接光照的近似值。我不会详细介绍 SH 的全部细节，因为这本身就足以写成一 [整篇文章](http://www.research.scea.com/gdc2003/spherical-harmonic-lighting.pdf) \[4\]。如果您没有 SH 的使用经验，那么需要了解的关键一点是，它基本上允许您使用少量系数（通常每个 RGB 通道 4 或 9 个浮点数）来近似定义在球面上的函数。这有点像您有一个紧凑的立方体贴图，您可以输入一个方向向量并得到与该方向相关的值。需要注意的是，您只能使用低阶 SH 来表示非常低频（模糊）的信号，这会限制您可以用它做的事情。你可以将精细的高频信号投影到 SH 上，但投影结果会非常模糊。以下示例展示了 HDR 环境贴图投影到 L2 SH 上的效果，该投影需要 27 个 RGB 系数：

[![Wells_Radiance](https://mynameismjp.wordpress.com/wp-content/uploads/2016/06/wells_radiance.png)](https://mynameismjp.wordpress.com/wp-content/uploads/2016/06/wells_radiance.png)

[![Wells_Radiance_SH](https://mynameismjp.wordpress.com/wp-content/uploads/2016/06/wells_radiance_sh.png)](https://mynameismjp.wordpress.com/wp-content/uploads/2016/06/wells_radiance_sh.png)

*The top image is an HDR environment map containing incoming radiance values about a sphere, while the bottom image shows the result of projecting that environment onto L2 spherical harmonics.*

In the case of irradiance, SH can work pretty well since it’s naturally low-frequency. The integration of incoming radiance against the cosine term effectively acts as a low-pass filter, which makes it a suitable candidate for approximation with SH. So if we project irradiance onto SH for every probe location or lightmap texel, we can now do an SH “lookup” (which is basically a few computations followed by a dot product with the coefficients) to get the irradiance in any direction on the sphere. This means we can get spatial variation from albedo and normal maps just like with the HL2 basis!

It also turns out that SH is pretty useful for *computing* irradiance from input radiance, since we can do it really cheaply. In fact it can do it so cheaply, it can be done at runtime by folding it into the SH lookup process. The reason it’s so cheap is because SH is effectively a frequency-domain representation of the signal, and when you’re in the frequency domain convolutions can be done with simple multiplication. In the spatial domain, convolution with a cubemap is an N^2 operation involving many samples from an input radiance cubemap. If you’re interested in the full details, the process was described in Ravi Ramamoorthi’s [seminal paper](https://cseweb.ucsd.edu/~ravir/papers/envmap/) \[5\] from 2001, with derivations provided in [another article](https://cseweb.ucsd.edu/~ravir/papers/invlamb/) \[6\].

[![SH_Diffuse](https://mynameismjp.wordpress.com/wp-content/uploads/2016/06/sh_diffuse.png?w=600)](https://mynameismjp.wordpress.com/wp-content/uploads/2016/06/sh_diffuse.png)

*The Stanford Bunny model being lit with diffuse lighting from an L2 spherical harmonics probe*

So we’ve established that SH works for approximating irradiance, and that we can convert from radiance to irradiance at runtime. But what does this have to do with specular? By storing an approximation of radiance instead of irradiance in our probes or lightmaps (albeit, a very blurry version of radiance), we now have the signal that we need to integrate our specular BRDF against in order to produce specular reflections. All we need is an SH representation of our BRDF, and we’re a dot product away from environment specular! The only problem we have to solve is how to actually *get* an SH representation of our BRDF.

Unfortunately a microfacet specular BRDF is quite a bit more complicated than a Lambertian diffuse BRDF, which makes our lives more difficult. For diffuse lighting we only needed to worry about the cosine lobe, which has the same shape regardless of the material or viewing direction. However a specular lobe will vary in shape and intensity depending on the viewing direction, material roughness, and the fresnel term at zero incidence (AKA F <sub>0</sub>). If all else fails, we can always use monte-carlo techniques to pre-compute the coefficients and store the result in a lookup texture. At first it may seem like we need at parameterize our lookup table on 4 terms, since the viewing direction is two-dimensional. However we can drop a dimension if we follow in the [footsteps](https://developer.amd.com/wordpress/media/2012/10/S2008-Chen-Lighting_and_Material_of_Halo3.pdf) \[7\] of the intrepid engineers at Bungie, who used a neat trick for their [SH specular implementation in Halo 3](http://developer.amd.com/wordpress/media/2013/01/Chapter01-Chen-Lighting_and_Material_of_Halo3.pdf) \[8\]. The key insight that they shared was that the specular lobe shape doesn’t actually change as the viewer rotates around the local Z axis of the shading point (AKA the surface normal). It actually only changes based on the *viewing angle*, which is the angle between the view vector and the local Z axis of the surface. If we exploit this knowledge, we can pre-compute the coefficients for the set of possible viewing directions that are aligned with the local X axis. Then at runtime, we can rotate the coefficients so that the resulting lobe lines up with the actual viewing direction. Here’s an image to show you what I mean:  
不幸的是，微面镜面反射 BRDF 比朗伯漫反射 BRDF 复杂得多，这给我们带来了更多困难。对于漫反射光照，我们只需要关注余弦瓣，它的形状与材质或观察方向无关。然而，镜面反射瓣的形状和强度会根据观察方向、材质粗糙度和零入射角下的菲涅尔项（即 F <sub>0</sub> ）而变化。如果所有方法都失败了，我们始终可以使用蒙特卡罗方法来预先计算系数并将结果存储在查找纹理中。起初，我们似乎需要将查找表参数化为 4 个项，因为观察方向是二维的。但是，如果我们效仿 Bungie 的工程师们\[7\]的 [做法](https://developer.amd.com/wordpress/media/2012/10/S2008-Chen-Lighting_and_Material_of_Halo3.pdf) ，就可以去掉一个维度。他们 [在《光环 3》\[8\]中为 SH 镜面反射的实现](http://developer.amd.com/wordpress/media/2013/01/Chapter01-Chen-Lighting_and_Material_of_Halo3.pdf) 使用了一个巧妙的技巧。他们分享的关键见解是，当观察者绕着色点的局部 Z 轴（即表面法线）旋转时，镜面反射瓣的形状实际上并不会改变。它只根据 *视角* 而变化，视角是视向量与表面局部 Z 轴之间的夹角。如果我们利用这一知识，就可以预先计算与局部 X 轴对齐的一系列可能视角方向的系数。然后在运行时，我们可以旋转这些系数，使最终的反射瓣与实际的视角方向对齐。下图可以帮助您理解我的意思：

[![SH_Rotation](https://mynameismjp.wordpress.com/wp-content/uploads/2016/06/sh_rotation.png?w=592)](https://mynameismjp.wordpress.com/wp-content/uploads/2016/06/sh_rotation.png)

*Rotating a specular lobe from the X axis to its actual location based on the viewing direction, which is helpful for pre-computing the SH coefficients into a lookup texture  
根据观察方向，将镜面反射瓣从 X 轴旋转到其实际位置，这有助于预先计算查找纹理中的镜面反射系数。*

So in this image the checkerboard is the surface being shaded, and the red, green and blue arrows are the local X, Y, and Z axes of the surface. The transparent lobe represents the specular lobe that we precomputed for a viewpoint that’s aligned with the X axis, but has the same viewing angle. The blue arrow shows how we can rotate the specular lobe from its original position to the actual position of the lobe based on the current viewing position, giving us the desired specular response. Here’s a comparison showing what it looks like it in action:  
图中，棋盘格图案代表被着色的表面，红色、绿色和蓝色箭头分别代表该表面的局部 X、Y 和 Z 轴。透明的瓣状结构代表我们预先计算的镜面反射瓣，该反射瓣是针对与 X 轴对齐但视角相同的视点计算的。蓝色箭头展示了如何根据当前视角将镜面反射瓣从其原始位置旋转到实际位置，从而获得所需的镜面反射效果。以下对比图展示了实际效果：

[![sh_specular_comparison](https://mynameismjp.wordpress.com/wp-content/uploads/2016/09/sh_specular_comparison.png?w=962)](https://mynameismjp.wordpress.com/wp-content/uploads/2016/09/sh_specular_comparison.png)

*The top image is a scene rendered with a path tracer. The middle image shows the indirect specular as rendered by a path tracer, with exposure increased 4x. The bottom image shows the indirect specular term computing an L2 SH lightmap, also with exposure increased by 4x.  
顶部图像是使用路径追踪器渲染的场景。中间图像显示了使用路径追踪器渲染的间接镜面反射，曝光度提高了 4 倍。底部图像显示了计算 L2 SH 光照贴图的间接镜面反射项，曝光度也提高了 4 倍。*

Not too bad, eh? Or at least…not too bad as long as we’re willing to store 27 coefficients per lightmap texel, and we’re only concerned with rough materials. The comparison image used a GGX α parameter of 0.39, which is fairly rough.  
还不错吧？或者说……至少……如果我们愿意为每个光照贴图纹素存储 27 个系数，并且只关注粗糙材质的话，那就还不错。对比图像使用了 GGX α 参数 0.39，这相当粗糙。

One common issue with issue with SH is a phenomenon known as “ringing”, which is described in Peter-Pike Sloan’s [Stupid Spherical Harmonics Tricks](http://www.ppsloan.org/publications/StupidSH36.pdf) \[9\]. Ringing artifacts tends to show up when you have a very intense light source one side of the sphere. When this happens, the SH projection will naturally result in negative lobes on the opposite side of the sphere, which can result very low (or even negative!) values when evaluated. It’s generally not too much of an issue for 2D lightmaps, since lightmaps are only concerned with the incoming radiance for a hemisphere surrounding the surface normal. However they often show up in probes, which store radiance or irradiance about the entire sphere. The solution suggested by Peter-Pike Sloan is to apply a windowing function to the SH coefficients, which will filter out the ringing artifacts. However the windowing will also introduce additional blurring, which may remove high-frequency components from the original signal being projected. The following image shows how ringing artifacts manifest when using SH to compute irradiance from an environment with a bright area light, and also shows how windowing affects the final result:  
球谐函数（SH）的一个常见问题是“振铃”现象，Peter-Pike Sloan 在其著作 [《愚蠢的球谐函数技巧》](http://www.ppsloan.org/publications/StupidSH36.pdf) \[9\] 中对此进行了描述。当球体一侧存在强光源时，振铃伪影往往会出现。此时，SH 投影自然会在球体的另一侧产生负瓣，导致计算结果非常低（甚至为负值！）。对于二维光照贴图而言，这通常不是什么大问题，因为光照贴图只关注围绕表面法线的半球的入射辐射率。然而，在存储整个球体周围辐射率或辐照度的探针中，振铃伪影却经常出现。Peter-Pike Sloan 提出的解决方案是对 SH 系数应用加窗函数，以滤除振铃伪影。但是，加窗也会引入额外的模糊，这可能会去除原始投影信号中的高频分量。下图显示了使用 SH 计算具有明亮面光源的环境辐照度时，振铃伪影是如何产生的，同时也显示了窗口化如何影响最终结果：

![sh_ringing_comparison](https://mynameismjp.wordpress.com/wp-content/uploads/2016/09/sh_ringing_comparison.png)

*A sphere with a Lambertian diffuse BRDF being lit by a lighting environment with a strong area light source. The left image shows the ground-truth result of using monte-carlo integration. The middle image shows the result of projecting radiance onto L2 SH, and then computing irradiance. The right image shows the result of applying a windowing function to the L2 SH coefficients before computing irradiance.  
一个具有朗伯漫反射 BRDF 的球体被强面光源照射。左图显示了使用蒙特卡罗积分得到的真实值结果。中间图显示了将辐射度投影到 L2 SH 上并计算辐照度的结果。右图显示了在计算辐照度之前对 L2 SH 系数应用窗口函数的结果。*

### References 参考

\[1\] Shading in Valve’s Source Engine (SIGGRAPH 2006) – [http://www.valvesoftware.com/publications/2006/SIGGRAPH06\_Course\_ShadingInValvesSourceEngine.pdf  
\[1\] Valve Source 引擎中的着色（SIGGRAPH 2006）– http://www.valvesoftware.com/publications/2006/SIGGRAPH06\_Course\_ShadingInValvesSourceEngine.pdf  
](http://www.valvesoftware.com/publications/2006/SIGGRAPH06_Course_ShadingInValvesSourceEngine.pdf)\[2\] Half Life 2 / Valve Source Shading – [http://www2.ati.com/developer/gdc/D3DTutorial10\_Half-Life2\_Shading.pdf  
\[2\] Half-Life 2 / Valve Source Shading – http://www2.ati.com/developer/gdc/D3DTutorial10\_Half-Life2\_Shading.pdf  
](http://www2.ati.com/developer/gdc/D3DTutorial10_Half-Life2_Shading.pdf)\[3\] Real Shading in Unreal Engine 4 – [http://blog.selfshadow.com/publications/s2013-shading-course/karis/s2013\_pbs\_epic\_notes\_v2.pdf  
\[3\] 虚幻引擎 4 中的真实着色 – http://blog.selfshadow.com/publications/s2013-shading-course/karis/s2013\_pbs\_epic\_notes\_v2.pdf  
](http://blog.selfshadow.com/publications/s2013-shading-course/karis/s2013_pbs_epic_notes_v2.pdf)\[4\] Spherical Harmonic Lighting: The Gritty Details – [https://basesandframes.files.wordpress.com/2016/05/spherical\_harmonic\_lighting\_gritty\_details\_green\_2003.pdf](https://basesandframes.files.wordpress.com/2016/05/spherical_harmonic_lighting_gritty_details_green_2003.pdf)  
\[4\] 球谐照明：细节之处 – [https://basesandframes.files.wordpress.com/2016/05/spherical\_harmonic\_lighting\_gritty\_details\_green\_2003.pdf](https://basesandframes.files.wordpress.com/2016/05/spherical_harmonic_lighting_gritty_details_green_2003.pdf) \[5\] An Efficient Representation for Irradiance Environment Maps – [https://cseweb.ucsd.edu/~ravir/papers/envmap/  
\[5\] 辐照度环境图的高效表示方法 – https://cseweb.ucsd.edu/~ravir/papers/envmap/  
](https://cseweb.ucsd.edu/~ravir/papers/envmap/)\[6\] On the Relationship between Radiance and Irradiance: Determining the illumination from images of a convex Lambertian object – [https://cseweb.ucsd.edu/~ravir/papers/invlamb/  
\[6\] 关于辐射度和辐照度之间的关系：从凸朗伯物体的图像确定照明 – https://cseweb.ucsd.edu/~ravir/papers/invlamb/  
](https://cseweb.ucsd.edu/~ravir/papers/invlamb/)\[7\] The Lighting and Material of Halo 3 (Slides) – [https://developer.amd.com/wordpress/media/2012/10/S2008-Chen-Lighting\_and\_Material\_of\_Halo3.pdf  
\[7\] Halo 3 的光照和材质（幻灯片）– https://developer.amd.com/wordpress/media/2012/10/S2008-Chen-Lighting\_and\_Material\_of\_Halo3.pdf  
](https://developer.amd.com/wordpress/media/2012/10/S2008-Chen-Lighting_and_Material_of_Halo3.pdf)\[8\] The Lighting and Material of Halo 3 (Course Notes) – [http://developer.amd.com/wordpress/media/2013/01/Chapter01-Chen-Lighting\_and\_Material\_of\_Halo3.pdf  
\[8\]《光环 3》的光照和材质（课程笔记）– http://developer.amd.com/wordpress/media/2013/01/Chapter01-Chen-Lighting\_and\_Material\_of\_Halo3.pdf  
](http://developer.amd.com/wordpress/media/2013/01/Chapter01-Chen-Lighting_and_Material_of_Halo3.pdf)\[9\] Stupid Spherical Harmonics Tricks – [http://www.ppsloan.org/publications/StupidSH36.pdf](http://www.ppsloan.org/publications/StupidSH36.pdf)  
\[9\] 愚蠢的球谐函数技巧 – [http://www.ppsloan.org/publications/StupidSH36.pdf](http://www.ppsloan.org/publications/StupidSH36.pdf)