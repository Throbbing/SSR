Texture2D VisiblePositionMap;
Texture2D VisibleNormalMap;
//RWTexture2D<float4> debug0;
//RWTexture2D<float4> debug1;
//RWTexture2D<float4> debug2;
//RWTexture2D<float4> debug3;
SamplerState samPoint;

//������Z Range�ֲ㣬ÿ��һ��RWTexture
//������Ӧ��Fragment




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
    float mipCount;
    float S;
};



bool intersectsDepthBuffer(float z, float minZ, float maxZ)
{

    
    float pixelZMax = z + 0.5f;
    float pixelZMin = z - 1.f;


    return (maxZ >= pixelZMin) && (minZ <= pixelZMax);
    
    
//    float depthScale = min(1.0f, z * 1);
//    z += 1 + lerp(0.0f, 2.0f, depthScale);
//    return (maxZ >= z) && (minZ - 1 <= z);
    
}

/*
//DDA����׷�ٺ���
bool traceRayInScreenSpace(in float3 oriV, in float3 dirV,
	out float2 hitPixel,out float4 result)
{
    hitPixel = float2(0.f, 0.0f);
    result = float4(0.f, 0.f, 0.f, 0.f);
    
    uint maxSteps = (uint) maxStep;
    float maxDis = -0.1f;
    
	//�жϹ��ߵĿɼ���
    
//	[unroll]
//    for (uint i = 0; i < 6; ++i)
//    {
//        float tt = (plane[i].w - dot(oriV, plane[i].xyz)) / dot(dirV, plane[i].xyz);
//        if (tt > maxDis)
//            maxDis = tt;
//    }
    
//    if (maxDis < 0.1f)
//        return false;
    

	//�����ߵ��յ��ԭ��ͶӰ��Sceen Space
    float3 rStartV = oriV + 10 * dirV;
    float3 rEndV = oriV + 500 * dirV;
    float4 rStartS = mul(float4(rStartV, 1.f), projTex);
    float4 rEndS = mul(float4(rEndV, 1.f), projTex);
    float invZStart = 1.f / rStartS.w;
    float invZEnd = 1.f / rEndS.w;

    rStartS /= rStartS.w;
    rEndS/= rEndS.w;
    //ת����screen space 
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

	//��Ļ�����
    float2 screenDelta = (rEndS.xy - rStartS.xy) + float2(0.01f, 0.01f);

	//DDA Step 1: �ж��Ƿ���Ҫ����xy
    bool exchange = false;
    if (abs(screenDelta.x) < abs(screenDelta.y))
    {
        exchange = true;
        screenDelta = screenDelta.yx;
        rStartS.xy = rStartS.yx;
        rEndS.xy = rEndS.yx;
    }

	//DDA Step 2: �������"X" ��Ŀ��
    float sig = sign(screenDelta.x);
    float invDx = sig / screenDelta.x;

	//�������б��

	//����DDA�㷨
	//d=(stepX,stepX*slope)
	//����stepX=1
    float2 dP = float2(sig, screenDelta.y * invDx);

	//���� 1/z��б�� ���Բ�ֵ
    float dinvZ = (invZEnd - invZStart) * invDx;

    //����stride�����п���������
    dP *=gstride;
    dinvZ*=gstride;

//    hitPixel.xy = screenDelta.xy;
//    hitPoint.xy = dP;
//    return false;

    //rayZMax,rayZMin������������ray�ڵ�ǰpixel���ж���ֵ
    float rayZMin = 0.f;
    float rayZMax = oriV.z;

    //ѭ������
    float invZ = invZStart;
    uint stepCount = 0;
    float pixelEndX = sig * rEndS.x;
    float2 pixel = rStartS.xy;

    //
   
    //DDA step 3�����߸���
    for (;
        (sig * pixel.x) < pixelEndX && (stepCount < maxSteps);
        pixel += dP, invZ += dinvZ, ++stepCount)
    {
        //��ǰ������������
        //��Ҫ���� exchange���б任
        hitPixel = exchange ? pixel.yx : pixel.xy;

        //����ray��z��Χ
        rayZMin = rayZMax;
        //ray ��ǰ��Խ������ؾ�
        rayZMax = 1.f / (invZ + 0.5f * dinvZ);

        if (rayZMax < rayZMin)
        {
            float t = rayZMax;
            rayZMax =rayZMin;
            rayZMin = t;
        }


        float pixelZ = VisiblePositionMap[uint2(hitPixel)].w;
        float pixelZMin = pixelZ - 1.5f;
        float pixelZMax = pixelZ + 1.5f;

        //�ж���ײ
        if((rayZMax>=pixelZMin)&&(rayZMin<=pixelZMax))
        {
 
            result = VisibleColorMap[uint2(hitPixel)];
            return true;
        }
        
        

    }
    return false;
	
}

//DDA����׷�ٺ���
//ͬʱ�������
float4 traceRay(in float3 oriV, in float3 dirV)
{
    float2 hitPixel;
    float4 result = float4(1.f, 1.f, 0.f, 0.f);

    traceRayInScreenSpace(oriV, dirV,
         hitPixel, result);

    return result;
   
}
*/



