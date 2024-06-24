#pragma once

#include "pch.h"
#include "Util.h"
#include "Config.h"
#include "Logger.h"
#include <vulkan/vulkan.hpp>

typedef NVSDK_NGX_Result(*PFN_CUDA_Init)(unsigned long long InApplicationId, const wchar_t* InApplicationDataPath, const NVSDK_NGX_FeatureCommonInfo* InFeatureInfo, NVSDK_NGX_Version InSDKVersion);
typedef NVSDK_NGX_Result(*PFN_CUDA_Init_ProjectID)(const char* InProjectId, NVSDK_NGX_EngineType InEngineType, const char* InEngineVersion, const wchar_t* InApplicationDataPath, NVSDK_NGX_Version InSDKVersion, const NVSDK_NGX_FeatureCommonInfo* InFeatureInfo);
typedef NVSDK_NGX_Result(*PFN_CUDA_Shutdown)(void);
typedef NVSDK_NGX_Result(*PFN_CUDA_GetParameters)(NVSDK_NGX_Parameter** OutParameters);
typedef NVSDK_NGX_Result(*PFN_CUDA_AllocateParameters)(NVSDK_NGX_Parameter** OutParameters);
typedef NVSDK_NGX_Result(*PFN_CUDA_GetCapabilityParameters)(NVSDK_NGX_Parameter** OutParameters);
typedef NVSDK_NGX_Result(*PFN_CUDA_DestroyParameters)(NVSDK_NGX_Parameter* InParameters);
typedef NVSDK_NGX_Result(*PFN_CUDA_GetScratchBufferSize)(NVSDK_NGX_Feature InFeatureId, const NVSDK_NGX_Parameter* InParameters, size_t* OutSizeInBytes);
typedef NVSDK_NGX_Result(*PFN_CUDA_CreateFeature)(NVSDK_NGX_Feature InFeatureID, const NVSDK_NGX_Parameter* InParameters, NVSDK_NGX_Handle** OutHandle);
typedef NVSDK_NGX_Result(*PFN_CUDA_EvaluateFeature)(const NVSDK_NGX_Handle* InFeatureHandle, const NVSDK_NGX_Parameter* InParameters, PFN_NVSDK_NGX_ProgressCallback InCallback);
typedef NVSDK_NGX_Result(*PFN_CUDA_ReleaseFeature)(NVSDK_NGX_Handle* InHandle);

typedef NVSDK_NGX_Result(*PFN_D3D11_Init)(unsigned long long InApplicationId, const wchar_t* InApplicationDataPath, ID3D11Device* InDevice, const NVSDK_NGX_FeatureCommonInfo* InFeatureInfo, NVSDK_NGX_Version InSDKVersion);
typedef NVSDK_NGX_Result(*PFN_D3D11_Init_ProjectID)(const char* InProjectId, NVSDK_NGX_EngineType InEngineType, const char* InEngineVersion, const wchar_t* InApplicationDataPath, ID3D11Device* InDevice, NVSDK_NGX_Version InSDKVersion, const NVSDK_NGX_FeatureCommonInfo* InFeatureInfo);
typedef NVSDK_NGX_Result(*PFN_D3D11_Init_Ext)(unsigned long long InApplicationId, const wchar_t* InApplicationDataPath, ID3D11Device* InDevice, NVSDK_NGX_Version InSDKVersion, const NVSDK_NGX_FeatureCommonInfo* InFeatureInfo);
typedef NVSDK_NGX_Result(*PFN_D3D11_Shutdown)(void);
typedef NVSDK_NGX_Result(*PFN_D3D11_Shutdown1)(ID3D11Device* InDevice);
typedef NVSDK_NGX_Result(*PFN_D3D11_GetParameters)(NVSDK_NGX_Parameter** OutParameters);
typedef NVSDK_NGX_Result(*PFN_D3D11_AllocateParameters)(NVSDK_NGX_Parameter** OutParameters);
typedef NVSDK_NGX_Result(*PFN_D3D11_GetCapabilityParameters)(NVSDK_NGX_Parameter** OutParameters);
typedef NVSDK_NGX_Result(*PFN_D3D11_DestroyParameters)(NVSDK_NGX_Parameter* InParameters);
typedef NVSDK_NGX_Result(*PFN_D3D11_GetScratchBufferSize)(NVSDK_NGX_Feature InFeatureId, const NVSDK_NGX_Parameter* InParameters, size_t* OutSizeInBytes);
typedef NVSDK_NGX_Result(*PFN_D3D11_CreateFeature)(ID3D11DeviceContext* InDevCtx, NVSDK_NGX_Feature InFeatureID, NVSDK_NGX_Parameter* InParameters, NVSDK_NGX_Handle** OutHandle);
typedef NVSDK_NGX_Result(*PFN_D3D11_ReleaseFeature)(NVSDK_NGX_Handle* InHandle);
typedef NVSDK_NGX_Result(*PFN_D3D11_GetFeatureRequirements)(IDXGIAdapter* Adapter, const NVSDK_NGX_FeatureDiscoveryInfo* FeatureDiscoveryInfo, NVSDK_NGX_FeatureRequirement* OutSupported);
typedef NVSDK_NGX_Result(*PFN_D3D11_EvaluateFeature)(ID3D11DeviceContext* InDevCtx, const NVSDK_NGX_Handle* InFeatureHandle, const NVSDK_NGX_Parameter* InParameters, PFN_NVSDK_NGX_ProgressCallback InCallback);

