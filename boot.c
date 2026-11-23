#include "efi.h"

#define KERNEL_LOAD_ADDR 0x100000
#define KERNEL_EXTRA 0x100000

typedef void (__attribute__((sysv_abi)) *KernelEntry)(BootInfo *);

static void fail(EFI_SYSTEM_TABLE *st, const char *msg16){
	st->ConOut->OutputString(st->ConOut, (CHAR16*)msg16);
	while(1)__asm__ volatile("cli; hlt");
}

EFI_STATUS EFIAPI efi_main(EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE *SystemTable){
	EFI_STATUS status;
	EFI_BOOT_SERVICES *BS = SystemTable->BootServices;
	SystemTable->ConOut->ClearScreen(SystemTable->ConOut);
	SystemTable->ConOut->OutputString(SystemTable->ConOut, (CHAR16*)u"Booting HobbyOS...\r\n");

	// 1. Get loadImageProtocol to find boot device handle
	EFI_GUID loadedImageGuid = EFI_LOADED_IMAGE_PROTOCOL_GUID;
	EFI_LOADED_IMAGE_PROTOCOL *loadedImage = NULL;
	status = BS->HandleProtocol(ImageHandle, &loadedImageGuid, (VOID**)&loadedImage);
	if(EFI_ERROR(status) || !loadedImage) {
		fail(SystemTable, (const char*)u"ERROR: LoadedImage failed!\r\n");
	} else {

		SystemTable->ConOut->OutputString(SystemTable->ConOut, (CHAR16*)u"[OK] Loaded Image Protocol acquired.\r\n");
	}
	// 2. Open the Simple File System Protocol on boot device
	EFI_GUID fsGuid = EFI_SIMPLE_FILE_SYSTEM_PROTOCOL_GUID;
	EFI_SIMPLE_FILE_SYSTEM_PROTOCOL *fs = NULL;
	status = BS->HandleProtocol(loadedImage->DeviceHandle, &fsGuid, (VOID**)&fs);
	if(EFI_ERROR(status) || !fs) fail(SystemTable, (const char*)u"ERROR: SimpleFS failed!\r\n");

	EFI_FILE_PROTOCOL *root = NULL;
	status = fs->OpenVolume(fs, &root);
	if(EFI_ERROR(status) || !root) {
		fail(SystemTable,(const char*)u"ERROR: OpenVolume failed!\r\n");
	} else {
		
		SystemTable->ConOut->OutputString(SystemTable->ConOut, (CHAR16*)u"[OK] File system volume opened.\r\n");
	}

	// 3. Open KERNEL.BIN from the root directory
	EFI_FILE_PROTOCOL *kernelFile = NULL;
	status = root->Open(root, &kernelFile, (CHAR16*)u"KERNEL.BIN", EFI_FILE_MODE_READ, 0);
	if (EFI_ERROR(status) || !kernelFile) {
		fail(SystemTable, (const char*)u"ERROR: KERNEL.BIN not found!\r\n");
	} else {
	SystemTable->ConOut->OutputString(SystemTable->ConOut, (CHAR16*)u"[OK] KERNEL.BIN found.\r\n");
	}

	// 4 Get file size using EFI_FILE_INFO
	EFI_GUID fileInfoGuid = EFI_FILE_INFO_ID;
	UINTN infoSize = 0;
	status = kernelFile->GetInfo(kernelFile, &fileInfoGuid, &infoSize, NULL);
	if(status != EFI_BUFFER_TOO_SMALL ) fail(SystemTable, (const char*)u"ERROR: GetInfo size query failed!\r\n");

	EFI_FILE_INFO *fileInfo = NULL;
	status = BS->AllocatePool(EfiLoaderData, infoSize, (VOID**)&fileInfo);
	if(EFI_ERROR(status)) fail(SystemTable, (const char*)u"ERROR: info alloc failed!\r\n");

	status = kernelFile->GetInfo(kernelFile, &fileInfoGuid, &infoSize, fileInfo);
	if(EFI_ERROR(status) || !root) fail(SystemTable, (const char*)u"ERROR: GetInfo failed!\r\n");

	UINTN kernelSize = fileInfo->FileSize;
	BS->FreePool(fileInfo);

	// 5 Allocate physical pages at 0x100000 and read kernel into memory
	EFI_PHYSICAL_ADDRESS kernelAddr = KERNEL_LOAD_ADDR;
	UINTN pages = (kernelSize + KERNEL_EXTRA + 0xFFF)/0x1000;
	if (BS->AllocatePages(AllocateAddress, EfiLoaderData, pages, &kernelAddr) != EFI_SUCCESS){
		SystemTable->ConOut->OutputString(SystemTable->ConOut, (CHAR16*)u"ERROR: Could not allocate memory at 0x100000!\r\n");
		while(1) __asm__("hlt");
	}
	
	kernelFile->Read(kernelFile, &kernelSize, (VOID*)kernelAddr);
	kernelFile->Close(kernelFile);
	root->Close(root);
	SystemTable->ConOut->OutputString(SystemTable->ConOut, (CHAR16*)u"[OK] Kernel loaded into memory at 0x100000.\r\n");


	// 6. Locate Graphics Output Protocol
	EFI_GUID gopGuid = EFI_GRAPHICS_OUTPUT_PROTOCOL_GUID;
	EFI_GRAPHICS_OUTPUT_PROTOCOL *gop = NULL;
	
	status = BS->LocateProtocol(&gopGuid, NULL, (VOID**)&gop);
	if (EFI_ERROR(status)){
		SystemTable->ConOut->OutputString(SystemTable->ConOut, (CHAR16*)u"ERROR: GOP Not found!\r\n");
		while(1) __asm__("cli; hlt");
	}

	UINT32 pf = gop->Mode->Info->PixelFormat; // RGB = 0, BGR = 1, bitmask = 2, no framebuffer = 3
	if(pf != 0 && pf != 1) fail(SystemTable, (const char*)u"ERROR: unsupported pixel format \r\n");
	
	BootInfo bootInfo;
	bootInfo.FrameBufferBase	= gop->Mode->FrameBufferBase;
	bootInfo.Width			= gop->Mode->Info->HorizontalResolution;
	bootInfo.Height			= gop->Mode->Info->VerticalResolution;
	bootInfo.Pitch			= gop->Mode->Info->PixelsPerScanLine;
	bootInfo.PixelFormat		= pf;
	SystemTable->ConOut->OutputString(SystemTable->ConOut, (CHAR16*)u"[OK] GOP Framebuffer locked.\r\n");


	// 7. Retrieve Memory Map and Exit Boot Service
	UINTN mapSize = 0, bufSize = 0, mapKey = 0, descSize = 0;
	UINT32 descVersion = 0;
	EFI_MEMORY_DESCRIPTOR *memMap = NULL;
		

	// Initial query to get required buffer size
	BS->GetMemoryMap(&mapSize, NULL, &mapKey, &descSize, &descVersion);
	bufSize = mapSize + 8 * descSize;
	status = BS->AllocatePool(EfiLoaderData, bufSize, (VOID**)&memMap);
	if(EFI_ERROR(status)) {
		SystemTable->ConOut->OutputString(SystemTable->ConOut, (CHAR16*)u"ERROR: map alloc failed.\r\n");
		while(1) __asm__("cli; hlt");
	}


	SystemTable->ConOut->OutputString(SystemTable->ConOut, (CHAR16*)u"[OK] Exiting boot services.\r\n");


	for (int tries = 0; tries < 3;tries++) {
		mapSize = bufSize;
		status = BS->GetMemoryMap(&mapSize, memMap, &mapKey, &descSize, &descVersion);
		if(EFI_ERROR(status)) { continue; }
		status = BS->ExitBootServices(ImageHandle, mapKey);
		if(!EFI_ERROR(status)){	break; }
	}


	if (EFI_ERROR(status)){
		//Fallback: If firmware stubbornly refuses, halt visibly
		SystemTable->ConOut->OutputString(SystemTable->ConOut, (CHAR16*)u"ERROR: ExitBootServices failed permenently!\r\n");
		while (1) __asm__("cli; hlt");
	}
	bootInfo.MemoryMap = memMap;
	bootInfo.MemoryMapSize = mapSize;
	bootInfo.DescriptorSize = descSize;

	// 8. Jump to Hoby OS Kernel
	KernelEntry entry = (KernelEntry)KERNEL_LOAD_ADDR;
	entry(&bootInfo);
	while(1) __asm__("cli; hlt");
	return EFI_SUCCESS;

}
