Texture2D VisibleColorMap;
Texture2D VisiblePositionMap;
Texture2D VisibleNormalMap;
Texture2D VisibleTangentMap;
Texture2DArray<float4> normalBuckets;
Texture2DArray<float4> colorBuckets;
RWTexture2D<float4> debug0;
RWTexture2D<float4> debug1;
RWTexture2D<float4> debug2;
RWTexture2D<float4> debug3;
SamplerState samPoint;

//将整个Z Range分层，每层一个RWTexture
//保存相应的Fragment




cbuffer cbTrans
{
    float4x4 view;
    float4x4 projTex;
};

cbuffer cbTraceInfo
{
    float gstride;
    float maxStep;
    float nearZ;
    float farZ;
    float screenWidth;
    float screenHeight;
    float N;
    float S;
};



bool intersectsDepthBuffer(float z, float minZ, float maxZ)
{
 /*
 * Based on how far away from the camera the depth is,
 * adding a bit of extra thickness can help improve some
 * artifacts. Driving this value up too high can cause
 * artifacts of its own.
 */
    float depthScale = min(1.0f, z * 1);
    z += 1 + lerp(0.0f, 2.0f, depthScale);
    return (maxZ >= z) && (minZ - 1 <= z);
}


//DDA光线追踪函数
bool traceRayInScreenSpace(in float3 oriV, in float3 dirV,
	out float2 hitPixel, out float4 result)
{
    hitPixel = float2(0.f, 0.0f);
    result = float4(0.f, 0.f, 0.f, 0.f);
    
    uint maxSteps = (uint) maxStep;
    float maxDis = -0.1f;
    
	//判断光线的可见性
    /*
	[unroll]
    for (uint i = 0; i < 6; ++i)
    {
        float tt = (plane[i].w - dot(oriV, plane[i].xyz)) / dot(dirV, plane[i].xyz);
        if (tt > maxDis)
            maxDis = tt;
    }
    
    if (maxDis < 0.1f)
        return false;
    */

	//将光线的终点和原点投影至Sceen Space
    float3 rStartV = oriV + 10 * dirV;
    float3 rEndV = oriV + 500 * dirV;
    float4 rStartS = mul(float4(rStartV, 1.f), projTex);
    float4 rEndS = mul(float4(rEndV, 1.f), projTex);
    float invZStart = 1.f / rStartS.w;
    float invZEnd = 1.f / rEndS.w;

    rStartS /= rStartS.w;
    rEndS /= rEndS.w;
    //转换到screen space 
    rStartS.xy = 0.5f * rStartS.xy * float2(screenWidth, -screenHeight) +
    0.5f * float2(screenWidth, screenHeight);
    rEndS.xy = 0.5f * rEndS.xy * float2(screenWidth, -screenHeight) +
    0.5f * float2(screenWidth, screenHeight);

    rStartS.x = clamp(rStartS.x, 0.f, screenWidth);
    rStartS.y = clamp(rStartS.y, 0.f, screenHeight);
    rEndS.x = clamp(rEndS.x, 0.f, screenWidth);
    rEndS.y = clamp(rEndS.y, 0.f, screenHeight);

//    hitPixel.xy = rStartS.xy;
//    hitPoint.xy = rEndS.xy;
//    return true;

	//屏幕两点差
    float2 screenDelta = (rEndS.xy - rStartS.xy) + float2(0.01f, 0.01f);

	//DDA Step 1: 判断是否需要交换xy
    bool exchange = false;
    if (abs(screenDelta.x) < abs(screenDelta.y))
    {
        exchange = true;
        screenDelta = screenDelta.yx;
        rStartS.xy = rStartS.yx;
        rEndS.xy = rEndS.yx;
    }

	//DDA Step 2: 计算关于"X" 轴的跨距
    float sig = sign(screenDelta.x);
    float invDx = sig / screenDelta.x;

	//计算相关斜率

	//根据DDA算法
	//d=(stepX,stepX*slope)
	//其中stepX=1
    float2 dP = float2(sig, screenDelta.y * invDx);

	//计算 1/z的斜率 用以插值
    float dinvZ = (invZEnd - invZStart) * invDx;

    //根据stride对所有跨距进行缩放
    dP *= gstride;
    dinvZ *= gstride;

//    hitPixel.xy = screenDelta.xy;
//    hitPoint.xy = dP;
//    return false;

    //rayZMax,rayZMin两个变量代表ray在当前pixel的判断阈值
    float rayZMin = 0.f;
    float rayZMax = oriV.z;

    //循环变量
    float invZ = invZStart;
    uint stepCount = 0;
    float pixelEndX = sig * rEndS.x;
    float2 pixel = rStartS.xy;

    //
   
    //DDA step 3：光线跟踪
    for (;
        (sig * pixel.x) < pixelEndX && (stepCount < maxSteps);
        pixel += dP, invZ += dinvZ, ++stepCount)
    {
        //当前所在像素坐标
        //需要根据 exchange进行变换
        hitPixel = exchange ? pixel.yx : pixel.xy;

        //更新ray的z范围
        rayZMin = rayZMax;
        //ray 向前跨越半个像素距
        rayZMax = 1.f / (invZ + 0.5f * dinvZ);

        if (rayZMax < rayZMin)
        {
            float t = rayZMax;
            rayZMax = rayZMin;
            rayZMin = t;
        }


        float pixelZ = VisiblePositionMap[uint2(hitPixel)].w;
        float pixelZMin = pixelZ - 1.5f;
        float pixelZMax = pixelZ + 1.5f;

        //判断碰撞
        if ((rayZMax >= pixelZMin) && (rayZMin <= pixelZMax))
        {
 
            result = VisibleColorMap[uint2(hitPixel)];
            return true;
        }
        
        

    }
    return false;
	
}