typedef NVSDK_NGX_Result(*PFN_D3D12_Init)(unsigned long long InApplicationId, const wchar_t* InApplicationDataPath, ID3D12Device* InDevice, const NVSDK_NGX_FeatureCommonInfo* InFeatureInfo, NVSDK_NGX_Version InSDKVersion);
typedef NVSDK_NGX_Result(*PFN_D3D12_Init_ProjectID)(const char* InProjectId, NVSDK_NGX_EngineType InEngineType, const char* InEngineVersion, const wchar_t* InApplicationDataPath, ID3D12Device* InDevice, NVSDK_NGX_Version InSDKVersion, const NVSDK_NGX_FeatureCommonInfo* InFeatureInfo);
typedef NVSDK_NGX_Result(*PFN_D3D12_Init_Ext)(unsigned long long InApplicationId, const wchar_t* InApplicationDataPath, ID3D12Device* InDevice, NVSDK_NGX_Version InSDKVersion, const NVSDK_NGX_FeatureCommonInfo* InFeatureInfo);
typedef NVSDK_NGX_Result(*PFN_D3D12_Shutdown)(void);
typedef NVSDK_NGX_Result(*PFN_D3D12_Shutdown1)(ID3D12Device* InDevice);
typedef NVSDK_NGX_Result(*PFN_D3D12_GetParameters)(NVSDK_NGX_Parameter** OutParameters);
typedef NVSDK_NGX_Result(*PFN_D3D12_AllocateParameters)(NVSDK_NGX_Parameter** OutParameters);
typedef NVSDK_NGX_Result(*PFN_D3D12_GetCapabilityParameters)(NVSDK_NGX_Parameter** OutParameters);
typedef NVSDK_NGX_Result(*PFN_D3D12_DestroyParameters)(NVSDK_NGX_Parameter* InParameters);
typedef NVSDK_NGX_Result(*PFN_D3D12_GetScratchBufferSize)(NVSDK_NGX_Feature InFeatureId, const NVSDK_NGX_Parameter* InParameters, size_t* OutSizeInBytes);
typedef NVSDK_NGX_Result(*PFN_D3D12_CreateFeature)(ID3D12GraphicsCommandList* InCmdList, NVSDK_NGX_Feature InFeatureID, NVSDK_NGX_Parameter* InParameters, NVSDK_NGX_Handle** OutHandle);
typedef NVSDK_NGX_Result(*PFN_D3D12_ReleaseFeature)(NVSDK_NGX_Handle* InHandle);
typedef NVSDK_NGX_Result(*PFN_D3D12_GetFeatureRequirements)(IDXGIAdapter* Adapter, const NVSDK_NGX_FeatureDiscoveryInfo* FeatureDiscoveryInfo, NVSDK_NGX_FeatureRequirement* OutSupported);
typedef NVSDK_NGX_Result(*PFN_D3D12_EvaluateFeature)(ID3D12GraphicsCommandList* InCmdList, const NVSDK_NGX_Handle* InFeatureHandle, const NVSDK_NGX_Parameter* InParameters, PFN_NVSDK_NGX_ProgressCallback InCallback);

typedef NVSDK_NGX_Result(*PFN_UpdateFeature)(const NVSDK_NGX_Application_Identifier* ApplicationId, const NVSDK_NGX_Feature FeatureID);

typedef NVSDK_NGX_Result(*PFN_VULKAN_RequiredExtensions)(unsigned int* OutInstanceExtCount, const char*** OutInstanceExts, unsigned int* OutDeviceExtCount, const char*** OutDeviceExts);
typedef NVSDK_NGX_Result(*PFN_VULKAN_Init)(unsigned long long InApplicationId, const wchar_t* InApplicationDataPath, VkInstance InInstance, VkPhysicalDevice InPD, VkDevice InDevice, PFN_vkGetInstanceProcAddr InGIPA, PFN_vkGetDeviceProcAddr InGDPA, const NVSDK_NGX_FeatureCommonInfo* InFeatureInfo, NVSDK_NGX_Version InSDKVersion);
typedef NVSDK_NGX_Result(*PFN_VULKAN_Init_Ext)(unsigned long long InApplicationId, const wchar_t* InApplicationDataPath, VkInstance InInstance, VkPhysicalDevice InPD, VkDevice InDevice, NVSDK_NGX_Version InSDKVersion, const NVSDK_NGX_FeatureCommonInfo* InFeatureInfo);
typedef NVSDK_NGX_Result(*PFN_VULKAN_Init_Ext2)(unsigned long long InApplicationId, const wchar_t* InApplicationDataPath, VkInstance InInstance, VkPhysicalDevice InPD, VkDevice InDevice, PFN_vkGetInstanceProcAddr InGIPA, PFN_vkGetDeviceProcAddr InGDPA, NVSDK_NGX_Version InSDKVersion, const NVSDK_NGX_FeatureCommonInfo* InFeatureInfo);
typedef NVSDK_NGX_Result(*PFN_VULKAN_Init_ProjectID_Ext)(const char* InProjectId, NVSDK_NGX_EngineType InEngineType, const char* InEngineVersion, const wchar_t* InApplicationDataPath, VkInstance InInstance, VkPhysicalDevice InPD, VkDevice InDevice, PFN_vkGetInstanceProcAddr InGIPA, PFN_vkGetDeviceProcAddr InGDPA, NVSDK_NGX_Version InSDKVersion, const NVSDK_NGX_FeatureCommonInfo* InFeatureInfo);
typedef NVSDK_NGX_Result(*PFN_VULKAN_Init_ProjectID)(const char* InProjectId, NVSDK_NGX_EngineType InEngineType, const char* InEngineVersion, const wchar_t* InApplicationDataPath, VkInstance InInstance, VkPhysicalDevice InPD, VkDevice InDevice, PFN_vkGetInstanceProcAddr InGIPA, PFN_vkGetDeviceProcAddr InGDPA, NVSDK_NGX_Version InSDKVersion, const NVSDK_NGX_FeatureCommonInfo* InFeatureInfo);
typedef NVSDK_NGX_Result(*PFN_VULKAN_Shutdown)(void);
typedef NVSDK_NGX_Result(*PFN_VULKAN_Shutdown1)(VkDevice InDevice);
typedef NVSDK_NGX_Result(*PFN_VULKAN_GetParameters)(NVSDK_NGX_Parameter** OutParameters);
typedef NVSDK_NGX_Result(*PFN_VULKAN_AllocateParameters)(NVSDK_NGX_Parameter** OutParameters);
typedef NVSDK_NGX_Result(*PFN_VULKAN_GetCapabilityParameters)(NVSDK_NGX_Parameter** OutParameters);
typedef NVSDK_NGX_Result(*PFN_VULKAN_DestroyParameters)(NVSDK_NGX_Parameter* InParameters);
typedef NVSDK_NGX_Result(*PFN_VULKAN_GetScratchBufferSize)(NVSDK_NGX_Feature InFeatureId, const NVSDK_NGX_Parameter* InParameters, size_t* OutSizeInBytes);
typedef NVSDK_NGX_Result(*PFN_VULKAN_CreateFeature)(VkCommandBuffer InCmdBuffer, NVSDK_NGX_Feature InFeatureID, NVSDK_NGX_Parameter* InParameters, NVSDK_NGX_Handle** OutHandle);
typedef NVSDK_NGX_Result(*PFN_VULKAN_CreateFeature1)(VkDevice InDevice, VkCommandBuffer InCmdList, NVSDK_NGX_Feature InFeatureID, NVSDK_NGX_Parameter* InParameters, NVSDK_NGX_Handle** OutHandle);
typedef NVSDK_NGX_Result(*PFN_VULKAN_ReleaseFeature)(NVSDK_NGX_Handle* InHandle);
typedef NVSDK_NGX_Result(*PFN_VULKAN_GetFeatureRequirements)(const VkInstance Instance, const VkPhysicalDevice PhysicalDevice, const NVSDK_NGX_FeatureDiscoveryInfo* FeatureDiscoveryInfo, NVSDK_NGX_FeatureRequirement* OutSupported);
typedef NVSDK_NGX_Result(*PFN_VULKAN_GetFeatureInstanceExtensionRequirements)(const NVSDK_NGX_FeatureDiscoveryInfo* FeatureDiscoveryInfo, uint32_t* OutExtensionCount, VkExtensionProperties** OutExtensionProperties);
typedef NVSDK_NGX_Result(*PFN_VULKAN_GetFeatureDeviceExtensionRequirements)(VkInstance Instance, VkPhysicalDevice PhysicalDevice, const NVSDK_NGX_FeatureDiscoveryInfo* FeatureDiscoveryInfo, uint32_t* OutExtensionCount, VkExtensionProperties** OutExtensionProperties);
typedef NVSDK_NGX_Result(*PFN_VULKAN_EvaluateFeature)(VkCommandBuffer InCmdList, const NVSDK_NGX_Handle* InFeatureHandle, const NVSDK_NGX_Parameter* InParameters, PFN_NVSDK_NGX_ProgressCallback InCallback);

