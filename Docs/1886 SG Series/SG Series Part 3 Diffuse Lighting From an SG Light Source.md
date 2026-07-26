---
title: "SG Series Part 3: Diffuse Lighting From an SG Light Source"
source: https://mynameismjp.wordpress.com/2016/10/09/sg-series-part-3-diffuse-lighting-from-an-sg-light-source/
author:
  - "[[MJP]]"
published: 2016-10-10
created: 2026-06-14
description: "You can find an ad-free static site version of this post here: https://therealmjp.github.io/posts/sg-series-part-3-diffuse-lighting-from-an-sg-light-source/ This is part 3 of a series on Spherical Gaussians and their applications for pre-computed lighting. You can find the other articles here: Part 1 - A Brief (and Incomplete) History of Baked Lighting Representations Part 2 - Spherical Gaussians 101 Part 3 - Diffuse…"
tags:
  - Clippings review
updated: 2026-06-14T17:15
---
**You can find an ad-free static site version of this post here: [https://therealmjp.github.io/posts/sg-series-part-3-diffuse-lighting-from-an-sg-light-source/](https://therealmjp.github.io/posts/sg-series-part-3-diffuse-lighting-from-an-sg-light-source/)  
您可以在这里找到这篇文章的无广告静态网站版本： [https://therealmjp.github.io/posts/sg-series-part-3-diffuse-lighting-from-an-sg-light-source/](https://therealmjp.github.io/posts/sg-series-part-3-diffuse-lighting-from-an-sg-light-source/)**

*This is part 3 of a series on Spherical Gaussians and their applications for pre-computed lighting. You can find the other articles here:  
这是关于球面高斯函数及其在预计算光照中应用系列文章的第三部分。您可以在这里找到其他文章：*

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

### A Big Gaussian In The Sky天空中一个巨大的高斯光束

In the [previous post](https://mynameismjp.wordpress.com/2016/10/09/sg-series-part-2-spherical-gaussians-101/) we covered a few of the universal properties of SG’s. Now that we have a few tools on our utility belt, let’s discuss an example of how we can actually use those properties to our advantage in a rendering scenario. Let’s say we have a surface point **x** being lit by a light source **L**, with the light source being represented by an SG named **G <sub>L</sub>**. Recall from the previous article that the equation for computing the outgoing radiance towards the eye for a surface with a Lambertian diffuse BRDF looks like the following:  
在 [上一篇文章](https://mynameismjp.wordpress.com/2016/10/09/sg-series-part-2-spherical-gaussians-101/) 中，我们介绍了 SG 的一些通用属性。现在我们已经掌握了一些工具，接下来让我们讨论一个如何在渲染场景中实际利用这些属性的例子。假设我们有一个表面点 **x，** 它被光源 **L** 照射，光源由名为 **G <sub>L</sub>** 的 SG 表示。回顾上一篇文章，对于具有朗伯漫反射 BRDF 的表面，计算朝向眼睛的出射辐射度的公式如下：

$$
L_{o}(\mathbf{o}, \mathbf{x}) = \frac{C_{diffuse}}{\pi} \int_{\Omega} L_{i}(\mathbf{i}, \mathbf{x})cos(\theta_{i})d\Omega
$$
 

For punctual light sources that are essentially a scaled delta function, computing this is as easy as N dot L. But we’re in trouble if we have an area light source, since we typically don’t have a closed form solution to the integral. But let’s suppose that we have some strange Gaussian light source, whose angular falloff can be exactly represented by an SG (normally area light sources are considered to have uniform emission over their surface, but let’s imagine we have case where the emission is non-uniform). If we can treat the light as an SG, then we can start to consider some of the handy Gaussian tools that we laid out earlier. In particular the inner product starts to seem really useful: it gives us the result of integrating the product of two SG’s, which is basically what we’re trying to accomplish in our diffuse lighting equation. The big catch is that we’re not integrating the product of two SG’s, we’re instead integrating the product of an SG with a clamped cosine lobe. Obviously a Gaussian lobe has a different shape compared to a clamped cosine lobe, but perhaps if we squint our eyes from a distance you could substitute one for another. This approach was taken by [Wang et al.](https://www.microsoft.com/en-us/research/wp-content/uploads/2009/12/sg.pdf)\[1\], who suggested fitting a cosine lobe to a single SG with **λ** =2.133 and **a** =1.17. If we follow in their footsteps, the diffuse calculation is straightforward:  
对于本质上是缩放δ函数的点光源，计算起来就像 N·L 一样简单。但如果是面光源，我们就遇到麻烦了，因为通常情况下，积分没有闭合形式的解。不过，假设我们有一个特殊的高斯光源，其角度衰减可以用 SG 精确表示（通常认为面光源在其表面均匀发射，但假设发射不均匀）。如果我们能将光源视为 SG，那么我们就可以开始考虑之前介绍的一些便捷的高斯工具。特别是内积开始变得非常有用：它给出了两个 SG 乘积的积分结果，这正是我们在漫射光照方程中试图实现的目标。关键在于，我们积分的不是两个 SG 的乘积，而是 SG 与一个夹紧余弦瓣的乘积的积分。显然，高斯瓣的形状与夹紧余弦瓣的形状不同，但或许我们可以眯起眼睛从远处观察，将二者相互替代。Wang [等人](https://www.microsoft.com/en-us/research/wp-content/uploads/2009/12/sg.pdf) \[1\] 就采用了这种方法，他们建议将余弦瓣拟合到单个 SG，其中 **λ** = 2.133， **a** = 1.17。如果我们沿用他们的方法，漫反射的计算就非常简单了：

```cpp
SG CosineLobeSG(in float3 direction)
{
    SG cosineLobe;
    cosineLobe.Axis = direction;
    cosineLobe.Sharpness = 2.133f;
    cosineLobe.Amplitude = 1.17f;
 
    return cosineLobe;
}
 
float3 SGIrradianceInnerProduct(in SG lightingLobe,
                                in float3 normal)
{
    SG cosineLobe = CosineLobeSG(normal);
    return max(SGInnerProduct(lightingLobe, cosineLobe), 0.0f);
}
 
float3 SGDiffuseInnerProduct(in SG lightingLobe, in float3 normal,
                             in float3 albedo)
{
    float3 brdf = albedo / Pi;
    return SGIrradianceInnerProduct(lightingLobe, normal) * brdf;
}
Error AnalysisNot too bad, eh? Of course it’s worth taking a closer look at our cosine lobe approximation, since that’s definitely going to introduce some error. Perhaps the best way to do this is to look at the graphs of a real cosine lobe and our SG approximation side-by-side:Comparison of a clamped cosine cosine lobe (red) with an SG approximation (blue)Just from looking at the graph it’s fairly obvious that an SG isn’t necessarily a great fit for a cosine lobe. First of all, the amplitude actually goes above 1, which might seem a bit weird at first glance. However it’s necessary to ensure that the area under the curve remains somewhat consistent with the cosine lobe, since there would otherwise be a loss of energy. The other weirdness stems from the fact that an SG never actually hits 0 anywhere on the sphere, hence the long “tail” on the graph of the SG. This essentially means that if the SG were integrated against a punctual light source, the lighting would “wrap” around the sphere past the point where N dot L is equal to 0. The situation actually isn’t all that different from an SH representation of a cosine lobe, which also extends past π/2:L1 (green) and L2 (purple) SH approximation of a clamped cosine lobe compared with an SG approximation (blue) and the actual clamped cosine (red).In the SH case the approximation actually goes negative, which is arguably worse than the long tail of the SG approximation. The L1 approximation is particularly bad in this regard. If at this point you’re trying to imagine what these approximations look like on a sphere, let me save you the trouble by providing an image:From left to right: actual clamped cosine lobe, SG cosine approximation, L2 SH cosine approximationNow that we’ve finished analyzing the approximation of a cosine lobe, we need to take a look at the actual results of computing diffuse lighting from an SG light source. Let’s start off by graphing the results of computing irradiance using an SG inner product, and compare it against what we get by using brute-force numerical integration to compute the result of multiplying the SG with an actual clamped cosine (not the approximate SG cosine lobe that we use for the inner product):The resulting irradiance from an SG light source (with sharpness of 4.0) as a function of the angle between the light source and the surface normal. The red graph is the result of using numerical integration to compute the integral of the SG light source multiplied with a clamped cosine, while the blue graph was computed using an SG inner product of the light source with a cosine lobe approximated as an SG.As you might expect, the inner product approximation has some error when compared with the “ground truth” provided by numerical integration. It’s worth pointing out that this error is purely a consequence of approximating the clamped cosine lobe as an SG: the inner product provides the exact result of the integral, and thus shouldn’t introduce any error on its own. Despite this error, the resulting irradiance isn’t hugely far off from our ground truth. The biggest difference is for the angles facing away from the light, where the SG inner product version has a stronger tail. Visualizing the resulting diffuse on a sphere gives us the following:The left sphere shows the resulting diffuse lighting from an SG light source with a sharpness of 4.0, where the irradiance was computed using monte carlo importance sampling. The right sphere shows the resulting diffuse lighting from computing irradiance using an SG inner product with an approximation of a cosine lobe.A Cheaper ApproximationAs an alternative to representing the cosine lobe with an SG and computing the inner product, we can consider a cheaper approximation. One advantage of working with SG’s is that each lobe is always symmetrical about its axis, which is also where its value is the highest. We also discussed earlier how we can compute the integral of an SG over the sphere, which gives us its total energy. This suggests that if we want to be frugal with our shader cycles, we can pull terms out of the integral over the sphere/hemisphere and only evaluate them for the SG axis direction. This obviously introduces error, but that error may be acceptable if the term we pull out is relatively “smooth”. If we apply this approximation to computing irradiance and diffuse lighting, we get this:L_{o}(\mathbf{o}, \mathbf{x}) = \frac{C_{diffuse}}{\pi} \int_{\Omega} G_{L}(\mathbf{i};\mathbf{\mu},\lambda,a)cos(\theta_{i})d\Omega  L_{o}(\mathbf{o}, \mathbf{x}) \approx cos(\theta_{\mu}) \frac{C_{diffuse}}{\pi} \int_{\Omega} G_{L}(\mathbf{i};\mathbf{\mu},\lambda,a)d\Omega  Translating to HLSL, we get the following functions:float3 SGIrradiancePunctual(in SG lightingLobe, in float3 normal)
{
    float cosineTerm = saturate(dot(lightingLobe.Axis, normal));
    return cosineTerm * 2.0f * Pi * (lightingLobe.Amplitude) /
                                     lightingLobe.Sharpness;
}
 
float3 SGDiffusePunctual(in SG lightingLobe, in float3 normal,
                         in float3 albedo)
{
    float3 brdf = albedo / Pi;
    return SGIrradiancePunctual(lightingLobe, normal) * brdf;
}
If we overlay the graph of our super-cheap irradiance approximation on the graph we were looking at earlier, we get this:The resulting irradiance from an SG light source (with sharpness of 4.0) as function of the angle between the light source and the surface normal. The red graph was computed using numerical integration, while the blue graph was computed using an SG inner product of the light source with a cosine lobe approximated as an SG. The green graph was computed by pulling the cosine term out of the integral, and multiplying it with the result of integrating the SG light about the sphere.The result shouldn’t be a surprise: it’s just a scaled version of the standard clamped cosine.It’s pretty obvious just by looking that this particular optimization will introduce quite a bit of error, particularly where theta is greater than π/2. But it is cheap, since we’ve effectively turned an SG into a point light. This is makes it a useful tool for cases where we may want to approximate the convolution of an SG light source with a BRDF or some other function that isn’t easily represented as an SG.A More Accurate ApproximationSo it’s nice to have a cheap option, but what if we want more accuracy than our inner product approximation? Fortunately for us, Stephen Hill was able to formulate another alternative approximation that directly fits a curve to the integral of a cosine lobe with an SG. His implementation is actually formulated for a normalized SG (where the integral about the sphere is equal to 1.0), but we can easily account for this by computing the integral and scaling the result by that value:float3 SGIrradianceFitted(in SG lightingLobe, in float3 normal)
{
    const float muDotN = dot(lightingLobe.Axis, normal);
    const float lambda = lightingLobe.Sharpness;
 
    const float c0 = 0.36f;
    const float c1 = 1.0f / (4.0f * c0);
 
    float eml  = exp(-lambda);
    float em2l = eml * eml;
    float rl   = rcp(lambda);
 
    float scale = 1.0f + 2.0f * em2l - rl;
    float bias  = (eml - em2l) * rl - em2l;
 
    float x  = sqrt(1.0f - scale);
    float x0 = c0 * muDotN;
    float x1 = c1 * x;
 
    float n = x0 + x1;
 
    float y = saturate(muDotN);
    if(abs(x0) &amp;lt;= x1)
        y = n * n / x;
 
    float result = scale * y + bias;
 
    return result * ApproximateSGIntegral(lightingLobe);
}
The result is very close to the ground truth, which is very cool considering that it might actually be cheaper than our inner product approximation!The resulting irradiance from an SG light source (with sharpness of 4.0) as function of the angle between the light source and the surface normal. The red graph was computed using numerical integration, while the blue graph was computed using an SG inner product of the light source with a cosine lobe approximated as an SG. The orange graph was computed using Stephen Hill’s fitted curve approximation.If we once again visualize the result on the sphere and compare with our previous results, we get the following:The left sphere shows the resulting diffuse lighting from an SG light source with a sharpness of 4.0, where the irradiance was computed using an SG inner product with an approximation of a cosine lobe. The middle sphere shows the resulting diffuse lighting from computing irradiance using monte carlo importance sampling. The right sphere shows the resulting diffuse lighting from Stephen Hill’s fitted approximation.References[1] All-Frequency Rendering of Dynamic, Spatially-Varying Reflectance – https://www.microsoft.com/en-us/research/wp-content/uploads/2009/12/sg.pdfShare this:Like Loading...

    Related
New Blog Series: Lightmap Baking and Spherical GaussiansLiked by 1 personSG Series Part 2: Spherical Gaussians 101Liked by 1 personSG Series Part 4: Specular Lighting From an SG Light SourceIn "Graphics"
```