//DDA光线追踪函数
//同时计算光照
float4 traceRay(in float3 oriV, in float3 dirV)
{
    float2 hitPixel;
    float4 result = float4(1.f, 1.f, 0.f, 0.f);

    traceRayInScreenSpace(oriV, dirV,
         hitPixel, result);

    return result;
   
}



// Returns true if the ray hit something
bool traceScreenSpaceRay(float2 oriPixel,
 // Camera-space ray origin, which must be within the view volume
 float3 csOrig,
 // Unit length camera-space ray direction
 float3 csDir,
 // Number between 0 and 1 for how far to bump the ray in stride units
 // to conceal banding artifacts. Not needed if stride == 1.
 float jitter,
 // Pixel coordinates of the first intersection with the scene
 out int bindex,
 out float2 hitPixel,
 // Camera space location of the ray hit
 out float3 hitPoint)
{
    bindex = -1;
 // Clip to the near plane
    float rayLength = ((csOrig.z + csDir.z * 1000.f) < nearZ) ?
 (nearZ - csOrig.z) / csDir.z : 1000.f;
    float3 csEndPoint = csOrig + csDir * rayLength;
  
 

 // Project into homogeneous clip space
    float4 H0 = mul(float4(csOrig, 1.0f), projTex);
    H0.xy *= float2(screenWidth, screenHeight);
    float4 H1 = mul(float4(csEndPoint, 1.0f), projTex);
    H1.xy *= float2(screenWidth, screenHeight);
    float k0 = 1.0f / H0.w;
    float k1 = 1.0f / H1.w;

 
 // The interpolated homogeneous version of the camera-space points
    float3 Q0 = csOrig * k0;
    float3 Q1 = csEndPoint * k1;
 
 // Screen-space endpoints
    float2 P0 = H0.xy * k0;
    float2 P1 = H1.xy * k1;
 
 // If the line is degenerate, make it cover at least one pixel
 // to avoid handling zero-pixel extent as a special case later
    P1 += (distance(P0, P1) < 0.0001f) ? float2(0.001f, 0.001f) : 0.0f;
    float2 delta = P1 - P0;
 
 // Permute so that the primary iteration is in x to collapse
 // all quadrant-specific DDA cases later
    bool permute = false;
    if (abs(delta.x) < abs(delta.y))
    {
 // This is a more-vertical line
        permute = true;
        delta = delta.yx;
        P0 = P0.yx;
        P1 = P1.yx;
    }
 
    float stepDir = sign(delta.x);
    float invdx = stepDir / delta.x;
 
 // Track the derivatives of Q and k
    float3 dQ = (Q1 - Q0) * invdx;
    float dk = (k1 - k0) * invdx;
    float2 dP = float2(stepDir, delta.y * invdx);
 
 // Scale derivatives by the desired pixel stride and then
 // offset the starting values by the jitter fraction

    dP *= gstride;
    dQ *= gstride;
    dk *= gstride;
 
    P0 += dP * jitter;
    Q0 += dQ * jitter;
    k0 += dk * jitter;
 
 // Slide P from P0 to P1, (now-homogeneous) Q from Q0 to Q1, k from k0 to k1
    float4 PQk = float4(P0, Q0.z, k0);
    float4 dPQk = float4(dP, dQ.z, dk);
    float3 Q = Q0;
 
 // Adjust end condition for iteration direction
    float end = P1.x * stepDir;
 
    float stepCount = 0.0f;
    float prevZMaxEstimate = csOrig.z;
    float rayZMin = prevZMaxEstimate;
    float rayZMax = prevZMaxEstimate;
    float sceneZMax = rayZMax + 100.0f;
    for (;
 ((PQk.x * stepDir) <= end) && (stepCount < maxStep);
// !intersectsDepthBuffer(sceneZMax, rayZMin, rayZMax) &&
// (sceneZMax != 0.0f);
 ++stepCount)
    {
        rayZMin = prevZMaxEstimate;
        rayZMax = (dPQk.z * 0.5f + PQk.z) / (dPQk.w * 0.5f + PQk.w);
        prevZMaxEstimate = rayZMax;
        if (rayZMin > rayZMax)
        {
            float t = rayZMin;
            rayZMin = rayZMax;
            rayZMax = t;
        }
 
        hitPixel = permute ? PQk.yx : PQk.xy;
 // You may need hitPixel.y = depthBufferSize.y - hitPixel.y; here if your vertical axis
 // is different than ours in screen space
        sceneZMax = VisibleColorMap[uint2(hitPixel)].w;
        if(sceneZMax==0.0f)
        {
            int b = max(0.f, (uint) log2(rayZMax - nearZ) - 1);
            sceneZMax = colorBuckets[uint3(hitPixel, b)].w;
            bindex = sceneZMax == 0.f ? -1 : b;
        }
        
        PQk += dPQk;

        if(intersectsDepthBuffer(sceneZMax, rayZMin, rayZMax))
            break;

    }
 
 // Advance Q based on the number of steps
    Q.xy += dQ.xy * stepCount;
    hitPoint = Q * (1.0f / PQk.w);
    return intersectsDepthBuffer(sceneZMax, rayZMin, rayZMax);
}


