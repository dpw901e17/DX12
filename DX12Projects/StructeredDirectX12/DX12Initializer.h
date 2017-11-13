#pragma once
#include "TargetWindow.h"
#include <iostream>
#include <d3d12.h>
#include <dxgi1_4.h>
#include <D3Dcompiler.h>
#include <DirectXMath.h>
#include "d3dx12.h"
#include <string>
#include <Mmsystem.h>
#include <mciapi.h>

class DX12Intializer
{
	bool InitD3D(TargetWindow& tw);
};