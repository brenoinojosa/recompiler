#include "build.h"
#include "xenonLibUtils.h" 
#include "xenonLibNatives.h"
#include "xenonPlatform.h"
#include "xenonGraphics.h"

//---------------------------------------------------------------------------

uint64 __fastcall XboxVideo_XGetVideoMode( uint64 ip, cpu::CpuRegs& regs )
{
	xnative::XVIDEO_MODE* pMode = GetPointer<xnative::XVIDEO_MODE>( regs.R3 );

	// 720p
	memset( pMode, 0, sizeof(xnative::XVIDEO_MODE) );
	pMode->dwDisplayWidth.Set(1280);
	pMode->dwDisplayHeight.Set(720);
	pMode->fIsInterlaced.Set(FALSE);
	pMode->fIsWideScreen.Set(TRUE);
	pMode->fIsHiDef.Set(TRUE);
	pMode->RefreshRate.Set(60);
	pMode->VideoStandard.Set(xnative::XC_VIDEO_STANDARD_PAL_I);

	RETURN_ARG(0);
}

//---------------------------------------------------------------------------

uint64 __fastcall XboxVideo_MmAllocatePhysicalMemoryEx( uint64 ip, cpu::CpuRegs& regs )
{
	const uint32 size = (const uint32)(regs.R4);
	const uint32 addr = (const uint32)(regs.R7);
	const uint32 align = (const uint32)(regs.R8);
	const uint32 protect = (const uint32)(regs.R5);
	GLog.Log( "XPhysicalAlloc: %d, %d, %d, %d", size, addr, align, protect );

	// validate params
	if (regs.R3 != 0 || addr != 0xFFFFFFFF || regs.R6 != 0)
	{
		GLog.Err( "Invalid XPhysicalAlloc format!" );
		RETURN_ARG(0);
	}

	// flags
	uint32 newFlags = 0;
	if ( protect & xnative::XPAGE_READONLY ) { GLog.Log( "Flag: PAGE_READONLY"); newFlags |= native::IMemory::eFlags_ReadOnly; }
	if ( protect & xnative::XPAGE_READWRITE ) { GLog.Log( "Flag: XPAGE_READWRITE"); newFlags |= native::IMemory::eFlags_ReadWrite; }
	if ( protect & xnative::XPAGE_NOCACHE ) { GLog.Log( "Flag: XPAGE_NOCACHE"); newFlags |= native::IMemory::eFlags_NotCached; }
	if ( protect & xnative::XPAGE_WRITECOMBINE ) { GLog.Log( "Flag: XPAGE_WRITECOMBINE"); newFlags |= native::IMemory::eFlags_WriteCombine; }
	if ( protect & xnative::XMEM_LARGE_PAGES ) { GLog.Log( "Flag: XMEM_LARGE_PAGES"); }
	if ( protect & xnative::XMEM_16MB_PAGES ) { GLog.Log( "Flag: XMEM_16MB_PAGES"); }

	// Calculate page size.
	// Default            = 4KB
	// X_MEM_LARGE_PAGES  = 64KB
	// X_MEM_16MB_PAGES   = 16MB
	uint32 pageSize = 4 * 1024;
	if ( protect & xnative::XMEM_LARGE_PAGES )
	{
	    pageSize = 64 * 1024;
	}
	else if ( protect & xnative::XMEM_16MB_PAGES )
	{
	    pageSize = 16 * 1024 * 1024;
	}

	// Round up the region size and alignment to the next page.
	const uint32 adjustedSize = (size + (pageSize-1)) & ~(pageSize-1);
	const uint32 adjustedAlign = (align + (pageSize-1)) & ~(pageSize-1);
	if ( adjustedSize != size )
		GLog.Log( "XPhysicalAlloc: adjusted size %d->%d (%08X->%08X)", size, adjustedSize, size, adjustedSize );
	if ( adjustedAlign != align )
		GLog.Log( "XPhysicalAlloc: adjusted align %d->%d (%08X->%08X)", align, adjustedAlign, align, adjustedAlign );

	// allocate physical memory
	void* ptr = GPlatform.GetMemory().PhysicalAlloc( adjustedSize, adjustedAlign, protect );
	uint32 retAddr = (const uint32)ptr;
	//retAddr |= 0xA0000000;
	//0xFFC57000
		//0x00000000ffc56000

	GLog.Log( "XPhysicalAlloc: allocated %08Xh", retAddr );
	RETURN_ARG(retAddr);
}