class NVNGXProxy
{
private:
	inline static HMODULE _dll = nullptr;
	inline static bool _cudaInited = false;
	inline static bool _dx11Inited = false;
	inline static bool _dx12Inited = false;
	inline static bool _vulkanInited = false;

	inline static PFN_CUDA_Init _CUDA_Init = nullptr;
	inline static PFN_CUDA_Init_ProjectID _CUDA_Init_ProjectID = nullptr;
	inline static PFN_CUDA_Shutdown _CUDA_Shutdown = nullptr;
	inline static PFN_CUDA_GetParameters _CUDA_GetParameters = nullptr;
	inline static PFN_CUDA_AllocateParameters _CUDA_AllocateParameters = nullptr;
	inline static PFN_CUDA_GetCapabilityParameters _CUDA_GetCapabilityParameters = nullptr;
	inline static PFN_CUDA_DestroyParameters _CUDA_DestroyParameters = nullptr;
	inline static PFN_CUDA_GetScratchBufferSize _CUDA_GetScratchBufferSize = nullptr;
	inline static PFN_CUDA_CreateFeature _CUDA_CreateFeature = nullptr;
	inline static PFN_CUDA_EvaluateFeature _CUDA_EvaluateFeature = nullptr;
	inline static PFN_CUDA_ReleaseFeature _CUDA_ReleaseFeature = nullptr;

	inline static PFN_D3D11_Init _D3D11_Init = nullptr;
	inline static PFN_D3D11_Init_ProjectID _D3D11_Init_ProjectID = nullptr;
	inline static PFN_D3D11_Init_Ext _D3D11_Init_Ext = nullptr;
	inline static PFN_D3D11_Shutdown _D3D11_Shutdown = nullptr;
	inline static PFN_D3D11_Shutdown1 _D3D11_Shutdown1 = nullptr;
	inline static PFN_D3D11_GetParameters _D3D11_GetParameters = nullptr;
	inline static PFN_D3D11_AllocateParameters _D3D11_AllocateParameters = nullptr;
	inline static PFN_D3D11_GetCapabilityParameters _D3D11_GetCapabilityParameters = nullptr;
	inline static PFN_D3D11_DestroyParameters _D3D11_DestroyParameters = nullptr;
	inline static PFN_D3D11_GetScratchBufferSize _D3D11_GetScratchBufferSize = nullptr;
	inline static PFN_D3D11_CreateFeature _D3D11_CreateFeature = nullptr;
	inline static PFN_D3D11_ReleaseFeature _D3D11_ReleaseFeature = nullptr;
	inline static PFN_D3D11_GetFeatureRequirements _D3D11_GetFeatureRequirements = nullptr;
	inline static PFN_D3D11_EvaluateFeature _D3D11_EvaluateFeature = nullptr;

