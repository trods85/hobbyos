#ifndef EFI_H
#define EFI_H

#include <stdint.h>
#include <stddef.h>

#define EFIAPI __attribute__((ms_abi))


typedef uint8_t		UINT8;
typedef uint16_t	UINT16;
typedef	uint32_t 	UINT32;
typedef uint64_t 	UINT64;
typedef uint64_t 	UINTN;
typedef	int64_t 	INTN;
typedef uint16_t 	CHAR16;
typedef	void		VOID;
typedef	UINT8		BOOLEAN;
typedef UINTN		EFI_STATUS;
typedef VOID*		EFI_HANDLE;
typedef UINT64		EFI_PHYSICAL_ADDRESS;
typedef UINT64		EFI_VIRTUAL_ADDRESS;

#define EFI_SUCCESS	0
#define EFI_BUFFER_TOO_SMALL	(0x8000000000000000ULL | 5)
#define EFI_NOT_READY		(0x8000000000000000Ull | 6)

typedef struct {
	UINT32 Data1;
	UINT16 Data2;
	UINT16 Data3;
	UINT8 Data4[8];
}EFI_GUID;

//Protocol GUIDs
#define EFI_LOADED_IMAGE_PROTOCOL_GUID {0x5b1b31a1, 0x9562, 0x11d2, {0x8e, 0x3f, 0x00, 0xa0, 0xc9, 0x69, 0x72, 0x3b}}
#define EFI_SIMPLE_FILE_SYSTEM_PROTOCOL_GUID {0x0964e5b22, 0x6459, 0x11d2, {0x8e, 0x39, 0x00, 0xa0, 0xc9, 0x69, 0x72, 0x3b}}
#define EFI_GRAPHICS_OUTPUT_PROTOCOL_GUID {0x9042a9de, 0x23dc, 0x4a38, {0x96, 0xfb, 0x7a, 0xde, 0xd0, 0x80, 0x51, 0x6a}}
#define EFI_FILE_INFO_ID {0x09576e92, 0x6d3f, 0x11d2, {0x8e, 0x39, 0x00, 0xa0, 0xc9, 0x69, 0x72, 0x3b}}
#define EFI_ERROR(Status)(((INTN)(Status)) < 0)

typedef struct {
	UINT64 Signature;
	UINT32 Revision;
	UINT32 HeaderSize;
	UINT32 CRC32;
	UINT32 Reserved;
}EFI_TABLE_HEADER;

//Mem allocation types and descriptors
typedef enum{
	AllocateAnyPages,
	AllocateMaxAddress,
	AllocateAddress,
	MaxAllocateType
}EFI_ALLOCATE_TYPE;

typedef enum {
	EfiReservedMemoryType,
	EfiLoaderCode,
	EfiLoaderData,
	EfiBootServicesCode,
	EfiBootServicesData,
	EfiRuntimeServicesCode,
	EfiRuntimeServicesData,
	EfiConventionalMemory,
	EfiUnusableMemory,
	EfiACPIReclaimMemory,
	EfiACPIMemoryNVS,
	EfiMemoryMappedIO,
	EfiMemoryMappedIOPortSpace,
	EfiPalCode,
	EfiPersistentMemory,
	EfiMaxMemoryType
}EFI_MEMORY_TYPE;

typedef struct {
	UINT32 Type;
	EFI_PHYSICAL_ADDRESS PhysicalStart;
	EFI_VIRTUAL_ADDRESS VirtualStart;
	UINT64	NumberOfPages;
	UINT64	Attribute;
}EFI_MEMORY_DESCRIPTOR;

// Text Output Protocol

typedef struct _EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL {
	VOID *Reset;
	EFI_STATUS(EFIAPI *OutputString)(struct _EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL *This, CHAR16 *String);
	VOID *TestString;
	VOID *QueryMode;
	VOID *SetMode;
	VOID *SetAttribute;
	EFI_STATUS (EFIAPI *ClearScreen)(struct _EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL *This);
	VOID *SetCursorPosition;
	VOID *EnableCursor;
	VOID *Mode;
} EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL;

#define EFI_FILE_MODE_READ 0x0000000000000001ULL

typedef struct _EFI_FILE_PROTOCOL {
	UINT64 Revision;
	EFI_STATUS (EFIAPI *Open)(struct _EFI_FILE_PROTOCOL *This, struct _EFI_FILE_PROTOCOL **NewHandle, CHAR16 *FileName, UINT64 OpenMode, UINT64 Attribute);
	EFI_STATUS (EFIAPI *Close)(struct _EFI_FILE_PROTOCOL *This);
	VOID *Delete;
	EFI_STATUS (EFIAPI *Read)(struct _EFI_FILE_PROTOCOL *This, UINTN *BufferSize, VOID *Buffer);
	VOID *Write;
	VOID *GetPosition;
	VOID *SetPosition;
	EFI_STATUS (EFIAPI *GetInfo)(struct _EFI_FILE_PROTOCOL *This, EFI_GUID *InformationType, UINTN *BufferSize, VOID *Buffer);
	VOID *SetInfo;
	VOID *Flush;
}EFI_FILE_PROTOCOL;

typedef struct {
	UINT64 Size;
	UINT64 FileSize;
	UINT64 PhysicalSize;
	UINT64 CreateTime[2];
	UINT64 LastAccessTime[2];
	UINT64 ModificationTime[2];
	UINT64 FAttribute;
	CHAR16 FileName[1];
}EFI_FILE_INFO;