uint64 __fastcall XboxVideo_MmGetPhysicalAddress( uint64 ip, cpu::CpuRegs& regs )
{
	const uint32 addr = (uint32)(regs.R3);
	GLog.Log( "MmGetPhysicalAddress: %08Xh", addr );
	RETURN_ARG( addr );
}

uint64 __fastcall XboxVideo_MmFreePhysicalMemory( uint64 ip, cpu::CpuRegs& regs )
{
	const uint32 type = (const uint32) regs.R3;
	const uint32 addr = (const uint32) regs.R4;
	GLog.Warn("MmFreePhysicalAddress(%d, %08X)", type, addr);
	RETURN_ARG( 0 );
}

//---------------------------------------------------------------------------

uint64 __fastcall XboxVideo_VdGetCurrentDisplayGamma( uint64 ip, cpu::CpuRegs& regs )
{
	const uint32 arg0_ptr = (const uint32) regs.R3;
	const uint32 arg1_ptr = (const uint32) regs.R4;

	GLog.Log("VdGetCurrentDisplayGamma(%08X, %08X)", arg0_ptr, arg1_ptr);
	mem::storeAddr<uint32>( arg0_ptr, 2 );
	mem::storeAddr<float>( arg0_ptr, 2.22222233f );
	RETURN_ARG(0);
}

uint64 __fastcall XboxVideo_VdGetCurrentDisplayInformation( uint64 ip, cpu::CpuRegs& regs )
{
	const uint32 ptr = (const uint32) regs.R3;
	GLog.Log( "VdGetCurrentDisplayInformation(%08X)", ptr );

	// experimental...
	mem::storeAddr<uint32>(ptr + 0, (1280 << 16) | 720);
	mem::storeAddr<uint32>(ptr + 4, 0);
	mem::storeAddr<uint32>(ptr + 8, 0);
	mem::storeAddr<uint32>(ptr + 12, 0);
	mem::storeAddr<uint32>(ptr + 16, 1280);  // backbuffer width?
	mem::storeAddr<uint32>(ptr + 20, 720);   // backbuffer height?
	mem::storeAddr<uint32>(ptr + 24, 1280);
	mem::storeAddr<uint32>(ptr + 28, 720);
	mem::storeAddr<uint32>(ptr + 32, 1);
	mem::storeAddr<uint32>(ptr + 36, 0);
	mem::storeAddr<uint32>(ptr + 40, 0);
	mem::storeAddr<uint32>(ptr + 44, 0);
	mem::storeAddr<uint32>(ptr + 48, 1);
	mem::storeAddr<uint32>(ptr + 52, 0);
	mem::storeAddr<uint32>(ptr + 56, 0);
	mem::storeAddr<uint32>(ptr + 60, 0);
	mem::storeAddr<uint32>(ptr + 64, 0x014000B4);          // ?
	mem::storeAddr<uint32>(ptr + 68, 0x014000B4);          // ?
	mem::storeAddr<uint32>(ptr + 72, (1280 << 16) | 720);  // actual display size?
	mem::storeAddr<uint32>(ptr + 76, 0x42700000);
	mem::storeAddr<uint32>(ptr + 80, 0);
	mem::storeAddr<uint32>(ptr + 84, 1280);  // display width
	RETURN_ARG(0);
}

uint64 __fastcall XboxVideo_VdQueryVideoFlags( uint64 ip, cpu::CpuRegs& regs )
{
	const uint32 videoFlags = 0x00000003; // widescreen + >=1024
	//const uint32 videoFlags = 0x00000007; // widescreen + >=1920
	RETURN_ARG(videoFlags);
}

