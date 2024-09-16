#pragma once
#include "../../pch.h"
#include "../../Config.h"
#include "../../Util.h"

#include "FSR31Feature_Dx11.h"

#define ASSIGN_DESC(dest, src) dest.Width = src.Width; dest.Height = src.Height; dest.Format = src.Format; dest.BindFlags = src.BindFlags; 

#define SAFE_RELEASE(p)		\
do {						\
	if(p && p != nullptr)	\
	{						\
		(p)->Release();		\
		(p) = nullptr;		\
	}						\
} while((void)0, 0);	
FSR31FeatureDx11::FSR31FeatureDx11(unsigned int InHandleId, NVSDK_NGX_Parameter * InParameters) : FSR31Feature(InHandleId, InParameters), IFeature_Dx11(InHandleId, InParameters), IFeature(InHandleId, InParameters)
{
    _moduleLoaded = true;
}


bool FSR31FeatureDx11::Init(ID3D11Device* InDevice, ID3D11DeviceContext* InContext, NVSDK_NGX_Parameter* InParameters)
{
    LOG_FUNC();

    if (IsInited())
        return true;

    Device = InDevice;

    if (InitFSR3(InParameters))
    {
        if (!Config::Instance()->OverlayMenu.value_or(true) && (Imgui == nullptr || Imgui.get() == nullptr))
            Imgui = std::make_unique<Imgui_Dx11>(Util::GetProcessWindow(), Device);

        OutputScaler = std::make_unique<OS_Dx11>("Output Scaling", InDevice, (TargetWidth() < DisplayWidth()));
        RCAS = std::make_unique<RCAS_Dx11>("RCAS", InDevice);
        Bias = std::make_unique<Bias_Dx11>("Bias", InDevice);

        return true;
    }

    return false;
}

// register a DX11 resource to the backend
Fsr31::FfxResource ffxGetResource(ID3D11Resource* dx11Resource,
                                  wchar_t const* ffxResName,
                                  Fsr31::FfxResourceStates state = Fsr31::FFX_RESOURCE_STATE_COMPUTE_READ)
{
	Fsr31::FfxResource resource = {};
    resource.resource = reinterpret_cast<void*>(const_cast<ID3D11Resource*>(dx11Resource));
    resource.state = state;
    resource.description = Fsr31::GetFfxResourceDescriptionDX11(dx11Resource);

#ifdef _DEBUG
    if (ffxResName) {
        wcscpy_s(resource.name, ffxResName);
    }
#endif

    return resource;
}

