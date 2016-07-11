#include"Sponza.h"
#define NN 5
#define SS 2

extern s32  gMouseX;
extern s32  gMouseY;
extern s32  gLastMouseX;
extern s32  gLastMouseY;
extern bool gMouseDown;
extern bool gMouseMove;
extern HWND  gHwnd;
using namespace jmxRCore;

struct cbTrans
{
	XMFLOAT4X4 mat0;
	XMFLOAT4X4 mat1;
};

struct cbView
{
	XMFLOAT4X4 view;
};

struct cbToFarPlane
{
	XMFLOAT4 farPlaneCorner[4];
};

struct cbTraceInfo
{
	f32 stride;
	f32 maxStep;
	f32 nearZ;
	f32 farZ;
	f32 screenWidth;
	f32 screenHeight;
	f32 mipCount;
	f32 S;
};

struct cbBlur
{
	int level;
};

struct cbMaterial
{
	XMFLOAT4 specular;
};

void SponzaDemo::init()
{
	DirectXDemo::init();

//	std::vector<string> meshName = { "Mesh/BoxObj.obj", "Mesh/plane.obj" };
	std::vector<string> meshName = { "Mesh/sponza.obj" };
//	std::vector<string> meshName = { "Mesh/city.obj","Mesh/teapot.obj" };
	mMeshsWorld.resize(meshName.size());
//	XMStoreFloat4x4(&mMeshsWorld[0], XMMatrixRotationX(XM_PI/2)*XMMatrixTranslation(0, 0, 0));
	XMStoreFloat4x4(&mMeshsWorld[0], XMMatrixIdentity());
//	XMStoreFloat4x4(&mMeshsWorld[1], XMMatrixScaling(30, 30, 30)*XMMatrixTranslation(300, 100, -150));

	
	for (int i = 0; i < meshName.size(); ++i)
	{

		std::vector<tinyobj::shape_t> shapes;
		std::vector<tinyobj::material_t> materials;
		std::string err;
		tinyobj::LoadObj(shapes, materials, err, meshName[i].c_str(), "Mesh/mtl/", true);
		if (!err.empty())
			cout << err << std::endl;
		mMeshs.push_back(new D3DMesh(OBJMesh(shapes, materials), 
			BBox(), XMFLOAT4X4(), mgr, md3dDevice));
		mMeshs[i]->genTangent(md3dDevice);
	}
	D3D11_INPUT_ELEMENT_DESC input[]=
	{
		{ "POSITION",0,DXGI_FORMAT_R32G32B32_FLOAT,0,0,D3D11_INPUT_PER_VERTEX_DATA,0 },
		{ "NORMAL",0,DXGI_FORMAT_R32G32B32_FLOAT,0,12,D3D11_INPUT_PER_VERTEX_DATA,0 },
		{ "TEXCOORD",0,DXGI_FORMAT_R32G32_FLOAT,0,24,D3D11_INPUT_PER_VERTEX_DATA,0 },
		{ "TANGENT" ,0,DXGI_FORMAT_R32G32B32_FLOAT,0,32,D3D11_INPUT_PER_VERTEX_DATA,0}
	};
	mVS = (ID3D11VertexShader*)createShaderAndLayout(L"SponzaVS.hlsl",
		nullptr, nullptr,"main" ,
		"vs_5_0", EVs, md3dDevice,
		input, 4, &mLayout);
	mPS = (ID3D11PixelShader*)createShaderAndLayout(L"SponzaPS.hlsl",
		nullptr, nullptr, "main",
		"ps_5_0", EPs, md3dDevice);
	mPSN = (ID3D11PixelShader*)createShaderAndLayout(L"SponzaPSN.hlsl",
		nullptr, nullptr, "main",
		"ps_5_0", EPs, md3dDevice);

	mPSA = (ID3D11PixelShader*)createShaderAndLayout(L"SponzaPSA.hlsl",
		nullptr, nullptr, "main",
		"ps_5_0", EPs, md3dDevice);

	mPSNA = (ID3D11PixelShader*)createShaderAndLayout(L"SponzaPSNA.hlsl",
		nullptr, nullptr, "main",
		"ps_5_0", EPs, md3dDevice);



	mCBWVP = createConstantBuffer(sizeof(XMFLOAT4X4), md3dDevice);
	mCBCamera = createConstantBuffer(PAD16(sizeof(XMFLOAT3)), md3dDevice);


	

	mCamera.setLens(PI / 2, f32(mWidth) / f32(mHeight), 0.1f, 3000.f);
	mCamera.setPos(XMFLOAT3(0, 0, -40));


	D3D11_SAMPLER_DESC sd;
	sd.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	sd.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	sd.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	sd.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	sd.MaxAnisotropy = 16;
	sd.ComparisonFunc = D3D11_COMPARISON_NEVER;
	sd.MipLODBias = 0;
	sd.MinLOD = -FLT_MAX;
	sd.MaxLOD = FLT_MAX;
	HR(md3dDevice->CreateSamplerState(&sd, &mSampler));

	sd.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;
	HR(md3dDevice->CreateSamplerState(&sd, &mSamPoint));


	createQuad();
	createDDA();
	createDDAView();
}
int meshIndex = 0;
void SponzaDemo::update(f32 dt)
{
	DirectXDemo::update(dt);
	/*
	XMFLOAT3 cameraPos = mCamera.getPos();
	XMFLOAT3 look = mCamera.getLook();
	XMFLOAT3 up = mCamera.getUp();
	std::wostringstream ss;
	ss << "Pos: " << cameraPos.x << " " << cameraPos.y << " " << cameraPos.z << " " <<
		"look" << look.x << " " << look.y << " " << look.z <<
		"up " << up.x << " " << up.y << " " << up.z;
	SetWindowText(mHwnd, ss.str().c_str());
	*/
	static float speed = 200.0f;
	if (GetAsyncKeyState('W') & 0x8000)
		mCamera.walk(speed*dt);

	if (GetAsyncKeyState('S') & 0x8000)
		mCamera.walk(-speed*dt);

	if (GetAsyncKeyState('A') & 0x8000)
		mCamera.strafe(-speed*dt);

	if (GetAsyncKeyState('D') & 0x8000)
		mCamera.strafe(speed*dt);



	if (gMouseMove)
	{
		float dx = XMConvertToRadians(0.25f*static_cast<float>(gMouseX - gLastMouseX));
		float dy = XMConvertToRadians(0.25f*static_cast<float>(gMouseY - gLastMouseY));

		mCamera.pitch(dy);
		mCamera.rotateY(dx);


		gLastMouseX = gMouseX;
		gLastMouseY = gMouseY;

		gMouseMove = false;

	}

	mCamera.updateMatrix();
}

