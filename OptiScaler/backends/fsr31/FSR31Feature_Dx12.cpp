#pragma once
#include "../../pch.h"
#include "../../Config.h"

#include "FSR31Feature_Dx12.h"


FSR31FeatureDx12::FSR31FeatureDx12(unsigned int InHandleId, NVSDK_NGX_Parameter* InParameters) : FSR31Feature(InHandleId, InParameters), IFeature_Dx12(InHandleId, InParameters), IFeature(InHandleId, InParameters)
{
    spdlog::debug("FSR31FeatureDx12::FSR31FeatureDx12 Loading amd_fidelityfx_dx12.dll methods");

    _configure = (PfnFfxConfigure)DetourFindFunction("amd_fidelityfx_dx12.dll", "ffxConfigure");
    _createContext = (PfnFfxCreateContext)DetourFindFunction("amd_fidelityfx_dx12.dll", "ffxCreateContext");
    _destroyContext = (PfnFfxDestroyContext)DetourFindFunction("amd_fidelityfx_dx12.dll", "ffxDestroyContext");
    _dispatch = (PfnFfxDispatch)DetourFindFunction("amd_fidelityfx_dx12.dll", "ffxDispatch");
    _query = (PfnFfxQuery)DetourFindFunction("amd_fidelityfx_dx12.dll", "ffxQuery");

    _moduleLoaded = _configure != nullptr;

    if(_moduleLoaded)
        spdlog::info("FSR31FeatureDx12::FSR31FeatureDx12 amd_fidelityfx_dx12.dll methods loaded!");
    else
        spdlog::error("FSR31FeatureDx12::FSR31FeatureDx12 can't load amd_fidelityfx_dx12.dll methods!");
}

bool FSR31FeatureDx12::Init(ID3D12Device* InDevice, ID3D12GraphicsCommandList* InCommandList, NVSDK_NGX_Parameter* InParameters)
{
    spdlog::debug("FSR31FeatureDx12::Init");

    if (IsInited())
        return true;

    Device = InDevice;

    if (InitFSR3(InParameters))
    {
        if (!Config::Instance()->OverlayMenu.value_or(true) && (Imgui == nullptr || Imgui.get() == nullptr))
            Imgui = std::make_unique<Imgui_Dx12>(Util::GetProcessWindow(), InDevice);

        OutputScaler = std::make_unique<BS_Dx12>("Output Downsample", InDevice, (TargetWidth() < DisplayWidth()));
        RCAS = std::make_unique<RCAS_Dx12>("RCAS", InDevice);

        return true;
    }

    return false;
}

