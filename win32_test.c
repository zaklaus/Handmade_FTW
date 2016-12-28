// (c) 2016 ZaKlaus; All Rights Reserved

#define HANDMADE_SLOW

#include "hftw.h"

#include "formats/hformat_4ds.h"
#include "formats/hformat_klz.h"
#include "formats/hformat_bmp.h"

global_variable b32 Running = 1;
global_variable window_bitmap WindowBitmap;

internal void
RenderGradient(s32 XOffset, s32 YOffset)
{
    s32 Width = WindowBitmap.Width;
    s32 Height = WindowBitmap.Height;
    s32 BytesPerPixel = WindowBitmap.BPP;
    s32 Pitch = Width * BytesPerPixel;
    u8 *Row = (u8 *)WindowBitmap.BitmapMemory;
    for(s32 Y = 0;
        Y < Height;
        ++Y)
    {
        u32 *Pixel = (u32 *)Row;
        for(s32 X = 0;
            X < Width;
            ++X)
        {
            u8 Blue = (u8)(X + XOffset);
            u8 Green = (u8)(Y + YOffset);
            *Pixel++ = ((Green << 8) | Blue);
            }
        Row += Pitch;
    }
}

internal void
RenderPixel(v3 Color, v2 Position, s32 Margin)
{
    s32 Width = WindowBitmap.Width;
    s32 Height = WindowBitmap.Height;
    s32 BytesPerPixel = WindowBitmap.BPP;
    s32 Pitch = Width * BytesPerPixel;
    u32 *Memory = (u32 *)WindowBitmap.BitmapMemory;
    
    if(!Margin)
    {
        s32 X = (s32)Position.X;
        s32 Y = (s32)Position.Y;
        u8 Red = (u8)Color.R;
        u8 Green = (u8)Color.G;
        u8 Blue = (u8)Color.B;
        Memory[Y*Width+X] = ((Red << 16) | (Green << 8) | Blue);
        return;
    }
    
    for(s32 Y = 0;
        Y < Height;
        ++Y)
    {
        for(s32 X = 0;
            X < Width;
            ++X)
        {
            // TODO(zaklaus): Replace with direct write!
            if((Position.X >= X - Margin && Position.X <= X + Margin) &&
               (Position.Y >= Y - Margin && Position.Y <= Y + Margin))
            {
                u8 Red = (u8)Color.R;
                u8 Green = (u8)Color.G;
                u8 Blue = (u8)Color.B;
                Memory[Y*Width+X] = ((Red << 16) | (Green << 8) | Blue);
            }
        }
    }
}

internal void
RenderBlank(u8 Red, u8 Green, u8 Blue)
{
    s32 Width = WindowBitmap.Width;
    s32 Height = WindowBitmap.Height;
    s32 BytesPerPixel = WindowBitmap.BPP;
    s32 Pitch = Width * BytesPerPixel;
    u8 *Row = (u8 *)WindowBitmap.BitmapMemory;
    for(s32 Y = 0;
        Y < Height;
        ++Y)
    {
        u32 *Pixel = (u32 *)Row;
        for(s32 X = 0;
            X < Width;
            ++X)
        {
            *Pixel++ = ((Red << 16) | (Green << 8) | Blue);
        }
        Row += Pitch;
    }
}

internal void
RenderBMPImage(s32 sX, s32 sY, hformat_bmp *Image)
{
    s32 Width = Image->Header.Width;
    s32 Height = Image->Header.Height;
    u8 *Memory = (u8 *)WindowBitmap.BitmapMemory;
    u8 *ImageMemory = Image->Data;
    s32 BytesPerPixel = WindowBitmap.BPP;
    s32 Pitch = WindowBitmap.Width * BytesPerPixel;
    u8 *Row = (u8 *)WindowBitmap.BitmapMemory;
    
    for(s32 Y = 0;
        Y < Height;
        ++Y)
    {
        u32 *Pixel = (u32 *)Row;
        for(s32 X = 0;
            X < Width;
            ++X)
        {
            u8 Blue = *ImageMemory++;
            u8 Green = *ImageMemory++;
            u8 Red = *ImageMemory++;
            u32 Data = ((Red << 16) | (Green << 8) | Blue);
            
            *Pixel++ = Data;
        }
        Row += Pitch;
    }
}

