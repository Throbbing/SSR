/**
*　　　　　　　　�芥掘　　？芥�+ +
*　　　　　　　�芥食潯ォォォ食潯� + +
*　　　　　　　�А　　　　　　？� 　
*　　　　　　　�А　　？ァ　　？� ++ + + +
*　　　　　　 ���������エ������� ��+
*　　　　　　　�А　　　　　　？� +
*　　　　　　　�А　　？漾　　？�
*　　　　　　　�А　　　　　　？� + +
*　　　　　　　�皐ォ掘　　？芥ォ�
*　　　　　　　　　�А　　？А　　　　　　　　　　�
*　　　　　　　　　�А　　？� + + + +
*　　　　　　　　　�А　　？А　　　�Code is far away from bug with the animal protecting　　　　　　　
*　　　　　　　　　�А　　？� + 　　　　舞舗隠嗷,旗鷹涙bug　　
*　　　　　　　　　�А　　？�
*　　　　　　　　　�А　　？А　�+　　　　　　　　　
*　　　　　　　　　�А� 　　�皐ォォォ� + +
*　　　　　　　　　�� 　　　　　　　�禰�
*　　　　　　　　　�� 　　　　　　　�芥�
*　　　　　　　　　�皐粂粂芥ォ廩粂芥� + + + +
*　　　　　　　　　　�З路蓮？З路�
*　　　　　　　　　　�皐潯拭�
*/
#pragma once
#include"JmxRDemo.h"
#include"Timer.h"
#include"D3DUti.h"
#include<xnamath.h>
#include<windows.h>



namespace jmxRCore
{

	LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
	class DirectXDemo :public Demo
	{
	public:
		DirectXDemo(u32 width,u32 height):
			mWidth(width), mHeight(height) {}

		virtual ~DirectXDemo()
		{
			ReleaseCom(mDepthStencilView);
			ReleaseCom(mRenderTargetView);
			ReleaseCom(mBackBuffer);
			ReleaseCom(mDepthStencilBuffer);
			ReleaseCom(mSwapChain);
			ReleaseCom(md3dImmediateContext);
			ReleaseCom(md3dDevice);

		}

		virtual void init();
		virtual void update(f32 dt);
		virtual void run();
		virtual void calFPS(f32 dt);
		
	protected:
		
		ID3D11Device*  md3dDevice;
		ID3D11DeviceContext* md3dImmediateContext;
		IDXGISwapChain*		mSwapChain;
		ID3D11Texture2D*	mDepthStencilBuffer;

		ID3D11Texture2D*		 mBackBuffer;
		ID3D11RenderTargetView*  mRenderTargetView;
		ID3D11DepthStencilView*  mDepthStencilView;

		D3D11_VIEWPORT			mViewPort;

		Timer*					mTimer;

		u32						mWidth;
		u32						mHeight;

		HINSTANCE				mHinstance;
		HWND					mHwnd;

	private:
		void					initWindow();
		void					initD3D();
	};
}