	inline static PFN_D3D12_Init _D3D12_Init = nullptr;
	inline static PFN_D3D12_Init_ProjectID _D3D12_Init_ProjectID = nullptr;
	inline static PFN_D3D12_Init_Ext _D3D12_Init_Ext = nullptr;
	inline static PFN_D3D12_Shutdown _D3D12_Shutdown = nullptr;
	inline static PFN_D3D12_Shutdown1 _D3D12_Shutdown1 = nullptr;
	inline static PFN_D3D12_GetParameters _D3D12_GetParameters = nullptr;
	inline static PFN_D3D12_AllocateParameters _D3D12_AllocateParameters = nullptr;
	inline static PFN_D3D12_GetCapabilityParameters _D3D12_GetCapabilityParameters = nullptr;
	inline static PFN_D3D12_DestroyParameters _D3D12_DestroyParameters = nullptr;
	inline static PFN_D3D12_GetScratchBufferSize _D3D12_GetScratchBufferSize = nullptr;
	inline static PFN_D3D12_CreateFeature _D3D12_CreateFeature = nullptr;
	inline static PFN_D3D12_ReleaseFeature _D3D12_ReleaseFeature = nullptr;
	inline static PFN_D3D12_GetFeatureRequirements _D3D12_GetFeatureRequirements = nullptr;
	inline static PFN_D3D12_EvaluateFeature _D3D12_EvaluateFeature = nullptr;

	inline static PFN_VULKAN_RequiredExtensions _VULKAN_RequiredExtensions = nullptr;
	inline static PFN_VULKAN_Init _VULKAN_Init = nullptr;
	inline static PFN_VULKAN_Init_ProjectID _VULKAN_Init_ProjectID = nullptr;
	inline static PFN_VULKAN_Init_Ext _VULKAN_Init_Ext = nullptr;
	inline static PFN_VULKAN_Init_Ext2 _VULKAN_Init_Ext2 = nullptr;
	inline static PFN_VULKAN_Init_ProjectID_Ext _VULKAN_Init_ProjectID_Ext = nullptr;
	inline static PFN_VULKAN_Shutdown _VULKAN_Shutdown = nullptr;
	inline static PFN_VULKAN_Shutdown1 _VULKAN_Shutdown1 = nullptr;
	inline static PFN_VULKAN_GetParameters _VULKAN_GetParameters = nullptr;
	inline static PFN_VULKAN_AllocateParameters _VULKAN_AllocateParameters = nullptr;
	inline static PFN_VULKAN_GetCapabilityParameters _VULKAN_GetCapabilityParameters = nullptr;
	inline static PFN_VULKAN_DestroyParameters _VULKAN_DestroyParameters = nullptr;
	inline static PFN_VULKAN_GetScratchBufferSize _VULKAN_GetScratchBufferSize = nullptr;
	inline static PFN_VULKAN_CreateFeature _VULKAN_CreateFeature = nullptr;
	inline static PFN_VULKAN_CreateFeature1 _VULKAN_CreateFeature1 = nullptr;
	inline static PFN_VULKAN_ReleaseFeature _VULKAN_ReleaseFeature = nullptr;
	inline static PFN_VULKAN_GetFeatureRequirements _VULKAN_GetFeatureRequirements = nullptr;
	inline static PFN_VULKAN_GetFeatureInstanceExtensionRequirements _VULKAN_GetFeatureInstanceExtensionRequirements = nullptr;
	inline static PFN_VULKAN_GetFeatureDeviceExtensionRequirements _VULKAN_GetFeatureDeviceExtensionRequirements = nullptr;
	inline static PFN_VULKAN_EvaluateFeature _VULKAN_EvaluateFeature = nullptr;