bool FSR31FeatureDx11::Evaluate(ID3D11DeviceContext* DeviceContext, NVSDK_NGX_Parameter* InParameters)
{
	LOG_FUNC();

    if (!IsInited())
        return false;

    if (!RCAS->IsInit())
        Config::Instance()->RcasEnabled = false;

	Fsr31::FfxFsr3UpscalerDispatchDescription params{};

    params.commandList = DeviceContext;

    InParameters->Get(NVSDK_NGX_Parameter_Jitter_Offset_X, &params.jitterOffset.x);
    InParameters->Get(NVSDK_NGX_Parameter_Jitter_Offset_Y, &params.jitterOffset.y);

    unsigned int reset;
    InParameters->Get(NVSDK_NGX_Parameter_Reset, &reset);
    params.reset = (reset == 1);

    GetRenderResolution(InParameters, &params.renderSize.width, &params.renderSize.height);

    LOG_DEBUG("Input Resolution: {0}x{1}", params.renderSize.width, params.renderSize.height);

    bool useSS = Config::Instance()->OutputScalingEnabled.value_or(false) && !Config::Instance()->DisplayResolution.value_or(false);

    if (Config::Instance()->OverrideSharpness.value_or(false))
        _sharpness = Config::Instance()->Sharpness.value_or(0.3);
    else 
        _sharpness = GetSharpness(InParameters);

    if (Config::Instance()->RcasEnabled.value_or(false))
    {
        params.enableSharpening = false;
        params.sharpness = 0.0f;
    }
    else
    {
        if (_sharpness > 1.0f)
            _sharpness = 1.0f;

        params.enableSharpening = _sharpness > 0.0f;
        params.sharpness = _sharpness;
    }

    ID3D11Resource* paramColor;
    if (InParameters->Get(NVSDK_NGX_Parameter_Color, &paramColor) != NVSDK_NGX_Result_Success)
        InParameters->Get(NVSDK_NGX_Parameter_Color, (void**)&paramColor);

    if (paramColor)
    {
        LOG_DEBUG("Color exist..");
    	params.color = ffxGetResource(paramColor, L"FSR3_Input_OutputColor", Fsr31::FFX_RESOURCE_STATE_PIXEL_COMPUTE_READ);
    }
    else
    {
        LOG_ERROR("Color not exist!!");
        return false;
    }

    ID3D11Resource* paramVelocity;
    if (InParameters->Get(NVSDK_NGX_Parameter_MotionVectors, &paramVelocity) != NVSDK_NGX_Result_Success)
        InParameters->Get(NVSDK_NGX_Parameter_MotionVectors, (void**)&paramVelocity);

    if (paramVelocity)
    {
        LOG_DEBUG("MotionVectors exist..");
        params.motionVectors = ffxGetResource(paramVelocity, L"FSR3_InputMotionVectors", Fsr31::FFX_RESOURCE_STATE_PIXEL_COMPUTE_READ);

        if (!Config::Instance()->DisplayResolution.has_value())
        {
            D3D11_TEXTURE2D_DESC desc;
            ((ID3D11Texture2D*)paramVelocity)->GetDesc(&desc);
            bool lowResMV = desc.Width < TargetWidth();
            bool displaySizeEnabled = !(GetFeatureFlags() & NVSDK_NGX_DLSS_Feature_Flags_MVLowRes);

            if (displaySizeEnabled && lowResMV)
            {
                LOG_WARN("MotionVectors MVWidth: {0}, DisplayWidth: {1}, Flag: {2} Disabling DisplaySizeMV!!", desc.Width, DisplayWidth(), displaySizeEnabled);
                Config::Instance()->DisplayResolution = false;
                Config::Instance()->changeBackend = true;
                return true;
            }

            Config::Instance()->DisplayResolution = displaySizeEnabled;
        }
    }
    else
    {
        LOG_ERROR("MotionVectors not exist!!");
        return false;
    }

    ID3D11Resource* paramOutput;
    if (InParameters->Get(NVSDK_NGX_Parameter_Output, &paramOutput) != NVSDK_NGX_Result_Success)
        InParameters->Get(NVSDK_NGX_Parameter_Output, (void**)&paramOutput);

    if (paramOutput)
    {
        LOG_DEBUG("Output exist..");

        if (useSS)
        {
            if (OutputScaler->CreateBufferResource(Device, paramOutput, TargetWidth(), TargetHeight()))
            {
                params.output = ffxGetResource(OutputScaler->Buffer(),L"FSR3_Output", Fsr31::FFX_RESOURCE_STATE_UNORDERED_ACCESS);
            }
            else
                params.output = ffxGetResource(paramOutput, L"FSR3_Output", Fsr31::FFX_RESOURCE_STATE_UNORDERED_ACCESS);
        }
        else
            params.output = ffxGetResource(paramOutput, L"FSR3_Output", Fsr31::FFX_RESOURCE_STATE_UNORDERED_ACCESS);

        if (Config::Instance()->RcasEnabled.value_or(false) &&
            (_sharpness > 0.0f || (Config::Instance()->MotionSharpnessEnabled.value_or(false) && Config::Instance()->MotionSharpness.value_or(0.4) > 0.0f)) &&
            RCAS != nullptr && RCAS.get() != nullptr && RCAS->IsInit() && RCAS->CreateBufferResource(Device, (ID3D11Texture2D*)params.output.resource))
        {
            params.output = ffxGetResource(RCAS->Buffer(), L"FSR3_Output", Fsr31::FFX_RESOURCE_STATE_UNORDERED_ACCESS);
        }
    }
    else
    {
        LOG_ERROR("Output not exist!!");
        return false;
    }

    ID3D11Resource* paramDepth;
    if (InParameters->Get(NVSDK_NGX_Parameter_Depth, &paramDepth) != NVSDK_NGX_Result_Success)
        InParameters->Get(NVSDK_NGX_Parameter_Depth, (void**)&paramDepth);

    if (paramDepth)
    {
        LOG_DEBUG("Depth exist..");
        params.depth = ffxGetResource(paramDepth, L"FSR3_InputDepth", Fsr31::FFX_RESOURCE_STATE_PIXEL_COMPUTE_READ);
    }
    else
    {
        if (!Config::Instance()->DisplayResolution.value_or(false))
            LOG_ERROR("Depth not exist!!");
        else
            LOG_INFO("Using high res motion vectors, depth is not needed!!");
    }

    ID3D11Resource* paramExp = nullptr;
    if (!Config::Instance()->AutoExposure.value_or(false))
    {
        if (InParameters->Get(NVSDK_NGX_Parameter_ExposureTexture, &paramExp) != NVSDK_NGX_Result_Success)
            InParameters->Get(NVSDK_NGX_Parameter_ExposureTexture, (void**)&paramExp);

        if (paramExp)
        {
            params.exposure = ffxGetResource(paramExp, L"FSR3_InputExposure", Fsr31::FFX_RESOURCE_STATE_PIXEL_COMPUTE_READ);
            LOG_DEBUG("ExposureTexture exist..");
        }
        else
        {
            LOG_DEBUG("AutoExposure disabled but ExposureTexture is not exist, it may cause problems!!");
            Config::Instance()->AutoExposure = true;
            Config::Instance()->changeBackend = true;
            return true;
        }
    }
    else
        LOG_DEBUG("AutoExposure enabled!");

    ID3D11Resource* paramReactiveMask = nullptr;
    if (InParameters->Get(NVSDK_NGX_Parameter_DLSS_Input_Bias_Current_Color_Mask, &paramReactiveMask) != NVSDK_NGX_Result_Success)
        InParameters->Get(NVSDK_NGX_Parameter_DLSS_Input_Bias_Current_Color_Mask, (void**)&paramReactiveMask);

    if (!Config::Instance()->DisableReactiveMask.value_or(paramReactiveMask == nullptr))
    {
        if (paramReactiveMask)
        {
            LOG_DEBUG("Input Bias mask exist..");
            Config::Instance()->DisableReactiveMask = false;

            if (Config::Instance()->FsrUseMaskForTransparency.value_or(true))
                params.transparencyAndComposition = ffxGetResource(paramReactiveMask, L"FSR3_TransparencyAndCompositionMap", Fsr31::FFX_RESOURCE_STATE_PIXEL_COMPUTE_READ);

            if (Config::Instance()->DlssReactiveMaskBias.value_or(0.45f) > 0.0f &&
                Bias->IsInit() && Bias->CreateBufferResource(Device, paramReactiveMask) && Bias->CanRender())
            {
                if (Bias->Dispatch(Device, DeviceContext, (ID3D11Texture2D*)paramReactiveMask, Config::Instance()->DlssReactiveMaskBias.value_or(0.45f), Bias->Buffer()))
                {
                    params.reactive = ffxGetResource(Bias->Buffer(),L"FSR3_InputReactiveMap", Fsr31::FFX_RESOURCE_STATE_PIXEL_COMPUTE_READ);
                }
            }
            else
            {
                LOG_DEBUG("Skipping reactive mask, Bias: {0}, Bias Init: {1}, Bias CanRender: {2}",
                          Config::Instance()->DlssReactiveMaskBias.value_or(0.45f), Bias->IsInit(), Bias->CanRender());
            }
        }
    }

    _hasColor = params.color.resource != nullptr;
    _hasDepth = params.depth.resource != nullptr;
    _hasMV = params.motionVectors.resource != nullptr;
    _hasExposure = params.exposure.resource != nullptr;
    _hasTM = params.transparencyAndComposition.resource != nullptr;
    _accessToReactiveMask = paramReactiveMask != nullptr;
    _hasOutput = params.output.resource != nullptr;

    float MVScaleX = 1.0f;
    float MVScaleY = 1.0f;

    if (InParameters->Get(NVSDK_NGX_Parameter_MV_Scale_X, &MVScaleX) == NVSDK_NGX_Result_Success &&
        InParameters->Get(NVSDK_NGX_Parameter_MV_Scale_Y, &MVScaleY) == NVSDK_NGX_Result_Success)
    {
        params.motionVectorScale.x = MVScaleX;
        params.motionVectorScale.y = MVScaleY;
    }
    else
    {
        LOG_WARN("Can't get motion vector scales!");

        params.motionVectorScale.x = MVScaleX;
        params.motionVectorScale.y = MVScaleY;
    }

    if (IsDepthInverted())
    {
        params.cameraFar = Config::Instance()->FsrCameraNear.value_or(0.01f);
        params.cameraNear = Config::Instance()->FsrCameraFar.value_or(0.99f);
    }
    else
    {
        params.cameraFar = Config::Instance()->FsrCameraFar.value_or(0.99f);
        params.cameraNear = Config::Instance()->FsrCameraNear.value_or(0.01f);
    }

    if (Config::Instance()->FsrVerticalFov.has_value())
        params.cameraFovAngleVertical = Config::Instance()->FsrVerticalFov.value() * 0.0174532925199433f;
    else if (Config::Instance()->FsrHorizontalFov.value_or(0.0f) > 0.0f)
        params.cameraFovAngleVertical = 2.0f * atan((tan(Config::Instance()->FsrHorizontalFov.value() * 0.0174532925199433f) * 0.5f) / (float)DisplayHeight() * (float)DisplayWidth());
    else
        params.cameraFovAngleVertical = 1.0471975511966f;

    if (InParameters->Get(NVSDK_NGX_Parameter_FrameTimeDeltaInMsec, &params.frameTimeDelta) != NVSDK_NGX_Result_Success || params.frameTimeDelta < 1.0f)
        params.frameTimeDelta = (float)GetDeltaTime();

    params.preExposure = 1.0f;
    InParameters->Get(NVSDK_NGX_Parameter_DLSS_Pre_Exposure, &params.preExposure);

    LOG_DEBUG("Dispatch!!");
    auto result = ffxFsr3UpscalerContextDispatch(&_upscalerContext, &params);

    if (result != Fsr31::FFX_OK)
    {
        LOG_ERROR("ffxFsr3UpscalerContextDispatch error: {0}", ResultToString(result));
        return false;
    }

    // apply rcas
    if (Config::Instance()->RcasEnabled.value_or(false) &&
        (_sharpness > 0.0f || (Config::Instance()->MotionSharpnessEnabled.value_or(false) && Config::Instance()->MotionSharpness.value_or(0.4) > 0.0f)) &&
        RCAS != nullptr && RCAS.get() != nullptr && RCAS->CanRender())
    {
        RcasConstants rcasConstants{};

        rcasConstants.Sharpness = _sharpness;
        rcasConstants.DisplayWidth = TargetWidth();
        rcasConstants.DisplayHeight = TargetHeight();
        InParameters->Get(NVSDK_NGX_Parameter_MV_Scale_X, &rcasConstants.MvScaleX);
        InParameters->Get(NVSDK_NGX_Parameter_MV_Scale_Y, &rcasConstants.MvScaleY);
        rcasConstants.DisplaySizeMV = !(GetFeatureFlags() & NVSDK_NGX_DLSS_Feature_Flags_MVLowRes);
        rcasConstants.RenderHeight = RenderHeight();
        rcasConstants.RenderWidth = RenderWidth();


        if (useSS)
        {
            if (!RCAS->Dispatch(Device, DeviceContext, (ID3D11Texture2D*)params.output.resource, (ID3D11Texture2D*)params.motionVectors.resource,
                rcasConstants, OutputScaler->Buffer()))
            {
                Config::Instance()->RcasEnabled = false;
                return true;
            }
        }
        else
        {
            if (!RCAS->Dispatch(Device, DeviceContext, (ID3D11Texture2D*)params.output.resource, (ID3D11Texture2D*)params.motionVectors.resource,
                rcasConstants, (ID3D11Texture2D*)paramOutput))
            {
                Config::Instance()->RcasEnabled = false;
                return true;
            }
        }
    }

    if (useSS)
    {
        LOG_DEBUG("scaling output...");
        if (!OutputScaler->Dispatch(Device, DeviceContext, OutputScaler->Buffer(), (ID3D11Texture2D*)paramOutput))
        {
            Config::Instance()->OutputScalingEnabled = false;
            Config::Instance()->changeBackend = true;
            return true;
        }
    }

    // imgui
    if (!Config::Instance()->OverlayMenu.value_or(true) && _frameCount > 30)
    {
        if (Imgui != nullptr && Imgui.get() != nullptr)
        {
            if (Imgui->IsHandleDifferent())
            {
                Imgui.reset();
            }
            else
                Imgui->Render(DeviceContext, paramOutput);
        }
        else
        {
            if (Imgui == nullptr || Imgui.get() == nullptr)
                Imgui = std::make_unique<Imgui_Dx11>(GetForegroundWindow(), Device);
        }
    }

    _frameCount++;

    return true;
}