uint64 __fastcall XboxVideo_VdQueryVideoMode( uint64 ip, cpu::CpuRegs& regs )
{
	xnative::XVIDEO_MODE* videoMode = (xnative::XVIDEO_MODE*) (uint32)regs.R3;
	if (videoMode != NULL)
	{
		memset( videoMode, 0, sizeof(xnative::XVIDEO_MODE) );
		videoMode->dwDisplayWidth.Set(1280);
		videoMode->dwDisplayHeight.Set(720);
		videoMode->fIsInterlaced.Set(0);
		videoMode->fIsWideScreen.Set(1);
		videoMode->fIsHiDef.Set(1);
		videoMode->RefreshRate.Set(60.0f);
		videoMode->VideoStandard.Set(1);
		videoMode->ReservedA.Set(0x4a);
		videoMode->ReservedB.Set(1);
	}

	RETURN_ARG(0);
}

uint64 __fastcall XboxVideo_VdInitializeEngines( uint64 ip, cpu::CpuRegs& regs )
{
	const uint32 unk0 = (const uint32) regs.R3;
	const uint32 callback = (const uint32) regs.R4;
	const uint32 unk1 = (const uint32) regs.R5;
	const uint32 unk2_ptr = (const uint32) regs.R6;
	const uint32 unk3_ptr = (const uint32) regs.R7;

	GLog.Log( "VdInitializeEngines(%08X, %08X, %08X, %08X, %08X)", unk0, callback, unk1, unk2_ptr, unk3_ptr);
	RETURN_ARG(0);
}

uint64 __fastcall XboxVideo_VdShutdownEngines( uint64 ip, cpu::CpuRegs& regs )
{
	GLog.Log( "VdShutdownEngines!" );
	RETURN_ARG(0);
}

uint64 __fastcall XboxVideo_VdSetGraphicsInterruptCallback( uint64 ip, cpu::CpuRegs& regs )
{
	const uint32 callback = (const uint32) regs.R3;
	const uint32 user_data = (const uint32) regs.R4;

	GLog.Log( "VdSetGraphicsInterruptCallback(%08X, %08X)", callback, user_data );

	// callback takes 2 params
	// r3 = bool 0/1 - 0 is normal interrupt, 1 is some acquire/lock mumble
	// r4 = user_data (r4 of VdSetGraphicsInterruptCallback)
	GPlatform.GetGraphics().SetInterruptCallbackAddr( callback, user_data );
	RETURN_ARG(0);
}

uint64 __fastcall XboxVideo_VdEnableRingBufferRPtrWriteBack( uint64 ip, cpu::CpuRegs& regs )
{
	const uint32 ptr = (const uint32) regs.R3;
	const uint32 blockSize = (const uint32) regs.R4;

	GLog.Log( "VdEnableRingBufferRPtrWriteBack(%08X, %08X)", ptr, blockSize);
	GPlatform.GetGraphics().EnableReadPointerWriteBack(ptr, blockSize);
	RETURN_ARG(0);
}

uint64 __fastcall XboxVideo_VdGetSystemCommandBuffer( uint64 ip, cpu::CpuRegs& regs )
{
	uint32* p0_ptr = GetPointer<uint32>( regs.R3 );
	uint32* p1_ptr = GetPointer<uint32>( regs.R4 );
	*p0_ptr = 0xBEEF0000;
	*p1_ptr = 0xBEEF0001;
	RETURN_ARG(0);
}

uint64 __fastcall XboxVideo_VdInitializeRingBuffer( uint64 ip, cpu::CpuRegs& regs )
{
	const uint32 ptr = (const uint32) regs.R3;
	const uint32 pageCount = (const uint32) regs.R4;

	GLog.Warn("VdInitializeRingBuffer(%08X, %d)", ptr, pageCount);

	// r3 = result of MmGetPhysicalAddress
	// r4 = number of pages? page size?
	//      0x8000 -> cntlzw=16 -> 0x1C - 16 = 12
	// Buffer pointers are from MmAllocatePhysicalMemory with WRITE_COMBINE.
	// Sizes could be zero? XBLA games seem to do this. Default sizes?
	// D3D does size / region_count - must be > 1024
	GPlatform.GetGraphics().InitializeRingBuffer( (void*) ptr, pageCount );
	RETURN_ARG(0);
}