	inline static PFN_UpdateFeature _UpdateFeature = nullptr;

public:
	static void InitNVNGX()
	{
		// if dll already loaded
		if (_dll != nullptr)
			return;

		spdlog::debug("NVNGXProxy::InitNVNGX");

		Config::Instance()->dlssDisableHook = true;

		do
		{
			// From DLSS Enabler
			_dll = LoadLibrary(L"dlss-enabler-ngx.dll");
			spdlog::info("NVNGXProxy::InitNVNGX trying to load dlss-enabler-ngx.dll");

			if (_dll)
			{
				Config::Instance()->DE_Available = true;
				spdlog::info("NVNGXProxy::InitNVNGX dlss-enabler-ngx.dll loaded from DLSS Enabler, ptr: {0:X}", (ULONG64)_dll);
				break;
			}

			// From ini
			if (Config::Instance()->DLSSLibrary.has_value())
			{
				spdlog::info("NVNGXProxy::InitNVNGX trying to load nvngx from ini path!");

				std::filesystem::path cfgPath(Config::Instance()->DLSSLibrary.value().c_str());
				auto path = cfgPath / L"_nvngx.dll";

				spdlog::info("NVNGXProxy::InitNVNGX trying to load _nvngx.dll path: {0}", path.string());
				_dll = LoadLibraryW(path.c_str());

				if (_dll)
				{
					spdlog::info("NVNGXProxy::InitNVNGX _nvngx.dll loaded from {0}, ptr: {1:X}", path.string(), (ULONG64)_dll);
					break;
				}

				path = cfgPath / L"nvngx.dll";
				spdlog::info("NVNGXProxy::InitNVNGX trying to load nvngx.dll path: {0}", path.string());
				_dll = LoadLibraryW(path.c_str());

				if (_dll)
				{
					spdlog::info("NVNGXProxy::InitNVNGX nvngx.dll loaded from {0}, ptr: {1:X}", path.string(), (ULONG64)_dll);
					break;
				}
			}

			// From registry
			auto regNGXCorePath = Util::NvngxPath();
			if (!regNGXCorePath.has_value())
				break;

			auto nvngxPath = regNGXCorePath.value() / "_nvngx.dll";
			spdlog::info("NVNGXProxy::InitNVNGX trying to load _nvngx.dll path: {0}", nvngxPath.string());

			_dll = LoadLibraryW(nvngxPath.wstring().c_str());
			if (_dll)
			{
				spdlog::info("NVNGXProxy::InitNVNGX _nvngx.dll loaded from {0}, ptr: {1:X}", nvngxPath.string(), (ULONG64)_dll);
				break;
			}

			nvngxPath = regNGXCorePath.value() / "nvngx.dll";
			spdlog::info("NVNGXProxy::InitNVNGX trying to load nvngx.dll path: {0}", nvngxPath.string());

			_dll = LoadLibraryW(nvngxPath.wstring().c_str());
			if (_dll)
				spdlog::info("NVNGXProxy::InitNVNGX nvngx.dll loaded from {0}, ptr: {1:X}", nvngxPath.string(), (ULONG64)_dll);

		} while (false);

		if (_dll)
		{
			spdlog::info("NVNGXProxy::InitNVNGX getting nvngx method addresses");

			_D3D11_Init = (PFN_D3D11_Init)GetProcAddress(_dll, "NVSDK_NGX_D3D11_Init");
			_D3D11_Init_ProjectID = (PFN_D3D11_Init_ProjectID)GetProcAddress(_dll, "NVSDK_NGX_D3D11_Init_ProjectID");
			_D3D11_Init_Ext = (PFN_D3D11_Init_Ext)GetProcAddress(_dll, "NVSDK_NGX_D3D11_Init_Ext");
			_D3D11_Shutdown = (PFN_D3D11_Shutdown)GetProcAddress(_dll, "NVSDK_NGX_D3D11_Shutdown");
			_D3D11_Shutdown1 = (PFN_D3D11_Shutdown1)GetProcAddress(_dll, "NVSDK_NGX_D3D11_Shutdown1");
			_D3D11_GetParameters = (PFN_D3D11_GetParameters)GetProcAddress(_dll, "NVSDK_NGX_D3D11_GetParameters");
			_D3D11_AllocateParameters = (PFN_D3D11_AllocateParameters)GetProcAddress(_dll, "NVSDK_NGX_D3D11_AllocateParameters");
			_D3D11_GetCapabilityParameters = (PFN_D3D11_GetCapabilityParameters)GetProcAddress(_dll, "NVSDK_NGX_D3D11_GetCapabilityParameters");
			_D3D11_DestroyParameters = (PFN_D3D11_DestroyParameters)GetProcAddress(_dll, "NVSDK_NGX_D3D11_DestroyParameters");
			_D3D11_GetScratchBufferSize = (PFN_D3D11_GetScratchBufferSize)GetProcAddress(_dll, "NVSDK_NGX_D3D11_GetScratchBufferSize");
			_D3D11_CreateFeature = (PFN_D3D11_CreateFeature)GetProcAddress(_dll, "NVSDK_NGX_D3D11_CreateFeature");
			_D3D11_ReleaseFeature = (PFN_D3D11_ReleaseFeature)GetProcAddress(_dll, "NVSDK_NGX_D3D11_ReleaseFeature");
			_D3D11_GetFeatureRequirements = (PFN_D3D11_GetFeatureRequirements)GetProcAddress(_dll, "NVSDK_NGX_D3D11_GetFeatureRequirements");
			_D3D11_EvaluateFeature = (PFN_D3D11_EvaluateFeature)GetProcAddress(_dll, "NVSDK_NGX_D3D11_EvaluateFeature");

			_D3D12_Init = (PFN_D3D12_Init)GetProcAddress(_dll, "NVSDK_NGX_D3D12_Init");
			_D3D12_Init_ProjectID = (PFN_D3D12_Init_ProjectID)GetProcAddress(_dll, "NVSDK_NGX_D3D12_Init_ProjectID");
			_D3D12_Init_Ext = (PFN_D3D12_Init_Ext)GetProcAddress(_dll, "NVSDK_NGX_D3D12_Init_Ext");
			_D3D12_Shutdown = (PFN_D3D12_Shutdown)GetProcAddress(_dll, "NVSDK_NGX_D3D12_Shutdown");
			_D3D12_Shutdown1 = (PFN_D3D12_Shutdown1)GetProcAddress(_dll, "NVSDK_NGX_D3D12_Shutdown1");
			_D3D12_GetParameters = (PFN_D3D12_GetParameters)GetProcAddress(_dll, "NVSDK_NGX_D3D12_GetParameters");
			_D3D12_AllocateParameters = (PFN_D3D12_AllocateParameters)GetProcAddress(_dll, "NVSDK_NGX_D3D12_AllocateParameters");
			_D3D12_GetCapabilityParameters = (PFN_D3D12_GetCapabilityParameters)GetProcAddress(_dll, "NVSDK_NGX_D3D12_GetCapabilityParameters");
			_D3D12_DestroyParameters = (PFN_D3D12_DestroyParameters)GetProcAddress(_dll, "NVSDK_NGX_D3D12_DestroyParameters");
			_D3D12_GetScratchBufferSize = (PFN_D3D12_GetScratchBufferSize)GetProcAddress(_dll, "NVSDK_NGX_D3D12_GetScratchBufferSize");
			_D3D12_CreateFeature = (PFN_D3D12_CreateFeature)GetProcAddress(_dll, "NVSDK_NGX_D3D12_CreateFeature");
			_D3D12_ReleaseFeature = (PFN_D3D12_ReleaseFeature)GetProcAddress(_dll, "NVSDK_NGX_D3D12_ReleaseFeature");
			_D3D12_GetFeatureRequirements = (PFN_D3D12_GetFeatureRequirements)GetProcAddress(_dll, "NVSDK_NGX_D3D12_GetFeatureRequirements");
			_D3D12_EvaluateFeature = (PFN_D3D12_EvaluateFeature)GetProcAddress(_dll, "NVSDK_NGX_D3D12_EvaluateFeature");

			_VULKAN_RequiredExtensions = (PFN_VULKAN_RequiredExtensions)GetProcAddress(_dll, "NVSDK_NGX_VULKAN_RequiredExtensions");
			_VULKAN_Init = (PFN_VULKAN_Init)GetProcAddress(_dll, "NVSDK_NGX_VULKAN_Init");
			_VULKAN_Init_Ext = (PFN_VULKAN_Init_Ext)GetProcAddress(_dll, "NVSDK_NGX_VULKAN_Init_Ext");
			_VULKAN_Init_Ext2 = (PFN_VULKAN_Init_Ext2)GetProcAddress(_dll, "NVSDK_NGX_VULKAN_Init_Ext2");
			_VULKAN_Init_ProjectID_Ext = (PFN_VULKAN_Init_ProjectID_Ext)GetProcAddress(_dll, "NVSDK_NGX_VULKAN_Init_ProjectID_Ext");
			_VULKAN_Init_ProjectID = (PFN_VULKAN_Init_ProjectID)GetProcAddress(_dll, "NVSDK_NGX_VULKAN_Init_ProjectID");
			_VULKAN_Shutdown = (PFN_VULKAN_Shutdown)GetProcAddress(_dll, "NVSDK_NGX_VULKAN_Shutdown");
			_VULKAN_Shutdown1 = (PFN_VULKAN_Shutdown1)GetProcAddress(_dll, "NVSDK_NGX_VULKAN_Shutdown1");
			_VULKAN_GetParameters = (PFN_VULKAN_GetParameters)GetProcAddress(_dll, "NVSDK_NGX_VULKAN_GetParameters");
			_VULKAN_AllocateParameters = (PFN_VULKAN_AllocateParameters)GetProcAddress(_dll, "NVSDK_NGX_VULKAN_AllocateParameters");
			_VULKAN_GetCapabilityParameters = (PFN_VULKAN_GetCapabilityParameters)GetProcAddress(_dll, "NVSDK_NGX_VULKAN_GetCapabilityParameters");
			_VULKAN_DestroyParameters = (PFN_VULKAN_DestroyParameters)GetProcAddress(_dll, "NVSDK_NGX_VULKAN_DestroyParameters");
			_VULKAN_GetScratchBufferSize = (PFN_VULKAN_GetScratchBufferSize)GetProcAddress(_dll, "NVSDK_NGX_VULKAN_GetScratchBufferSize");
			_VULKAN_CreateFeature = (PFN_VULKAN_CreateFeature)GetProcAddress(_dll, "NVSDK_NGX_VULKAN_CreateFeature");
			_VULKAN_CreateFeature1 = (PFN_VULKAN_CreateFeature1)GetProcAddress(_dll, "NVSDK_NGX_VULKAN_CreateFeature1");
			_VULKAN_ReleaseFeature = (PFN_VULKAN_ReleaseFeature)GetProcAddress(_dll, "NVSDK_NGX_VULKAN_ReleaseFeature");
			_VULKAN_GetFeatureRequirements = (PFN_VULKAN_GetFeatureRequirements)GetProcAddress(_dll, "NVSDK_NGX_VULKAN_GetFeatureRequirements");
			_VULKAN_GetFeatureInstanceExtensionRequirements = (PFN_VULKAN_GetFeatureInstanceExtensionRequirements)GetProcAddress(_dll, "NVSDK_NGX_VULKAN_GetFeatureInstanceExtensionRequirements");
			_VULKAN_GetFeatureDeviceExtensionRequirements = (PFN_VULKAN_GetFeatureDeviceExtensionRequirements)GetProcAddress(_dll, "NVSDK_NGX_VULKAN_GetFeatureDeviceExtensionRequirements");
			_VULKAN_EvaluateFeature = (PFN_VULKAN_EvaluateFeature)GetProcAddress(_dll, "NVSDK_NGX_VULKAN_EvaluateFeature");

			_UpdateFeature = (PFN_UpdateFeature)GetProcAddress(_dll, "NVSDK_NGX_UpdateFeature");
		}

		Config::Instance()->dlssDisableHook = false;
	}