void SponzaDemo::run()
{
	
//	rasterizer();
	rayCasting();
//	ssr();
}


void SponzaDemo::createQuad()
{
	QuadVertex vertices[4];
	vertices[0].posH = XMFLOAT3(-1.f, -1.f, 0.f);
	vertices[0].tex = XMFLOAT3(0.f, 1.f, 0);

	vertices[1].posH = XMFLOAT3(-1.f, 1.f, 0.f);
	vertices[1].tex = XMFLOAT3(0.f, 0.f, 1);

	vertices[2].posH = XMFLOAT3(1.f, 1.f, 0.f);
	vertices[2].tex = XMFLOAT3(1.f, 0.f, 2);

	vertices[3].posH = XMFLOAT3(1.f, -1.f, 0.f);
	vertices[3].tex = XMFLOAT3(1.f, 1.f, 3);


	u32 indices[6] =
	{
		0, 1, 2,
		0, 2, 3
	};

	mQuadVB = createBuffer(D3D11_USAGE_DEFAULT, D3D11_BIND_VERTEX_BUFFER,
		4 * sizeof(QuadVertex), 0, 0, 0, &vertices[0], md3dDevice);
	mQuadIB = createBuffer(D3D11_USAGE_DEFAULT, D3D11_BIND_INDEX_BUFFER,
		6 * sizeof(u32), 0, 0, 0, &indices[0], md3dDevice);
}

void SponzaDemo::createDDA()
{
	D3D11_INPUT_ELEMENT_DESC input[]=
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 }
	};
	mDDAVS = (ID3D11VertexShader*)createShaderAndLayout(L"HLSL/DDA/DDAVS.hlsl",
		nullptr, nullptr,
		"main", "vs_5_0", EVs,
		md3dDevice,
		input, 2, &mQuadLayout);
	mDDAPS = (ID3D11PixelShader*)createShaderAndLayout(L"HLSL/DDA/DDAPS.hlsl",
		nullptr, nullptr,
		"main", "ps_5_0", EPs,
		md3dDevice);


	mBlurVS = (ID3D11VertexShader*)createShaderAndLayout(L"HLSL/DDA/BlurVS.hlsl",
		nullptr, nullptr,
		"main", "vs_5_0", EVs,
		md3dDevice);
	mBlurPSVertical = (ID3D11PixelShader*)createShaderAndLayout(L"HLSL/DDA/BlurPSVertical.hlsl",
		nullptr, nullptr,
		"main", "ps_5_0", EPs,
		md3dDevice);
	mBlurPSHorizontal = (ID3D11PixelShader*)createShaderAndLayout(L"HLSL/DDA/BlurPSHorizontal.hlsl",
		nullptr, nullptr,
		"main", "ps_5_0", EPs,
		md3dDevice);

	mConeTracingVS = (ID3D11VertexShader*)createShaderAndLayout(L"HLSL/DDA/ConeTracingVS.hlsl",
		nullptr, nullptr,
		"main", "vs_5_0", EVs,
		md3dDevice);
	mConeTracingPS = (ID3D11PixelShader*)createShaderAndLayout(L"HLSL/DDA/ConeTracingPS.hlsl",
		nullptr, nullptr,
		"main", "ps_5_0", EPs,
		md3dDevice);


	mABufferVS = (ID3D11VertexShader*)createShaderAndLayout(L"HLSL/DDA/ABufferVS.hlsl",
		nullptr, nullptr,
		"main", "vs_5_0", EVs,
		md3dDevice);;
	mABufferPS = (ID3D11PixelShader*)createShaderAndLayout(L"HLSL/DDA/ABufferPS.hlsl",
		nullptr, nullptr,
		"main", "ps_5_0",EPs,
		md3dDevice);
	mABufferPSN = (ID3D11PixelShader*)createShaderAndLayout(L"HLSL/DDA/ABufferPSN.hlsl",
		nullptr, nullptr,
		"main", "ps_5_0", EPs,
		md3dDevice);
	mABufferPSA = (ID3D11PixelShader*)createShaderAndLayout(L"HLSL/DDA/ABufferPSA.hlsl",
		nullptr, nullptr,
		"main", "ps_5_0", EPs,
		md3dDevice);
	mABufferPSNA = (ID3D11PixelShader*)createShaderAndLayout(L"HLSL/DDA/ABufferPSNA.hlsl",
		nullptr, nullptr,
		"main", "ps_5_0", EPs,
		md3dDevice);

	
}