typedef struct _EFI_SIMPLE_FILE_SYSTEM_PROTOCOL {
	UINT64 Revision;
	EFI_STATUS (EFIAPI *OpenVolume)(struct _EFI_SIMPLE_FILE_SYSTEM_PROTOCOL *This, EFI_FILE_PROTOCOL **Root);
}EFI_SIMPLE_FILE_SYSTEM_PROTOCOL;

// Load Image Protocol

typedef struct {
	UINT32 Revision;
	EFI_HANDLE ParentHandle;
	VOID *SystemTable;
	EFI_HANDLE DeviceHandle;
	VOID *FilePath;
	VOID *Reserved;
	UINT32 LoadOptionsSize;
	VOID *LoadOptions;
	VOID *ImageBase;
	UINT64 ImageSize;
	EFI_MEMORY_TYPE ImageCodeType;
	EFI_MEMORY_TYPE ImageDataType;
	VOID *Unload;
}EFI_LOADED_IMAGE_PROTOCOL;

typedef struct {
	UINT32 Version;
	UINT32 HorizontalResolution;
	UINT32 VerticalResolution;
	UINT32 PixelFormat;
	UINT32 PixelInformation[4];
	UINT32 PixelsPerScanLine;
}EFI_GRAPHICS_OUTPUT_MODE_INFORMATION;

typedef struct {
	UINT32 MaxMode;
	UINT32 Mode;
	EFI_GRAPHICS_OUTPUT_MODE_INFORMATION *Info;
	UINTN SizeOfInfo;
	EFI_PHYSICAL_ADDRESS FrameBufferBase;
	UINTN FrameBufferSize;
} EFI_GRAPHICS_OUTPUT_PROTOCOL_MODE;

typedef struct _EFI_GRAPHICS_OUTPUT_PROTOCOL {
	VOID *QueryMode;
	VOID *SetMode;
	VOID *Bit;
	EFI_GRAPHICS_OUTPUT_PROTOCOL_MODE *Mode;
}EFI_GRAPHICS_OUTPUT_PROTOCOL;

// Boot Services

typedef struct {
	EFI_TABLE_HEADER Hdr;
	VOID *RaiseTPL;
	VOID *RestoreTPL;
	EFI_STATUS (EFIAPI *AllocatePages)(EFI_ALLOCATE_TYPE Type, EFI_MEMORY_TYPE MemoryType, UINTN Pages, EFI_PHYSICAL_ADDRESS *Memory);
	EFI_STATUS (EFIAPI *FreePages)(EFI_PHYSICAL_ADDRESS Memory, UINTN Pages);
	EFI_STATUS (EFIAPI *GetMemoryMap)(UINTN *MemoryMapSize, EFI_MEMORY_DESCRIPTOR *MemoryMap, UINTN *MapKey, UINTN *DescriptorSize, UINT32 *DescriptorVersion);
	EFI_STATUS (EFIAPI *AllocatePool)(EFI_MEMORY_TYPE PoolType, UINTN Size, VOID **Buffer);
	EFI_STATUS (EFIAPI *FreePool)(VOID *Buffer);
	VOID *CreateEvent;
	VOID *SetTimer;
	VOID *WaitForEvent;
	VOID *SignalEvent;
	VOID *CloseEvent;
	VOID *CheckEvent;
	VOID *InstallProtocolInterface;
	VOID *ReinstallProtocolInterface;
	VOID *UninstallProtocolInterface;
	EFI_STATUS (EFIAPI *HandleProtocol)(EFI_HANDLE Handle, EFI_GUID *Protocol, VOID **Interface);
	VOID *Reserved;
	VOID *ReservedProtocolNotify;
	VOID *LocateHandle;
	VOID *LocateDevicePath;
	VOID *InstallConfiruationTable;
	VOID *LoadImage;
	VOID *StartImage;
	VOID *Exit;
	VOID *UnloadImage;
	EFI_STATUS (EFIAPI *ExitBootServices)(EFI_HANDLE ImageHandle, UINTN MapKey);
	VOID *GetNextMonotonicCount;
	VOID *Stall;
	VOID *SetWatchdogtimer;
	VOID *ConnectController;
	VOID *DisconnectController;
	VOID *OpenProtocol;
	VOID *CloseProtocol;
	VOID *OpenProtocolInformation;
	VOID *ProtocolsPerHandle;
	VOID *LocateHandleBuffer;
	EFI_STATUS (EFIAPI *LocateProtocol)(EFI_GUID *Protocol, VOID *Registration, VOID **Interface);
}EFI_BOOT_SERVICES;

typedef struct {
	EFI_TABLE_HEADER Hdr;
	CHAR16 *FirmwareVendor;
	UINT32 FirmwareRevision;
	EFI_HANDLE ConsoleInHandle;
	VOID *ConIn;
	EFI_HANDLE ConsoleOutHandle;
	EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL *ConOut;
	EFI_HANDLE StandardErrorHandle;
	EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL *StdErr;
	VOID *RuntimeServices;
	EFI_BOOT_SERVICES *BootServices;
	UINTN NumberOfTableEntries;
	VOID *COnfigurationTable;
}EFI_SYSTEM_TABLE;

// Custom Bootinfo Struct passed to hobby OS kernel

typedef struct{
	UINT64 FrameBufferBase;
	UINT32 Width;
	UINT32 Height;
	UINT32 Pitch;
	EFI_MEMORY_DESCRIPTOR *MemoryMap;
	UINTN MemoryMapSize;
	UINTN DescriptorSize;
	UINT32 PixelFormat;
} BootInfo;

#endif //EFI_H