	static void GetFeatureCommonInfo(NVSDK_NGX_FeatureCommonInfo* fcInfo)
	{
		// Allocate memory for the array of const wchar_t*
		wchar_t const** paths = new const wchar_t* [Config::Instance()->NVNGX_FeatureInfo_Paths.size()];

		// Copy the strings from the vector to the array
		for (size_t i = 0; i < Config::Instance()->NVNGX_FeatureInfo_Paths.size(); ++i)
		{
			paths[i] = Config::Instance()->NVNGX_FeatureInfo_Paths[i].c_str();

			std::string str(Config::Instance()->NVNGX_FeatureInfo_Paths[i].length(), 0);
			std::transform(Config::Instance()->NVNGX_FeatureInfo_Paths[i].begin(), Config::Instance()->NVNGX_FeatureInfo_Paths[i].end(), str.begin(), [](wchar_t c) { return (char)c; });

			spdlog::debug("NVNGXProxy::GetFeatureCommonInfo paths[{0}]: {1}", i, str);
		}

		fcInfo->PathListInfo.Path = paths;
		fcInfo->PathListInfo.Length = static_cast<unsigned int>(Config::Instance()->NVNGX_FeatureInfo_Paths.size());
	}

	static HMODULE NVNGXModule()
	{
		return _dll;
	}

	// DirectX11
	static bool InitDx11(ID3D11Device* InDevice)
	{
		if (_dx11Inited)
			return true;

		InitNVNGX();

		if (_dll == nullptr)
			return false;

		NVSDK_NGX_FeatureCommonInfo fcInfo{};
		GetFeatureCommonInfo(&fcInfo);
		NVSDK_NGX_Result nvResult = NVSDK_NGX_Result_Fail;

		if (Config::Instance()->NVNGX_ProjectId != "" && _D3D11_Init_ProjectID != nullptr)
		{
			spdlog::debug("NVNGXProxy::InitDx11 _D3D11_Init_ProjectID!");

			nvResult = _D3D11_Init_ProjectID(Config::Instance()->NVNGX_ProjectId.c_str(), Config::Instance()->NVNGX_Engine, Config::Instance()->NVNGX_EngineVersion.c_str(),
				Config::Instance()->NVNGX_ApplicationDataPath.c_str(), InDevice, Config::Instance()->NVNGX_Version, &fcInfo);
		}
		else if (_D3D11_Init_Ext != nullptr)
		{
			spdlog::debug("NVNGXProxy::InitDx11 _D3D11_Init_Ext!");
			nvResult = _D3D11_Init_Ext(Config::Instance()->NVNGX_ApplicationId, Config::Instance()->NVNGX_ApplicationDataPath.c_str(), InDevice, Config::Instance()->NVNGX_Version, &fcInfo);
		}

		spdlog::debug("NVNGXProxy::InitDx11 result: {0:X}", (UINT)nvResult);

		_dx11Inited = (nvResult == NVSDK_NGX_Result_Success);
		return _dx11Inited;
	}

