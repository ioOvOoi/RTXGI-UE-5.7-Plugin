---
title: "SG Series Part 4: Specular Lighting From an SG Light Source"
source: https://mynameismjp.wordpress.com/2016/10/09/sg-series-part-4-specular-lighting-from-an-sg-light-source/
author:
  - "[[MJP]]"
published: 2016-10-10
created: 2026-06-14
description: "You can find an ad-free static site version of this post here: https://therealmjp.github.io/posts/sg-series-part-4-specular-lighting-from-an-sg-light-source/ This is part 4 of a series on Spherical Gaussians and their applications for pre-computed lighting. You can find the other articles here: Part 1 - A Brief (and Incomplete) History of Baked Lighting Representations Part 2 - Spherical Gaussians 101 Part 3 - Diffuse…"
tags:
  - Clippings review
updated: 2026-06-14T17:15
---
**You can find an ad-free static site version of this post here: [https://therealmjp.github.io/posts/sg-series-part-4-specular-lighting-from-an-sg-light-source/](https://therealmjp.github.io/posts/sg-series-part-4-specular-lighting-from-an-sg-light-source/)  
您可以在这里找到这篇文章的无广告静态网站版本： [https://therealmjp.github.io/posts/sg-series-part-4-specular-lighting-from-an-sg-light-source/](https://therealmjp.github.io/posts/sg-series-part-4-specular-lighting-from-an-sg-light-source/)**

*This is part 4 of a series on Spherical Gaussians and their applications for pre-computed lighting. You can find the other articles here:  
这是关于球面高斯函数及其在预计算光照中应用系列文章的第四部分。您可以在这里找到其他文章：*

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

In the [previous article](https://mynameismjp.wordpress.com/2016/10/09/sg-series-part-3-diffuse-lighting-from-an-sg-light-source/), we explored a few ways to compute the contribution of an SG light source when using a diffuse BRDF. While this is already useful, it would be nice to be able work with more complex view-dependent BRDF’s so that we can also compute a specular contribution. In this article I’ll explain a possible approach for approximating the response of a microfacet specular BRDF when applied to an SG light, and also introduce the concept of Anisotropic Spherical Gaussians.  
在 [上一篇文章](https://mynameismjp.wordpress.com/2016/10/09/sg-series-part-3-diffuse-lighting-from-an-sg-light-source/) 中，我们探讨了使用漫反射 BRDF 计算 SG 光源贡献的几种方法。虽然这些方法已经很有用，但如果能够处理更复杂的、与视角相关的 BRDF，以便计算镜面反射贡献，那就更好了。在本文中，我将解释一种近似计算应用于 SG 光源的微面镜面 BRDF 响应的方法，并介绍各向异性球面高斯函数的概念。

### Microfacet Specular Recap微面光谱回顾

You probably recall from the past 5 years of [physically based rendering presentations](http://blog.selfshadow.com/publications/s2013-shading-course/hoffman/s2013_pbs_physics_math_notes.pdf) \[1\] that a standard microfacet BRDF takes the following structure:  
您可能还记得过去 5 年 [基于物理的渲染演示](http://blog.selfshadow.com/publications/s2013-shading-course/hoffman/s2013_pbs_physics_math_notes.pdf) \[1\] 中提到的，标准的微面 BRDF 采用以下结构：

$$
f(\mathbf{i}, \mathbf{o}) = \frac{F(\mathbf{o}, \mathbf{h})\,G(\mathbf{i}, \mathbf{o}, \mathbf{h})\,D(\mathbf{h})} {4\,(\mathbf{n} \cdot \mathbf{i})\,(\mathbf{n} \cdot \mathbf{o})}
$$
 

Recall that *D* (h) is the *distribution* term, which tells us the percentage of active microfacets for a particular combination of view and light vectors. It’s also commonly known as the *normal distribution function*, or “NDF” for short. It’s generally parameterized on a *roughness* parameter, which essentially describes the “bumpiness” of the underlying microgeometry. Lower roughness values lead to sharp mirror-like reflections with a very narrow and intense specular lobe, while higher roughness values lead to more broad reflections with a wider specular lobe. Most modern games (including The Order: 1886) use the GGX (AKA Trowbridge-Reitz) distribution for this purpose.  
回想一下， *D* (h) 是 *分布* 项，它表示在特定视角和光照矢量组合下，活跃微面的百分比。它也常被称为 *正态分布函数* ，简称“NDF”。它通常以 *粗糙度* 参数为参数，该参数本质上描述了底层微观几何体的“凹凸程度”。较低的粗糙度值会导致清晰的镜面反射，具有非常窄且强烈的镜面反射瓣；而较高的粗糙度值会导致更宽的反射，具有更宽的镜面反射瓣。大多数现代游戏（包括《教团：1886》）都使用 GGX（又名 Trowbridge-Reitz）分布来实现此目的。[![GGX_Distribution_0_1](https://mynameismjp.wordpress.com/wp-content/uploads/2016/07/ggx_distribution_0_1.png)](https://www.desmos.com/calculator/hrybtwxnbi)

*The top image shows a GGX distribution term with a roughness parameter of 0.5. The bottom image shows the same GGX distribution term with a roughness parameter of 0.1. For both graphs, the X axis represents the angle between the surface normal and the half vector. Click on either image for an interactive graph.  
上图显示的是粗糙度参数为 0.5 的 GGX 分布项。下图显示的是粗糙度参数为 0.1 的相同 GGX 分布项。两幅图中，X 轴均表示表面法线与半向量之间的夹角。点击任一图像即可查看交互式图表。*

Next we have G(i, o, h) which is referred to as the *geometry* *term* or the *masking-shadow function*. Eric Heitz’s [paper on the masking-shadow function](http://jcgt.org/published/0003/02/03/) \[2\] has a fantastic explanation of how these terms work and why they’re important, so I would strongly recommend reading through it if you haven’t already. As Eric explains, the geometry term actually accounts for two different phenomena. The first is the local occlusion of reflections by other neighboring microfacets. Depending on the angle of the incoming lighting and the bumpiness of the microsurface, a certain amount of the lighting will be occluded by the surface itself, and this term attempts to model that. The other phenomenon handled by this term is visibility of the surface from the viewer. A surface that isn’t visible naturally can’t reflect light towards the viewer, and so the geometry term models this masking effect using the view direction and the roughness of the microsurface.  
接下来是 G(i, o, h)，它被称为 *几何* *项* 或 *遮蔽阴影函数* 。Eric Heitz [关于遮蔽阴影函数的论文](http://jcgt.org/published/0003/02/03/) \[2\] 对这些项的工作原理及其重要性进行了精彩的解释，因此我强烈建议您阅读一下（如果您还没有读过的话）。正如 Eric 所解释的，几何项实际上考虑了两种不同的现象。第一种是相邻微面反射的局部遮挡。根据入射光的角度和微表面的粗糙度，一定量的光线会被表面本身遮挡，而该项正是试图模拟这种现象。该项处理的另一个现象是观察者对表面的可见性。一个不可见的表面自然无法将光反射到观察者身上，因此几何项利用观察方向和微表面的粗糙度来模拟这种遮挡效应。

[![GGX_Visibility_0_25_0_0](https://mynameismjp.wordpress.com/wp-content/uploads/2016/07/ggx_visibility_0_25_0_0.png)](https://www.desmos.com/calculator/w7wnltroeg)

*The Smith visibility term for GGX as a function of the angle between the surface normal and the light direction. The roughness used is 0.25, and the angle between the normal and the view direction is 0. Click on the image for an interactive graph.  
GGX 的史密斯可见性项是表面法线与光照方向之间夹角的函数。粗糙度设置为 0.25，法线与视角方向之间的夹角为 0。点击图片查看交互式图表。*

Finally we have F(o, h), which is the *Fresnel* term. This familiar term determines the amount of light that is reflected vs. the amount that is refracted or absorbed, which varies depending on the index of refraction for a particular interface as well as the angle of incidence. For a microfacet BRDF we compute the Fresnel term using the angle between the active microfacet direction (the half vector) and the light or view direction (it is equivalent to use either). This generally causes the reflected intensity to increase when both the viewing direction and the light direction are grazing with respect to the surface normal. In real-time graphics Schlick’s approximation is typically used to represent the Fresnel term, since it’s a bit cheaper than evaluating the actual Fresnel equations. It is also common to parameterize the Fresnel term on the reflectance at zero incidence (referred to as “F0”) instead of directly working with an index of refraction.  
最后，我们得到 F(o, h)，即 *菲涅尔* 项。这个熟悉的项决定了反射光与折射或吸收光的比例，而该比例取决于特定界面的折射率以及入射角。对于微面双向反射分布函数 (BRDF)，我们使用活动微面方向（半矢量）与光线或观察方向之间的夹角来计算菲涅尔项（两者等效）。通常，当观察方向和光线方向都与表面法线呈掠射角时，反射强度会增加。在实时图形学中，通常使用 Schlick 近似来表示菲涅尔项，因为它比计算实际的菲涅尔方程更高效。此外，通常使用零入射时的反射率（称为“F0”）来参数化菲涅尔项，而不是直接使用折射率。

[![Schlick_Fresnel](https://mynameismjp.wordpress.com/wp-content/uploads/2016/07/schlick_fresnel.png)](https://www.desmos.com/calculator/wcwkzazku0)

*Schlick’s approximation of the Fresnel term as a function of the angle between the half vector and the light direction. The graph uses a value of 0.04 for F0. Click on the image for an interactive graph.  
施利克近似表示菲涅耳项与半矢量和光线方向之间夹角的关系。图中 F0 的值为 0.04。点击图片可查看交互式图表。*

### Specular From an SG LightSG 灯的壮观景象

Let’s now return to our example of a Spherical Gaussian light source. In the previous article we explored how to approximate the purely diffuse contribution from such a light source, but how do we handle the specular response? If we go down this road, we ideally want to do it in a way that uses a microfacet specular model so that we can remain consistent with our lighting from punctual light sources and IBL probes. If we start out by just plugging our BRDF and SG light into the rendering equation we get this monstrosity:  
现在让我们回到球面高斯光源的例子。在上一篇文章中，我们探讨了如何近似计算这种光源的纯漫反射贡献，但如何处理镜面反射响应呢？理想情况下，我们希望使用微面镜面反射模型，以便与点光源和 IBL 探针的光照保持一致。如果我们一开始就直接将 BRDF 和 SG 光代入渲染方程，就会得到如下糟糕的结果：

$$
L_{o}(\mathbf{o}, \mathbf{x}) = \int_{\Omega}  \frac{F(\mathbf{o}, \mathbf{h})\,G(\mathbf{i}, \mathbf{o}, \mathbf{h})\,D(\mathbf{h})G_{l}(\mathbf{i};\mathbf{\mu},\lambda,a)cos(\theta_{i})d\Omega} {4cos(\theta_{i})cos(\theta_{o})}
$$

$$
L_{o}(\mathbf{o}, \mathbf{x}) = \int_{\Omega}  \frac{F(\mathbf{o}, \mathbf{h})\,G(\mathbf{i}, \mathbf{o}, \mathbf{h})\,D(\mathbf{h})ae^{\lambda(\mathbf{\mu} \cdot \mathbf{i} - 1)}d\Omega} {4cos(\theta_{o})}
$$
 

Unlike diffuse we now have multiple terms inside the integral, many of which are view-dependent. This suggests that we will need to combine several aggressive optimizations in order to get anywhere close to our desired result.  
与漫反射不同，现在积分中包含多个项，其中许多项与视角相关。这意味着我们需要结合几种激进的优化方法才能接近我们想要的结果。

#### The Distribution Term 分配条款

Let’s start out by taking the distribution term on its own, since it’s arguably the most important part of the specular BRDF. The distribution determines the overall shape and intensity of the resulting specular highlight, and has a widely-varying frequency response over the domain of possible roughness values. If we look at the graph of the GGX distribution from the earlier section, the shape does resemble a Gaussian. Unfortunately it’s not an exact match: the GGX distribution has the characteristic narrow highlight and long tails which we can’t match using a single SG. We could get a closer fit by summing multiple Gaussians, but for this article we’ll keep things simple by sticking with a single lobe. If we go back to [Wang et al.](http://research.microsoft.com/en-us/um/people/johnsny/papers/sg.pdf)‘s paper\[3\] that we referenced earlier, we can see that they suggest a very simple fit of an SG to a Cook-Torrance distribution term:  
我们先单独来看分布项，因为它可以说是镜面反射 BRDF 中最重要的部分。分布决定了最终镜面高光的整体形状和强度，并且在可能的粗糙度值范围内具有变化很大的频率响应。如果我们看一下前面章节中的 GGX 分布图，它的形状确实类似于高斯分布。遗憾的是，它并不完全匹配：GGX 分布具有特征性的窄高光和长尾，而我们无法用单个 SG 来拟合。我们可以通过对多个高斯分布求和来获得更精确的拟合，但为了本文的简洁性，我们将只使用单个瓣。如果我们回顾一下前面引用的 [Wang 等人的](http://research.microsoft.com/en-us/um/people/johnsny/papers/sg.pdf) 论文\[3\]，我们可以看到他们提出了一种非常简单的 SG 拟合方法，用 Cook-Torrance 分布项来拟合：

$$
D(\mathbf{h})= e^{-(arccos(\mathbf{h} \cdot \mathbf{n})/m)^{2}} \approx G(\mathbf{h};\mathbf{n},\frac{2}{m^2},1)
$$
 

If we look closely, we can see that they’re actually fitting for the Gaussian model mentioned in the [original Cook-Torrance paper](http://inst.cs.berkeley.edu/~cs294-13/fa09/lectures/cookpaper.pdf) \[4\], which is similar to the one used in the [Torrance-Sparrow model](http://www.graphics.cornell.edu/~westin/pubs/TorranceSparrowJOSA1967.pdf) \[5\]. Note that this should not be confused the with Beckmann distribution that’s also mentioned in the Cook-Torrance paper, which is actually a 2D Gaussian in the slope domain (AKA parallel plane domain). However the shape isn’t even the biggest problem here, as the variant they’re using isn’t normalized. This means the peak of the Gaussian will always have a height of 1.0, rather than shrinking when the roughness increases and growing when the roughness decreases. Fortunately this is really easy to fix, since we have a simple analytical formula for computing the integral of an SG. Therefore if we set the amplitude to 1 over the integral, we end up with a normalized distribution:  
仔细观察，我们可以发现它们实际上拟合的是 [Cook-Torrance 原始论文](http://inst.cs.berkeley.edu/~cs294-13/fa09/lectures/cookpaper.pdf) \[4\] 中提到的高斯模型，该模型与 [Torrance-Sparrow 模型](http://www.graphics.cornell.edu/~westin/pubs/TorranceSparrowJOSA1967.pdf) \[5\] 中使用的模型类似。需要注意的是，这不应与 Cook-Torrance 论文中提到的 Beckmann 分布混淆，后者实际上是斜率域（也称为平行平面域）中的二维高斯分布。然而，形状甚至还不是这里最大的问题，因为他们使用的变体没有进行归一化。这意味着高斯分布的峰值高度始终为 1.0，而不是随着粗糙度的增加而缩小，随着粗糙度的减小而增大。幸运的是，这个问题很容易解决，因为我们有一个简单的解析公式来计算 SG 的积分。因此，如果我们把振幅设为积分的 1，最终就能得到一个归一化的分布：

$$
D_{SG}(\mathbf{h})= G(\mathbf{h};\mathbf{n},\frac{2}{m^2}, \frac{1}{\pi m^2})
$$

```cpp
SG DistributionTermSG(in float3 direction, in float roughness)
{
    SG distribution;
    distribution.Axis = direction;
    float m2 = roughness * roughness;
    distribution.Sharpness = 2 / m2;
    distribution.Amplitude = 1.0f / (Pi * m2);
 
    return distribution;
}
Let’s take a look at a graph of our distribution term, and see how far off it is from our target:Top graph shows a comparison between GGX, Beckmann, normalized Gaussian, and SG distribution terms with a roughness of 0.25. The bottom shows the same comparison with  a roughness of 0.5. Click on either image for an interactive graph.It should come as no surprise that our SG distribution is almost an exact match for a normalized version of a Gaussian distribution. When the roughness is lower it’s also a fairly close match for Beckmann, but for higher roughness the difference gets to be quite large. In both cases our distribution isn’t a perfect fit for GGX, but it’s a workable approximation.Warping Between DomainsSo we now have a normal distribution function in SG format, but unfortunately we’re not quite ready to use it as-is. The problem is that we’ve defined our distribution in the half-vector domain: the axis of the SG points in the direction of the surface normal, and we use the half-vector as our sampling direction. If we want to use an SG product to compute the result of multiplying our distribution with an SG light source, then we need to ensure that the distribution lobe is in the same domain as our light source. Another way to think about this is that center of our distribution shifts depending on viewing angle, since the half-vector also shifts as the camera moves.In order to make sure that our distribution lobe is in the correct domain, we need “warp” our distribution so that it lines up with the current BRDF slice for our viewing direction. If you’re wondering what a BRDF slice is, it basically tells you “if I pick a particular view direction. what is the value of my  BRDF for a given light direction?”. So if you had a mirror BRDF, the slice would just be a single ray pointing in the direction of the view ray reflected off the surface normal. For microfacet specular BRDF’s, you get a lobe that’s roughly centered around the reflected view direction. Here’s what a polar graph of a GGX BRDF slice looks like if we only consider the distribution term:Polar graph of the GGX distribution term from two different viewing angles. The blue line is the view direction, the green line is the surface normal, and the pink line is the reflected view direction. The top image shows the resulting slice when the viewing angle is 0 degrees, and the bottom images shows the resulting slice when the viewing angle is 45 degrees.Wang et al. proposed a simple spherical warp operator that would orient the distribution lobe about the reflected view direction, while also modifying the sharpness to take into account the differential area at the original center of the lobe:\mu_{w}=2(\mathbf{o} \cdot \mu_{d})\mu_{d} - \mathbf{o}  \lambda_{w}=\frac{\lambda_{d}}{4|\mu_{d} \cdot \mathbf{o}|}  a_{w} = a_{d}  SG WarpDistributionSG(in SG ndf, in float3 view)
{
    SG warp;
 
    warp.Axis = reflect(-view, ndf.Axis);
    warp.Amplitude = ndf.Amplitude;
    warp.Sharpness = ndf.Sharpness;
    warp.Sharpness /= (4.0f * max(dot(ndf.Axis, view), 0.0001f));
 
    return warp;
}
Let’s now look at the result of that warp, and compare it to what the actual GGX distribution looks like:Result of applying a spherical warp to the SG distribution term (green) compared with the actual GGX distribution (red). The top graph shows a viewing angle of 0 degrees, and the bottom graph shows a viewing angle of 45 degrees.A quick look at the graph shows us that the shape is a bit off, but our warped lobe is ending up in approximately in the right spot. We’ll revisit the lobe shape later, but for now let’s try combining our distribution with the rest of our BRDF.Approximating The Remaining TermsIn the previous section we figured out how to obtain an SG approximation of our distribution term, and also warp it so that it’s in the correct domain for multiplication with our SG light source. Using our SG product operator would allow to us to represent the result of multiplying the distribution with the light source as another SG, which we could then multiply with other terms using SG operators…or at least we could if we were to represent the remaining terms as SG’s. Unfortunately this turns out to be a problem: the geometry and Fresnel terms are nothing at all like a Gaussian, which rules out approximating them as an SG. Wang et al. sidestep this issue by making the somewhat-weak assumption that the values of these terms will be constant across the entire BRDF lobe, which allows them to pull the terms out of the integral and evaluate them only for the axis direction of the BRDF lobe. This allows the resulting BRDF to still capture some of the glancing angle effects, with similar performance cost to evaluating those terms for a punctual light source. The downside is that the error of these terms will increase as the BRDF lobe becomes wider (increasing roughness), since the value of the geometry and Fresnel terms will vary more the further they are from the lobe center. Putting it all together gives the following specular BRDF:f_{sg}(\mathbf{i}, \mathbf{o}) = \frac{F(\mathbf{o}, \mathbf{h_{w}})\,G(\mathbf{\mu_{w}}, \mathbf{o}, \mathbf{h_{w}})\, \frac{1}{\pi m^2}e^{\lambda_{w}(\mathbf{\mu_{w}} \cdot \mathbf{i} - 1)}} {4\,(\mathbf{n} \cdot \mathbf{\mu_{w}})\,(\mathbf{n} \cdot \mathbf{o})}  \mathbf{h_{w}} = \frac{\mathbf{o} + \mathbf{\mu_{w}}}{||\mathbf{o} + \mathbf{\mu_{w}}||}  The last thing we need to account for is the cosine term that needs to be multiplied with the BRDF inside of the hemispherical integral. Wang et al. suggest using an SG product to compute an SG representing the result of multiplying the distribution term SG and their SG approximation of a clamped cosine lobe, which can then be multiplied with the lighting lobe using an SG inner product. In order to avoid another expensive SG operation, we will instead use the same approach that we used for geometry and Fresnel terms and evaluate the cosine term using the BRDF lobe axis direction. Implementing it in shader code gives us the following:float GGX_V1(in float m2, in float nDotX)
{
    return 1.0f / (nDotX + sqrt(m2 + (1 - m2) * nDotX * nDotX));
}
 
float3 SpecularTermSGWarp(in SG light, in float3 normal,
                          in float roughness, in float3 view,
                          in float3 specAlbedo)
{
    // Create an SG that approximates the NDF.
    SG ndf = DistributionTermSG(normal, roughness);
 
    // Warp the distribution so that our lobe is in the same
    // domain as the lighting lobe
    SG warpedNDF = WarpDistributionSG(ndf, view);
 
     // Convolve the NDF with the SG light
    float3 output = SGInnerProduct(warpedNDF, light);
 
    // Parameters needed for the visibility
    float3 warpDir = warpedNDF.Axis;
    float m2 = roughness * roughness;
    float nDotL = saturate(dot(normal, warpDir));
    float nDotV = saturate(dot(normal, view));
    float3 h = normalize(warpedNDF.Axis + view);
 
    // Visibility term evaluated at the center of
    // our warped BRDF lobe
    output *= GGX_V1(m2, nDotL) * GGX_V1(m2, nDotV);
 
    // Fresnel evaluated at the center of our warped BRDF lobe
    float powTerm = pow((1.0f - saturate(dot(warpDir, h))), 5);
    output *= specAlbedo + (1.0f - specAlbedo) * powTerm;
 
    // Cosine term evaluated at the center of the BRDF lobe
    output *= nDotL;
 
    return max(output, 0.0f);
}
Let’s now (finally) take a look at what our specular approximation looks like for a scene being lit by an SG light source:A scene being lit by an SG light source using our diffuse and specular approximations. The scene is using a uniform roughness of 0.128.Going AnisotropicSo if we look at the specular highlights in the above image, you may notice that while the highlights on the red and green walls look pretty good, the highlight on floor seems a bit off. The highlight is rather wide and rounded, and our intuition tells that a highlight viewed at a grazing angle should appear vertically stretched across the floor. To determine why the look is so off, we need to revisit our warp of the distribution term. Previously when we looked at the polar graph of the distribution I noted that the shape of the resulting lobe was a bit off, even though it was oriented in approximately the right direction to line up with the BRDF slice. To get a better idea of what’s going on, let’s now take a look at a 3D graph of the GGX distribution term:3D graph of the GGX distribution term. The left image shows the distribution when the viewing angle is very low, while the right image shows the distribution when the viewing angle is very steep.Looking at the left image where the angle between the view direction and  the surface normal are very low, the distribution is radially symmetrical just like an SG lobe. However as the viewing angle increases the lobe begins to stretch, looking more and more like the non-symmetrical lobe that we see in the right image. The stretching of the lobe is what causes the stretched highlights that occur when applying the BRDF, and our warped SG is unable to properly represent it since it must remain radially symmetric about its axis.Luckily for us, there is a better way. In 2013 Xu et al.released a paper titled Anisotropic Spherical Gaussians[6], where they explain how they extended SG’s to support anisotropic lobe width/sharpness. They’re defined like this:G(\mathbf{v};[\mu_x, \mu_y, \mu_z],[\lambda_x, \lambda_y],a) = a \cdot S(\mathbf{v},\mu_z) e^{-\lambda_x(\mathbf{v} \cdot \mu_x) - \lambda_y(\mathbf{v} \cdot \mu_y)}  Instead of having a single axis direction, an ASG now has  \mu_x  ,  \mu_y  , and  \mu_z  , which are three orthogonal vectors forming a complete basis. It’s very similar to a tangent frame, where the normal, tangent, and bitangent together make up an orthonormal basis. With an ASG you also now have two separate sharpness parameters,  \lambda_x   and \lambda_y  , which control the sharpness with respect to  \mu_x   and  \mu_y  . So for example setting  \lambda_x   to 16 and \lambda_y   to 64 will result in stretched Gaussian lobe that’s skinnier along the \lambda_y   direction, and with its center located at  \mu_z  . Visualizing such an ASG on the surface of a sphere gives you this: An Anisotropic Spherical Gaussian visualized on the surface of a sphere.  \lambda_x   has a value of 16, and  \lambda_y   has a value of 64.Like SG’s, the equations lend themselves to simple HLSL implementations:struct ASG
{
    float3 Amplitude;
    float3 BasisZ;
    float3 BasisX;
    float3 BasisY;
    float SharpnessX;
    float SharpnessY;
};
 
float3 EvaluateASG(in ASG asg, in float3 dir)
{
    float sTerm = saturate(dot(asg.BasisZ, dir));
    float lambdaTerm = asg.SharpnessX * dot(dir, asg.BasisX)
                                      * dot(dir, asg.BasisX);
    float muTerm = asg.SharpnessY * dot(dir, asg.BasisY)
                                  * dot(dir, asg.BasisY);
    return asg.Amplitude * sTerm* exp(-lambdaTerm - muTerm);
}
The ASG paper provides us with formulas for two operations that we can use to improve the quality of our specular approximation for SG light sources. The first is a new warping operator that can take an NDF represented as an isotropic SG, and stretch it along the view direction to produce an ASG that better represents the actual BRDF. The other helpful forumula it provides is for convolving an ASG with an SG, which we can use to convolve a anisotropically warped NDF lobe with an SG lighting lobe. Let’s take a look at how their improved warp looks when graphing the NDF for a large viewing angle:The left image is a 3D graph of the distribution term when using a spherical warp. The middle image is the resulting distribution term when using an anisotropic warp. The right image is the actual GGX distribution term.The anisotropic distribution looks much closer to the actual GGX NDF, since it now has the vertical stretching that we were missing. Let’s now implement their formulas in HLSL so we can try the new warp in our test scene:float3 ConvolveASG_SG(in ASG asg, in SG sg) {
    // The ASG paper specifes an isotropic SG as
    // exp(2 * nu * (dot(v, axis) - 1)),
    // so we must divide our SG sharpness by 2 in order
    // to get the nup parameter expected by the ASG formula
    float nu = sg.Sharpness * 0.5f;
 
    ASG convolveASG;
    convolveASG.BasisX = asg.BasisX;
    convolveASG.BasisY = asg.BasisY;
    convolveASG.BasisZ = asg.BasisZ;
 
    convolveASG.SharpnessX = (nu * asg.SharpnessX) /
                             (nu + asg.SharpnessX);
    convolveASG.SharpnessY = (nu * asg.SharpnessY) /
                             (nu + asg.SharpnessY);
 
    convolveASG.Amplitude = Pi / sqrt((nu + asg.SharpnessX) *
                                 (nu + asg.SharpnessY));
 
    float3 asgResult = EvaluateASG(convolveASG, sg.Axis);
    return asgResult * sg.Amplitude * asg.Amplitude;
}
 
ASG WarpDistributionASG(in SG ndf, in float3 view)
{
    ASG warp;
 
    // Generate any orthonormal basis with Z pointing in the
    // direction of the reflected view vector
    warp.BasisZ = reflect(-view, ndf.Axis);
    warp.BasisX = normalize(cross(ndf.Axis, warp.BasisZ));
    warp.BasisY = normalize(cross(warp.BasisZ, warp.BasisX));
 
    float dotDirO = max(dot(view, ndf.Axis), 0.0001f);
 
    // Second derivative of the sharpness with respect to how
    // far we are from basis Axis direction
    warp.SharpnessX = ndf.Sharpness / (8.0f * dotDirO * dotDirO);
    warp.SharpnessY = ndf.Sharpness / 8.0f;
 
    warp.Amplitude = ndf.Amplitude;
 
    return warp;
}
 
float3 SpecularTermASGWarp(in SG light, in float3 normal,
                           in float roughness, in float3 view,
                           in float3 specAlbedo)
{
    // Create an SG that approximates the NDF
    SG ndf = DistributionTermSG(normal, roughness);
 
    // Apply a warpring operation that will bring the SG from
    // the half-angle domain the the the lighting domain.
    ASG warpedNDF = WarpDistributionASG(ndf, view);
 
    // Convolve the NDF with the light
    float3 output = ConvolveASG_SG(warpedNDF, light);
 
    // Parameters needed for evaluating the visibility term
    float3 warpDir = warpedNDF.BasisZ;
    float m2 = roughness * roughness;
    float nDotL = saturate(dot(normal, warpDir));
    float nDotV = saturate(dot(normal, view));
    float3 h = normalize(warpDir + view);
 
    // Visibility term
    output *= GGX_V1(m2, nDotL) * GGX_V1(m2, nDotV);
 
    // Fresnel
    float powTerm = pow((1.0f - saturate(dot(warpDir, h))), 5);
    output *= specAlbedo + (1.0f - specAlbedo) * powTerm;
 
    // Cosine term
    output *= nDotL;
 
    return max(output, 0.0f);
}
If we swap out our old specular approximation for one that uses an anisotropic warp, our test scene now looks much better!Diffuse and specular approximations applied to an SG light source, using an anisotropic warp to approximate the NDF as an ASG.Visually Comparing the BRDFMany of the images that I used for visually comparing the SG BRDF approximations were generated using Disney’s BRDF Explorer. When we were doing our initial research into SG’s and figuring out how to implement them, BRDF Explorer was extremely valuable both for understanding the concepts and for experimenting with different variations. If you’d like to this yourself, there’s a very easy way to do that courtesy of Nick Brancaccio. Nick was kind of enough to create his own awesome WebGL version of BRDF Explorer, and it comes pre-loaded with options for comparing an approximate SG specular BRDF with the GGX BRDF. I would recommend checking it out if you’d like to play around with the BRDF’s and make some pretty 3D graphs!References[1] Background: Physics and Math of Shading (SIGGRAPH 2013 Course: Physically Based Shading in Theory and Practice) – http://blog.selfshadow.com/publications/s2013-shading-course/hoffman/s2013_pbs_physics_math_notes.pdf

[2] Understanding the Masking-Shadowing Function in Microfacet-Based BRDFs – http://jcgt.org/published/0003/02/03/

[3] All-Frequency Rendering of Dynamic, Spatially-Varying Reflectance – http://research.microsoft.com/en-us/um/people/johnsny/papers/sg.pdf

[4] A Reflectance Model for Computer Graphics – http://inst.cs.berkeley.edu/~cs294-13/fa09/lectures/cookpaper.pdf

[5] Theory for Off-Specular Reflection From Roughened Surfaces – http://www.graphics.cornell.edu/~westin/pubs/TorranceSparrowJOSA1967.pdf

[6] Anisotropic Spherical Gaussians – http://cg.cs.tsinghua.edu.cn/people/~kun/asg/

[7] BRDF Explorer – https://github.com/wdas/brdf

[8] WebGL BRDF Explorer – https://depot.floored.com/brdf_explorerShare this:Like Loading...

    Related
New Blog Series: Lightmap Baking and Spherical GaussiansLiked by 1 personSG Series Part 2: Spherical Gaussians 101Liked by 1 personSG Series Part 5: Approximating Radiance and Irradiance With SG’sWith 3 comments
```