struct VertexOut
{
    float4 posH : SV_POSITION;
    float3 ray : RAY;
    float2 tex : TEXCOORD;
};



float4 main(VertexOut pin) : SV_TARGET
{
	//可见性判断
    clip(VisibleNormalMap[uint2(pin.posH.xy)].w - 1.f);


    //采集间接光照
    float3 ori = VisiblePositionMap[uint2(pin.posH.xy)].xyz;
    float3 n = VisibleNormalMap[uint2(pin.posH.xy)].xyz;
    float3 t = VisibleTangentMap[uint2(pin.posH.xy)].xyz;
    float3 b = normalize(cross(n, t));
    float4 indirect = float4(0, 0, 0, 0);
    float3 r = normalize(reflect(normalize(pin.ray), normalize(n)));
    float2 hitPixel = 0;
    float3 hitPoint = 0;
    int bindex;
    if (traceScreenSpaceRay(pin.posH.xy, ori + 20.f * r, r, 0.2f,
        bindex,hitPixel, hitPoint))
    {
        hitPixel.x = clamp(hitPixel.x, 0.f, screenWidth);
        hitPixel.y = clamp(hitPixel.y, 0.f, screenHeight);
        if(bindex==-1)
            indirect += 0.25 * VisibleColorMap[hitPixel];
        else
            indirect += 0.25 * colorBuckets[uint3(hitPixel, bindex)];
    }

    float4 direct = VisibleColorMap[uint2(pin.posH.xy)];
    indirect.w = direct.a + dot(n, t);

    float d = VisiblePositionMap[uint2(hitPixel.xy)].z;


    debug0[uint2(pin.posH.xy)] = float4(ori, 0.f);
    debug1[uint2(pin.posH.xy)] = float4(r, 0.f);
    debug2[uint2(pin.posH.xy)] = float4(normalize(pin.ray), 0);


//    return float4(hitPixel/float2(screenWidth,screenHeight),d/farZ,0.f); //+ indirect;
    return direct + indirect;
    
    
}