void SponzaDemo::createDDAView()
{
	//创建Visible View
	ID3D11Texture2D* tex;
	tex = createTex2D(D3D11_USAGE_DEFAULT,
		D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE, mWidth, mHeight,
		DXGI_FORMAT_R32G32B32A32_FLOAT,
		1, 1, 0, 0, 1, 0,
		nullptr, md3dDevice);
	mVisiblePositionRTV = createRTV(tex, DXGI_FORMAT_R32G32B32A32_FLOAT,
		D3D11_RTV_DIMENSION_TEXTURE2D,
		0, 0, 0,
		md3dDevice);
	mVisiblePositionSRV = createSRV(tex, DXGI_FORMAT_R32G32B32A32_FLOAT,
		D3D11_SRV_DIMENSION_TEXTURE2D,
		1, 0, 0,
		md3dDevice);
	tex->Release();

	tex = createTex2D(D3D11_USAGE_DEFAULT,
		D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE, mWidth, mHeight,
		DXGI_FORMAT_R32G32B32A32_FLOAT,
		1, 1, 0, 0, 1, 0,
		nullptr, md3dDevice);
	mVisibleNormalRTV = createRTV(tex, DXGI_FORMAT_R32G32B32A32_FLOAT,
		D3D11_RTV_DIMENSION_TEXTURE2D,
		0, 0, 0,
		md3dDevice);
	mVisibleNormalSRV = createSRV(tex, DXGI_FORMAT_R32G32B32A32_FLOAT,
		D3D11_SRV_DIMENSION_TEXTURE2D,
		1, 0, 0,
		md3dDevice);
	ReleaseCom(tex);

	tex = createTex2D(D3D11_USAGE_DEFAULT,
		D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE, mWidth, mHeight,
		DXGI_FORMAT_R32G32B32A32_FLOAT,
		1, 1, 0, 0, 1, 0,
		nullptr, md3dDevice);
	mVisibleSpecularRTV = createRTV(tex, DXGI_FORMAT_R32G32B32A32_FLOAT,
		D3D11_RTV_DIMENSION_TEXTURE2D,
		0, 0, 0,
		md3dDevice);
	mVisibleSpecularSRV = createSRV(tex, DXGI_FORMAT_R32G32B32A32_FLOAT,
		D3D11_SRV_DIMENSION_TEXTURE2D,
		1, 0, 0,
		md3dDevice);
	ReleaseCom(tex);

	/*
	tex = createTex2D(D3D11_USAGE_DEFAULT,
		D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE, mWidth, mHeight,
		DXGI_FORMAT_R32G32B32A32_FLOAT,
		1, 1, 0, 0, 1, 0,
		nullptr, md3dDevice);
	mVisibleTangentRTV = createRTV(tex, DXGI_FORMAT_R32G32B32A32_FLOAT,
		D3D11_RTV_DIMENSION_TEXTURE2D,
		0, 0, 0,
		md3dDevice);
	mVisibleTangentSRV = createSRV(tex, DXGI_FORMAT_R32G32B32A32_FLOAT,
		D3D11_SRV_DIMENSION_TEXTURE2D,
		1, 0, 0,
		md3dDevice);
	tex->Release();
	*/

	
	tex = createTex2D(D3D11_USAGE_DEFAULT,
		D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE, mWidth, mHeight,
		DXGI_FORMAT_R32G32B32A32_FLOAT,
		BlurLevel,1,
		D3D11_RESOURCE_MISC_GENERATE_MIPS,0,
		1,0,
		nullptr, md3dDevice);
	mVisibleColorSRV = createSRV(tex, DXGI_FORMAT_R32G32B32A32_FLOAT,
		D3D11_SRV_DIMENSION_TEXTURE2D,
		BlurLevel, 0, 0,
		md3dDevice);
	md3dImmediateContext->GenerateMips(mVisibleColorSRV);
	for (u32 i = 0; i < mVisibleColorMipRTV.size(); ++i)
	{
		mVisibleColorMipRTV[i] = createRTV(tex, DXGI_FORMAT_R32G32B32A32_FLOAT,
			D3D11_RTV_DIMENSION_TEXTURE2D,
			i, 0, 0,
			md3dDevice);
	}
	ReleaseCom(tex);


	tex = createTex2D(D3D11_USAGE_DEFAULT,
		D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE, mWidth, mHeight,
		DXGI_FORMAT_R32G32B32A32_FLOAT,
		1, 1, 0, 0, 1, 0,
		nullptr, md3dDevice);
	mRayTracingBufferRTV = createRTV(tex, DXGI_FORMAT_R32G32B32A32_FLOAT,
		D3D11_RTV_DIMENSION_TEXTURE2D,
		0, 0, 0,
		md3dDevice);
	mRayTracingBufferSRV = createSRV(tex, DXGI_FORMAT_R32G32B32A32_FLOAT,
		D3D11_SRV_DIMENSION_TEXTURE2D,
		1, 0, 0,
		md3dDevice);
	ReleaseCom(tex);

	u32 w = mWidth, h = mHeight;
	for (u32 i = 0; i < BlurLevel - 1; ++i)
	{

		tex = createTex2D(D3D11_USAGE_DEFAULT,
			D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE, w, h,
			DXGI_FORMAT_R32G32B32A32_FLOAT,
			1, 1, 0, 0, 1, 0,
			nullptr, md3dDevice);
		mTemporalRTV[i] = createRTV(tex, DXGI_FORMAT_R32G32B32A32_FLOAT,
			D3D11_RTV_DIMENSION_TEXTURE2D,
			0, 0, 0,
			md3dDevice);
		mTemporalSRV[i] = createSRV(tex, DXGI_FORMAT_R32G32B32A32_FLOAT,
			D3D11_SRV_DIMENSION_TEXTURE2D,
			1, 0, 0,
			md3dDevice);
		ReleaseCom(tex);

		w /= 2;
		h /= 2;
	}

	//创建Buckets和Counters
	/*
	u32 bucketsCount = u32(std::log2f(mCamera.getFarZ() - mCamera.getNearZ())) + 1;
//	u32 bucketsCount = mCamera.getFarZ() / 200.f;

	ID3D11Texture2D* buckets;
	buckets = createTex2D(D3D11_USAGE_DEFAULT,
		D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_UNORDERED_ACCESS,
		mWidth, mHeight, DXGI_FORMAT_R32G32B32A32_FLOAT,
		1, bucketsCount,
		0, 0, 1, 0, nullptr,
		md3dDevice);
	mNormalBucketsUAV = createUAV(buckets, DXGI_FORMAT_R32G32B32A32_FLOAT,
		D3D11_UAV_DIMENSION_TEXTURE2DARRAY,
		0, 0, bucketsCount,
		md3dDevice);
	mNormalBucketsSRV = createSRV(buckets, DXGI_FORMAT_R32G32B32A32_FLOAT,
		D3D11_SRV_DIMENSION_TEXTURE2DARRAY,
		bucketsCount, 0, 1,
		md3dDevice);
	buckets->Release();

	buckets = createTex2D(D3D11_USAGE_DEFAULT,
		D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_UNORDERED_ACCESS,
		mWidth, mHeight, DXGI_FORMAT_R32G32B32A32_FLOAT,
		1, bucketsCount,
		0, 0, 1, 0, nullptr,
		md3dDevice);
	mColorBucketsUAV = createUAV(buckets, DXGI_FORMAT_R32G32B32A32_FLOAT,
		D3D11_UAV_DIMENSION_TEXTURE2DARRAY,
		0, 0, bucketsCount,
		md3dDevice);
	mColorBucketsSRV = createSRV(buckets, DXGI_FORMAT_R32G32B32A32_FLOAT,
		D3D11_SRV_DIMENSION_TEXTURE2DARRAY,
		bucketsCount, 0, 1,
		md3dDevice);
	buckets->Release();
	
	*/
		



	mCBTrans = createConstantBuffer(PAD16(sizeof(cbTrans)), md3dDevice);
	mCBTrans2 = createConstantBuffer(PAD16(sizeof(cbTrans)), md3dDevice);
	mCBView = createConstantBuffer(PAD16(sizeof(cbView)), md3dDevice);
	mCBToFarPlane = createConstantBuffer(PAD16(sizeof(cbToFarPlane)), md3dDevice);
	mCBTraceInfo = createConstantBuffer(PAD16(sizeof(cbTraceInfo)), md3dDevice);
	mCBBlur = createConstantBuffer(PAD16(sizeof(cbBlur)), md3dDevice);
	mCBMaterial = createConstantBuffer(PAD16(sizeof(cbMaterial)), md3dDevice);


	//计算Far Plane Corner
	XMFLOAT4 farCorner[4];
	f32 halfWidth = 0.5f*mCamera.getFarWindowWidth();
	f32 halfHeight = 0.5f*mCamera.getFarWindowHeight();
	f32 farZ = mCamera.getFarZ();
	farCorner[0] = XMFLOAT4(-halfWidth, -halfHeight, farZ, 0.f);
	farCorner[1] = XMFLOAT4(-halfWidth, +halfHeight, farZ, 0.f);
	farCorner[2] = XMFLOAT4(+halfWidth, +halfHeight, farZ, 0.f);
	farCorner[3] = XMFLOAT4(+halfWidth, -halfHeight, farZ, 0.f);

	D3D11_MAPPED_SUBRESOURCE ms;
	md3dImmediateContext->Map(mCBToFarPlane, 0, D3D11_MAP_WRITE_DISCARD, 0, &ms);
	cbToFarPlane* fp = (cbToFarPlane*)ms.pData;
	fp->farPlaneCorner[0] = farCorner[0];
	fp->farPlaneCorner[1] = farCorner[1];
	fp->farPlaneCorner[2] = farCorner[2];
	fp->farPlaneCorner[3] = farCorner[3];
	md3dImmediateContext->Unmap(mCBToFarPlane, 0);

	//保存TraceInfo
	ZeroMemory(&ms, sizeof(ms));
	md3dImmediateContext->Map(mCBTraceInfo, 0, D3D11_MAP_WRITE_DISCARD, 0, &ms);
	cbTraceInfo* ti = (cbTraceInfo*)ms.pData;
	ti->stride = 1.f;
	ti->maxStep = 2000.f;
	ti->nearZ = mCamera.getNearZ();
	ti->farZ = mCamera.getFarZ();
	ti->screenWidth = mWidth;
	ti->screenHeight = mHeight;
	ti->mipCount = BlurLevel;
	ti->S = SS;
	md3dImmediateContext->Unmap(mCBTraceInfo, 0);



	//Debuger
	ID3D11Texture2D* tt = createTex2D(D3D11_USAGE_DEFAULT,
		D3D11_BIND_UNORDERED_ACCESS|D3D11_BIND_SHADER_RESOURCE, mWidth, mHeight,
		DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 1,
		0, 0, 1, 0, nullptr, md3dDevice);
	mDebugger0 = createUAV(tt, DXGI_FORMAT_R32G32B32A32_FLOAT,
		D3D11_UAV_DIMENSION_TEXTURE2D,
		mWidth*mHeight, 0, 0,
		md3dDevice);
	mDebugger0SRV = createSRV(tt, DXGI_FORMAT_R32G32B32A32_FLOAT,
		D3D11_SRV_DIMENSION_TEXTURE2D,
		1, 0, 0,
		md3dDevice);
	
	ReleaseCom(tt);
	

	tt = createTex2D(D3D11_USAGE_DEFAULT,
		D3D11_BIND_UNORDERED_ACCESS | D3D11_BIND_SHADER_RESOURCE, mWidth, mHeight,
		DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 1,
		0, 0, 1, 0, nullptr, md3dDevice);
	mDebugger1 = createUAV(tt, DXGI_FORMAT_R32G32B32A32_FLOAT,
		D3D11_UAV_DIMENSION_TEXTURE2D,
		mWidth*mHeight, 0, 0,
		md3dDevice);
	mDebugger1SRV = createSRV(tt, DXGI_FORMAT_R32G32B32A32_FLOAT,
		D3D11_SRV_DIMENSION_TEXTURE2D,
		1, 0, 0,
		md3dDevice);
	ReleaseCom(tt);

	tt = createTex2D(D3D11_USAGE_DEFAULT,
		D3D11_BIND_UNORDERED_ACCESS | D3D11_BIND_SHADER_RESOURCE, mWidth, mHeight,
		DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 1,
		0, 0, 1, 0, nullptr, md3dDevice);
	mDebugger2 = createUAV(tt, DXGI_FORMAT_R32G32B32A32_FLOAT,
		D3D11_UAV_DIMENSION_TEXTURE2D,
		mWidth*mHeight, 0, 0,
		md3dDevice);
	mDebugger2SRV = createSRV(tt, DXGI_FORMAT_R32G32B32A32_FLOAT,
		D3D11_SRV_DIMENSION_TEXTURE2D,
		1, 0, 0,
		md3dDevice);
	ReleaseCom(tt);

	tt = createTex2D(D3D11_USAGE_DEFAULT,
		D3D11_BIND_UNORDERED_ACCESS | D3D11_BIND_SHADER_RESOURCE, mWidth, mHeight,
		DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 1,
		0, 0, 1, 0, nullptr, md3dDevice);
	mDebugger3 = createUAV(tt, DXGI_FORMAT_R32G32B32A32_FLOAT,
		D3D11_UAV_DIMENSION_TEXTURE2D,
		mWidth*mHeight, 0, 0,
		md3dDevice);
	mDebugger3SRV = createSRV(tt, DXGI_FORMAT_R32G32B32A32_FLOAT,
		D3D11_SRV_DIMENSION_TEXTURE2D,
		1, 0, 0,
		md3dDevice);
	ReleaseCom(tt);
}