bool FSR31FeatureDx12::Evaluate(ID3D12GraphicsCommandList* InCommandList, NVSDK_NGX_Parameter* InParameters)
{
    spdlog::debug("FSR31FeatureDx12::Evaluate");

    if (!IsInited())
        return false;

    if (!RCAS->IsInit())
        Config::Instance()->RcasEnabled = false;

    if (!OutputScaler->IsInit())
        Config::Instance()->OutputScalingEnabled = false;

    struct ffxDispatchDescUpscale params = { 0 };
    params.header.type = FFX_API_DISPATCH_DESC_TYPE_UPSCALE;

    InParameters->Get(NVSDK_NGX_Parameter_Jitter_Offset_X, &params.jitterOffset.x);
    InParameters->Get(NVSDK_NGX_Parameter_Jitter_Offset_Y, &params.jitterOffset.y);

    float sharpness = 0.0f;

    if (Config::Instance()->OverrideSharpness.value_or(false))
    {
        sharpness = Config::Instance()->Sharpness.value_or(0.3);
    }
    else if (InParameters->Get(NVSDK_NGX_Parameter_Sharpness, &sharpness) == NVSDK_NGX_Result_Success)
    {
        if (sharpness > 1.0f)
            sharpness = 1.0f;

        _sharpness = sharpness;
    }

    if (Config::Instance()->RcasEnabled.value_or(false))
    {
        params.enableSharpening = false;
        params.sharpness = 0.0f;
    }
    else
    {
        params.enableSharpening = sharpness > 0.0f;
        params.sharpness = sharpness;
    }

    spdlog::debug("FSR31FeatureDx12::Evaluate Jitter Offset: {0}x{1}", params.jitterOffset.x, params.jitterOffset.y);

    unsigned int reset;
    InParameters->Get(NVSDK_NGX_Parameter_Reset, &reset);
    params.reset = (reset == 1);

    GetRenderResolution(InParameters, &params.renderSize.width, &params.renderSize.height);

    bool useSS = Config::Instance()->OutputScalingEnabled.value_or(false) && !Config::Instance()->DisplayResolution.value_or(false);

    spdlog::debug("FSR31FeatureDx12::Evaluate Input Resolution: {0}x{1}", params.renderSize.width, params.renderSize.height);

    params.commandList = InCommandList;

    ID3D12Resource* paramColor;
    if (InParameters->Get(NVSDK_NGX_Parameter_Color, &paramColor) != NVSDK_NGX_Result_Success)
        InParameters->Get(NVSDK_NGX_Parameter_Color, (void**)&paramColor);

    if (paramColor)
    {
        spdlog::debug("FSR31FeatureDx12::Evaluate Color exist..");

        if (Config::Instance()->ColorResourceBarrier.has_value())
        {
            ResourceBarrier(InCommandList, paramColor, (D3D12_RESOURCE_STATES)Config::Instance()->ColorResourceBarrier.value(), D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
        }
        else if (Config::Instance()->NVNGX_Engine == NVSDK_NGX_ENGINE_TYPE_UNREAL)
        {
            Config::Instance()->ColorResourceBarrier = (int)D3D12_RESOURCE_STATE_RENDER_TARGET;
            ResourceBarrier(InCommandList, paramColor, D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
        }

        params.color = ffxApiGetResourceDX12(paramColor, FFX_API_RESOURCE_STATE_COMPUTE_READ);
    }
    else
    {
        spdlog::error("FSR31FeatureDx12::Evaluate Color not exist!!");
        return false;
    }

    ID3D12Resource* paramVelocity;
    if (InParameters->Get(NVSDK_NGX_Parameter_MotionVectors, &paramVelocity) != NVSDK_NGX_Result_Success)
        InParameters->Get(NVSDK_NGX_Parameter_MotionVectors, (void**)&paramVelocity);

    if (paramVelocity)
    {
        spdlog::debug("FSR31FeatureDx12::Evaluate MotionVectors exist..");

        if (Config::Instance()->MVResourceBarrier.has_value())
            ResourceBarrier(InCommandList, paramVelocity, (D3D12_RESOURCE_STATES)Config::Instance()->MVResourceBarrier.value(), D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
        else if (Config::Instance()->NVNGX_Engine == NVSDK_NGX_ENGINE_TYPE_UNREAL)
        {
            Config::Instance()->MVResourceBarrier = (int)D3D12_RESOURCE_STATE_UNORDERED_ACCESS;
            ResourceBarrier(InCommandList, paramVelocity, D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
        }

        if (!Config::Instance()->DisplayResolution.has_value())
        {
            auto desc = paramVelocity->GetDesc();
            bool lowResMV = desc.Width < TargetWidth();
            bool displaySizeEnabled = !(GetFeatureFlags() & NVSDK_NGX_DLSS_Feature_Flags_MVLowRes);

            if (displaySizeEnabled && lowResMV)
            {
                spdlog::warn("FSR31FeatureDx12::Evaluate MotionVectors MVWidth: {0}, DisplayWidth: {1}, Flag: {2} Disabling DisplaySizeMV!!", desc.Width, TargetWidth(), displaySizeEnabled);
                Config::Instance()->DisplayResolution = false;
                Config::Instance()->changeBackend = true;
                return true;
            }

            Config::Instance()->DisplayResolution = displaySizeEnabled;
        }

        params.motionVectors = ffxApiGetResourceDX12(paramVelocity, FFX_API_RESOURCE_STATE_COMPUTE_READ);
    }
    else
    {
        spdlog::error("FSR31FeatureDx12::Evaluate MotionVectors not exist!!");
        return false;
    }

    ID3D12Resource* paramOutput;
    if (InParameters->Get(NVSDK_NGX_Parameter_Output, &paramOutput) != NVSDK_NGX_Result_Success)
        InParameters->Get(NVSDK_NGX_Parameter_Output, (void**)&paramOutput);

    if (paramOutput)
    {
        spdlog::debug("FSR31FeatureDx12::Evaluate Output exist..");

        if (Config::Instance()->OutputResourceBarrier.has_value())
            ResourceBarrier(InCommandList, paramOutput, (D3D12_RESOURCE_STATES)Config::Instance()->OutputResourceBarrier.value(), D3D12_RESOURCE_STATE_UNORDERED_ACCESS);

        if (useSS)
        {
            if (OutputScaler->CreateBufferResource(Device, paramOutput, TargetWidth(), TargetHeight(), D3D12_RESOURCE_STATE_UNORDERED_ACCESS))
            {
                OutputScaler->SetBufferState(InCommandList, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
                params.output = ffxApiGetResourceDX12(OutputScaler->Buffer(), FFX_API_RESOURCE_STATE_UNORDERED_ACCESS);
            }
            else
                params.output = ffxApiGetResourceDX12(paramOutput, FFX_API_RESOURCE_STATE_UNORDERED_ACCESS);
        }
        else
            params.output = ffxApiGetResourceDX12(paramOutput, FFX_API_RESOURCE_STATE_UNORDERED_ACCESS);

        if (Config::Instance()->RcasEnabled.value_or(false) &&
            (sharpness > 0.0f || (Config::Instance()->MotionSharpnessEnabled.value_or(false) && Config::Instance()->MotionSharpness.value_or(0.4) > 0.0f)) &&
            RCAS->IsInit() && RCAS->CreateBufferResource(Device, (ID3D12Resource*)params.output.resource, D3D12_RESOURCE_STATE_UNORDERED_ACCESS))
        {
            RCAS->SetBufferState(InCommandList, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
            params.output = ffxApiGetResourceDX12(RCAS->Buffer(), FFX_API_RESOURCE_STATE_UNORDERED_ACCESS);
        }
    }
    else
    {
        spdlog::error("FSR31FeatureDx12::Evaluate Output not exist!!");
        return false;
    }

    ID3D12Resource* paramDepth;
    if (InParameters->Get(NVSDK_NGX_Parameter_Depth, &paramDepth) != NVSDK_NGX_Result_Success)
        InParameters->Get(NVSDK_NGX_Parameter_Depth, (void**)&paramDepth);

    if (paramDepth)
    {
        spdlog::debug("FSR31FeatureDx12::Evaluate Depth exist..");

        if (Config::Instance()->DepthResourceBarrier.has_value())
            ResourceBarrier(InCommandList, paramDepth, (D3D12_RESOURCE_STATES)Config::Instance()->DepthResourceBarrier.value(), D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);

        params.depth = ffxApiGetResourceDX12(paramDepth, FFX_API_RESOURCE_STATE_COMPUTE_READ);
    }
    else
    {
        if (!Config::Instance()->DisplayResolution.value_or(false))
            spdlog::error("FSR31FeatureDx12::Evaluate Depth not exist!!");
        else
            spdlog::info("FSR31FeatureDx12::Evaluate Using high res motion vectors, depth is not needed!!");
    }

    ID3D12Resource* paramExp = nullptr;
    if (!Config::Instance()->AutoExposure.value_or(false))
    {
        if (InParameters->Get(NVSDK_NGX_Parameter_ExposureTexture, &paramExp) != NVSDK_NGX_Result_Success)
            InParameters->Get(NVSDK_NGX_Parameter_ExposureTexture, (void**)&paramExp);

        if (paramExp)
        {
            spdlog::debug("FSR31FeatureDx12::Evaluate ExposureTexture exist..");

            if (Config::Instance()->ExposureResourceBarrier.has_value())
                ResourceBarrier(InCommandList, paramExp, (D3D12_RESOURCE_STATES)Config::Instance()->ExposureResourceBarrier.value(), D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);

            params.exposure = ffxApiGetResourceDX12(paramExp, FFX_API_RESOURCE_STATE_COMPUTE_READ);
        }
        else
        {
            spdlog::debug("FSR31FeatureDx12::Evaluate AutoExposure disabled but ExposureTexture is not exist, it may cause problems!!");
            Config::Instance()->AutoExposure = true;
            Config::Instance()->changeBackend = true;
            return true;
        }
    }
    else
        spdlog::debug("FSR31FeatureDx12::Evaluate AutoExposure enabled!");

    ID3D12Resource* paramMask = nullptr;
    if (!Config::Instance()->DisableReactiveMask.value_or(true))
    {
        if (InParameters->Get(NVSDK_NGX_Parameter_DLSS_Input_Bias_Current_Color_Mask, &paramMask) != NVSDK_NGX_Result_Success)
            InParameters->Get(NVSDK_NGX_Parameter_DLSS_Input_Bias_Current_Color_Mask, (void**)&paramMask);

        if (paramMask)
        {
            spdlog::debug("FSR31FeatureDx12::Evaluate Bias mask exist..");

            if (Config::Instance()->MaskResourceBarrier.has_value())
                ResourceBarrier(InCommandList, paramMask, (D3D12_RESOURCE_STATES)Config::Instance()->MaskResourceBarrier.value(), D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);

            params.reactive = ffxApiGetResourceDX12(paramMask, FFX_API_RESOURCE_STATE_COMPUTE_READ);
        }
        else
        {
            spdlog::warn("FSR31FeatureDx12::Evaluate Bias mask not exist and its enabled in config, it may cause problems!!");
            Config::Instance()->DisableReactiveMask = true;
            Config::Instance()->changeBackend = true;
            return true;
        }
    }

    _hasColor = params.color.resource != nullptr;
    _hasDepth = params.depth.resource != nullptr;
    _hasMV = params.motionVectors.resource != nullptr;
    _hasExposure = params.exposure.resource != nullptr;
    _hasTM = params.reactive.resource != nullptr;
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
        spdlog::warn("FSR31FeatureDx12::Evaluate Can't get motion vector scales!");

        params.motionVectorScale.x = MVScaleX;
        params.motionVectorScale.y = MVScaleY;
    }

    spdlog::debug("FSR31FeatureDx12::Evaluate Sharpness: {0}", params.sharpness);

    if (IsDepthInverted())
    {
        params.cameraFar = 0.0f;
        params.cameraNear = 1.0f;
    }
    else
    {
        params.cameraFar = 1.0f;
        params.cameraNear = 0.0f;
    }

    if (Config::Instance()->FsrVerticalFov.has_value())
        params.cameraFovAngleVertical = Config::Instance()->FsrVerticalFov.value() * 0.0174532925199433f;
    else if (Config::Instance()->FsrHorizontalFov.value_or(0.0f) > 0.0f)
        params.cameraFovAngleVertical = 2.0f * atan((tan(Config::Instance()->FsrHorizontalFov.value() * 0.0174532925199433f) * 0.5f) / (float)TargetHeight() * (float)TargetWidth());
    else
        params.cameraFovAngleVertical = 1.0471975511966f;

    spdlog::debug("FSR31FeatureDx12::Evaluate FsrVerticalFov: {0}", params.cameraFovAngleVertical);

    if (InParameters->Get(NVSDK_NGX_Parameter_FrameTimeDeltaInMsec, &params.frameTimeDelta) != NVSDK_NGX_Result_Success || params.frameTimeDelta < 1.0f)
        params.frameTimeDelta = (float)GetDeltaTime();

    spdlog::debug("FSR31FeatureDx12::Evaluate FrameTimeDeltaInMsec: {0}", params.frameTimeDelta);

    params.preExposure = 1.0f;
    InParameters->Get(NVSDK_NGX_Parameter_DLSS_Pre_Exposure, &params.preExposure);

    spdlog::debug("FSR31FeatureDx12::Evaluate Dispatch!!");
    auto result = _dispatch(&_context, &params.header);

    if (result != FFX_API_RETURN_OK)
    {
        spdlog::error("FSR31FeatureDx12::Evaluate ffxFsr2ContextDispatch error: {0}", ResultToString(result));
        return false;
    }

    // apply rcas
    if (Config::Instance()->RcasEnabled.value_or(false) &&
        (sharpness > 0.0f || (Config::Instance()->MotionSharpnessEnabled.value_or(false) && Config::Instance()->MotionSharpness.value_or(0.4) > 0.0f)) &&
        RCAS->CanRender())
    {
        if (params.output.resource != RCAS->Buffer())
            ResourceBarrier(InCommandList, (ID3D12Resource*)params.output.resource, D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);

        RCAS->SetBufferState(InCommandList, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);

        RcasConstants rcasConstants{};

        rcasConstants.Sharpness = sharpness;
        rcasConstants.DisplayWidth = TargetWidth();
        rcasConstants.DisplayHeight = TargetHeight();
        InParameters->Get(NVSDK_NGX_Parameter_MV_Scale_X, &rcasConstants.MvScaleX);
        InParameters->Get(NVSDK_NGX_Parameter_MV_Scale_Y, &rcasConstants.MvScaleY);
        rcasConstants.DisplaySizeMV = !(GetFeatureFlags() & NVSDK_NGX_DLSS_Feature_Flags_MVLowRes);
        rcasConstants.RenderHeight = RenderHeight();
        rcasConstants.RenderWidth = RenderWidth();

        if (useSS)
        {
            if (!RCAS->Dispatch(Device, InCommandList, (ID3D12Resource*)params.output.resource, (ID3D12Resource*)params.motionVectors.resource,
                rcasConstants, OutputScaler->Buffer()))
            {
                Config::Instance()->RcasEnabled = false;
                return true;
            }
        }
        else
        {
            if (!RCAS->Dispatch(Device, InCommandList, (ID3D12Resource*)params.output.resource, (ID3D12Resource*)params.motionVectors.resource,
                rcasConstants, paramOutput))
            {
                Config::Instance()->RcasEnabled = false;
                return true;
            }
        }
    }

    if (useSS)
    {
        spdlog::debug("FSR31FeatureDx12::Evaluate downscaling output...");
        OutputScaler->SetBufferState(InCommandList, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);

        if (!OutputScaler->Dispatch(Device, InCommandList, OutputScaler->Buffer(), paramOutput))
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
                Imgui->Render(InCommandList, paramOutput);
        }
        else
        {
            if (Imgui == nullptr || Imgui.get() == nullptr)
                Imgui = std::make_unique<Imgui_Dx12>(GetForegroundWindow(), Device);
        }
    }

    // restore resource states
    if (paramColor && Config::Instance()->ColorResourceBarrier.has_value())
        ResourceBarrier(InCommandList, paramColor,
                        D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE,
                        (D3D12_RESOURCE_STATES)Config::Instance()->ColorResourceBarrier.value());

    if (paramVelocity && Config::Instance()->MVResourceBarrier.has_value())
        ResourceBarrier(InCommandList, paramVelocity,
                        D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE,
                        (D3D12_RESOURCE_STATES)Config::Instance()->MVResourceBarrier.value());

    if (paramOutput && Config::Instance()->OutputResourceBarrier.has_value())
        ResourceBarrier(InCommandList, paramOutput,
                        D3D12_RESOURCE_STATE_UNORDERED_ACCESS,
                        (D3D12_RESOURCE_STATES)Config::Instance()->OutputResourceBarrier.value());

    if (paramDepth && Config::Instance()->DepthResourceBarrier.has_value())
        ResourceBarrier(InCommandList, paramDepth,
                        D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE,
                        (D3D12_RESOURCE_STATES)Config::Instance()->DepthResourceBarrier.value());

    if (paramExp && Config::Instance()->ExposureResourceBarrier.has_value())
        ResourceBarrier(InCommandList, paramExp,
                        D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE,
                        (D3D12_RESOURCE_STATES)Config::Instance()->ExposureResourceBarrier.value());

    if (paramMask && Config::Instance()->MaskResourceBarrier.has_value())
        ResourceBarrier(InCommandList, paramMask,
                        D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE,
                        (D3D12_RESOURCE_STATES)Config::Instance()->MaskResourceBarrier.value());

    _frameCount++;

    return true;
}

FSR31FeatureDx12::~FSR31FeatureDx12()
{
}

bool FSR31FeatureDx12::InitFSR3(const NVSDK_NGX_Parameter* InParameters)
{
    spdlog::debug("FSR31FeatureDx12::InitFSR3");

    if (!ModuleLoaded())
        return false;

    if (IsInited())
        return true;

    if (Device == nullptr)
    {
        spdlog::error("FSR31FeatureDx12::InitFSR3 D3D12Device is null!");
        return false;
    }

    _contextDesc.header.type = FFX_API_CREATE_CONTEXT_DESC_TYPE_UPSCALE;

    _contextDesc.fpMessage = FfxLogCallback;

    unsigned int featureFlags;
    InParameters->Get(NVSDK_NGX_Parameter_DLSS_Feature_Create_Flags, &featureFlags);

    _initFlags = featureFlags;

    bool Hdr = featureFlags & NVSDK_NGX_DLSS_Feature_Flags_IsHDR;
    bool EnableSharpening = featureFlags & NVSDK_NGX_DLSS_Feature_Flags_DoSharpening;
    bool DepthInverted = featureFlags & NVSDK_NGX_DLSS_Feature_Flags_DepthInverted;
    bool JitterMotion = featureFlags & NVSDK_NGX_DLSS_Feature_Flags_MVJittered;
    bool LowRes = featureFlags & NVSDK_NGX_DLSS_Feature_Flags_MVLowRes;
    bool AutoExposure = featureFlags & NVSDK_NGX_DLSS_Feature_Flags_AutoExposure;
    
    _contextDesc.flags = 0;
    _contextDesc.flags |= FFX_UPSCALE_ENABLE_DEBUG_CHECKING;

    if (Config::Instance()->DepthInverted.value_or(DepthInverted))
    {
        Config::Instance()->DepthInverted = true;
        _contextDesc.flags |= FFX_UPSCALE_ENABLE_DEPTH_INVERTED;
        SetDepthInverted(true);
        spdlog::info("FSR31FeatureDx12::InitFSR3 contextDesc.initFlags (DepthInverted) {0:b}", _contextDesc.flags);
    }
    else
        Config::Instance()->DepthInverted = false;

    if (Config::Instance()->AutoExposure.value_or(AutoExposure))
    {
        Config::Instance()->AutoExposure = true;
        _contextDesc.flags |= FFX_UPSCALE_ENABLE_AUTO_EXPOSURE;
        spdlog::info("FSR31FeatureDx12::InitFSR3 contextDesc.initFlags (AutoExposure) {0:b}", _contextDesc.flags);
    }
    else
    {
        Config::Instance()->AutoExposure = false;
        spdlog::info("FSR31FeatureDx12::InitFSR3 contextDesc.initFlags (!AutoExposure) {0:b}", _contextDesc.flags);
    }

    if (Config::Instance()->HDR.value_or(Hdr))
    {
        Config::Instance()->HDR = true;
        _contextDesc.flags |= FFX_UPSCALE_ENABLE_HIGH_DYNAMIC_RANGE;
        spdlog::info("FSR31FeatureDx12::InitFSR3 contextDesc.initFlags (HDR) {0:b}", _contextDesc.flags);
    }
    else
    {
        Config::Instance()->HDR = false;
        spdlog::info("FSR31FeatureDx12::InitFSR3 contextDesc.initFlags (!HDR) {0:b}", _contextDesc.flags);
    }

    if (Config::Instance()->JitterCancellation.value_or(JitterMotion))
    {
        Config::Instance()->JitterCancellation = true;
        _contextDesc.flags |= FFX_UPSCALE_ENABLE_MOTION_VECTORS_JITTER_CANCELLATION;
        spdlog::info("FSR31FeatureDx12::InitFSR3 contextDesc.initFlags (JitterCancellation) {0:b}", _contextDesc.flags);
    }
    else
        Config::Instance()->JitterCancellation = false;

    if (Config::Instance()->DisplayResolution.value_or(!LowRes))
    {
        _contextDesc.flags |= FFX_UPSCALE_ENABLE_DISPLAY_RESOLUTION_MOTION_VECTORS;
        spdlog::info("FSR31FeatureDx12::InitFSR3 contextDesc.initFlags (!LowResMV) {0:b}", _contextDesc.flags);
    }
    else
    {
        spdlog::info("FSR31FeatureDx12::InitFSR3 contextDesc.initFlags (LowResMV) {0:b}", _contextDesc.flags);
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

    _contextDesc.maxRenderSize.width = TargetWidth();
    _contextDesc.maxRenderSize.height = TargetHeight();
    _contextDesc.maxUpscaleSize.width = TargetWidth();
    _contextDesc.maxUpscaleSize.height = TargetHeight();

    ffxCreateBackendDX12Desc backendDesc = { 0 };
    backendDesc.header.type = FFX_API_CREATE_CONTEXT_DESC_TYPE_BACKEND_DX12;
    backendDesc.device = Device;
    _contextDesc.header.pNext = &backendDesc.header;

    spdlog::debug("FSR31FeatureDx12::InitFSR3 _createContext!");
    auto ret = _createContext(&_context, &_contextDesc.header, NULL);

    if (ret != FFX_API_RETURN_OK)
    {
        spdlog::error("FSR31FeatureDx12::InitFSR3 _createContext error: {0}", ResultToString(ret));
        return false;
    }

    SetInit(true);

    return true;
}