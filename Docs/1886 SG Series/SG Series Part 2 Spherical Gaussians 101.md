---
title: "SG Series Part 2: Spherical Gaussians 101"
source: https://mynameismjp.wordpress.com/2016/10/09/sg-series-part-2-spherical-gaussians-101/
author:
  - "[[MJP]]"
published: 2016-10-10
created: 2026-06-14
description: "You can find an ad-free static site version of this post here: https://therealmjp.github.io/posts/sg-series-part-2-spherical-gaussians-101/ This is part 2 of a series on Spherical Gaussians and their applications for pre-computed lighting. You can find the other articles here: Part 1 - A Brief (and Incomplete) History of Baked Lighting RepresentationsPart 2 - Spherical Gaussians 101Part 3 - Diffuse Lighting From…"
tags:
  - Clippings review
updated: 2026-06-14T17:15
---
**You can find an ad-free static site version of this post here: [https://therealmjp.github.io/posts/sg-series-part-2-spherical-gaussians-101/](https://therealmjp.github.io/posts/sg-series-part-2-spherical-gaussians-101/)  
您可以在这里找到这篇文章的无广告静态网站版本： [https://therealmjp.github.io/posts/sg-series-part-2-spherical-gaussians-101/](https://therealmjp.github.io/posts/sg-series-part-2-spherical-gaussians-101/)**

*This is part 2 of a series on Spherical Gaussians and their applications for pre-computed lighting. You can find the other articles here:  
这是关于球面高斯函数及其在预计算光照中应用系列文章的第二部分。您可以在这里找到其他文章：*

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

In the [previous article](https://mynameismjp.wordpress.com/2016/10/09/sg-series-part-1-a-brief-and-incomplete-history-of-baked-lighting-representations/), I gave a quick rundown of some of the available techniques for representing a pre-computed distribution of radiance or irradiance for each lightmap texel or probe location. In this article, I’m going cover the basics of Spherical Gaussians, which are a type of spherical radial basis function (SRBF for short). The concepts introduced here will serve as the core set of tools for working with Spherical Gaussians, and in later articles I’ll demonstrate how you can use those tools to form an alternative for approximating incoming radiance in pre-computed lightmaps or probes.  
在 [上一篇文章](https://mynameismjp.wordpress.com/2016/10/09/sg-series-part-1-a-brief-and-incomplete-history-of-baked-lighting-representations/) 中，我简要概述了一些可用于表示每个光照贴图纹素或探针位置的预计算辐射率或辐照度分布的技术。在本文中，我将介绍球面高斯函数的基础知识，它是一种球面径向基函数（简称 SRBF）。本文介绍的概念将作为使用球面高斯函数的核心工具集，在后续文章中，我将演示如何使用这些工具来构建一种替代方法来近似预计算光照贴图或探针中的入射辐射率。

I should point out that this article is still going to be somewhat high-level, in that it won’t provide full derivations and background details for all formulas and operations. However it is my hope that the material here will be sufficient to gain a basic understanding of SG’s, and also use them in practical scenarios.  
需要指出的是，本文仍将侧重于概括性内容，不会提供所有公式和运算的完整推导和背景细节。但我希望本文内容足以帮助读者对 SG（稳定同位素）有一个基本的了解，并能在实际场景中加以运用。

### What’s a Spherical Gaussian?什么是球面高斯分布？

A Spherical Gaussian, or “SG” for short, is essentially a [Gaussian function](https://en.wikipedia.org/wiki/Gaussian_function) \[1\] that’s defined on the surface of a sphere. If you’re reading this, then you’re probably already familar with how a Gaussian function works in 1D: you compute the distance from the center of the Gaussian, and use this distance as part of a base-e exponential. This produces the characteristic “hump” that you see when you graph it:  
球面高斯函数（简称“SG”）本质上是定义在球面上的 [高斯函数](https://en.wikipedia.org/wiki/Gaussian_function) \[1\]。如果您正在阅读本文，那么您可能已经熟悉一维高斯函数的工作原理：计算高斯函数中心到各点的距离，并将该距离作为以 e 为底的指数函数的一部分。这样就产生了您在绘制高斯函数图像时看到的特征性“驼峰”：

![Gaussian_1D](https://mynameismjp.wordpress.com/wp-content/uploads/2016/06/gaussian_1d.png?w=600)

Gaussian\_1D

*A Gaussian in 1D centered at x=0, with a height of 3  
以 x=0 为中心，高度为 3 的一维高斯分布*

You’re probably also familiar with how it looks in 2D, since it’s very commonly used in image processing as a filter kernel. It ends up looking like what you would get if you took the above graph and revolved it around its axis  
你可能也熟悉它在二维平面上的样子，因为它在图像处理中经常被用作滤波器内核。最终效果就像是将上面的图绕其轴旋转一周后得到的图像。

![Gaussian_2D](https://mynameismjp.wordpress.com/wp-content/uploads/2016/06/gaussian_2d.png)

Gaussian\_2D

*A Gaussian filter applied to a 2D image of a white dot, showing that the impulse response is effectively a Gaussian function in 2D  
将高斯滤波器应用于白色圆点的二维图像，结果表明，其脉冲响应在二维空间中实际上是一个高斯函数。*

A Spherical Gaussian still works the same way, except that it now lives on the surface of a sphere instead of on a line or a flat plane. If you’re having trouble visualizing that, imagine if you took the above image and wrapped it around a sphere like wrapping paper. It ends up looking like this:  
球面高斯函数的工作原理仍然相同，只是它现在位于球面上，而不是直线或平面上。如果您难以想象，可以想象一下将上面的图像像包装纸一样包裹在一个球体上。最终效果如下：

![SG_Sphere](https://mynameismjp.wordpress.com/wp-content/uploads/2016/06/sg_sphere.png?w=600)

*A Spherical Gaussian visualized on the surface of a sphere  
球面高斯分布在球体表面上的可视化图像*

Since an SG is defined on a sphere rather than a line or plane, it’s parameterized differently than a normal Gaussian. A 1D Gaussian function always has the following form:  
由于球面高斯函数定义在球面上而不是直线或平面上，因此它的参数化方式与普通高斯函数不同。一维高斯函数始终具有以下形式：

$$
ae^{\frac{-(x - b)^{2}}{2c^{2}}}
$$
 

The part that we need to change in order to define the function on a sphere is the “(x – b)” term. This part of the function essentially makes the Gaussian a function of the cartesian distance between a given point and the center of the Gaussian, which can be trivially extended into 2D using the standard distance formula. To make this work on a sphere, we must instead make our Gaussian a function of the angle between two unit direction vectors. In practice we do this by making an SG a function of the *cosine* of the angle between two vectors, which can be efficiently computed using a dot product like so:  
为了在球面上定义该函数，我们需要修改的是“(x – b)”项。该项本质上使高斯函数成为给定点到高斯中心的笛卡尔距离的函数，这可以通过标准的距离公式轻松地扩展到二维。为了使其适用于球面，我们必须将高斯函数改为两个单位方向向量之间夹角的函数。实际上，我们通过将高斯函数设为两个向量夹角的 *余弦* 函数来实现这一点，而余弦值可以使用点积高效地计算，如下所示：

$$
G(\mathbf{v};\mathbf{\mu},\lambda,a) = ae^{\lambda(\mathbf{\mu} \cdot \mathbf{v} - 1)}
$$

Just like a normal Gaussian, we have a few parameters that control the shape and location of the resulting lobe. First we have μ, which is the *axis*, or *direction* of the lobe. It effectively controls where the lobe is located on the sphere, and always points towards the exact center of the lobe. Next we have λ, which is the *sharpness* of the lobe. As this value increases, the lobe will get “skinnier”, meaning that the result will fall off more quickly as you get further from the lobe axis. Finally we have *a*, which is the *amplitude* or *intensity* of the lobe. If you were to look at a [polar graph of an SG](https://www.desmos.com/calculator/rvtqpze0g7), it would correspond to the height of the lobe at its peak. The amplitude can be a scalar value, or for graphics applications we may choose to make it an RGB triplet in order to support varying intensities for different color channels. This all lends itself to a simple HLSL code definition:  
与普通高斯函数类似，我们有一些参数来控制最终波瓣的形状和位置。首先是 μ，它是波瓣的 *轴线* 或 *方向* 。它有效地控制了波瓣在球面上的位置，并且始终指向波瓣的精确中心。其次是 λ，它是波瓣的 *锐度* 。随着该值的增加，波瓣会变得“更窄”，这意味着随着远离波瓣轴线，结果会更快地衰减。最后是 *振幅* ，它是波瓣的 *强度* 。如果您查看 [SG 的极坐标图](https://www.desmos.com/calculator/rvtqpze0g7) ，它将对应于波瓣峰值的高度。振幅可以是标量值，或者对于图形应用程序 *，* 我们可以选择将其设置为 RGB 三元组，以便支持不同颜色通道的不同强度。所有这些都可以简化为一个简单的 HLSL 代码定义：

```cpp
struct SG
{
    float3 Amplitude;
    float3 Axis;
    float Sharpness;
};
```

Evaluating an SG is also easily expressible in HLSL. All we need is a normalized direction vector representing the point on the sphere where we’d like to compute the value of the SG:  
在 HLSL 中，计算 SG 值也很容易。我们只需要一个归一化的方向向量，表示球面上我们要计算 SG 值的点：

```cpp
float3 EvaluateSG(in SG sg, in float3 dir)
{
    float cosAngle = dot(dir, sg.Axis);
    return sg.Amplitude * exp(sg.Sharpness * (cosAngle - 1.0f));
}
```

### Why Spherical Gaussians?为什么选择球面高斯分布？

Now that we know what a Spherical Gaussian is, what’s so useful about them anyway? One pontential benefit is that they’re fairly intuitive: it’s not terribly hard to understand how the 3 parameters work, and how each parameter affects the resulting lobe. The other main draw is that they inherit a lot of useful properties of “regular” Gaussians, which makes them useful for graphics and other related applications. These properties have been explored and utilized in several research papers that were primarily aimed at achieving pre-computed radiance transfer (PRT) with both diffuse and specular material response. In particular, the paper entitled “ [All-Frequency Rendering of Dynamic, Spatially-Varying Reflectance](http://research.microsoft.com/en-us/um/people/johnsny/papers/sg.pdf) \[2\]” by Wang et al. was our main inspiration for pursuing SG’s at RAD.  
既然我们已经了解了球面高斯函数的概念，那么它究竟有何用处呢？一个潜在的优势是它相当直观：理解这三个参数的作用以及每个参数如何影响最终的波瓣并不难。另一个主要优势是它继承了“常规”高斯函数的许多有用特性，这使得它在图形学和其他相关应用中非常有用。这些特性已经在多篇研究论文中得到探索和应用，这些论文主要旨在实现具有漫反射和镜面反射材质响应的预计算辐射传递（PRT）。特别是，Wang 等人的论文“ [动态空间变化反射率的全频渲染](http://research.microsoft.com/en-us/um/people/johnsny/papers/sg.pdf) \[2\]”是我们 RAD 研究球面高斯函数的主要灵感来源。

### Products 产品

So what are these useful Gaussian properties that we can exploit? For starters, taking the product of 2 Gaussians functions produces another Gaussian. For an SG, this is equivalent to visiting every point on the sphere, evaluating 2 different SG’s, and multiplying the two results. Since it’s an operation that takes 2 SG’s and produces another SG, it is sometimes referred to as a “vector” product. It’s defined as the following:  
那么，我们可以利用高斯分布的哪些有用性质呢？首先，两个高斯函数的乘积仍然是一个高斯函数。对于一个球形高斯函数（SG），这相当于遍历球面上的每个点，计算两个不同的 SG，然后将两个结果相乘。由于这是一个将两个 SG 相乘并生成另一个 SG 的操作，因此有时被称为“向量”乘积。它的定义如下：

$$
G_{1}(\mathbf{v})G_{2}(\mathbf{v}) = G(\mathbf{v};\frac{\mu_{m}}{||\mu_{m}||},a_{1}a_{2}e^{\lambda_{m}(||\mu_{m}|| - 1)})
$$

$$
\lambda_{m} = \lambda_{1} + \lambda_{2}
$$

$$
\mu_{m} = \frac{\lambda_{1}\mu_{1} + \lambda_{2}\mu_{2}}{\lambda_{1} + \lambda_{2}}
$$

In HLSL code, it looks like this:  
在 HLSL 代码中，它看起来像这样：

```cpp
SG SGProduct(in SG x, in SG y)
{
    float3 um = (x.Sharpness * x.Axis + y.Sharpness * y.Axis) /
                (x.Sharpness + y.Sharpness);
    float umLength = length(um);
    float lm = x.Sharpness + y.Sharpness;
 
    SG res;
    res.Axis = um * (1.0f / umLength);
    res.Sharpness = lm * umLength;
    res.Amplitude = x.Amplitude * y.Amplitude *
                    exp(lm * (umLength - 1.0f));
 
    return res;
}
```

### Integrals 积分

Gaussians have another really nice property in that their integrals have a closed-form solution, which is known as the [error function](https://en.wikipedia.org/wiki/Error_function) \[3\]. The property also extends to SG’s, where we can compute the integral of an SG over the entire sphere:  
高斯函数还有一个非常好的性质，那就是它们的积分有闭合形式的解，这个解被称为 [误差函数](https://en.wikipedia.org/wiki/Error_function) \[3\]。这个性质也适用于 SG 函数，我们可以计算 SG 函数在整个球面上的积分：

$$
\int_{\Omega} G(\mathbf{v})d\mathbf{v} = 2\pi\frac{a}{\lambda}(1 - e^{-2\lambda})
$$
 

Computing an integral will essentially tell us the total “energy” of an SG, which can be useful for lighting calculations. It can also be useful for *normalizing* an SG, which produces an SG that integrates to 1. Such a normalized SG is suitable for representing a probability distribution, such as an NDF. In fact, a normalized SG is actually equivalent to a [von Mises-Fisher distribution](https://en.wikipedia.org/wiki/Von_Mises%E2%80%93Fisher_distribution) \[4\] in 3D!  
计算积分本质上可以告诉我们 SG 的总“能量”，这对于光照计算非常有用。它还可以用于 *归一化* SG，从而得到积分值为 1 的 SG。这种归一化的 SG 适合表示概率分布，例如 NDF。事实上，归一化的 SG 在三维空间中等价于 [von Mises-Fisher 分布](https://en.wikipedia.org/wiki/Von_Mises%E2%80%93Fisher_distribution) \[4\]！

An SG integral is actually very cheap to compute…or at least it would be if we removed the exponential term. It turns out that the $(1 - e^{-2\lambda})$  term actually approaches 1 very quickly as the SG’s sharpness increases, which means we can potentially drop it with little error as long as we know that the sharpness is high enough. Here’s what a graph of $(1 - e^{-2\lambda})$  looks like for increasing sharpness:  
SG 积分的计算量其实非常小……或者说，如果我们去掉指数项的话。事实证明，随着 SG 积分的锐度增加， $(1 - e^{-2\lambda})$  项会很快趋近于 1，这意味着只要锐度足够高，我们就可以几乎无误差地去掉它。下图展示了随着锐度增加， $(1 - e^{-2\lambda})$  项的变化情况：

![SG_Integral_ExpTerm](https://mynameismjp.wordpress.com/wp-content/uploads/2016/07/sg_integral_expterm.png?w=600)

SG\_Integral\_ExpTerm

*A graph of the exponential term in computing the integral of an SG over a sphere, which approaches 1 as the sharpness increases. The X-axis is sharpness, and the Y-axis is the value of $(1 - e^{-2\lambda})$  .  
图中展示了计算球面上 SG 积分时指数项的变化，该指数项随着锐度的增加而趋近于 1。X 轴表示锐度，Y 轴表示 $(1 - e^{-2\lambda})$  的值。*

This all lends itself naturally to HLSL implementations for accurate and approximate versions of an SG integral:

```cpp
float3 SGIntegral(in SG sg)
{
    float expTerm = 1.0f - exp(-2.0f * sg.Sharpness);
    return 2 * Pi * (sg.Amplitude / sg.Sharpness) * expTerm;
}
 
float3 ApproximateSGIntegral(in SG sg)
{
    return 2 * Pi * (sg.Amplitude / sg.Sharpness);
}
```

### Inner Product

If we were to use our SG integral formula to compute the integral of the product of two SG’s, we can compute what’s known as the *inner product*, or *dot product* of those SG’s. The operation is usually defined like this:

$$
\int_{\Omega} G_{1}(\mathbf{v}) G_{2}(\mathbf{v}) d\mathbf{v} = \frac{4 \pi a_{0} a_{1}}{e^{\lambda_{m}}} \frac{sinh(d_{m})}{d_{m}}
$$
 

$$
d_{m} = || \lambda_{1}\mu_{1} + \lambda_{2}\mu_{2} ||
$$

However we can avoid numerical precision issues by using an alternate arrangement:  
但是，我们可以通过采用另一种方法来避免数值精度问题：

$$
\int_{\Omega} G_{1}(\mathbf{v}) G_{2}(\mathbf{v}) d\mathbf{v} = 2 \pi a_{0} a_{1}\frac{e^{d_{m} - \lambda_{m}} - e^{-d_{m} - \lambda_{m}}}{d_{m}}
$$
 

…which looks like this in HLSL:  
……在 HLSL 中看起来是这样的：

```cpp
float3 SGInnerProduct(in SG x, in SG y)
{
     float dm = length(x.Sharpness * x.Axis + y.Sharpness * y.Axis);
     float3 expo = exp(dm - x.Sharpness - y.Sharpness) * x.Amplitude * y.Amplitude;
     float other = 1.0f - exp(-2.0f * dm);
     return (2.0f * Pi * expo * other) / dm;
}
```

### Treshold

SG’s have what’s known as “compact-ε” support, which means that it’s possible to determine an angle θ such that all points within θ radians of the SG’s axis will have a value greater than ε. This property is potentially more useful if we flip it around so that we calculate a sharpness λ that results in a given θ for a particular value of ε:

$a e^{\lambda(cos\theta - 1)} = \epsilon$   
$\lambda = \frac{ln(\epsilon) - ln(a)}{cos\theta - 1}$

```cpp
float SGSharpnessFromThreshold(in float amplitude,
                               in float epsilon,
                               in float cosTheta)
{
    return (log(epsilon) - log(amplitude)) / (cosTheta - 1.0f);
}
```

### Rotation

One last operation I’ll discuss is rotation. Rotating an SG is trivial: all you need to do is apply your rotation transform to the SG’s axis vector and you have a rotated SG! You can apply the transform using a matrix, a quaternion, or any other means you might have for rotating a vector. This is a welcome change from SH, which requires a very complex transform once you go above L1.

\[1\] Gaussian Function – [https://en.wikipedia.org/wiki/Gaussian\_function  
](https://en.wikipedia.org/wiki/Gaussian_function)\[2\] All-Frequency Rendering of Dynamic, Spatially-Varying Reflectance – [http://research.microsoft.com/en-us/um/people/johnsny/papers/sg.pdf](http://research.microsoft.com/en-us/um/people/johnsny/papers/sg.pdf)  
\[3\] Error Function – [https://en.wikipedia.org/wiki/Error\_function](https://en.wikipedia.org/wiki/Error_function)  
\[4\] von-Mises Fisher Distribtion – [https://en.wikipedia.org/wiki/Von\_Mises%E2%80%93Fisher\_distribution](https://en.wikipedia.org/wiki/Von_Mises%E2%80%93Fisher_distribution)