void SponzaDemo::createBucketsView()
{
	
	
}

void SponzaDemo::rasterizer()
{
	
	f32 silver[4] = { 0.5f, 0.5f, 0.5f, 0.5f };
	md3dImmediateContext->ClearRenderTargetView(mRenderTargetView, silver);
	md3dImmediateContext->ClearDepthStencilView(mDepthStencilView,
		D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.f, 0);

	md3dImmediateContext->OMSetRenderTargets(1, &mRenderTargetView,
		mDepthStencilView);
//	md3dImmediateContext->RSSetState(mRSNoCull);


	md3dImmediateContext->IASetInputLayout(mLayout);
	md3dImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	u32 stride = sizeof(D3DMesh::D3DVertex);
	u32 offset = 0;

	int wi = 0;
	for (auto& mesh : mMeshs)
	{
		

//		XMMATRIX world = XMMatrixIdentity();
		XMMATRIX world = XMLoadFloat4x4(&mMeshsWorld[wi]); ++wi;
		XMMATRIX vp = mCamera.getViewProj();
		XMMATRIX wvp = world*vp;


		md3dImmediateContext->IASetVertexBuffers(0, 1, &mesh->vb, &stride, &offset);
		md3dImmediateContext->IASetIndexBuffer(mesh->ib, DXGI_FORMAT_R32_UINT, 0);


		D3D11_MAPPED_SUBRESOURCE ms;
		md3dImmediateContext->Map(mCBTrans, 0, D3D11_MAP_WRITE_DISCARD, 0, &ms);
		cbTrans* p = (cbTrans*)ms.pData;
		XMStoreFloat4x4(&p->mat0, XMMatrixTranspose(wvp));
		XMStoreFloat4x4(&p->mat1, XMMatrixTranspose(world));
		md3dImmediateContext->Unmap(mCBTrans, 0);

		md3dImmediateContext->Map(mCBCamera, 0, D3D11_MAP_WRITE_DISCARD, 0, &ms);
		XMFLOAT3* po = (XMFLOAT3*)ms.pData;
		*po = mCamera.getPos();
		md3dImmediateContext->Unmap(mCBCamera, 0);

		md3dImmediateContext->VSSetShader(mVS, nullptr, 0);

		md3dImmediateContext->PSSetSamplers(0, 1, &mSampler);
		md3dImmediateContext->VSSetConstantBuffers(0, 1, &mCBTrans);
		md3dImmediateContext->PSSetConstantBuffers(0, 1, &mCBCamera);
		for (u32 m = 0; m < mesh->subMeshs.size(); ++m)
		{
//			if (m != meshIndex)
//				continue;
			ID3D11ShaderResourceView* srv[] =
			{
				mesh->materials[mesh->subMeshs[m].matID].diffuseMap,
				mesh->materials[mesh->subMeshs[m].matID].bumpMap,
				mesh->materials[mesh->subMeshs[m].matID].alphaMap
			};
			md3dImmediateContext->PSSetShaderResources(0, 3,
				srv);

			//		if (mSponza->subMeshs[m].matID != 6)
			//			continue;

			if (mesh->materials[mesh->subMeshs[m].matID].bumpMap)
			{
				if (mesh->materials[mesh->subMeshs[m].matID].alphaMap)
				{
					//	cout << 1;
					md3dImmediateContext->PSSetShader(mPSNA, nullptr, 0);
				}
				else
				{
					//	cout << 2;
					md3dImmediateContext->PSSetShader(mPSN, nullptr, 0);
				}

			}
			else if (mesh->materials[mesh->subMeshs[m].matID].alphaMap)
			{
				md3dImmediateContext->PSSetShader(mPSA, nullptr, 0);
			}
			else
			{
				md3dImmediateContext->PSSetShader(mPS, nullptr, 0);
			}

			md3dImmediateContext->DrawIndexed(mesh->subMeshs[m].idxCount,
				mesh->subMeshs[m].idxOffset,
				mesh->subMeshs[m].vdxOffset);
		}
	}
	md3dImmediateContext->RSSetState(nullptr);
	mSwapChain->Present(0, 0);
}
void SponzaDemo::rayCasting()
{
	
	unbind();
	XMMATRIX toTex = { 0.5f, 0.0f, 0.0f, 0.0f,
		0.0f, -0.5f, 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f, 0.0f,
		0.5f, 0.5f, 0.0f, 1.0f
	};
	
	//清空RTV
	f32 black[4] = { 0.f };
	f32 white[4] = { 1.f };
	md3dImmediateContext->ClearRenderTargetView(mRenderTargetView, black);
	md3dImmediateContext->ClearRenderTargetView(mVisiblePositionRTV, black);
	md3dImmediateContext->ClearRenderTargetView(mVisibleNormalRTV, black);
	md3dImmediateContext->ClearRenderTargetView(mVisibleSpecularRTV, black);
	for (auto& mip : mVisibleColorMipRTV)
	{
		md3dImmediateContext->ClearRenderTargetView(mip, black);
	}
	md3dImmediateContext->ClearDepthStencilView(mDepthStencilView,
		D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL,
		1.f, 0);

	//清空Bucket
//	md3dImmediateContext->ClearUnorderedAccessViewFloat(mNormalBucketsUAV, black);
//	md3dImmediateContext->ClearUnorderedAccessViewFloat(mColorBucketsUAV, black);
	//设置RTV和SRV
	ID3D11RenderTargetView* rtvs[] = { mVisiblePositionRTV, mVisibleNormalRTV,
		 mVisibleColorMipRTV[0],mVisibleSpecularRTV };
//	ID3D11UnorderedAccessView* uavs[] = { mNormalBucketsUAV,mColorBucketsUAV };
//	md3dImmediateContext->OMSetRenderTargetsAndUnorderedAccessViews(
//		4,rtvs,mDepthStencilView,4,2,uavs,nullptr);
	md3dImmediateContext->OMSetRenderTargets(4, rtvs, mDepthStencilView);

	//更新矩阵
	XMMATRIX world; 
	XMMATRIX view = mCamera.getView();
	XMMATRIX proj = mCamera.getProj();
	XMMATRIX wvp;
	u32 stride;
	u32 offset;
	//Step 1 创建Bucket 正常渲染
	int wi = 0;
	for (auto& mesh : mMeshs)
	{
		md3dImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		md3dImmediateContext->IASetInputLayout(mLayout);
		stride = sizeof(D3DMesh::D3DVertex);
		offset = 0;
		md3dImmediateContext->IASetVertexBuffers(0, 1, &mesh->vb, &stride, &offset);
		md3dImmediateContext->IASetIndexBuffer(mesh->ib, DXGI_FORMAT_R32_UINT, 0);



		//VS :cbTrans
		world = XMLoadFloat4x4(&mMeshsWorld[wi]); ++wi;
		wvp = world*view*proj;
		D3D11_MAPPED_SUBRESOURCE ms;
		md3dImmediateContext->Map(mCBTrans, 0, D3D11_MAP_WRITE_DISCARD, 0, &ms);
		cbTrans* trans = (cbTrans*)ms.pData;
		XMStoreFloat4x4(&trans->mat1, XMMatrixTranspose(world*view));
		XMStoreFloat4x4(&trans->mat0, XMMatrixTranspose(wvp));
		md3dImmediateContext->Unmap(mCBTrans, 0);

		//PS :cbView
		md3dImmediateContext->Map(mCBView, 0, D3D11_MAP_WRITE_DISCARD, 0, &ms);
		cbView* v = (cbView*)ms.pData;
		XMStoreFloat4x4(&v->view, XMMatrixTranspose(view));
		md3dImmediateContext->Unmap(mCBView, 0);

		//设置Constant Buffer
		ID3D11Buffer* bufs[4] = { mCBMaterial,mCBView, nullptr };
		md3dImmediateContext->VSSetConstantBuffers(0, 1, &mCBTrans);
//		md3dImmediateContext->PSSetConstantBuffers(0, 2, bufs);

		//设置VertexShader
		md3dImmediateContext->VSSetShader(mABufferVS, nullptr, 0);


		//设置SamplerState
		md3dImmediateContext->PSSetSamplers(0, 1, &mSampler);


		//渲染Sponza
		for (u32 m = 0; m < mesh->subMeshs.size(); ++m)
		{
			md3dImmediateContext->Map(mCBMaterial, 0, D3D11_MAP_WRITE_DISCARD, 0, &ms); 
			cbMaterial* material = (cbMaterial*)ms.pData;
			material->specular = mesh->materials[mesh->subMeshs[m].matID].specular;
			md3dImmediateContext->Unmap(mCBMaterial, 0);
			md3dImmediateContext->PSSetConstantBuffers(0, 2, bufs); 

			ID3D11ShaderResourceView* srv[] =
			{
				mesh->materials[mesh->subMeshs[m].matID].diffuseMap,
				mesh->materials[mesh->subMeshs[m].matID].bumpMap,
				mesh->materials[mesh->subMeshs[m].matID].alphaMap
			};
			md3dImmediateContext->PSSetShaderResources(0, 3,
				srv);


			if (mesh->materials[mesh->subMeshs[m].matID].bumpMap)
			{
				if (mesh->materials[mesh->subMeshs[m].matID].alphaMap)
				{
					//	cout << 1;
					md3dImmediateContext->PSSetShader(mABufferPSNA, nullptr, 0);
				}
				else
				{
					//	cout << 2;
					md3dImmediateContext->PSSetShader(mABufferPSN, nullptr, 0);
				}

			}
			else if (mesh->materials[mesh->subMeshs[m].matID].alphaMap)
			{
				md3dImmediateContext->PSSetShader(mABufferPSA, nullptr, 0);
			}
			else
			{
				md3dImmediateContext->PSSetShader(mABufferPS, nullptr, 0);
			}

			md3dImmediateContext->DrawIndexed(mesh->subMeshs[m].idxCount,
				mesh->subMeshs[m].idxOffset,
				mesh->subMeshs[m].vdxOffset);



		}
	}
	unbind();
	//Step3 利用DDA算法，进行光线跟踪

	
	//设置RTV 
//	ID3D11UnorderedAccessView* debugs[] = { mDebugger0, mDebugger1, mDebugger2, mDebugger3 };
	md3dImmediateContext->ClearRenderTargetView(mRayTracingBufferRTV, black);
	md3dImmediateContext->ClearDepthStencilView(mDepthStencilView,
		D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL,
		1.f, 0.f);

//	md3dImmediateContext->OMSetRenderTargetsAndUnorderedAccessViews(1,
//		&mRenderTargetView, mDepthStencilView,
//		1,4,debugs,nullptr);
	md3dImmediateContext->OMSetRenderTargets(1, &mRayTracingBufferRTV, mDepthStencilView);

	//设置Quad的Layout ,VB,IB
	md3dImmediateContext->IASetInputLayout(mQuadLayout);
	stride = sizeof(QuadVertex);
	offset = 0;
	md3dImmediateContext->IASetVertexBuffers(0, 1, &mQuadVB, &stride, &offset);
	md3dImmediateContext->IASetIndexBuffer(mQuadIB, DXGI_FORMAT_R32_UINT, 0);


	//更新矩阵
	//PS cbTrans
	D3D11_MAPPED_SUBRESOURCE ms;
	md3dImmediateContext->Map(mCBTrans, 0, D3D11_MAP_WRITE_DISCARD, 0, &ms);
	cbTrans* trans = (cbTrans*)ms.pData;
	XMStoreFloat4x4(&trans->mat0, XMMatrixTranspose(view));
	XMStoreFloat4x4(&trans->mat1, XMMatrixTranspose(proj*toTex));
	md3dImmediateContext->Unmap(mCBTrans, 0);

	//设置Constant Buffer
	md3dImmediateContext->VSSetConstantBuffers(0, 1, &mCBToFarPlane);
	ID3D11Buffer* bufss[] = { mCBTrans, mCBTraceInfo };
	md3dImmediateContext->PSSetConstantBuffers(0, 2, bufss);

	//设置SRV
	ID3D11ShaderResourceView* srvs[] = { mVisiblePositionSRV, mVisibleNormalSRV };
	md3dImmediateContext->PSSetShaderResources(0, 2, srvs);

	//设置SamplerState
	md3dImmediateContext->PSSetSamplers(0, 1, &mSamPoint);

	//设置VS,PS
	md3dImmediateContext->VSSetShader(mDDAVS, nullptr, 0);
	md3dImmediateContext->PSSetShader(mDDAPS, nullptr, 0);

	//渲染Quad
	md3dImmediateContext->DrawIndexed(6, 0, 0);
	
	
	unbind();

	blur();
//	unbind();
	
	conetracing();
	
	mSwapChain->Present(0, 0);
	
	
}