FSR31FeatureDx11::~FSR31FeatureDx11()
{
    if (!IsInited())
        return;

    auto errorCode = Fsr31::ffxFsr3UpscalerContextDestroy(&_upscalerContext);

    if (errorCode != Fsr31::FFX_OK)
        spdlog::error("FSR31FeatureDx11::~FSR31FeatureDx11 ffxFsr3UpscalerContextDestroy error: {0:x}", errorCode);

    free(_upscalerContextDesc.backendInterface.scratchBuffer);

    SetInit(false);
}

bool FSR31FeatureDx11::InitFSR3(const NVSDK_NGX_Parameter* InParameters)
{
    LOG_FUNC();

    if (!ModuleLoaded())
        return false;

    if (IsInited())
        return true;

    if (Device == nullptr)
    {
        LOG_ERROR("D3D12Device is null!");
        return false;
    }

    Config::Instance()->dxgiSkipSpoofing = true;

    const size_t scratchBufferSize = Fsr31::ffxGetScratchMemorySizeDX11(FFX_FSR3UPSCALER_CONTEXT_COUNT);
    void* scratchBuffer = malloc(scratchBufferSize);
    memset(scratchBuffer, 0, scratchBufferSize);

    auto errorCode = Fsr31::ffxGetInterfaceDX11(&_upscalerContextDesc.backendInterface, Fsr31::ffxGetDeviceDX11_Fsr31(Device), scratchBuffer, scratchBufferSize, FFX_FSR3UPSCALER_CONTEXT_COUNT);

    if (errorCode != Fsr31::FFX_OK)
    {
        LOG_ERROR("ffxGetInterfaceDX11 error: {0}", ResultToString(errorCode));
        free(scratchBuffer);
        return false;
    }

    _upscalerContextDesc.flags = 0;

    _upscalerContextDesc.fpMessage = FfxLogCallback;

    unsigned int featureFlags = 0;
    if (!_initFlagsReady)
    {
        InParameters->Get(NVSDK_NGX_Parameter_DLSS_Feature_Create_Flags, &featureFlags);
        _initFlags = featureFlags;
        _initFlagsReady = true;
    }
    else
        featureFlags = _initFlags;
    
    bool Hdr = featureFlags & NVSDK_NGX_DLSS_Feature_Flags_IsHDR;
    bool EnableSharpening = featureFlags & NVSDK_NGX_DLSS_Feature_Flags_DoSharpening;
    bool DepthInverted = featureFlags & NVSDK_NGX_DLSS_Feature_Flags_DepthInverted;
    bool JitterMotion = featureFlags & NVSDK_NGX_DLSS_Feature_Flags_MVJittered;
    bool LowRes = featureFlags & NVSDK_NGX_DLSS_Feature_Flags_MVLowRes;
    bool AutoExposure = featureFlags & NVSDK_NGX_DLSS_Feature_Flags_AutoExposure;

    _upscalerContextDesc.flags |= Fsr31::FFX_FSR3UPSCALER_ENABLE_DEBUG_CHECKING;

    if (Config::Instance()->DepthInverted.value_or(DepthInverted))
    {
        Config::Instance()->DepthInverted = true;
        _upscalerContextDesc.flags |= Fsr31::FFX_FSR3UPSCALER_ENABLE_DEPTH_INVERTED;
        SetDepthInverted(true);
        LOG_INFO("contextDesc.initFlags (DepthInverted) {0:b}", _upscalerContextDesc.flags);
    }
    else
        Config::Instance()->DepthInverted = false;

    if (Config::Instance()->AutoExposure.value_or(AutoExposure))
    {
        Config::Instance()->AutoExposure = true;
        _upscalerContextDesc.flags |= Fsr31::FFX_FSR3UPSCALER_ENABLE_AUTO_EXPOSURE;
        LOG_INFO("contextDesc.initFlags (AutoExposure) {0:b}", _upscalerContextDesc.flags);
    }
    else
    {
        Config::Instance()->AutoExposure = false;
        LOG_INFO("contextDesc.initFlags (!AutoExposure) {0:b}", _upscalerContextDesc.flags);
    }

    if (Config::Instance()->HDR.value_or(Hdr))
    {
        Config::Instance()->HDR = true;
        _upscalerContextDesc.flags |= Fsr31::FFX_FSR3UPSCALER_ENABLE_HIGH_DYNAMIC_RANGE;
        LOG_INFO("contextDesc.initFlags (HDR) {0:b}", _upscalerContextDesc.flags);
    }
    else
    {
        Config::Instance()->HDR = false;
        LOG_INFO("contextDesc.initFlags (!HDR) {0:b}", _upscalerContextDesc.flags);
    }

    if (Config::Instance()->JitterCancellation.value_or(JitterMotion))
    {
        Config::Instance()->JitterCancellation = true;
        _upscalerContextDesc.flags |= Fsr31::FFX_FSR3UPSCALER_ENABLE_MOTION_VECTORS_JITTER_CANCELLATION;
        LOG_INFO("contextDesc.initFlags (JitterCancellation) {0:b}", _upscalerContextDesc.flags);
    }
    else
        Config::Instance()->JitterCancellation = false;

    if (Config::Instance()->DisplayResolution.value_or(!LowRes))
    {
        _upscalerContextDesc.flags |= Fsr31::FFX_FSR3UPSCALER_ENABLE_DISPLAY_RESOLUTION_MOTION_VECTORS;
        LOG_INFO("contextDesc.initFlags (!LowResMV) {0:b}", _upscalerContextDesc.flags);
    }
    else
    {
        LOG_INFO("contextDesc.initFlags (LowResMV) {0:b}", _upscalerContextDesc.flags);
    }

    if (Config::Instance()->OutputScalingEnabled.value_or(false) && !Config::Instance()->DisplayResolution.value_or(false))
    {
        float ssMulti = Config::Instance()->OutputScalingMultiplier.value_or(1.5f);

        if (ssMulti < 0.5f)
        {
            ssMulti = 0.5f;
            Config::Instance()->OutputScalingMultiplier = ssMulti;
        }
        else if (ssMulti > 3.0f)
        {
            ssMulti = 3.0f;
            Config::Instance()->OutputScalingMultiplier = ssMulti;
        }

        _targetWidth = DisplayWidth() * ssMulti;
        _targetHeight = DisplayHeight() * ssMulti;
    }
    else
    {
        _targetWidth = DisplayWidth();
        _targetHeight = DisplayHeight();
    }

    // extended limits changes how resolution 
    if (Config::Instance()->ExtendedLimits.value_or(false) && RenderWidth() > DisplayWidth())
    {
        _upscalerContextDesc.maxRenderSize.width = RenderWidth();
        _upscalerContextDesc.maxRenderSize.height = RenderHeight();

        Config::Instance()->OutputScalingMultiplier = 1.0f;

        // if output scaling active let it to handle downsampling
        if (Config::Instance()->OutputScalingEnabled.value_or(false) && !Config::Instance()->DisplayResolution.value_or(false))
        {
            _upscalerContextDesc.displaySize.width = _upscalerContextDesc.maxRenderSize.width;
            _upscalerContextDesc.displaySize.height = _upscalerContextDesc.maxRenderSize.height;

            // update target res
            _targetWidth = _upscalerContextDesc.maxRenderSize.width;
            _targetHeight = _upscalerContextDesc.maxRenderSize.height;

        }
        else
        {
            _upscalerContextDesc.displaySize.width = DisplayWidth();
            _upscalerContextDesc.displaySize.height = DisplayHeight();
        }
    }
    else
    {
        _upscalerContextDesc.maxRenderSize.width = TargetWidth() > DisplayWidth() ? TargetWidth() : DisplayWidth();
        _upscalerContextDesc.maxRenderSize.height = TargetHeight() > DisplayHeight() ? TargetHeight() : DisplayHeight();
        _upscalerContextDesc.displaySize.width = TargetWidth();
        _upscalerContextDesc.displaySize.height = TargetHeight();
    }

	LOG_DEBUG("_createContext!");
    auto ret = ffxFsr3UpscalerContextCreate(&_upscalerContext, &_upscalerContextDesc);

    if (ret != Fsr31::FFX_OK)
    {
        LOG_ERROR("_createContext error: {0}", ResultToString(ret));
        return false;
    }

    LOG_INFO("_createContext success!");

    auto version = "3.0.3";
    _name = std::format("FSR {}", version);
    parse_version(version);

    Config::Instance()->dxgiSkipSpoofing = false;

    SetInit(true);

    LOG_INFO("Bye!");

    return true;
}