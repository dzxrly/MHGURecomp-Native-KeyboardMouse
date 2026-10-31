# NGX integration for MHGURecomp Enhanced

> DLSS SR means NVIDIA DLSS Super Resolution. DLAA is native-resolution DLSS anti-aliasing. NR means DLSS 5 Neural Rendering. SM89 is the Ada architecture used by GeForce RTX 40 GPUs.

## Components and sources

The public headers and `LICENSE.txt` are from [NVIDIA/DLSS](https://github.com/NVIDIA/DLSS/tree/9981c6350dc58c0ce21f024fd65e31e851c31689), commit `9981c6350dc58c0ce21f024fd65e31e851c31689` (310.9.16). The SDK library and runtimes retain NVIDIA's separate terms; the project MIT license does not replace them. No private NR header is required: the host uses the public feature-18 enum slot and the runtime interface.

`runtime/nvn/vulkan/ngx_sdk.cpp` is a C interface around the official SDK, compiled with Visual Studio x64 tools to isolate the Microsoft C++ ABI from the MinGW host. It uses the renderer's existing Vulkan handles. It creates no additional Vulkan instance/device, injects no layer and uses no graphics-API translation.

```powershell
./tools/build_ngx.ps1 -VulkanInclude 'C:/path/to/Vulkan-Headers/include'
```

The build script downloads the pinned public static SDK library when missing and writes `build/mhgu-nvngx.dll`. The Windows release includes that adapter and the unmodified official SR runtime. The host reports unavailability if a selected component is missing.

## NR runtime installation

NR model DLLs are user-supplied. Place the selected file relative to `mhgurecomp.exe`:

```text
mhgurecomp.exe
mhgu-nvngx.dll
nvngx_dlss.dll
dlss/nr/official/nvngx_dlssnr.dll
dlss/nr/sm89/nvngx_dlssnr.dll
```

- **Official:** NVIDIA-signed NR 310.9.16.0 is provided in NVIDIA's `rtx-remix-ngx_sdk_dlnr@2` dependency package, referenced by the [RTX Remix dependency manifest](https://github.com/NVIDIAGameWorks/dxvk-remix/blob/main/packman-external.xml). Use it in the `official` directory and select `NRRuntime = Official`.
- **SM89:** the tested compatibility runtime is the SF 310.9.16 build from [HoYoShadeHub-DLSSNR-Runtimes](https://github.com/thx114/HoYoShadeHub-DLSSNR-Runtimes/releases/tag/nr-310.9.16), asset `nvngx_dlssnr_310.9.16-SF.zip`. Put its model DLL in `sm89` and explicitly select `NRRuntime = SM89`. Its DLL matches [OpenNR](https://github.com/clshortfuse/openNR)'s compatibility output `CDFA6C1EC784867182164C7836F324F087B5C9F374D0C9FE46F54999FEABF3ED`.

NR defaults off. Official is the default runtime choice; missing files and failures never substitute SM89 automatically. Restart after changing NR settings. Keep runtime binaries out of Git and outside the public package's NR payload.

## Dispatch and controls

Official uses public SDK dispatch. SM89 invokes the runtime's Vulkan snippet with parameter blocks from the initialized SDK and the same Vulkan device. This works around older driver cores that reject feature 18 before dispatch. The adapter name contains `nvngx.dll`, and non-tail-call wrappers retain the caller module expected by the snippet. It does not rename the game or patch its driver.

NR selects the original FP16 model, receives an opaque SDR scene/depth/motion, and evaluates before the HUD. INI/F1 expose intensity, style, local structure, local tone, skin structure, automatic skin masking and 1–3 effect layers. Auto Mask defaults on; Skin defaults to zero. Effect layers amplify one result's residual and never add model evaluations. See [graphics architecture and formula](../../docs/DEVELOPMENT.md#graphics).

References: [bevy_dlss5 native Vulkan bindings](https://github.com/AlrikOlson/bevy_dlss5/blob/28be5dc1b4b907ddba63a0c73d531e629809a865/src/ngx.rs), [OptiScaler Vulkan NR implementation](https://github.com/Dagherbou/OptiScaler_DLSSNR/blob/973761621353b99bee3dc7d4bb27b117fef2644f/OptiScaler/dlssnr/DlssNrFeature_Vk.cpp), and [caller-module behavior](https://github.com/Dagherbou/OptiScaler_DLSSNR/blob/973761621353b99bee3dc7d4bb27b117fef2644f/OptiScaler/dlssnr/forwarder/README.md).

Basic NR and SR+NR have produced visible in-game output on RTX 4080 / driver 610.88 with the pinned SF model. This is not validation for RTX 20/30/50, arbitrary runtime builds or exhaustive scenes.
