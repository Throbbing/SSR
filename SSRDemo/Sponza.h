/**
*������������������������������+ +
*�������������������ߩ��������ߩ� + +
*�������������������������������� ��
*�������������������������������� ++ + + +
*������������ ������������������ ��+
*�������������������������������� +
*�����������������������ߡ�������
*�������������������������������� + +
*��������������������������������
*��������������������������������������������������
*���������������������������� + + + +
*������������������������������������Code is far away from bug with the animal protecting��������������
*���������������������������� + �����������ޱ���,������bug����
*����������������������������
*��������������������������������+������������������
*���������������������� �������������� + +
*�������������������� ���������������ǩ�
*�������������������� ������������������
*�����������������������������ש����� + + + +
*�����������������������ϩϡ����ϩ�
*�����������������������ߩ���
*/
#pragma once
#include"DirectXDemo.h"
#include"Camera.h"
#include"D3DMesh.h"
#include<array>

#define BlurLevel 5
namespace jmxRCore
{
	struct FragmentInfo
	{
		XMFLOAT4 posV;
		XMFLOAT4 normal;
		XMFLOAT4 color;
	};



	class SponzaDemo :public DirectXDemo
	{
	public:
		SponzaDemo(u32 width, u32 height) :DirectXDemo(width, height){}
		~SponzaDemo()
		{
			ReleaseCom(mVS);
			ReleaseCom(mPS);
			ReleaseCom(mPSN);
			ReleaseCom(mPSA);
			ReleaseCom(mPSNA);
			ReleaseCom(mLayout);
			ReleaseCom(mSampler);

			ReleaseCom(mCBWVP);
			for (auto& m : mMeshs)
				delete m;


			ReleaseCom(mQuadVB);
			ReleaseCom(mQuadIB);
			ReleaseCom(mQuadLayout);
			ReleaseCom(mVisiblePositionRTV);
			ReleaseCom(mVisiblePositionSRV);
			ReleaseCom(mVisibleNormalRTV);
			ReleaseCom(mVisibleNormalSRV);
			for (auto& p : mVisibleColorMipRTV)
			{
				ReleaseCom(p);
			}
			ReleaseCom(mVisibleColorSRV);
			ReleaseCom(mRayTracingBufferRTV);
			ReleaseCom(mRayTracingBufferSRV);
			ReleaseCom(mCBToFarPlane);
			ReleaseCom(mCBMaterial);
			ReleaseCom(mABufferVS);
			ReleaseCom(mABufferPS);
			ReleaseCom(mABufferPSN);
			ReleaseCom(mABufferPSA);
			ReleaseCom(mABufferPSNA);
			ReleaseCom(mDDAVS);
			ReleaseCom(mDDAPS);
			ReleaseCom(mConeTracingVS);
			ReleaseCom(mConeTracingPS);
			ReleaseCom(mSamPoint);


		}
		virtual void init();
		virtual void update(f32 dt);
		virtual void run();
	private:
		std::vector<D3DMesh*>		mMeshs;
		std::vector<XMFLOAT4X4>		mMeshsWorld;
		ID3D11RasterizerState*		mRSNoCull;
		
		Camera		mCamera;

		ID3D11VertexShader* mVS;
		ID3D11PixelShader*	mPS;
		ID3D11PixelShader*	mPSN;
		ID3D11PixelShader*  mPSA;
		ID3D11PixelShader*  mPSNA;
		ID3D11InputLayout*  mLayout;
		ID3D11SamplerState* mSampler;

		ID3D11Buffer*		mCBWVP;
		ID3D11Buffer*		mCBCamera;
		SRVMgr				mgr;



		/*
			����DDA������Ray Casting
			���ö�RTV������Fragment����Ⱦ��Ϣ
			��Ⱦһ��Quad,����PS�׶ν���DDA
			Ԥ���Զ�˲ü�����Ķ˵�
			Quad��ÿ���˵��־һ��FarPlaneCorner
			���Դ���PS�׶�����׷�ٹ���

			Ϊ����Ⱦ���̣���Ⱦ��Ϣֻ���淨�ߺ���ɫ
			������������ӵ���Ϣ(������ϵ����radiance��)

		*/
		struct QuadVertex
		{
			XMFLOAT3 posH;
			//FarPlaneCorner�ı�־����TexCoord��z������
			XMFLOAT3 tex;
		};
		void								createQuad();
		void								createDDA();
		void								createDDAView();
		void								createBucketsView();
		void								rayCasting();
		void								rasterizer();
		void								blur();
		void								conetracing();
//		void								ssr();
		void								unbind();
		ID3D11Buffer*						mQuadVB;
		ID3D11Buffer*						mQuadIB;
		ID3D11InputLayout*					mQuadLayout;


		//ABuffer ���Fragment�ķ��ߺ���ɫ

		
		//�ɼ���Buffer
		//FORMAT_R32G32B32A32_FLOAT
		ID3D11RenderTargetView*				mVisiblePositionRTV;
		ID3D11ShaderResourceView*			mVisiblePositionSRV;
		ID3D11RenderTargetView*				mVisibleNormalRTV;
		ID3D11ShaderResourceView*			mVisibleNormalSRV;
		ID3D11RenderTargetView*				mVisibleSpecularRTV;
		ID3D11ShaderResourceView*			mVisibleSpecularSRV;


		ID3D11Texture2D*					mVisibleColorTex;
		std::array<ID3D11RenderTargetView*, BlurLevel>				mVisibleColorMipRTV;
		ID3D11ShaderResourceView*			mVisibleColorSRV;

			
		ID3D11RenderTargetView*				mRayTracingBufferRTV;
		ID3D11ShaderResourceView*			mRayTracingBufferSRV;
		

		std::array<ID3D11RenderTargetView*,BlurLevel-1>				mTemporalRTV;
		std::array<ID3D11ShaderResourceView*,BlurLevel-1>			mTemporalSRV;
		
		ID3D11Buffer*						mCBToFarPlane;
		ID3D11Buffer*						mCBTrans;
		ID3D11Buffer*						mCBTrans2;
		ID3D11Buffer*						mCBView;
		ID3D11Buffer*						mCBTraceInfo;
		ID3D11Buffer*						mCBBlur;
		ID3D11Buffer*						mCBMaterial;





		ID3D11UnorderedAccessView*			mDebugger0;
		ID3D11ShaderResourceView*			mDebugger0SRV;
		ID3D11UnorderedAccessView*			mDebugger1;
		ID3D11ShaderResourceView*			mDebugger1SRV;
		ID3D11UnorderedAccessView*			mDebugger2;
		ID3D11ShaderResourceView*			mDebugger2SRV;
		ID3D11UnorderedAccessView*			mDebugger3;
		ID3D11ShaderResourceView*			mDebugger3SRV;

		ID3D11SamplerState*					mSamPoint;
		
		ID3D11VertexShader*					mABufferVS;
		ID3D11PixelShader*					mABufferPS;
		ID3D11PixelShader*					mABufferPSN;
		ID3D11PixelShader*					mABufferPSA;
		ID3D11PixelShader*					mABufferPSNA;
		ID3D11VertexShader*					mDDAVS;
		ID3D11PixelShader*					mDDAPS;
		ID3D11VertexShader*					mBlurVS;
		ID3D11PixelShader*					mBlurPSVertical;
		ID3D11PixelShader*					mBlurPSHorizontal;
		ID3D11VertexShader*					mConeTracingVS;
		ID3D11PixelShader*					mConeTracingPS;




	};
}