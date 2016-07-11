Texture2D rayTracingMap;
Texture2D visibleColorMap;
Texture2D visibleSpecularMap;
//Texture2D visibleNormalMap;
RWTexture2D<float4> debug0;
RWTexture2D<float4> debug1;
RWTexture2D<float4> debug2;
RWTexture2D<float4> debug3;

SamplerState sam;



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



float specularPower2ConeAngle(float sp)
{
//    if(sp>36.f)
//        return 0.f;

    float f = 1.f / (sp + 1.f);
    return acos(pow(0.244, f));
}

float calIsoscelesOpposite(float angle,float adjacent)
{
    return 2.0f * tan(angle) * adjacent;
}

float calInCircleRadius(float a,float h)
{
    float sq = sqrt(4 * h * h + a * a);
    return (a * sq - a * a) / (4.f * h);
}

float4 calDiffuseAndSpecular(float2 posSS,float4 insectInfo,float4 specular,float miplevel)
{

    
    //提取直接光照
    float4 direct = visibleColorMap.SampleLevel(sam, posSS, 0);

    //提取间接光照
    float4 indirect = insectInfo.z > 0.f ?
        visibleColorMap.SampleLevel(sam, insectInfo.xy, miplevel) : 0.f;
    direct.a = indirect.a;
    return direct + specular.r * indirect;


}

struct VertexOut
{
    float4 posH : SV_Position;
    float2 tex : TEXCOORD;
};

float4 main(VertexOut pin) : SV_TARGET
{
    //raytracing 信息
    float4 insectInfo = rayTracingMap[pin.posH.xy];
    
    float4 specular = visibleSpecularMap[pin.posH.xy];
    //根据specular power计算cone angle
    float coneAngle = specularPower2ConeAngle(specular.r) * 0.5f;
    //计算adjacentLength
    float adjacentLength = length(insectInfo.xy - pin.tex);
    //计算oppositeLength
    float oppositeLength = calIsoscelesOpposite(coneAngle, adjacentLength);
    //
    float inradiuse = calInCircleRadius(adjacentLength, oppositeLength);
    //
    float miplevel = clamp(log2(inradiuse * max(screenWidth, screenHeight)), 1.f, mipCount - 1);
    debug0[pin.posH.xy] = float4(inradiuse*max(screenWidth,screenHeight),adjacentLength,oppositeLength,0);

    return calDiffuseAndSpecular(pin.tex, insectInfo, specular, miplevel);
    
    
    

   
    
    /*
    if(insectInfo.w<=0.f)
        return direct;

    //fetch visible position from VisiblePositionMap
    float4 positionSS = float4(pin.tex, 0, 0);
    float4 positionVS = visiblePositionMap[pin.posH.xy];
    //fetch normal
    float3 normalVS = normalize(visibleNormalMap[pin.posH.xy].xyz);

    //命中点
    float4 hitPosSS = float4(insectInfo.xy, 0.f, 0.f);
    debug0[pin.posH.xy] = hitPosSS;

    float4 specular = 0.3f;
    float gloss = 1.f - specular.r;
    float roughness = 0.3f;

    float coneAngle = specularPower2ConeAngle(specular.a) * 0.5f;
    
    //calculate adjacentLength 
    float2 delta = insectInfo.xy - positionSS.xy;
    float2 adjUnit = normalize(delta);
    float adjacentLength = length(delta);
 

    float4 totalColor = 0.f;
     [unroll]
    for (int i = 0; i < 7;++i)
    {
        //cal opposite length
        float oppositeLength = calIsoscelesOpposite(coneAngle, adjacentLength);

        float inradius = calInCircleRadius(adjacentLength, oppositeLength);
        //Sample Postion
        float2 samPos = positionSS.xy+ adjUnit * (adjacentLength - inradius);
       
        //计算Mipmap
        //根据《GPU Pro 5》
        //将inradius转换至screen space  并对其求2的对数
        float miplevel = clamp(log2(inradius * max(screenWidth, screenHeight)), 0.f, mipCount - 1);
        debug1[pin.posH.xy] = float4(samPos, inradius, miplevel);
        totalColor += float4(visibleColorMap.SampleLevel(sam, samPos,1).xyz * gloss, gloss);

        if (totalColor.a >= 1.f)
        {
            totalColor.xyz = normalize(totalColor.xyz);
            break;
        }

        //计算next adjacentLenght
        adjacentLength = adjacentLength - 2.f * inradius;
        gloss *= gloss;
    }
    

   
    direct.w = insectInfo.a;
    return direct+totalColor ;
    */
}