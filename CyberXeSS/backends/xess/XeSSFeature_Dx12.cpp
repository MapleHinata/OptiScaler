#pragma once
#include "../../pch.h"
#include "../../Config.h"

#include "XeSSFeature_Dx12.h"

bool XeSSFeatureDx12::Init(ID3D12Device* InDevice, const NVSDK_NGX_Parameter* InParameters)
{
	spdlog::debug("XeSSFeatureDx12::Init");

	if (IsInited())
		return true;

	Device = InDevice;

	if (InitXeSS(InDevice, InParameters))
	{
		Imgui = std::make_unique<Imgui_Dx12>(GetForegroundWindow(), Device);
		return true;
	}

	return false;
}

bool XeSSFeatureDx12::Evaluate(ID3D12GraphicsCommandList* InCommandList, const NVSDK_NGX_Parameter* InParameters)
{
	spdlog::debug("XeSSFeatureDx12::Evaluate");

	if (!IsInited() || !_xessContext)
		return false;

	xess_result_t xessResult;
	xess_d3d12_execute_params_t params{};

	InParameters->Get(NVSDK_NGX_Parameter_Jitter_Offset_X, &params.jitterOffsetX);
	InParameters->Get(NVSDK_NGX_Parameter_Jitter_Offset_Y, &params.jitterOffsetY);

	if (InParameters->Get(NVSDK_NGX_Parameter_DLSS_Exposure_Scale, &params.exposureScale) != NVSDK_NGX_Result_Success)
		params.exposureScale = 1.0f;

	InParameters->Get(NVSDK_NGX_Parameter_Reset, &params.resetHistory);

	GetRenderResolution(InParameters, &params.inputWidth, &params.inputHeight);

	spdlog::debug("XeSSFeatureDx12::Evaluate Input Resolution: {0}x{1}", params.inputWidth, params.inputHeight);

	ID3D12Resource* paramColor;
	if (InParameters->Get(NVSDK_NGX_Parameter_Color, &paramColor) != NVSDK_NGX_Result_Success)
		InParameters->Get(NVSDK_NGX_Parameter_Color, (void**)&paramColor);

	if (paramColor)
	{
		spdlog::debug("XeSSFeatureDx12::Evaluate Color exist..");

		if (Config::Instance()->ColorResourceBarrier.has_value())
			ResourceBarrier(InCommandList, paramColor,
				(D3D12_RESOURCE_STATES)Config::Instance()->ColorResourceBarrier.value(),
				D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
		else if (Config::Instance()->NVNGX_Engine == NVSDK_NGX_ENGINE_TYPE_UNREAL)
		{
			Config::Instance()->ColorResourceBarrier = (int)D3D12_RESOURCE_STATE_RENDER_TARGET;

			ResourceBarrier(InCommandList, paramColor,
				D3D12_RESOURCE_STATE_RENDER_TARGET,
				D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
		}

		if (Config::Instance()->ColorSpaceFix.value_or(false) &&
			CreateRecBufferResource(paramColor, Device, &_recBufferDecode) &&
			RecDecode(Device, InCommandList, paramColor, _recBufferDecode))
		{
			ResourceBarrier(InCommandList, _recBufferDecode,
				D3D12_RESOURCE_STATE_UNORDERED_ACCESS,
				D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);

			params.pColorTexture = _recBufferDecode;
		}
		else
			params.pColorTexture = paramColor;
	}
	else
	{
		spdlog::error("XeSSFeatureDx12::Evaluate Color not exist!!");
		return false;
	}

	if (InParameters->Get(NVSDK_NGX_Parameter_MotionVectors, &params.pVelocityTexture) != NVSDK_NGX_Result_Success)
		InParameters->Get(NVSDK_NGX_Parameter_MotionVectors, (void**)&params.pVelocityTexture);

	if (params.pVelocityTexture)
	{
		spdlog::debug("XeSSFeatureDx12::Evaluate MotionVectors exist..");

		if (Config::Instance()->MVResourceBarrier.has_value())
			ResourceBarrier(InCommandList, params.pVelocityTexture,
				(D3D12_RESOURCE_STATES)Config::Instance()->MVResourceBarrier.value(),
				D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
	}
	else
	{
		spdlog::error("XeSSFeatureDx12::Evaluate MotionVectors not exist!!");
		return false;
	}

	ID3D12Resource* paramOutput;
	if (InParameters->Get(NVSDK_NGX_Parameter_Output, &paramOutput) != NVSDK_NGX_Result_Success)
		InParameters->Get(NVSDK_NGX_Parameter_Output, (void**)&paramOutput);

	if (paramOutput)
	{
		spdlog::debug("XeSSFeatureDx12::Evaluate Output exist..");

		if (Config::Instance()->OutputResourceBarrier.has_value())
			ResourceBarrier(InCommandList, paramOutput,
				(D3D12_RESOURCE_STATES)Config::Instance()->OutputResourceBarrier.value(),
				D3D12_RESOURCE_STATE_UNORDERED_ACCESS);

		if (Config::Instance()->CasEnabled.value_or(true) && casSharpness > 0.0f)
		{
			if (casBuffer == nullptr && !CreateCasBufferResource(paramOutput, Device))
			{
				spdlog::error("XeSSFeatureDx12::Evaluate Can't create cas buffer!");
				return false;
			}

			params.pOutputTexture = casBuffer;
		}
		else
		{
			if (Config::Instance()->ColorSpaceFix.value_or(false))
			{
				CreateRecBufferResource(paramOutput, Device, &_recBufferEncode);
				params.pOutputTexture = _recBufferEncode;
			}
			else
				params.pOutputTexture = paramOutput;
		}
	}
	else
	{
		spdlog::error("XeSSFeatureDx12::Evaluate Output not exist!!");
		return false;
	}

	if (InParameters->Get(NVSDK_NGX_Parameter_Depth, &params.pDepthTexture) != NVSDK_NGX_Result_Success)
		InParameters->Get(NVSDK_NGX_Parameter_Depth, (void**)&params.pDepthTexture);

	if (params.pDepthTexture)
	{
		spdlog::debug("XeSSFeatureDx12::Evaluate Depth exist..");

		if (Config::Instance()->DepthResourceBarrier.has_value())
			ResourceBarrier(InCommandList, params.pDepthTexture,
				(D3D12_RESOURCE_STATES)Config::Instance()->DepthResourceBarrier.value(),
				D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
	}
	else
	{
		if (!Config::Instance()->DisplayResolution.value_or(false))
			spdlog::error("XeSSFeatureDx12::Evaluate Depth not exist!!");
		else
			spdlog::info("XeSSFeatureDx12::Evaluate Using high res motion vectors, depth is not needed!!");

		params.pDepthTexture = nullptr;
	}

	if (!Config::Instance()->AutoExposure.value_or(false))
	{
		if (InParameters->Get(NVSDK_NGX_Parameter_ExposureTexture, &params.pExposureScaleTexture) != NVSDK_NGX_Result_Success)
			InParameters->Get(NVSDK_NGX_Parameter_ExposureTexture, (void**)&params.pExposureScaleTexture);

		if (params.pExposureScaleTexture)
			spdlog::debug("XeSSFeatureDx12::Evaluate ExposureTexture exist..");
		else
			spdlog::debug("XeSSFeatureDx12::Evaluate AutoExposure disabled but ExposureTexture is not exist, it may cause problems!!");

		if (Config::Instance()->ExposureResourceBarrier.has_value())
			ResourceBarrier(InCommandList, params.pExposureScaleTexture,
				(D3D12_RESOURCE_STATES)Config::Instance()->ExposureResourceBarrier.value(),
				D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
	}
	else
		spdlog::debug("XeSSFeatureDx12::Evaluate AutoExposure enabled!");

	if (!Config::Instance()->DisableReactiveMask.value_or(true))
	{
		if (InParameters->Get(NVSDK_NGX_Parameter_DLSS_Input_Bias_Current_Color_Mask, &params.pResponsivePixelMaskTexture) != NVSDK_NGX_Result_Success)
			InParameters->Get(NVSDK_NGX_Parameter_DLSS_Input_Bias_Current_Color_Mask, (void**)&params.pResponsivePixelMaskTexture);

		if (params.pResponsivePixelMaskTexture)
			spdlog::debug("XeSSFeatureDx12::Evaluate Bias mask exist..");
		else
			spdlog::debug("XeSSFeatureDx12::Evaluate Bias mask not exist and its enabled in config, it may cause problems!!");

		if (Config::Instance()->MaskResourceBarrier.has_value())
			ResourceBarrier(InCommandList, params.pResponsivePixelMaskTexture,
				(D3D12_RESOURCE_STATES)Config::Instance()->MaskResourceBarrier.value(),
				D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
	}

	_hasColor = params.pColorTexture != nullptr;
	_hasMV = params.pVelocityTexture != nullptr;
	_hasOutput = params.pOutputTexture != nullptr;
	_hasDepth = params.pDepthTexture != nullptr;
	_hasExposure = params.pExposureScaleTexture != nullptr;
	_hasTM = params.pResponsivePixelMaskTexture != nullptr;

	float MVScaleX;
	float MVScaleY;

	if (InParameters->Get(NVSDK_NGX_Parameter_MV_Scale_X, &MVScaleX) == NVSDK_NGX_Result_Success &&
		InParameters->Get(NVSDK_NGX_Parameter_MV_Scale_Y, &MVScaleY) == NVSDK_NGX_Result_Success)
	{
		xessResult = xessSetVelocityScale(_xessContext, MVScaleX, MVScaleY);

		if (xessResult != XESS_RESULT_SUCCESS)
		{
			spdlog::error("XeSSFeatureDx12::Evaluate xessSetVelocityScale: {0}", ResultToString(xessResult));
			return false;
		}
	}
	else
		spdlog::warn("XeSSFeatureDx12::Evaluate Can't get motion vector scales!");

	spdlog::debug("XeSSFeatureDx12::Evaluate Executing!!");
	xessResult = xessD3D12Execute(_xessContext, InCommandList, &params);

	if (xessResult != XESS_RESULT_SUCCESS)
	{
		spdlog::error("XeSSFeatureDx12::Evaluate xessD3D12Execute error: {0}", ResultToString(xessResult));
		return false;
	}

	//apply cas
	if (Config::Instance()->CasEnabled.value_or(true) && casSharpness > 0.0f)
	{
		if (Config::Instance()->ColorSpaceFix.value_or(false))
		{
			if (!CreateRecBufferResource(casBuffer, Device, &_recBufferEncode) || !CasDispatch(InCommandList, InParameters, casBuffer, _recBufferEncode))
			{
				Config::Instance()->CasEnabled = false;
				return true;
			}
		}
		else
		{
			if (!CasDispatch(InCommandList, InParameters, casBuffer, paramOutput))
			{
				Config::Instance()->CasEnabled = false;
				return true;
			}
		}
	}

	// imgui
	if (Imgui)
	{
		if (Imgui->IsHandleDifferent())
			Imgui.reset();
		else
		{
			if (Config::Instance()->ColorSpaceFix.value_or(false))
				Imgui->Render(InCommandList, _recBufferEncode);
			else
				Imgui->Render(InCommandList, paramOutput);
		}
	}
	else
		Imgui = std::make_unique<Imgui_Dx12>(GetForegroundWindow(), Device);

	if (Config::Instance()->ColorSpaceFix.value_or(false))
	{
		ResourceBarrier(InCommandList, _recBufferEncode, D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
		RecEncode(Device, InCommandList, _recBufferEncode, paramOutput);
	}

	// restore resource states
	if (params.pColorTexture && Config::Instance()->ColorResourceBarrier.has_value())
		ResourceBarrier(InCommandList, params.pColorTexture,
			D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE,
			(D3D12_RESOURCE_STATES)Config::Instance()->ColorResourceBarrier.value());

	if (params.pVelocityTexture && Config::Instance()->MVResourceBarrier.has_value())
		ResourceBarrier(InCommandList, params.pVelocityTexture,
			D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE,
			(D3D12_RESOURCE_STATES)Config::Instance()->MVResourceBarrier.value());

	if (paramOutput && Config::Instance()->OutputResourceBarrier.has_value())
		ResourceBarrier(InCommandList, paramOutput,
			D3D12_RESOURCE_STATE_UNORDERED_ACCESS,
			(D3D12_RESOURCE_STATES)Config::Instance()->OutputResourceBarrier.value());

	if (params.pDepthTexture && Config::Instance()->DepthResourceBarrier.has_value())
		ResourceBarrier(InCommandList, params.pDepthTexture,
			D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE,
			(D3D12_RESOURCE_STATES)Config::Instance()->DepthResourceBarrier.value());

	if (params.pExposureScaleTexture && Config::Instance()->ExposureResourceBarrier.has_value())
		ResourceBarrier(InCommandList, params.pExposureScaleTexture,
			D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE,
			(D3D12_RESOURCE_STATES)Config::Instance()->ExposureResourceBarrier.value());

	if (params.pResponsivePixelMaskTexture && Config::Instance()->MaskResourceBarrier.has_value())
		ResourceBarrier(InCommandList, params.pResponsivePixelMaskTexture,
			D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE,
			(D3D12_RESOURCE_STATES)Config::Instance()->MaskResourceBarrier.value());

	return true;
}

XeSSFeatureDx12::~XeSSFeatureDx12()
{
}