	static void SetDx11Inited()
	{
		_dx11Inited = true;
	}

	static bool IsDx11Inited()
	{
		return _dx11Inited;
	}

	static PFN_D3D11_Init_ProjectID D3D11_Init_ProjectID()
	{
		return _D3D11_Init_ProjectID;
	}

	static PFN_D3D11_Init D3D11_Init()
	{
		return _D3D11_Init;
	}

	static PFN_D3D11_Init_Ext D3D11_Init_Ext()
	{
		return _D3D11_Init_Ext;
	}

	static PFN_D3D11_GetFeatureRequirements D3D11_GetFeatureRequirements()
	{
		return _D3D11_GetFeatureRequirements;
	}

	static PFN_D3D11_GetCapabilityParameters D3D11_GetCapabilityParameters()
	{
		if (!_dx11Inited)
			return nullptr;

		return _D3D11_GetCapabilityParameters;
	}

	static PFN_D3D11_AllocateParameters D3D11_AllocateParameters()
	{
		if (!_dx11Inited)
			return nullptr;

		return _D3D11_AllocateParameters;
	}

	static PFN_D3D11_GetParameters D3D11_GetParameters()
	{
		if (!_dx11Inited)
			return nullptr;

		return _D3D11_GetParameters;
	}

	static PFN_D3D11_DestroyParameters D3D11_DestroyParameters()
	{
		if (!_dx11Inited)
			return nullptr;

		return _D3D11_DestroyParameters;
	}

	static PFN_D3D11_CreateFeature D3D11_CreateFeature()
	{
		if (!_dx11Inited)
			return nullptr;

		return _D3D11_CreateFeature;
	}

	static PFN_D3D11_EvaluateFeature D3D11_EvaluateFeature()
	{
		if (!_dx11Inited)
			return nullptr;

		return _D3D11_EvaluateFeature;
	}

	static PFN_D3D11_ReleaseFeature D3D11_ReleaseFeature()
	{
		if (!_dx11Inited)
			return nullptr;

		return _D3D11_ReleaseFeature;
	}

	static PFN_D3D11_Shutdown D3D11_Shutdown()
	{
		if (!_dx11Inited)
			return nullptr;

		return _D3D11_Shutdown;
	}

	static PFN_D3D11_Shutdown1 D3D11_Shutdown1()
	{
		if (!_dx11Inited)
			return nullptr;

		return _D3D11_Shutdown1;
	}

	// DirectX12
	static bool InitDx12(ID3D12Device* InDevice)
	{
		if (_dx12Inited)
			return true;

		InitNVNGX();

		if (_dll == nullptr)
			return false;

		NVSDK_NGX_FeatureCommonInfo fcInfo{};
		GetFeatureCommonInfo(&fcInfo);
		NVSDK_NGX_Result nvResult = NVSDK_NGX_Result_Fail;

		if (Config::Instance()->NVNGX_ProjectId != "" && _D3D12_Init_ProjectID != nullptr)
		{
			spdlog::info("NVNGXProxy::InitDx12 _D3D12_Init_ProjectID!");

			nvResult = _D3D12_Init_ProjectID(Config::Instance()->NVNGX_ProjectId.c_str(), Config::Instance()->NVNGX_Engine, Config::Instance()->NVNGX_EngineVersion.c_str(),
				Config::Instance()->NVNGX_ApplicationDataPath.c_str(), InDevice, Config::Instance()->NVNGX_Version, &fcInfo);
		}
		else if (_D3D12_Init_Ext != nullptr)
		{
			spdlog::info("NVNGXProxy::InitDx12 _D3D12_Init_Ext!");
			nvResult = _D3D12_Init_Ext(Config::Instance()->NVNGX_ApplicationId, Config::Instance()->NVNGX_ApplicationDataPath.c_str(), InDevice, Config::Instance()->NVNGX_Version, &fcInfo);
		}

		spdlog::info("NVNGXProxy::InitDx12 result: {0:X}", (UINT)nvResult);

		_dx12Inited = (nvResult == NVSDK_NGX_Result_Success);
		return _dx12Inited;
	}

	static void SetDx12Inited()
	{
		_dx12Inited = true;
	}

	static bool IsDx12Inited()
	{
		return _dx12Inited;
	}

	static PFN_D3D12_Init_ProjectID D3D12_Init_ProjectID()
	{
		return _D3D12_Init_ProjectID;
	}

	static PFN_D3D12_Init D3D12_Init()
	{
		return _D3D12_Init;
	}

	static PFN_D3D12_Init_Ext D3D12_Init_Ext()
	{
		return _D3D12_Init_Ext;
	}

	static PFN_D3D12_GetFeatureRequirements D3D12_GetFeatureRequirements()
	{
		return _D3D12_GetFeatureRequirements;
	}

	static PFN_D3D12_GetCapabilityParameters D3D12_GetCapabilityParameters()
	{
		if (!_dx12Inited)
			return nullptr;

		return _D3D12_GetCapabilityParameters;
	}

	static PFN_D3D12_AllocateParameters D3D12_AllocateParameters()
	{
		if (!_dx12Inited)
			return nullptr;

		return _D3D12_AllocateParameters;
	}

	static PFN_D3D12_GetParameters D3D12_GetParameters()
	{
		if (!_dx12Inited)
			return nullptr;

		return _D3D12_GetParameters;
	}

	static PFN_D3D12_DestroyParameters D3D12_DestroyParameters()
	{
		if (!_dx12Inited)
			return nullptr;

		return _D3D12_DestroyParameters;
	}

