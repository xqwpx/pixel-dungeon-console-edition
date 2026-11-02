# PS Vita backend

Status: DEVELOPMENT

The active implementation is retained at `src/platform/VitaInput.*` during the include-path migration and is linked exclusively as `pd_platform_psvita`. It translates `SceCtrl` and `SceTouch` data to `GameButton`/`IInputBackend`; no gameplay source includes VitaSDK.