void SponzaDemo::blur()
{
	//清空临时缓存
	f32 black[] = { 0.f };
	for (auto& p : mTemporalRTV)
	{
		md3dImmediateContext->ClearRenderTargetView(p, black);
	}
	md3dImmediateContext->ClearDepthStencilView(mDepthStencilView,
		D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL,
		1.f, 0);
	

	//
	md3dImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	md3dImmediateContext->IASetInputLayout(mQuadLayout);
	u32 stride = sizeof(QuadVertex);
	u32 offset = 0;
	md3dImmediateContext->IASetVertexBuffers(0, 1, &mQuadVB, &stride, &offset);
	md3dImmediateContext->IASetIndexBuffer(mQuadIB, DXGI_FORMAT_R32_UINT, 0);

	//设置Shader
	md3dImmediateContext->VSSetShader(mBlurVS, nullptr, 0);

	D3D11_VIEWPORT tempVP = mViewPort;
	for (u32 i = 0; i < BlurLevel-1; ++i)
	{
	
		
		//垂直Blur,保存至temporal map
		md3dImmediateContext->OMSetRenderTargets(1, &mTemporalRTV[i],nullptr);
		D3D11_MAPPED_SUBRESOURCE ms;
		ZeroMemory(&ms, sizeof(ms));
		md3dImmediateContext->Map(mCBBlur, 0, D3D11_MAP_WRITE_DISCARD, 0, &ms);
		cbBlur* b = (cbBlur*)ms.pData;
		b->level = i ;
		md3dImmediateContext->Unmap(mCBBlur, 0);

		md3dImmediateContext->PSSetShader(mBlurPSVertical, nullptr, 0);
		md3dImmediateContext->PSSetSamplers(0, 1, &mSamPoint);
		md3dImmediateContext->PSSetConstantBuffers(0, 1, &mCBBlur);
		md3dImmediateContext->PSSetShaderResources(0, 1, &mVisibleColorSRV);

		md3dImmediateContext->DrawIndexed(6, 0, 0); 

//		unbind();
		tempVP.Width /= 2.f;
		tempVP.Height /= 2.f;
		md3dImmediateContext->RSSetViewports(1, &tempVP);

		//水平Blur,保存至lower mip map
		md3dImmediateContext->OMSetRenderTargets(1, &mVisibleColorMipRTV[i + 1], nullptr);
		md3dImmediateContext->PSSetShader(mBlurPSHorizontal, nullptr, 0);
		md3dImmediateContext->PSSetSamplers(0, 1, &mSamPoint);
		md3dImmediateContext->PSSetShaderResources(0, 1, &mTemporalSRV[i]);

		md3dImmediateContext->DrawIndexed(6, 0, 0);



		
	}
	md3dImmediateContext->RSSetViewports(1, &mViewPort);
	unbind();
}