	static PFN_D3D12_CreateFeature D3D12_CreateFeature()
	{
		if (!_dx12Inited)
			return nullptr;

		return _D3D12_CreateFeature;
	}

	static PFN_D3D12_EvaluateFeature D3D12_EvaluateFeature()
	{
		if (!_dx12Inited)
			return nullptr;

		return _D3D12_EvaluateFeature;
	}

	static PFN_D3D12_ReleaseFeature D3D12_ReleaseFeature()
	{
		if (!_dx12Inited)
			return nullptr;

		return _D3D12_ReleaseFeature;
	}

	static PFN_D3D12_Shutdown D3D12_Shutdown()
	{
		if (!_dx12Inited)
			return nullptr;

		return _D3D12_Shutdown;
	}

	static PFN_D3D12_Shutdown1 D3D12_Shutdown1()
	{
		if (!_dx12Inited)
			return nullptr;

		return _D3D12_Shutdown1;
	}

	// Vulkan
	static bool InitVulkan(VkInstance InInstance, VkPhysicalDevice InPD, VkDevice InDevice, PFN_vkGetInstanceProcAddr InGIPA, PFN_vkGetDeviceProcAddr InGDPA)
	{
		if (_vulkanInited)
			return true;

		InitNVNGX();

		if (_dll == nullptr)
			return false;

		NVSDK_NGX_FeatureCommonInfo fcInfo{};
		GetFeatureCommonInfo(&fcInfo);
		NVSDK_NGX_Result nvResult = NVSDK_NGX_Result_Fail;

		if (Config::Instance()->NVNGX_ProjectId != "" && _VULKAN_Init_ProjectID != nullptr)
		{
			spdlog::debug("NVNGXProxy::InitVulkan _D3D12_Init_ProjectID!");

			nvResult = _VULKAN_Init_ProjectID(Config::Instance()->NVNGX_ProjectId.c_str(), Config::Instance()->NVNGX_Engine, Config::Instance()->NVNGX_EngineVersion.c_str(),
				Config::Instance()->NVNGX_ApplicationDataPath.c_str(), InInstance, InPD, InDevice, InGIPA, InGDPA, Config::Instance()->NVNGX_Version, &fcInfo);
		}
		else if (_VULKAN_Init_Ext != nullptr)
		{
			spdlog::debug("NVNGXProxy::InitVulkan _D3D12_Init_Ext!");
			nvResult = _VULKAN_Init_Ext(Config::Instance()->NVNGX_ApplicationId, Config::Instance()->NVNGX_ApplicationDataPath.c_str(), InInstance, InPD, InDevice, Config::Instance()->NVNGX_Version, &fcInfo);
		}

		spdlog::debug("NVNGXProxy::InitVulkan result: {0:X}", (UINT)nvResult);

		_vulkanInited = (nvResult == NVSDK_NGX_Result_Success);

		return true;
	}

	static void SetVulkanInited()
	{
		_vulkanInited = true;
	}

	static bool IsVulkanInited()
	{
		return _vulkanInited;
	}

	static PFN_VULKAN_Init_ProjectID VULKAN_Init_ProjectID()
	{
		return _VULKAN_Init_ProjectID;
	}

	static PFN_VULKAN_Init_ProjectID_Ext VULKAN_Init_ProjectID_Ext()
	{
		return _VULKAN_Init_ProjectID_Ext;
	}

	static PFN_VULKAN_Init_Ext VULKAN_Init_Ext()
	{
		return _VULKAN_Init_Ext;
	}

	static PFN_VULKAN_Init_Ext2 VULKAN_Init_Ext2()
	{
		return _VULKAN_Init_Ext2;
	}

	static PFN_VULKAN_Init VULKAN_Init()
	{
		return _VULKAN_Init;
	}

	static PFN_VULKAN_GetCapabilityParameters VULKAN_GetCapabilityParameters()
	{
		if (!_vulkanInited)
			return nullptr;

		return _VULKAN_GetCapabilityParameters;
	}

	static PFN_VULKAN_AllocateParameters VULKAN_AllocateParameters()
	{
		if (!_vulkanInited)
			return nullptr;

		return _VULKAN_AllocateParameters;
	}

	static PFN_VULKAN_GetParameters VULKAN_GetParameters()
	{
		if (!_vulkanInited)
			return nullptr;

		return _VULKAN_GetParameters;
	}

	static PFN_VULKAN_DestroyParameters VULKAN_DestroyParameters()
	{
		if (!_vulkanInited)
			return nullptr;

		return _VULKAN_DestroyParameters;
	}

	static PFN_VULKAN_CreateFeature VULKAN_CreateFeature()
	{
		if (!_vulkanInited)
			return nullptr;

		return _VULKAN_CreateFeature;
	}

	static PFN_VULKAN_CreateFeature1 VULKAN_CreateFeature1()
	{
		if (!_vulkanInited)
			return nullptr;

		return _VULKAN_CreateFeature1;
	}

	static PFN_VULKAN_EvaluateFeature VULKAN_EvaluateFeature()
	{
		if (!_vulkanInited)
			return nullptr;

		return _VULKAN_EvaluateFeature;
	}

	static PFN_VULKAN_ReleaseFeature VULKAN_ReleaseFeature()
	{
		if (!_vulkanInited)
			return nullptr;

		return _VULKAN_ReleaseFeature;
	}

	static PFN_VULKAN_Shutdown VULKAN_Shutdown()
	{
		if (!_vulkanInited)
			return nullptr;

		return _VULKAN_Shutdown;
	}

	static PFN_VULKAN_Shutdown1 VULKAN_Shutdown1()
	{
		if (!_vulkanInited)
			return nullptr;

		return _VULKAN_Shutdown1;
	}

	static PFN_UpdateFeature UpdateFeature()
	{
		if (_dll == nullptr)
			InitNVNGX();

		return _UpdateFeature;
	}

};