uint64 __fastcall XboxVideo_VdIsHSIOTrainingSucceeded( uint64 ip, cpu::CpuRegs& regs )
{
	RETURN_ARG(1);
}

uint64 __fastcall XboxVideo_VdPersistDisplay( uint64 ip, cpu::CpuRegs& regs )
{
	RETURN_ARG(1);
}

uint64 __fastcall XboxVideo_VdRetrainEDRAMWorker( uint64 ip, cpu::CpuRegs& regs )
{
	const uint32 unk0 = (const uint32) regs.R3;
	GLog.Log("VdRetrainEDRAMWorker(%08X)", unk0);
	RETURN_ARG(0);
}

uint64 __fastcall XboxVideo_VdRetrainEDRAM( uint64 ip, cpu::CpuRegs& regs )
{
	const uint32 param0 = (const uint32) regs.R3;
	const uint32 param1 = (const uint32) regs.R4;
	const uint32 param2 = (const uint32) regs.R5;
	const uint32 param3 = (const uint32) regs.R6;
	const uint32 param4 = (const uint32) regs.R7;
	const uint32 param5 = (const uint32) regs.R8;
	GLog.Log("VdRetrainEDRAM(%08X, %08X, %08X, %08X, %08X, %08X)", param0, param1, param2, param3, param4, param5);
	RETURN_ARG(0);
}

uint64 __fastcall XboxVideo_VdSetDisplayMode( uint64 ip, cpu::CpuRegs& regs )
{
	GLog.Log("VdSetDisplayMod!");
	//GVideo.SetDisplayMod();
	RETURN_ARG(0);
}

uint32 GLastFrontBufferWidth = 0;
uint32 GLastFrontBufferHeight = 0;

uint64 __fastcall XboxVideo_VdSwap( uint64 ip, cpu::CpuRegs& regs )
{
	uint32 ringBufferPtr = (const uint32) regs.R3; // RB ?
	uint32 param1 = (const uint32) regs.R4;
	uint32 param2 = (const uint32) regs.R5;
	uint32 param3 = (const uint32) regs.R6;
	uint32 param4 = (const uint32) regs.R7;             // 0xBEEF0001
	uint32 fronBufferPtr = (const uint32) regs.R8;  // ptr to frontbuffer address
	uint32 param6 = (const uint32) regs.R9;
	uint32 param7 = (const uint32) regs.R10;

	uint32 frontbuffer = mem::loadAddr<uint32>(fronBufferPtr);

	GLog.Log( "VdSwap, frontBuffer=0x%08X, ringBuffer=0x%08X", frontbuffer, ringBufferPtr );

	// The caller seems to reserve 64 words (256b) in the primary ringbuffer
	// for this method to do what it needs. We just zero them out and send a
	// token value. It'd be nice to figure out what this is really doing so
	// that we could simulate it, though due to TCR I bet all games need to
	// use this method.
	memset( (uint32*)ringBufferPtr, 0, 64 * 4);

	static const uint32 PM4_HACK_SWAP = 0x42;
	mem::storeAddr<uint32>( ringBufferPtr + 0, (3<<30) | (62<<16) | (PM4_HACK_SWAP << 8) );
	mem::storeAddr<uint32>( ringBufferPtr + 4, frontbuffer);

	// Set by VdCallGraphicsNotificationRoutines.
	mem::storeAddr<uint32>( ringBufferPtr + 8, GLastFrontBufferWidth);
	mem::storeAddr<uint32>( ringBufferPtr + 12, GLastFrontBufferHeight);

	RETURN_ARG(0);	
}

uint64 __fastcall XboxVideo_VdEnableDisableClockGating( uint64 ip, cpu::CpuRegs& regs )
{
	const uint32 isEnabled = (const uint32) regs.R3;
	GLog.Log( "VdEnableDisableClockGating(%d)", isEnabled );

	RETURN_ARG(0);
}

