#include <Uefi.h>
#include <Library/UefiApplicationEntryPoint.h>
#include <Library/UefiLib.h>
#include <Protocol/DevicePathToText.h>
#include <Library/DebugLib.h>
#include <Protocol/BlockIo.h>

extern EFI_BOOT_SERVICES *gBS;

EFI_STATUS
EFIAPI
DumpDeviceHandle (
  IN EFI_DEVICE_PATH_PROTOCOL *DevicePath,
  IN EFI_DEVICE_PATH_TO_TEXT_PROTOCOL *PathToText
  )
{
  CHAR16 *String;

  String = PathToText->ConvertDevicePathToText (DevicePath, TRUE, TRUE);
  Print(L"%s\n", String);

  gBS->FreePool (String);
  return EFI_SUCCESS;
}

EFI_STATUS
EFIAPI
UefiMain (
  IN EFI_HANDLE        ImageHandle,
  IN EFI_SYSTEM_TABLE  *SystemTable
  )
{
  EFI_STATUS Status;
  EFI_HANDLE *Handles;
  UINTN NoHandles;
  UINTN Index;
  EFI_GUID DevPathProtoGuid = EFI_DEVICE_PATH_PROTOCOL_GUID;
  EFI_GUID DevPathToTextProtoGuid = EFI_DEVICE_PATH_TO_TEXT_PROTOCOL_GUID;
  EFI_GUID BlockIoProtoGuid = EFI_BLOCK_IO_PROTOCOL_GUID;
  EFI_DEVICE_PATH_PROTOCOL *ImageDevicePath;
  EFI_BLOCK_IO_PROTOCOL *BlockDevice;
  EFI_DEVICE_PATH_TO_TEXT_PROTOCOL *PathToText;

  gBS = SystemTable->BootServices;
  Handles = NULL;
  NoHandles = 0;

  Status = gBS->LocateHandleBuffer (ByProtocol, &BlockIoProtoGuid,
                                    NULL, &NoHandles, &Handles);
  ASSERT_EFI_ERROR (Status);

  Print (L"List of removable media:\n");

  Status = gBS->LocateProtocol (&DevPathToTextProtoGuid,
                                NULL,
                                (VOID *)&PathToText);
  ASSERT_EFI_ERROR (Status);

  for (Index = 0; Index < NoHandles; Index++) {
    Status = gBS->OpenProtocol (Handles[Index], &BlockIoProtoGuid,
                                (VOID *)&BlockDevice,
                                ImageHandle,
                                NULL,
                                EFI_OPEN_PROTOCOL_GET_PROTOCOL);
    ASSERT_EFI_ERROR (Status);

    if (BlockDevice->Media->RemovableMedia == TRUE) {
      Status = gBS->OpenProtocol (Handles[Index], &DevPathProtoGuid,
                                  (VOID *)&ImageDevicePath,
                                  ImageHandle,
                                  NULL,
                                  EFI_OPEN_PROTOCOL_GET_PROTOCOL);
      ASSERT_EFI_ERROR (Status);
      DumpDeviceHandle (ImageDevicePath, PathToText);
      Status = gBS->CloseProtocol (Handles[Index], &DevPathProtoGuid,
                          ImageHandle, NULL);
      ASSERT_EFI_ERROR (Status);
    }

    Status = gBS->CloseProtocol (Handles[Index], &BlockIoProtoGuid,
                        ImageHandle, NULL);
    ASSERT_EFI_ERROR (Status);
  }

  gBS->FreePool (Handles);
  return EFI_SUCCESS;
}