LRESULT CALLBACK
WndProc(HWND Window,
        UINT Message,
        WPARAM WParam,
        LPARAM LParam)
{
    switch(Message)
    {
        case WM_ACTIVATEAPP:
        case WM_SIZE:
        {
            window_dim Dim = WindowGetClientRect(Window);
            
            window_resize_result ResizeResult = WindowResize(Dim.X, Dim.Y, WindowBitmap);
            
            WindowBitmap = ResizeResult;
            
            WindowBlit(Window, &WindowBitmap);
        }break;
        
        case WM_CLOSE:
        case WM_DESTROY:
        {
            Running = 0;
        }break;
        
        case WM_PAINT:
        {
            
        }break;
        
        default:
        {
            
        }break;
    }
    return(DefWindowProc(Window, Message, WParam, LParam));
}

int CALLBACK
WinMain(HINSTANCE Instance,
        HINSTANCE PrevInstance,
        LPSTR CmdLine,
        int CmdShow)
{
    WindowCreateClass(Instance, "Handmade FTW", &WndProc);
    
    window_dim PosDim = {0};
    window_dim ResDim = {0};
    ResDim.X = 800;
    ResDim.Y = 600;
    
    HWND Window;
    WindowCreateWindowed("Handmade FTW", "Win32 Test", Instance, 0, 0, ResDim, PosDim, CW_USEDEFAULT, &Window);
    
    window_dim Dim = WindowGetClientRect(Window);
    window_resize_result ResizeResult = WindowResize(Dim.X, Dim.Y, WindowBitmap);
    
    WindowBitmap = ResizeResult;
    
    WindowUpdate();
    TimeInit();
    RenderBlank(255, 255, 255);
    
    r64 OldTime = TimeGet();
    
    s32 FileIndex = IOFileOpenRead("data/test.bmp", 0);
    hformat_bmp *Image = HFormatLoadBMPImage(FileIndex, 0);
    IOFileClose(FileIndex);
    
#if 1
    {
    // NOTE(zaklaus): RLE test code!
    ms FileSize;
    s32 FileIndex2 = IOFileOpenRead("data/test.bmp", &FileSize);
    {
        u8 *FileData = PlatformMemAlloc(FileSize);
        IOFileRead(FileIndex2, FileData, FileSize);
        henc_rle Data = HENCCompressRLEMemory(FileData, FileSize);
        
        s32 OutFile = IOFileOpenWrite("data/test.rle");
        IOFileWrite(OutFile, Data.Memory, Data.MemorySize);
        IOFileClose(OutFile);
        PlatformMemFree(Data.Memory);
        
        ms InSize;
        s32 InFile = IOFileOpenRead("data/test.rle", &InSize);
        FileData = PlatformMemAlloc(InSize);
        IOFileRead(InFile, FileData, InSize);
        henc_rle BMP = HENCDecompressRLEMemory(FileData, InSize);
        
        OutFile = IOFileOpenWrite("data/test_rle.bmp");
        IOFileWrite(OutFile, BMP.Memory, BMP.MemorySize);
        IOFileClose(OutFile);
    }
    IOFileClose(FileIndex2);
}
#endif
    
    #if 1
{
    // NOTE(zaklaus): LZ test code!
     ms FileSize;
    s32 FileIndex2 = IOFileOpenRead("data/test.bmp", &FileSize);
    {
        u8 *FileData = PlatformMemAlloc(FileSize);
        IOFileRead(FileIndex2, FileData, FileSize);
        henc_lz Data = HENCCompressLZMemory(FileData, FileSize);
        
        s32 OutFile = IOFileOpenWrite("data/test.lz");
        IOFileWrite(OutFile, Data.Memory, Data.MemorySize);
        IOFileClose(OutFile);
        PlatformMemFree(Data.Memory);
        
         ms InSize;
        s32 InFile = IOFileOpenRead("data/test.lz", &InSize);
        FileData = PlatformMemAlloc(InSize);
        IOFileRead(InFile, FileData, InSize);
        henc_lz BMP = HENCDecompressLZMemory(FileData, InSize);
        
        OutFile = IOFileOpenWrite("data/test_lz.bmp");
        IOFileWrite(OutFile, BMP.Memory, BMP.MemorySize);
        IOFileClose(OutFile);
    }
    IOFileClose(FileIndex2);
}
    #endif
    
    while(0)//Running)
    {
        r64 NewTime = TimeGet();
        r64 DeltaTime = NewTime - OldTime;
        {
            WindowUpdate();
            {
                local_persist s32 XOffset = 0;
                local_persist s32 YOffset = 0;
                
                RenderBMPImage(0, 0, Image);
                
                ++XOffset;
                ++YOffset;
                    
                    if(XOffset > WindowBitmap.Width)
                    {
                        XOffset = 0;
                    }
                }
            WindowBlit(Window, &WindowBitmap);
            
            Sleep(10);
        }
        OldTime = NewTime;
    }
    
    return(0);
}