uint64 __fastcall XboxVideo_VdCallGraphicsNotificationRoutines( uint64 ip, cpu::CpuRegs& regs )
{
	const uint32 param0 = (const uint32) regs.R3;
	const uint32 argsPtr = (const uint32) regs.R4;

	uint16 frontWidth = mem::loadAddr<uint16>(argsPtr + 0);
	uint16 frontHeight = mem::loadAddr<uint16>(argsPtr + 2);
	uint16 backWidth = mem::loadAddr<uint16>(argsPtr + 4);
	uint16 backHeight = mem::loadAddr<uint16>(argsPtr + 6);

	GLastFrontBufferWidth = frontWidth;
	GLastFrontBufferHeight = frontHeight;

	GLog.Log( "VdCallGraphicsNotificationRoutines(%d, scale %dx%d -> %dx%d)", param0, backWidth, backHeight, frontWidth, frontHeight );	
	RETURN_ARG(0);	
}

uint64 __fastcall XboxVideo_VdSetSystemCommandBufferGpuIdentifierAddress( uint64 ip, cpu::CpuRegs& regs )
{
	const uint32 param0 = (const uint32) regs.R3;
	GLog.Log( "VdSetSystemCommandBufferGpuIdentifierAddres(%d)", param0 );	
	RETURN_ARG(0);	
}

uint64 __fastcall XboxVideo_VdInitializeScalerCommandBuffer( uint64 ip, cpu::CpuRegs& regs )
{
	const uint32 param0 = (const uint32) regs.R3;  // 0?
	const uint32 param1 = (const uint32) regs.R4;  // 0x050002d0 size of ?
	const uint32 param2 = (const uint32) regs.R5;  // 0?
	const uint32 param3 = (const uint32) regs.R6;  // 0x050002d0 size of ?
	const uint32 param4 = (const uint32) regs.R7;  // 0x050002d0 size of ?
	const uint32 param5 = (const uint32) regs.R8;  // 7?
	const uint32 param6 = (const uint32) regs.R9;  // 0x2004909c <-- points to zeros?
	const uint32 param7 = (const uint32) regs.R10;  // 7?

	const uint32 destPtr = mem::loadAddr<uint32>( (const uint32)(regs.R1 + 0x54) ); // 
	GLog.Log( "VdInitializeScalerCommandBuffer(destPtr=0x%08X)", destPtr );

	// We could fake the commands here, but I'm not sure the game checks for
	// anything but success (non-zero ret).
	// For now, we just fill it with NOPs.
	const uint32 numWords = 0x1CC / 4;
	uint32* mem = (uint32*)destPtr;
	for ( uint32 i=0; i<numWords; ++i )
	{
		mem[i] = _byteswap_ulong(0x80000000);
	}

	// returns memcpy size >> 2 for memcpy(...,...,ret << 2)
	RETURN_ARG(numWords >> 2);
}

//---------------------------------------------------------------------------

void RegisterXboxVideo(runtime::Symbols& symbols)
{
	#define REGISTER(x) symbols.RegisterFunction(#x, (runtime::TBlockFunc) &XboxVideo_##x);
	REGISTER(XGetVideoMode);

	REGISTER(MmAllocatePhysicalMemoryEx);
	REGISTER(MmGetPhysicalAddress);
	REGISTER(MmFreePhysicalMemory);

	REGISTER(VdGetCurrentDisplayGamma);
	REGISTER(VdGetCurrentDisplayInformation);
	REGISTER(VdQueryVideoFlags);
	REGISTER(VdQueryVideoMode);
	REGISTER(VdInitializeEngines);
	REGISTER(VdShutdownEngines);
	REGISTER(VdSetGraphicsInterruptCallback);
	REGISTER(VdInitializeRingBuffer);
	REGISTER(VdEnableRingBufferRPtrWriteBack);
	REGISTER(VdGetSystemCommandBuffer);
	REGISTER(VdIsHSIOTrainingSucceeded);
	REGISTER(VdPersistDisplay);
	REGISTER(VdRetrainEDRAMWorker);
	REGISTER(VdRetrainEDRAM);
	REGISTER(VdCallGraphicsNotificationRoutines);
	REGISTER(VdSwap);
	REGISTER(VdSetDisplayMode);
	REGISTER(VdEnableDisableClockGating);
	REGISTER(VdSetSystemCommandBufferGpuIdentifierAddress);
	REGISTER(VdInitializeScalerCommandBuffer);
	#undef REGISTER
}