void SponzaDemo::conetracing()
{
	//清空最终的RTV
	f32 black[] = { 0 };
	md3dImmediateContext->ClearRenderTargetView(mRenderTargetView, black); 
	md3dImmediateContext->ClearDepthStencilView(mDepthStencilView,
		D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL,
		1.f, 0);
	ID3D11UnorderedAccessView* debugs[] = {
		mDebugger0, mDebugger1, mDebugger2, mDebugger3
	};
//	md3dImmediateContext->OMSetRenderTargets(1, &mRenderTargetView, mDepthStencilView);
	md3dImmediateContext->OMSetRenderTargetsAndUnorderedAccessViews(1, &mRenderTargetView,
		mDepthStencilView, 1, 4, debugs, nullptr);
	md3dImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST); 
	md3dImmediateContext->IASetInputLayout(mQuadLayout);
	u32 stride = sizeof(QuadVertex);
	u32 offset = 0;
	md3dImmediateContext->IASetVertexBuffers(0, 1, &mQuadVB, &stride, &offset);
	md3dImmediateContext->IASetIndexBuffer(mQuadIB, DXGI_FORMAT_R32_UINT, 0);
	

	//更新矩阵
//	XMMATRIX world = XMMatrixIdentity();
//	XMMATRIX view = mCamera.getView();
//	D3D11_MAPPED_SUBRESOURCE ms;
//	md3dImmediateContext->Map(mCBTrans, 0, D3D11_MAP_WRITE_DISCARD, 0, &ms);
//	cbTrans* t = (cbTrans*)ms.pData;
//	XMStoreFloat4x4(&t->mat0, XMMatrixTranspose(world));
//	XMStoreFloat4x4(&t->mat1, XMMatrixTranspose(view));
//	md3dImmediateContext->Unmap(mCBTrans, 0);

	ID3D11ShaderResourceView* srvs[] =
	{
		mRayTracingBufferSRV,
		mVisibleColorSRV,
		mVisibleSpecularSRV
//		mVisibleNormalSRV
	};

	md3dImmediateContext->VSSetShader(mConeTracingVS, nullptr, 0);
	md3dImmediateContext->PSSetShader(mConeTracingPS, nullptr, 0);

	md3dImmediateContext->PSSetSamplers(0, 1, &mSampler);
	ID3D11Buffer* bufs[] = {mCBTraceInfo };
	md3dImmediateContext->PSSetConstantBuffers(0, 1, bufs);
	md3dImmediateContext->PSSetShaderResources(0, 3, srvs);

	md3dImmediateContext->DrawIndexed(6, 0, 0);

}
void SponzaDemo::unbind()
{
	ID3D11Buffer* bufs[8] = { nullptr };
	md3dImmediateContext->VSSetConstantBuffers(0, 8, bufs);
	md3dImmediateContext->PSSetConstantBuffers(0, 8, bufs);
	ID3D11ShaderResourceView* srvs[8] = { nullptr };
	md3dImmediateContext->PSSetShaderResources(0, 8, srvs);
	ID3D11RenderTargetView* rtvs[4] = { nullptr };
	ID3D11UnorderedAccessView* uavs[4] = { nullptr };
	md3dImmediateContext->OMSetRenderTargetsAndUnorderedAccessViews(4, rtvs,
		nullptr, 4, 4, uavs, nullptr);
	
}