---
title: "SG Series Part 6: Step Into The Baking Lab"
source: https://mynameismjp.wordpress.com/2016/10/09/sg-series-part-6-step-into-the-baking-lab/
author:
  - "[[MJP]]"
published: 2016-10-10
created: 2026-06-14
description: "You can find an ad-free static site version of this post here: https://therealmjp.github.io/posts/sg-series-part-6-step-into-the-baking-lab/ This is part 6 of a series on Spherical Gaussians and their applications for pre-computed lighting. You can find the other articles here: Part 1 - A Brief (and Incomplete) History of Baked Lighting Representations Part 2 - Spherical Gaussians 101 Part 3 - Diffuse…"
tags:
  - Clippings review
updated: 2026-06-14T17:17
---
**You can find an ad-free static site version of this post here: [https://therealmjp.github.io/posts/sg-series-part-6-step-into-the-baking-lab/](https://therealmjp.github.io/posts/sg-series-part-6-step-into-the-baking-lab/)  
您可以在这里找到这篇文章的无广告静态网站版本： [https://therealmjp.github.io/posts/sg-series-part-6-step-into-the-baking-lab/](https://therealmjp.github.io/posts/sg-series-part-6-step-into-the-baking-lab/)**

*This is part 6 of a series on Spherical Gaussians and their applications for pre-computed lighting. You can find the other articles here:  
这是关于球面高斯函数及其在预计算光照中应用系列文章的第六部分。您可以在这里找到其他文章：*

Part 1 – [A Brief (and Incomplete) History of Baked Lighting Representations](https://mynameismjp.wordpress.com/2016/10/09/sg-series-part-1-a-brief-and-incomplete-history-of-baked-lighting-representations/)  
第一部分—— [烘焙照明表示的简史（不完整）](https://mynameismjp.wordpress.com/2016/10/09/sg-series-part-1-a-brief-and-incomplete-history-of-baked-lighting-representations/)  
Part 2 – [Spherical Gaussians 101](https://mynameismjp.wordpress.com/2016/10/09/sg-series-part-2-spherical-gaussians-101/)  
第二部分—— [球面高斯分布入门](https://mynameismjp.wordpress.com/2016/10/09/sg-series-part-2-spherical-gaussians-101/)  
Part 3 – [Diffuse Lighting From an SG Light Source](https://mynameismjp.wordpress.com/2016/10/09/sg-series-part-3-diffuse-lighting-from-an-sg-light-source/)  
第三部分 – [来自 SG 光源的漫射光](https://mynameismjp.wordpress.com/2016/10/09/sg-series-part-3-diffuse-lighting-from-an-sg-light-source/)  
Part 4 – [Specular Lighting From an SG Light Source](https://mynameismjp.wordpress.com/2016/10/09/sg-series-part-4-specular-lighting-from-an-sg-light-source/)  
第四部分 – [来自 SG 光源的镜面反射照明](https://mynameismjp.wordpress.com/2016/10/09/sg-series-part-4-specular-lighting-from-an-sg-light-source/)  
Part 5 – [Approximating Radiance and Irradiance With SG’s](https://mynameismjp.wordpress.com/2016/10/09/sg-series-part-5-approximating-radiance-and-irradiance-with-sgs/)  
第五部分—— [利用 SG 近似计算辐射率和辐照度](https://mynameismjp.wordpress.com/2016/10/09/sg-series-part-5-approximating-radiance-and-irradiance-with-sgs/)  
Part 6 – [Step Into The Baking Lab](https://mynameismjp.wordpress.com/2016/10/09/sg-series-part-6-step-into-the-baking-lab/)  
第六部分—— [走进烘焙实验室](https://mynameismjp.wordpress.com/2016/10/09/sg-series-part-6-step-into-the-baking-lab/)

Get the code on GitHub: [https://github.com/TheRealMJP/BakingLab](https://github.com/TheRealMJP/BakingLab) (pre-compiled binaries available [here](https://github.com/TheRealMJP/BakingLab/releases))  
从 GitHub 获取代码： [https://github.com/TheRealMJP/BakingLab](https://github.com/TheRealMJP/BakingLab) （预编译二进制文件可 [在此处](https://github.com/TheRealMJP/BakingLab/releases) 获取）

Back in early 2014, myself and David Neubelt started doing serious research into using Spherical Gaussians as a compact representation for our pre-computed lighting probes. One of the first things I did back then was to create a testbed application that we could use to compare various lightmap representations (SH, H-basis, SG, etc.) and quickly experiment with new ideas. As part of that application I implemented my first path tracer, which was directly integrated into the app for A/B comparisons. This turned out to be extremely useful, since having quick feedback was really helpful for evaluating quality and also for finding and fixing bugs. Eventually we used this app to finalize the exact approach that we would use when integrating SG’s into The Order: 1886.  
早在 2014 年初，我和 David Neubelt 就开始认真研究如何利用球面高斯函数作为预计算光照探针的紧凑表示方法。当时我做的第一件事就是创建一个测试平台应用程序，用于比较各种光照贴图表示方法（球面高斯函数、H 基函数、球面高斯函数等），并快速尝试新的想法。在这个应用程序中，我实现了我的第一个路径追踪器，并将其直接集成到应用程序中，用于 A/B 对比。事实证明，这非常有用，因为快速反馈对于评估质量以及查找和修复错误都非常有帮助。最终，我们利用这个应用程序确定了将球面高斯函数集成到《教团：1886》中的具体方法。

A year later in 2015, Dave and I created another test application for experimenting with improvements that we were planning for future projects. This included things like a physically based exposure model utilizing real-world camera parameters, using the [ACES](https://en.wikipedia.org/wiki/Academy_Color_Encoding_System) \[1\] RRT/ODT for tone mapping, and [using real-world units](http://www.frostbite.com/wp-content/uploads/2014/11/course_notes_moving_frostbite_to_pbr_v2.pdf) \[2\] for specifying lighting intensities. At some point I integrated an improved version of SG baking into this app that would progressively compute results in the background while the app remained responsive, allowing for quick “preview-quality” feedback after adjusting the lighting parameters. Once we started working on our [SIGGRAPH presentation](http://blog.selfshadow.com/publications/s2015-shading-course/rad/s2015_pbs_rad_slides.pdf) \[3\] from the 2015 physically based shading course, it occurred to us that we should really package up this new testbed and release it alongside the presentation to serve as a working implementation of the concepts we were going to cover. But unfortunately this slipped through the cracks: the new testbed required a lot of work in order to make it useful, and both Dave and I were really pressed for time due to multiple new projects ramping up at the office.  
一年后的 2015 年，我和 Dave 创建了另一个测试应用程序，用于试验我们计划在未来项目中改进的功能。这些功能包括：使用真实相机参数的基于物理的曝光模型、使用 [ACES](https://en.wikipedia.org/wiki/Academy_Color_Encoding_System) \[1\] RRT/ODT 进行色调映射，以及 [使用真实世界单位](http://www.frostbite.com/wp-content/uploads/2014/11/course_notes_moving_frostbite_to_pbr_v2.pdf) \[2\]来指定光照强度。后来，我将改进版的 SG烘焙算法集成到这个应用程序中，它可以在后台逐步计算结果，同时保持应用程序的响应速度，从而在调整光照参数后快速获得“预览级”反馈。当我们开始准备 2015 年基于物理的着色课程的 [SIGGRAPH 演讲](http://blog.selfshadow.com/publications/s2015-shading-course/rad/s2015_pbs_rad_slides.pdf) \[3\]时，我们意识到应该将这个新的测试平台打包，并与演讲一起发布，作为我们将要讲解的概念的实际应用。但不幸的是，这件事被搁置了：新的测试平台需要大量的工作才能使其发挥作用，而我和 Dave 当时都因为办公室里多个新项目的启动而时间非常紧迫。

Now, more than a year after our SIGGRAPH presentation, I’m happy to announce that we’ve finally produced and published a working code sample that demonstrates baking of Spherical Gaussian lightmaps! This new app, which I call “The Baking Lab”, is essentially a combination of the two testbed applications that we created. It includes all of the fun features that we were researching in 2015, but also includes real-time progressive baking of 2D lightmaps in various formats. It also allows switching to a progressive path tracer at any time, which serves as the “ground truth” for evaluating lightmap quality and accuracy. Since it’s an amalgamation of two older apps, it uses D3D11 and the older version of my sample framework. So there’s no D3D12 fanciness, but it will run on Windows 7. If you’re just interested in looking at the code or running the app, then go ahead and head over to GitHub: [https://github.com/TheRealMJP/BakingLab](https://github.com/TheRealMJP/BakingLab). If you’re interested in the details of what’s implemented in the app, then keep reading.  
距离我们在 SIGGRAPH 大会上的演讲已经过去一年多了，我很高兴地宣布，我们终于制作并发布了一个可运行的代码示例，演示了球面高斯光照贴图的烘焙！这款名为“烘焙实验室”（The Baking Lab）的新应用，实际上是我们之前创建的两个测试应用的结合体。它包含了我们在 2015 年研究的所有有趣功能，还支持实时渐进式烘焙各种格式的 2D 光照贴图。此外，它还允许随时切换到渐进式路径追踪器，作为评估光照贴图质量和精度的“基准”。由于它是两个旧应用的融合，因此使用了 D3D11 和我之前的示例框架的旧版本。所以它没有 D3D12 的高级功能，但可以在 Windows 7 上运行。如果您只是想查看代码或运行该应用，请访问 GitHub： [https://github.com/TheRealMJP/BakingLab](https://github.com/TheRealMJP/BakingLab) 。如果您对应用程序中实现的具体功能感兴趣，请继续阅读。

### Lightmap Baking 光照贴图烘焙

The primary feature of The Baking Lab is lightmap baking. Each of the test scenes includes a secondary UV set that contains non-overlapping UV’s used for mapping the lightmap onto the scene. Whenever the app starts or a new scene is selected, the baker uses the GPU to rasterize the scene into lightmap UV space. The pixel shader outputs interpolated vertex components like position, tangent frame, and UV’s to several render targets, which use MSAA to simulate conservative rasterization. Once the rasterization is completed, the results are copied back into CPU-accessible memory. The CPU then scans the render targets, and extracts “bake points” from all texels covered by the scene geometry. Each of these bake points represents the location of a single hemispherical probe to be baked.  
烘焙实验室的主要功能是光照贴图烘焙。每个测试场景都包含一个辅助 UV 集，其中包含用于将光照贴图映射到场景上的非重叠 UV。每当应用程序启动或选择新场景时，烘焙器都会使用 GPU 将场景光栅化到光照贴图 UV 空间。像素着色器将插值后的顶点分量（例如位置、切线坐标和 UV）输出到多个渲染目标，这些渲染目标使用 MSAA 来模拟保守光栅化。光栅化完成后，结果会被复制回 CPU 可访问的内存。然后，CPU 扫描渲染目标，并从场景几何体覆盖的所有纹素中提取“烘焙点”。每个烘焙点代表一个待烘焙的半球形探针的位置。

Once all bake points are extracted, the baker begins running using a set of background threads on the CPU. Each thread continuously grabs a new work unit consisting of a group of contiguous bake points, and then loops over the bake points to compute the result for that probe. Each probe is computed by invoking a path tracer, which uses [Embree](https://embree.github.io/) \[4\] to allow for arbitrary ray tracing through the scene on the CPU. The path tracer returns the incoming radiance for a direction and starting point, where the radiance is the result of indirect lighting from various light sources as well as the direct lighting from the sky. The path tracer itself is a very simple unidirectional path tracer, using a few standard techniques like importance sampling, [correlated multi-jittered sampling](http://graphics.pixar.com/library/MultiJitteredSampling/paper.pdf) \[5\], and russian roulette to increase performance and/or convergence rates. The following baking modes are supported:  
提取所有烘焙点后，烘焙器开始在 CPU 上使用一组后台线程运行。每个线程持续获取一个新的工作单元，该工作单元由一组连续的烘焙点组成，然后遍历这些烘焙点以计算该探针的结果。每个探针的计算都是通过调用路径追踪器实现的，该追踪器使用 [Embree](https://embree.github.io/) \[4\] 来实现在 CPU 上对场景进行任意光线追踪。路径追踪器返回给定方向和起始点的入射辐射度，其中辐射度是由来自各种光源的间接光照以及来自天空的直接光照共同作用的结果。路径追踪器本身是一个非常简单的单向路径追踪器，它使用了一些标准技术，例如重要性采样、 [相关多抖动采样](http://graphics.pixar.com/library/MultiJitteredSampling/paper.pdf) \[5\] 和俄罗斯轮盘赌算法，以提高性能和/或收敛速度。支持以下烘焙模式：

- **Diffuse** – a single RGB value containing the result of applying a standard diffuse BRDF to the incoming lighting, with an albedo of 1.0  
	**漫反射** – 一个 RGB 值，表示将标准漫反射 BRDF 应用于入射光的结果，反照率为 1.0。
- **Half-Life 2** – directional irradiance projected onto the [Half-Life 2 basis](http://www.valvesoftware.com/publications/2006/SIGGRAPH06_Course_ShadingInValvesSourceEngine.pdf) \[6\], making for a total of 3 sets of RGB coefficients (9 floats total)  
	**Half-Life 2** – 将方向性辐照度投影到 [Half-Life 2 基](http://www.valvesoftware.com/publications/2006/SIGGRAPH06_Course_ShadingInValvesSourceEngine.pdf) 上 \[6\]，总共得到 3 组 RGB 系数（总共 9 个浮点数）
- **L1 SH** – radiance projected onto the first two orders of spherical harmonics, making for a total of 4 sets of RGB coefficients (12 floats total). Supports environment specular via a 3D lookup texture.  
	**L1 SH——** 将辐射度投影到前两阶球谐函数上，总共产生 4 组 RGB 系数（共 12 个浮点数）。支持通过 3D 查找纹理实现环境镜面反射。
- **L2 SH** – radiance projected on the first three orders of spherical harmonics, making for a total of 9 sets of RGB coefficients (27 floats total). Supports environment specular via a 3D lookup texture.  
	**L2 SH——** 将辐射度投影到前三个球谐函数上，总共产生 9 组 RGB 系数（共 27 个浮点数）。支持通过 3D 查找纹理实现环境镜面反射。
- **L1 H-basis** – irradiance projected onto the first two orders of [H-basis](https://www.cg.tuwien.ac.at/research/publications/2010/Habel-2010-EIN/) \[7\], making for a total of 4 sets of RGB coefficients (12 floats total).  
	**L1 H 基** – 将辐照度投影到 [H 基](https://www.cg.tuwien.ac.at/research/publications/2010/Habel-2010-EIN/) 的前两阶 \[7\]，从而得到 4 组 RGB 系数（总共 12 个浮点数）。
- **L2 H-basis** – irradiance projected onto the first three orders of [H-basis](https://www.cg.tuwien.ac.at/research/publications/2010/Habel-2010-EIN/), making for a total of 6 sets of RGB coefficients (18 floats total).  
	**L2 H 基** – 将辐照度投影到 [H 基](https://www.cg.tuwien.ac.at/research/publications/2010/Habel-2010-EIN/) 的前三个阶，总共得到 6 组 RGB 系数（总共 18 个浮点数）。
- **SG5** – radiance represented by the sum of 5 SG lobes with fixed directions and sharpness, making for a total of 5 sets of RGB coefficients (15 floats total). Supports environment specular via an approximate evaluation of per-lobe specular contribution.  
	**SG5** – 由 5 个具有固定方向和锐度的 SG 光瓣之和表示的辐射亮度，总共产生 5 组 RGB 系数（共 15 个浮点数）。通过对每个光瓣的镜面反射贡献进行近似评估来支持环境镜面反射。
- **SG6** – radiance represented by the sum of 6 SG lobes with fixed directions and sharpness, making for a total of 6 sets of RGB coefficients (18 floats total). Supports environment specular via an approximate evaluation of per-lobe specular contribution.  
	**SG6** – 由 6 个具有固定方向和锐度的 SG 光瓣之和表示的辐射亮度，总共产生 6 组 RGB 系数（共 18 个浮点数）。通过对每个光瓣的镜面反射贡献进行近似评估来支持环境镜面反射。
- **SG9** – radiance represented by the sum of 9 SG lobes with fixed directions and sharpness, making for a total of 9 sets of RGB coefficients (27 floats total). Supports environment specular via an approximate evaluation of per-lobe specular contribution.  
	**SG9——** 由 9 个具有固定方向和锐度的 SG 光瓣之和表示的辐射亮度，总共产生 9 组 RGB 系数（共 27 个浮点数）。它通过对每个光瓣的镜面反射贡献进行近似评估来支持环境镜面反射。
- **SG12** – radiance represented by the sum of 12 SG lobes with fixed directions and sharpness, making for a total of 12 sets of RGB coefficients (36 floats total). Supports environment specular via an approximate evaluation of per-lobe specular contribution.  
	**SG12** – 由 12 个具有固定方向和锐度的 SG 光瓣之和表示的辐射亮度，总共产生 12 组 RGB 系数（共 36 个浮点数）。通过对每个光瓣的镜面反射贡献进行近似评估，支持环境镜面反射。

For SH, H-basis, and HL2 basis baking modes the path tracer is evaluated for random rays distributed about the hemisphere so that Monte Carlo integration can be used to integrate the radiance samples onto the corresponding basis functions. This allows for true progressive integration, where the baker makes N passes over each bake point, each time adding a new sample with the appropriate weighting. It looks pretty cool in action:  
对于 SH、H-basis 和 HL2 基烘焙模式，路径追踪器会针对分布在半球上的随机光线进行评估，以便使用蒙特卡罗积分将辐射样本积分到相应的基函数上。这实现了真正的渐进式积分，烘焙器会遍历每个烘焙点 N 次，每次都添加一个具有适当权重的新样本。实际效果非常出色：

![](https://www.youtube.com/watch?v=aN0StrS2moI)

The same approach is used for the “Diffuse” baking mode, except that sampling rays are evaluated using a [cosine-weighted hemispherical sampling scheme](http://www.rorydriscoll.com/2009/01/07/better-sampling/) \[8\]. For SG baking, things get a little bit trickier. If the ad-hoc projection mode is selected, the result can be progressively evaluated in the same manner as the non-SG bake modes. However if either the Least Squares or Non-Negative Least Squares mode are active, we can’t run the solve unless we have all of the hemispherical radiance samples available to feed to the solver. In this case we switch to a different baking scheme where each thread fully computes the final value for every bake point that it operates on. However the thread only does this for a single bake point from each work group, and afterwards it fills in the rest of the neighboring bake points (which are arranged in a 8×8 group of texels) with the results it just computed. Each pass of of baker then fills in the next bake point in the work group, gradually computing the final result for all texels in the group. So instead of seeing the quality slowly improve across the light map, you see extrapolated results being filled in. It ends up looking like this:  
“漫反射”烘焙模式也采用相同的方法，只是采样光线采用 [余弦加权半球采样方案](http://www.rorydriscoll.com/2009/01/07/better-sampling/) 进行评估\[8\]。对于 SG 烘焙，情况则稍微复杂一些。如果选择临时投影模式，则可以像非 SG 烘焙模式一样逐步评估结果。但是，如果激活了最小二乘或非负最小二乘模式，则必须将所有半球辐射采样提供给求解器才能运行求解。在这种情况下，我们切换到不同的烘焙方案，其中每个线程都会完整计算其操作的每个烘焙点的最终值。但是，线程仅对每个工作组中的一个烘焙点执行此操作，然后将其计算的结果填充到其余相邻的烘焙点（这些烘焙点排列成 8×8 的纹素组）。烘焙器的每次迭代都会填充工作组中的下一个烘焙点，逐步计算该组中所有纹素的最终结果。因此，您看到的不是光照贴图质量的缓慢提升，而是外推结果的填充。最终效果如下：

![](https://www.youtube.com/watch?v=WhSlZgycQfM)

While it’s not as great as a true progressive bake, it’s still better than having no preview at all.  
虽然它不如真正的渐进式烘焙那么好，但仍然比完全没有预览要好。

The app supports a few settings that control some of the bake parameters, such as the number of samples evaluated per-texel and the overall lightmap resolution. The “Scene” group in the UI also has a few settings that allow toggling different components of the final render, such as the direct or indirect lighting or the diffuse/specular components. Under the “Debug” setting you can also toggle a neat visualizer that shows a visual representation of the raw data stored in the lightmap. It looks like this:  
该应用支持一些设置，用于控制部分烘焙参数，例如每个纹素评估的采样数和整体光照贴图分辨率。用户界面中的“场景”组也提供了一些设置，允许切换最终渲染的不同组件，例如直接光或间接光，或漫反射/镜面反射组件。在“调试”设置下，您还可以切换一个简洁的可视化工具，该工具以可视化的方式呈现存储在光照贴图中的原始数据。它看起来像这样：

![SG_Debug_Visualizer](https://mynameismjp.wordpress.com/wp-content/uploads/2016/08/sg_debug_visualizer.png)

### Ground Truth Path Tracer地面实况路径追踪器

The integrated path tracer is primarily there so that you can see how close or far off you are when computing environment diffuse or specular from a light map. It was also a lot of fun to write – I recommend doing it sometime if you haven’t already! Just be careful: it may make you depressed to see how poorly your real-time approximation holds up when compared with a proper offline render.

The ground truth renderer works in a similar vein to the lightmap baker: it kicks off multiple background threads that each grab work groups of 16×16 pixels that are contiguous in screen space. The renderer makes N passes over each pixel, where each pass adds an additional sample that’s weighted and summed with the previous results. This gives you a true progressive render, where the result starts out noisy and (very) gradually converges towards a noise-free image:

![](https://www.youtube.com/watch?v=LUrCpZYQbm0)

The ground truth renderer is activated by checking the “Show Ground Truth” setting under the “Ground Truth” group. There’s a few more parameters in that group to control the behavior of the renderer, such as the number of samples used per-pixel and the scheme used for generating random samples.

### Light Sources

There’s 3 different light sources supported in the app: a sun, a sky, and a spherical area light. For real-time rendering, the sun is handled as a directional light with an intensity computed automatically using the [Hosek-Wilkie solar radiance model](http://cgg.mff.cuni.cz/projects/SkylightModelling/) \[9\]. So as you change the position of the sun in the sky, you’ll see the color and intensity of the sunlight automatically change. To improve the real-time appearance, I used the disk area light approximation from the 2014 Frostbite presentation. The path tracer evaluates the sun as an infinitely-distant spherical area light with the appropriate angular radius, with uniform intensity and color also computed from the solar radiance model. Since the path tracer handles the sun as a true area light source, it produces correct specular reflections and soft shadows. In both cases the sun defaults to correct real-world intensities using actual photometric units. There is a parameter for adjusting the sun size, which will result in the sun being too bright or too dark if manipulated. However there’s another setting called “Normalize Sun Intensity” which will attempt to maintain roughly the same illumination regardless of the size, which allows for changing the sun appearance or shadow softness without changing the overall scene lighting.

The default sky mode (called “Procedural”) uses the Hosek-Wilkie sky model to compute a procedural sky from a few input parameters. These include turbidity, ground albedo, and the current sun position. Whenever the parameters are changed, the model is cached to a cubemap that’ s used for real-time rendering on the GPU. For CPU path tracing, the the sky model is directly evaluated for a direction using the sample code provided by the authors. When combined with the procedural sun model, the two light sources form a simple outdoor lighting environment that corresponds to real-world intensities. Several other sky modes are also supported for convenience. The “Simple”mode takes just a color and intensity as input parameter, and flood-fills the entire sky with a value equal to color \* intensity. The “Ennis”, “Grace Cathedral”, and “Uffizi Cross” modes use corresponding HDR environment maps to fill the sky instead of a procedural model.

For local lighting, the app supports enabling a single spherical area light using the “Enable Area Light” setting. The area light can be positioned using the Position X/Position Y/Position Z settings, and its radius can be specified with the “Size” setting. There are a 4 different modes for specifying the intensity of the light:

- **Luminance** – the intensity corresponds to the amount of light being emitted from the light source along an infinitesimally small ray towards the viewer or receiving surface. Uses units of cd/m <sup>2</sup>. Changing the size of the light source will change the overall illumination the scene.
- **Illuminance** – specifies the amount of light incident on a surface at a set distance, which is specified using the “Illuminance Distance” setting. So instead of saying “how much light is coming out of the light source” like you do with the “Luminance” mode, you’re saying “how much diffuse light is being reflected from a perpendicular surface N units away”. Uses units of lux, which are equivalent to lm/m <sup>2</sup>. Changing the size of the light source will *not* change the overall illumination the scene.
- **Luminous Power** – specifies the total amount of light being emitted from the light source in all directions. Uses units of lumens. Changing the size of the light source will *not* change the overall illumination the scene.
- **EV100** – this is an alternative way of specifying the luminance of the light source, using the [exposure value](https://en.wikipedia.org/wiki/Exposure_valuehttps://en.wikipedia.org/wiki/Exposure_value) \[10\] system originally suggested by [Nathan Reed](http://www.reedbeta.com/blog/2014/06/04/artist-friendly-hdr-with-exposure-values/) \[11\]. The base-2 logarithmic scale for this mode is really nice, since incrementing by 1 means doubling the perceived brightness. Changing the size of the light source will change the overall illumination the scene.

The ground truth renderer will evaluate the area light as a true spherical light source, using importance sampling to reduce variance. The real-time renderer approximates the light source as a single SG, and generates very simple hard shadows using an array of 6 shadow maps. By default only indirect lighting from the area light will be baked into the lightmap, with the direct lighting evaluated on the GPU. However if the “Bake Direct Area Light” setting is enabled, then the direct contribution from the area light will be baked into the lightmap.

Note that all light sources in the app are always scaled down by a factor of 2 <sup>-10</sup> before being using in rendering, as suggested by Nathan Reed in [his blog post](http://www.reedbeta.com/blog/2014/06/04/artist-friendly-hdr-with-exposure-values/) \[11\]. Doing this effectively shifts the window of values that can be represented in a 16-bit floating point value, which is necessary in order to represent specular reflections from the sun. However the UI always will always show the unshifted values, as will the debug luminance picker that shows the final color and intensity of any pixel on the screen.

### Exposure and Depth of Field曝光和景深

As I mentioned earlier, the app implements a physically based exposure system that attempts to models the behavior and parameters of a real-world camera. Much of the implementation was based on the code from Padraic Hennessy’s [excellent series of articles](https://placeholderart.wordpress.com/2014/11/16/implementing-a-physically-based-camera-understanding-exposure/) \[12\], which was in turn inspired by Sébastien Lagarde and Charles de Rousiers’s [SIGGRAPH presentation from 2014](http://www.frostbite.com/wp-content/uploads/2014/11/course_notes_moving_frostbite_to_pbr_v2.pdf) \[2\]. When the “Exposure Mode” setting is set to the “Manual (SBS)” or “Manual (SOS)” modes, the final exposure value applied before tone mapping will be computed based on the combination of aperture size, ISO rating, and shutter speed. There is also a “Manual (Simple)” mode available where a single value on a log2 scale can be used instead of the 3 camera parameters.  
正如我之前提到的，该应用程序实现了一个基于物理的曝光系统，力求模拟真实相机的行为和参数。其大部分实现基于 Padraic Hennessy 的 [一系列优秀文章](https://placeholderart.wordpress.com/2014/11/16/implementing-a-physically-based-camera-understanding-exposure/) \[12\] 中的代码，而这些文章又受到了 Sébastien Lagarde 和 Charles de Rousiers [在 2014 年 SIGGRAPH 大会上的演讲](http://www.frostbite.com/wp-content/uploads/2014/11/course_notes_moving_frostbite_to_pbr_v2.pdf) \[2\] 的启发。当“曝光模式”设置为“手动（SBS）”或“手动（SOS）”模式时，色调映射之前的最终曝光值将根据光圈大小、ISO 感光度和快门速度的组合计算得出。此外，还有一个“手动（简单）”模式，在该模式下，可以使用一个 log2 刻度上的单个值来代替这三个相机参数。

Mostly for fun, I integrated a post-process depth of field effect that uses the same camera parameters (along with focal length and film size) to compute per-pixel circle of confusion sizes. The effect is off by default, and can be toggled on using the “Enable DOF” setting. Polygonal and circular bokeh shapes are supported using the technique suggested by Tiago Sousa in his [2013 SIGGRAPH presentation](http://advances.realtimerendering.com/s2013/Sousa_Graphics_Gems_CryENGINE3.pptx) \[13\]. Depth of field is also implemented in the ground truth renderer, which is capable of achieving true multi-layer effects by virtue of using a ray tracer.

[![dof_gt](https://mynameismjp.wordpress.com/wp-content/uploads/2016/09/dof_gt.png)](https://mynameismjp.wordpress.com/wp-content/uploads/2016/09/dof_gt.png)

### Tone Mapping 色调映射

Several tone mapping operators are available for experimentation:

- **Linear** – no tone mapping, just a clamp to \[0, 1\]
- **Film Stock** – [Jim Hejl](https://twitter.com/jimhejl) and Richard Burgess-Dawson’s polyomial approximation of [Haarm-Peter Duiker](https://twitter.com/hpduiker) ‘s filmic curve, which was created by scanning actual film stock. Based on the implementation provided by [John Hable](http://filmicgames.com/archives/75) \[14\].
- **Hable (Uncharted2)** – [John Hable](https://twitter.com/FilmicWorlds) ‘s adjustable filmic curve from his [GDC 2010 presentation](http://www.gdcvault.com/play/1012351/Uncharted-2-HDR) \[15\]
- **Hejl 2015** – Jim Hejl’s filmic curve that he [posted on Twitter](https://twitter.com/jimhejl/status/633777619998130176) \[16\], which is a refinement of Duiker’s curve
- **ACES sRGB Monitor** – a fitted polynomial version of the [ACES](https://github.com/ampas/aces-dev) \[17\] reference rendering transform (RRT) combined with the sRGB monitor output display transform (ODT), generously provided by [Stephen Hill](https://twitter.com/self_shadow).

### Debug Settings

At the bottom of the settings UI are a group of debug options that can be selected. I already mentioned the bake data visualizer previously, but it’s worth mentioning again because it’s really cool. There’s also a “luminance picker”, which will enable a text output showing you the luminance and illuminance of the surface under the mouse cursor. This was handy for validating the physically based sun and sky model, since I could use the picker to make sure that the lighting values matched what you would expect from real-world conditions. The “View Indirect Specular” option causes both the real-time renderer and the ground truth renderer to only show the indirect specular component, which can be useful for gauging the accuracy of specular computed from the lightmap. After that there’s a pair of buttons for saving or loading light settings. This will serialize the settings that control the lighting environment (sun direction, sky mode, area light position, etc.) to a file, which can be loaded in whenever you like. The “Save EXR Screenshot” is fairly self-explanatory: it lets you save a screenshot to an EXR file that retains the HDR data. Finally there’s an option to show the current sun intensity that’s used for the real-time directional light.  
设置界面底部有一组调试选项可供选择。我之前已经提到过烘焙数据可视化工具，但它非常实用，值得再次提及。此外还有一个“亮度选择器”，它可以显示鼠标光标下方表面的亮度和照度，并以文本形式呈现。这对于验证基于物理的太阳和天空模型非常有用，因为我可以使用选择器来确保光照值与真实世界条件下的预期值相符。“查看间接镜面反射”选项会让实时渲染器和真实渲染器都只显示间接镜面反射分量，这有助于评估从光照贴图计算出的镜面反射的准确性。之后是一对用于保存或加载光照设置的按钮。这会将控制光照环境的设置（太阳方向、天空模式、区域光位置等）序列化到一个文件中，您可以随时加载该文件。 “保存 EXR 屏幕截图”功能顾名思义，允许您将屏幕截图保存为保留 HDR 数据的 EXR 文件。最后，还有一个选项可以显示用于实时方向光照的当前太阳强度。

### References 参考

\[1\] Academy Color Encoding System – [https://en.wikipedia.org/wiki/Academy\_Color\_Encoding\_System](https://en.wikipedia.org/wiki/Academy_Color_Encoding_System)  
\[1\] 学院色彩编码系统 – [https://en.wikipedia.org/wiki/Academy\_Color\_Encoding\_System](https://en.wikipedia.org/wiki/Academy_Color_Encoding_System)  
\[2\] Moving Frostbite to PBR (course notes) – [http://www.frostbite.com/wp-content/uploads/2014/11/course\_notes\_moving\_frostbite\_to\_pbr\_v2.pdf  
\[2\] 将 Frostbite 迁移到 PBR（课程笔记）– http://www.frostbite.com/wp-content/uploads/2014/11/course\_notes\_moving\_frostbite\_to\_pbr\_v2.pdf  
](http://www.frostbite.com/wp-content/uploads/2014/11/course_notes_moving_frostbite_to_pbr_v2.pdf)\[3\] Advanced Lighting R&D at Ready At Dawn Studios – [http://blog.selfshadow.com/publications/s2015-shading-course/rad/s2015\_pbs\_rad\_slides.pdf  
](http://blog.selfshadow.com/publications/s2015-shading-course/rad/s2015_pbs_rad_slides.pdf)\[4\] Embree: High Performance Ray Tracing Kernels – [https://embree.github.io/  
](https://embree.github.io/)\[5\] Correlated Multi-Jittered Sampling – [http://graphics.pixar.com/library/MultiJitteredSampling/paper.pdf](http://graphics.pixar.com/library/MultiJitteredSampling/paper.pdf)  
\[5\] 相关多抖动采样 – [http://graphics.pixar.com/library/MultiJitteredSampling/paper.pdf](http://graphics.pixar.com/library/MultiJitteredSampling/paper.pdf)  
\[6\] Shading in Valve’s Source Engine – [http://www.valvesoftware.com/publications/2006/SIGGRAPH06\_Course\_ShadingInValvesSourceEngine.pdf](http://www.valvesoftware.com/publications/2006/SIGGRAPH06_Course_ShadingInValvesSourceEngine.pdf)  
\[6\] Valve Source 引擎中的着色 – [http://www.valvesoftware.com/publications/2006/SIGGRAPH06\_Course\_ShadingInValvesSourceEngine.pdf](http://www.valvesoftware.com/publications/2006/SIGGRAPH06_Course_ShadingInValvesSourceEngine.pdf)  
\[7\] Efficient Irradiance Normal Mapping – [https://www.cg.tuwien.ac.at/research/publications/2010/Habel-2010-EIN/](https://www.cg.tuwien.ac.at/research/publications/2010/Habel-2010-EIN/)  
\[7\] 高效辐照度法线映射 – [https://www.cg.tuwien.ac.at/research/publications/2010/Habel-2010-EIN/](https://www.cg.tuwien.ac.at/research/publications/2010/Habel-2010-EIN/)  
\[8\] Better Sampling – [http://www.rorydriscoll.com/2009/01/07/better-sampling/](http://www.rorydriscoll.com/2009/01/07/better-sampling/)  
\[8\] 更好的采样 – [http://www.rorydriscoll.com/2009/01/07/better-sampling/](http://www.rorydriscoll.com/2009/01/07/better-sampling/)  
\[9\] Adding a Solar Radiance Function to the Hosek Skylight Model – [http://cgg.mff.cuni.cz/projects/SkylightModelling/](http://cgg.mff.cuni.cz/projects/SkylightModelling/)  
\[9\] 为 Hosek 天窗模型添加太阳辐射函数 – [http://cgg.mff.cuni.cz/projects/SkylightModelling/](http://cgg.mff.cuni.cz/projects/SkylightModelling/)  
\[10\] Exposure value – [https://en.wikipedia.org/wiki/Exposure\_value](https://en.wikipedia.org/wiki/Exposure_value)  
\[10\] 曝光值 – [https://en.wikipedia.org/wiki/Exposure\_value](https://en.wikipedia.org/wiki/Exposure_value)  
\[11\] Artist-Friendly HDR With Exposure Values – [http://www.reedbeta.com/blog/2014/06/04/artist-friendly-hdr-with-exposure-values/](http://www.reedbeta.com/blog/2014/06/04/artist-friendly-hdr-with-exposure-values/)  
\[11\] 艺术家友好的 HDR 及其曝光值 – [http://www.reedbeta.com/blog/2014/06/04/artist-friendly-hdr-with-exposure-values/](http://www.reedbeta.com/blog/2014/06/04/artist-friendly-hdr-with-exposure-values/)  
\[12\] Implementing a Physically Based Camera: Understanding Exposure – [https://placeholderart.wordpress.com/2014/11/16/implementing-a-physically-based-camera-understanding-exposure/](https://placeholderart.wordpress.com/2014/11/16/implementing-a-physically-based-camera-understanding-exposure/)  
\[12\] 实现基于物理的相机：理解曝光 – [https://placeholderart.wordpress.com/2014/11/16/implementing-a-physically-based-camera-understanding-exposure/](https://placeholderart.wordpress.com/2014/11/16/implementing-a-physically-based-camera-understanding-exposure/)  
\[13\]CryENGINE 3 Graphics Gems – [http://advances.realtimerendering.com/s2013/Sousa\_Graphics\_Gems\_CryENGINE3.pptx](http://advances.realtimerendering.com/s2013/Sousa_Graphics_Gems_CryENGINE3.pptx)  
\[13\]CryENGINE 3 图形引擎精髓 – [http://advances.realtimerendering.com/s2013/Sousa\_Graphics\_Gems\_CryENGINE3.pptx](http://advances.realtimerendering.com/s2013/Sousa_Graphics_Gems_CryENGINE3.pptx)  
\[14\] Filmic Tonemapping Operators – [http://filmicgames.com/archives/75](http://filmicgames.com/archives/75)  
\[14\] 电影色调映射运算符 – [http://filmicgames.com/archives/75](http://filmicgames.com/archives/75)  
\[15\] Uncharted 2: HDR Lighting – [http://www.gdcvault.com/play/1012351/Uncharted-2-HDR](http://www.gdcvault.com/play/1012351/Uncharted-2-HDR)  
\[15\] 神秘海域 2：HDR 光照 – [http://www.gdcvault.com/play/1012351/Uncharted-2-HDR](http://www.gdcvault.com/play/1012351/Uncharted-2-HDR)  
\[16\] Jim Hejl on Twitter – [https://twitter.com/jimhejl/status/633777619998130176](https://twitter.com/jimhejl/status/633777619998130176)  
\[16\] 吉姆·海吉在推特上 – [https://twitter.com/jimhejl/status/633777619998130176](https://twitter.com/jimhejl/status/633777619998130176)  
\[17\] Academy Color Encoding System Developer Resources – [https://github.com/ampas/aces-dev](https://github.com/ampas/aces-dev)  
\[17\] Academy 颜色编码系统开发者资源 – [https://github.com/ampas/aces-dev](https://github.com/ampas/aces-dev)