bool traceScreenSpaceRay(float2 oriPixel,
 float3 csOrig,
 float3 csDir,
 float jitter,
 out float2 hitPixel,
 out float3 hitPoint)
{
    //������ü�
    float rayLength = ((csOrig.z + csDir.z * 1000.f) < nearZ) ?
 (nearZ - csOrig.z) / csDir.z : 1000.f;
    float3 csEndPoint = csOrig + csDir * rayLength;
  
 

    // ͸��ͶӰ
    float4 H0 = mul(float4(csOrig, 1.0f), projTex);
    H0.xy *= float2(screenWidth,screenHeight);
    float4 H1 = mul(float4(csEndPoint, 1.0f), projTex);
    H1.xy *= float2(screenWidth,screenHeight);
    float k0 = 1.0f / H0.w;
    float k1 = 1.0f / H1.w;
    
    
 
    //���ռ���ת��Ϊƽ����
    float3 Q0 = csOrig * k0;
    float3 Q1 = csEndPoint * k1;
 
 
    float2 P0 = H0.xy * k0;
    float2 P1 = H1.xy * k1;
//    P0.x = clamp(P0.x, 0.f, screenWidth);
//    P1.x = clamp(P1.x, 0.f, screenWidth);
//    P0.y = clamp(P0.y, 0.f, screenHeight);
//    P1.y = clamp(P1.y, 0.f, screenHeight);
//    debug3[oriPixel] = float4(P0.xy, P1.xy);
 // If the line is degenerate, make it cover at least one pixel
 // to avoid handling zero-pixel extent as a special case later
    P1 += (distance(P0, P1) < 0.0001f) ? float2(0.001f, 0.001f) : 0.0f;
    float2 delta = P1 - P0;
 

    bool permute = false;
    if (abs(delta.x) < abs(delta.y))
    {

        permute = true;
        delta = delta.yx;
        P0 = P0.yx;
        P1 = P1.yx;
    }
 
    float stepDir = sign(delta.x);
    float invdx = stepDir / delta.x;
 
    float3 dQ = (Q1 - Q0) * invdx;
    float dk = (k1 - k0) * invdx;
    float2 dP = float2(stepDir, delta.y * invdx);
 


    dP *= gstride;
    dQ *= gstride;
    dk *= gstride;
 
    P0 += dP * jitter;
    Q0 += dQ * jitter;
    k0 += dk * jitter;
 
 
    float4 PQk = float4(P0, Q0.z, k0);
    float4 dPQk = float4(dP, dQ.z, dk);
    float3 Q = Q0;
 
 
    float end = P1.x * stepDir;
 
    float stepCount = 0.0f;
    float prevZMaxEstimate = csOrig.z;
    float rayZMin = prevZMaxEstimate;
    float rayZMax = prevZMaxEstimate;
    float sceneZMax = rayZMax + 100.0f;
    for (;
 ((PQk.x * stepDir) <= end) && (stepCount < maxStep) &&
 !intersectsDepthBuffer(sceneZMax, rayZMin, rayZMax)&&
 (sceneZMax != 0.0f);
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

        sceneZMax = VisiblePositionMap[uint2(hitPixel)].z;
 
        PQk += dPQk;
    }
 

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
	//�ɼ����ж�
    clip(VisibleNormalMap[uint2(pin.posH.xy)].w - 1.f);


    //�ɼ���ӹ���
    float3 ori = VisiblePositionMap[uint2(pin.posH.xy)].xyz;
    float3 n = VisibleNormalMap[uint2(pin.posH.xy)].xyz;
    float3 r = normalize(reflect(normalize(pin.ray), normalize(n)));
    float2 hitPixel = 0;
    float3 hitPoint = 0;
    
    bool insect = false;
    if (traceScreenSpaceRay(pin.posH.xy, ori + 20.f * r, r, 0.5f,
        hitPixel,hitPoint))
    {
        if (hitPixel.x <= screenWidth && hitPixel.x >= 0.0f &&
			hitPixel.y <= screenHeight && hitPixel.y >= 0.0f)
        {
            insect = true;
        }
    }

    float hitDepth = VisiblePositionMap[uint2(hitPixel.xy)].z;
    float rDotR = dot(normalize(pin.ray), r);

//    debug0[uint2(pin.posH.xy)] = float4(ori,0.f);
//    debug1[uint2(pin.posH.xy)] = float4(r, 0.f);
//    debug2[uint2(pin.posH.xy)] = float4(hitPixel.xy, 0, 0);


//    return float4(hitPixel/float2(screenWidth,screenHeight),d/farZ,0.f); //+ indirect;
    return float4(hitPixel / float2(screenWidth, screenHeight), hitDepth, rDotR) * (insect ? 1.f : 0.0f);
    
    
}
