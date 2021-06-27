/* ========================================================================
   $File: $
   $Date: $
   $Revision: $
   $Creator: Alexander Baskakov $
   ======================================================================== */
#include <windows.h>
#include <stdint.h>
#include <time.h>

//#include "../..//win32_code_templates/win32codetemplates.h"
#include "../../win32_code_templates/win32codetemplates.cpp"


#define global_variable static;
#define internal_func static;

typedef int8_t i8;
typedef int16_t i16;
typedef int32_t i32;
typedef int64_t i64;

typedef i32 bool32;

typedef uint8_t ui8;
typedef uint16_t ui16;
typedef uint32_t ui32;
typedef uint64_t ui64;

typedef float real32;
typedef double real64;

//TODO: Debug color drawing
//TODO: Backwards movement restriction
//TODO: Border area restriction
//TODO: Tail eating game over

#define TARGET_CLIENT_RECT_WIDTH 900
#define TARGET_CLIENT_RECT_HEIGHT 600

global_variable ui32 GlobalGameSpeedInMilliseconds = 150;

global_variable i32 GlobalConstantMoveOffsetX = 0;
global_variable i32 GlobalConstantMoveOffsetY = 0;
global_variable i32 GlobalConstantTileMoveOffsetX = 0;
global_variable i32 GlobalConstantTileMoveOffsetY = 0;

global_variable bool GlobalRunning = false;

global_variable i32 GlobalClientRectWidth = 0;
global_variable i32 GlobalClientRectHeight = 0;

global_variable const i32 GlobalTileMapWidthInRects = 30;
global_variable const i32 GlobalTileMapHeightInRects = 20;
global_variable const i32 GlobalGameFieldWidthInRects = 28;
global_variable const i32 GlobalGameFieldHeightInRects = 18;

struct TileMapField{
    i32 TileMap[GlobalTileMapHeightInRects][GlobalTileMapWidthInRects];
    i32 TileWidthInPX = 0;
    i32 TileHeightInPX = 0;

    i32 GameFieldStartX = GlobalClientRectWidth / GlobalTileMapWidthInRects;
    i32 GameFieldStartY = GlobalClientRectHeight / GlobalTileMapHeightInRects;
    i32 GameFieldWidthInPX = GlobalClientRectWidth - (GlobalClientRectWidth / GlobalTileMapWidthInRects);
    i32 GameFieldHeightInPX = GlobalClientRectHeight - (GlobalClientRectHeight / GlobalTileMapHeightInRects);
}GameField;

struct SnakeEntity{
    i32 HeadPosX = 0;
    i32 HeadPosY = 0;
    i32 HeadWidthInPX = GlobalClientRectWidth / GlobalTileMapWidthInRects;
    i32 HeadHeightInPX = GlobalClientRectHeight / GlobalTileMapHeightInRects;

    i32 HeadTilePosX = 1;
    i32 HeadTilePosY = 1;

    i32 HeadLastTilePosX = 0;
    i32 HeadLastTilePosY = 0;

    struct TailEntity{
        i32 TailTilePosX = 0;
        i32 TailTilePosY = 0;

        i32 TailTileLastPosX = 0;
        i32 TailTileLastPosY = 0;
    };
    
    TailEntity SnakeTail[GlobalGameFieldWidthInRects
                         * GlobalGameFieldHeightInRects] = { };      
    i32 TailLength = 0;
}Snake;

struct FruitEntity{
    i32 FruitTilePosX = 1;
    i32 FruitTilePosY = 1;
}Fruit;

global_variable HWND GlobalWindowHandle = { };
global_variable HDC GlobalDCHandle = { };
global_variable BITMAPINFO BitmapInfo = { };
global_variable void* GlobalBackbufferMemory = 0;

internal_func void
Game_InitGameField(TileMapField* Field)
{
    TileMapField TempField;
    *Field = TempField;
}

internal_func void
Game_InitSnakeEntity(SnakeEntity* Entity)
{
    SnakeEntity TempEntity;
    *Entity = TempEntity;
}

internal_func void
Win32_GetWindowRectDim(i32* WindowRectWidth, i32* WindowRectHeight)
{
    RECT Rect = { };
    GetWindowRect(GlobalWindowHandle, &Rect);
    *WindowRectWidth = Rect.right - Rect.left;
    *WindowRectHeight = Rect.bottom - Rect.top;
}

internal_func void
Win32_GetClientRectDim(i32* ClientRectWidth, i32* ClientRectHeight)
{
    RECT Rect = { };
    GetClientRect(GlobalWindowHandle, &Rect);
    *ClientRectWidth = Rect.right - Rect.left;
    *ClientRectHeight = Rect.bottom - Rect.top;
}

internal_func void
Win32_ResizeClientRectToTargetRes(i32 TargetWidth, i32 TargetHeight)
{
    i32 CurrentClientRectWidth = 0;
    i32 CurrentClientRectHeight = 0;

    i32 CurrentWindowRectWidht = 0;
    i32 CurrentWindowRectHeight = 0;

    Win32_GetClientRectDim(&CurrentClientRectWidth, &CurrentClientRectHeight);
    Win32_GetWindowRectDim(&CurrentWindowRectWidht, &CurrentWindowRectHeight);

    i32 ClientRectOffsetWidth = CurrentWindowRectWidht - CurrentClientRectWidth;
    i32 ClientRectOffsetHeight = CurrentWindowRectHeight - CurrentClientRectHeight;

    MoveWindow(GlobalWindowHandle, 0, 0 ,
               TargetWidth + ClientRectOffsetWidth,
               TargetHeight + ClientRectOffsetHeight,
               TRUE);  
}

internal_func void
Win32_ClearBackbufferWithBlack()
{
    Pixel32RGB Pixel = { };
    for(i32 Y = 0; Y < GlobalClientRectHeight; Y++)
    {
        for(i32 X = 0; X < GlobalClientRectWidth; X++)
        {
            Win32_DrawPixelToBitmap(GlobalBackbufferMemory, GlobalClientRectWidth,
                                    X, Y, &Pixel);
        }
    }
}

internal_func void
Game_DrawRectangle(i32 TargetX, i32 TargetY,
                   i32 RectWidth, i32 RectHeight, Pixel32RGB* Pixel)
{
    for(i32 Y = TargetY; Y < TargetY + RectHeight; Y++)
    {
        for(i32 X = TargetX; X < TargetX + RectWidth; X++)
        {
            Win32_DrawPixelToBitmap(GlobalBackbufferMemory, GlobalClientRectWidth,
                                    X, Y, Pixel);
        }
    }
}

internal_func void
Game_GetTileMapInfo(TileMapField* GameField, i32 TileMapWidthInRects, i32 TileMapHeightInRects)
{
    GameField->TileWidthInPX = GlobalClientRectWidth / TileMapWidthInRects;
    GameField->TileHeightInPX = GlobalClientRectHeight / TileMapHeightInRects;
}

internal_func void
Game_DrawTileMapBorder(TileMapField* GameField, Pixel32RGB* BorderColor)
{
    //Drawing horizontal border
    for(i32 Y = 0; Y < GlobalClientRectHeight; Y += GlobalClientRectHeight - GameField->TileHeightInPX)
    {
        for(i32 X = 0; X < GlobalClientRectWidth; X += GameField->TileWidthInPX)
        {
            Game_DrawRectangle(X, Y,
                               GameField->TileWidthInPX, GameField->TileHeightInPX, BorderColor);
        }
    }

    //Drawing vertical border
    for(i32 X = 0; X < GlobalClientRectWidth; X += GlobalClientRectWidth - GameField->TileWidthInPX)
    {
        for(i32 Y = 0; Y < GlobalClientRectHeight; Y += GameField->TileHeightInPX)
        {
            Game_DrawRectangle(X, Y,
                               GameField->TileWidthInPX, GameField->TileHeightInPX, BorderColor);
        }
    }
}

internal_func void
Game_DrawTargetTileWithColor(TileMapField* GameField, i32 TileX, i32 TileY, Pixel32RGB* Color)
{
    i32 TargetTileX = TileX * GameField->TileWidthInPX;
    i32 TargetTileY = TileY * GameField->TileHeightInPX;

    Game_DrawRectangle(TargetTileX, TargetTileY,
                       GameField->TileWidthInPX, GameField->TileHeightInPX, Color);
}

internal_func void
Game_DrawSnakeHeadToXY(TileMapField* TileMap, SnakeEntity* Snake,
                       i32 TargetX, i32 TargetY, Pixel32RGB* HeadColor)
{
    Game_DrawRectangle(TargetX, TargetY, Snake->HeadWidthInPX, Snake->HeadHeightInPX, HeadColor);
}

internal_func void
Game_DrawSnake(TileMapField* GameField, SnakeEntity* Snake, Pixel32RGB* SnakeColor)
{
    Game_DrawTargetTileWithColor(GameField,
                                 Snake->HeadTilePosX, Snake->HeadTilePosY, SnakeColor);
}

internal_func void
Game_DrawFruit(TileMapField* GameField, FruitEntity* Fruit, Pixel32RGB* FruitColor)
{
    Game_DrawTargetTileWithColor(GameField,
                                 Fruit->FruitTilePosX, Fruit->FruitTilePosY, FruitColor);
}

internal_func void
Game_GenerateRandomFruitInTile(SnakeEntity* Snake, FruitEntity* Fruit)
{
    i32 NextFruitX = rand() % GlobalGameFieldWidthInRects + 1;
    i32 NextFruitY = rand() % GlobalGameFieldHeightInRects + 1;
    bool CorrectFruitPlacement = false;
    while(!CorrectFruitPlacement)
    {
        //Check if the fruit spawn within the snake
        for(i32 TailIndex = 0; TailIndex < Snake->TailLength; TailIndex++)
        {
            if(NextFruitX == Snake->SnakeTail[TailIndex].TailTilePosX
               && NextFruitY == Snake->SnakeTail[TailIndex].TailTilePosY)
            {
                NextFruitX = rand() % GlobalGameFieldWidthInRects + 1;
                NextFruitY = rand() % GlobalGameFieldHeightInRects + 1;
                CorrectFruitPlacement = false;
            }
            else
            {
                CorrectFruitPlacement = true;
            }
        }
    }
    
    Fruit->FruitTilePosX = NextFruitX;
    Fruit->FruitTilePosY = NextFruitY;
}

internal_func void
Game_GrowSnakeTail(SnakeEntity* Snake)
{
    Snake->TailLength++;
    
}

internal_func void
Game_UpdateSnakeTail(SnakeEntity* Snake)
{
    for(i32 TailIndex = 0; TailIndex < Snake->TailLength; TailIndex++)
    {
        Snake->SnakeTail[TailIndex].TailTilePosX = Snake->HeadLastTilePosX;
        Snake->SnakeTail[TailIndex].TailTilePosY = Snake->HeadLastTilePosY;

        Snake->SnakeTail[TailIndex].TailTileLastPosX =
            Snake->SnakeTail[TailIndex].TailTilePosX;
        Snake->SnakeTail[TailIndex].TailTileLastPosY =
            Snake->SnakeTail[TailIndex].TailTilePosY;

        if((Snake->TailLength > 0) | ((TailIndex + 1) < Snake->TailLength))
        {
            Snake->SnakeTail[TailIndex + 1].TailTilePosX =
                Snake->SnakeTail[TailIndex].TailTileLastPosX;
            Snake->SnakeTail[TailIndex + 1].TailTilePosY =
                Snake->SnakeTail[TailIndex].TailTileLastPosY;
        }

    }
}

internal_func void
Game_DrawSnakeTail(TileMapField* GameField, SnakeEntity* Snake, Pixel32RGB* TailColor)
{
    for(i32 TailIndex = 0; TailIndex < Snake->TailLength; TailIndex++)
    {
        if(Snake->SnakeTail[TailIndex].TailTilePosX != 0
           && Snake->SnakeTail[TailIndex].TailTilePosY != 0)
        {
         Game_DrawTargetTileWithColor(GameField,
                                 Snake->SnakeTail[TailIndex].TailTilePosX,
                                 Snake->SnakeTail[TailIndex].TailTilePosY, TailColor);

        }
    }
}

internal_func bool
Game_CheckTailEatingCollision(TileMapField* GameField, SnakeEntity* Snake)
{
    bool CollisionExists = false;
    for(ui32 TailIndex = 1; TailIndex < Snake->TailLength; TailIndex++)
    {
        if(Snake->HeadTilePosX == Snake->SnakeTail[TailIndex].TailTilePosX
           && Snake->HeadTilePosY == Snake->SnakeTail[TailIndex].TailTilePosY)
        {
            CollisionExists = true;
        }
    }
    
    return CollisionExists;
}

//Get rid of static numbers
internal_func bool
Game_CheckSnakeBorderCollision(TileMapField* GameField, SnakeEntity* Snake)
{
    bool CollisionExists = false;
    if(Snake->HeadTilePosX == 0 | Snake->HeadTilePosX == 29
       | Snake->HeadTilePosY == 0 | Snake-> HeadTilePosY == 19)
    {
        CollisionExists = true;
    }
    return CollisionExists;
}
    
LRESULT CALLBACK
WindowMessageHandlerProcedure(HWND WindowHandle, UINT Message,
           WPARAM wParam, LPARAM lParam)
{
    LRESULT Result = { };

    switch(Message)
    {
        case WM_KEYDOWN:
        {
            switch(wParam)
            {
                case VK_RETURN:
                {
                    GlobalConstantTileMoveOffsetX = 1;
                    
                }break;
                
                case VK_RIGHT:
                {
                    GlobalConstantTileMoveOffsetX = 1;
                    GlobalConstantTileMoveOffsetY = 0;
                }break;

                case VK_LEFT:
                {
                    GlobalConstantTileMoveOffsetX = -1;
                    GlobalConstantTileMoveOffsetY = 0;

                }break;

                case VK_UP:
                {
                    GlobalConstantTileMoveOffsetX = 0;
                    GlobalConstantTileMoveOffsetY = -1;

                }break;

                case VK_DOWN:
                {
                    GlobalConstantTileMoveOffsetX = 0;
                    GlobalConstantTileMoveOffsetY = 1;

                }break;
                
            }
        }break;
    
        case WM_PAINT:
        {
            PAINTSTRUCT PaintStruct;
            BeginPaint(WindowHandle, &PaintStruct);

            

            EndPaint(WindowHandle, &PaintStruct);
            OutputDebugStringA("WM_PAINT\n");
        }break;
        
        case WM_SIZE:
        {
            Win32_GetClientRectDim(&GlobalClientRectWidth, &GlobalClientRectHeight);
            OutputDebugStringA("WM_SIZE\n");
        }break;

        case WM_CLOSE:
        {
            GlobalRunning = false;
        }break;
        
        default:
        {
            Result = DefWindowProc(WindowHandle, Message, wParam, lParam);
        }break;
    }

    return Result;
}

int CALLBACK
WinMain(HINSTANCE WindowInstance, HINSTANCE PrevWindowInstance,
        LPSTR CommandLine, int LineArgs)
{   
    WNDCLASSA WindowClass = { };
    WindowClass.style = CS_HREDRAW|CS_VREDRAW|CS_OWNDC;
    WindowClass.lpfnWndProc = WindowMessageHandlerProcedure;
    WindowClass.hInstance = WindowInstance;
    WindowClass.hCursor = 0; //Check cursor
    WindowClass.lpszClassName = "AsteroidsGameClassName";

    RegisterClass(&WindowClass);
    
    
    GlobalWindowHandle = CreateWindowEx(0,
                                  WindowClass.lpszClassName,
                                  "Win32AsteroidsGame",
                                  WS_OVERLAPPEDWINDOW,
                                  CW_USEDEFAULT, CW_USEDEFAULT,
                                  CW_USEDEFAULT, CW_USEDEFAULT,
                                  NULL,
                                  NULL, //Menu
                                  WindowInstance,
                                  NULL);

    if(GlobalWindowHandle)
    {
        Win32_ResizeClientRectToTargetRes(TARGET_CLIENT_RECT_WIDTH,TARGET_CLIENT_RECT_HEIGHT);
        Win32_GetClientRectDim(&GlobalClientRectWidth, &GlobalClientRectHeight);

        GlobalBackbufferMemory = Win32_GetBitmapMemory(&BitmapInfo, GlobalBackbufferMemory, 4,
                              GlobalClientRectWidth, GlobalClientRectHeight);

        Game_InitGameField(&GameField);
        Game_InitSnakeEntity(&Snake);

        GlobalDCHandle = GetDC(GlobalWindowHandle);

        
        ShowWindow(GlobalWindowHandle, LineArgs);
        GlobalRunning = true;

        //Snake.HeadPosX = 30;
        //Snake.HeadPosY = 30;

        //GlobalConstantMoveOffsetX = 0;
        //GlobalConstantMoveOffsetY = 0;

        GlobalConstantTileMoveOffsetX = 0;
        GlobalConstantTileMoveOffsetY = 0;
        //time init
        srand(time(NULL));
        
        while(GlobalRunning)
        {
            Win32_ClearBackbufferWithBlack();
            MSG Message = { };
            while(PeekMessage(&Message, GlobalWindowHandle, 0, 0, PM_REMOVE))
            {
                TranslateMessage(&Message);
                 DispatchMessage(&Message);
            }
            
            
            Game_GetTileMapInfo(&GameField, GlobalTileMapWidthInRects, GlobalTileMapHeightInRects);

            Pixel32RGB TileMapBorderColor = {255, 0, 0};
            Pixel32RGB SnakeHeadColor = {0, 255, 0};
            Pixel32RGB SnakeTailColor = {0, 90, 0};
            Pixel32RGB FruitColor = {255, 255, 255};
                      
            Game_DrawTileMapBorder(&GameField, &TileMapBorderColor);

            
            Snake.HeadLastTilePosX = Snake.HeadTilePosX;
            Snake.HeadLastTilePosY = Snake.HeadTilePosY;

            
            Snake.HeadTilePosX += GlobalConstantTileMoveOffsetX;
            Snake.HeadTilePosY += GlobalConstantTileMoveOffsetY;
            
            if(Snake.TailLength > 0)
            {
                
                
                    Snake.SnakeTail[0].TailTileLastPosX =
                        Snake.SnakeTail[0].TailTilePosX;
                    Snake.SnakeTail[0].TailTileLastPosY =
                        Snake.SnakeTail[0].TailTilePosY;

                    Snake.SnakeTail[0].TailTilePosX =
                        Snake.HeadTilePosX;
                    Snake.SnakeTail[0].TailTilePosY =
                        Snake.HeadTilePosY;

                    Snake.HeadLastTilePosX = Snake.HeadTilePosX;
                    Snake.HeadLastTilePosY = Snake.HeadTilePosY;
                
                    for(i32 TailIndex = 1; TailIndex < Snake.TailLength; TailIndex++)
                    {
                        Snake.SnakeTail[TailIndex].TailTileLastPosX =
                            Snake.SnakeTail[TailIndex].TailTilePosX;
                        Snake.SnakeTail[TailIndex].TailTileLastPosY =
                            Snake.SnakeTail[TailIndex].TailTilePosY;

                        Snake.SnakeTail[TailIndex].TailTilePosX =
                            Snake.SnakeTail[TailIndex - 1].TailTileLastPosX;
                        Snake.SnakeTail[TailIndex].TailTilePosY =
                            Snake.SnakeTail[TailIndex - 1].TailTileLastPosY;
                    }    
                
                
            }
            
            if(Snake.HeadTilePosX == Fruit.FruitTilePosX
               && Snake.HeadTilePosY == Fruit.FruitTilePosY)
            {
                Game_GrowSnakeTail(&Snake);     
                Game_GenerateRandomFruitInTile(&Snake, &Fruit);
                GlobalGameSpeedInMilliseconds -= 2;
            }
            // Game_UpdateSnakeTail(&Snake);


            if(Game_CheckTailEatingCollision(&GameField, &Snake))
            {
                OutputDebugStringA("Game end cause by tail eating\n");
                GlobalRunning = false;
                break; //get rid of this break;
            }
         
            if(Game_CheckSnakeBorderCollision(&GameField, &Snake))
            {
                OutputDebugStringA("Game end cause by border collision\n");
                GlobalRunning = false;
                break; //get rid of this break;
            }

            
            Game_DrawSnakeTail(&GameField, &Snake, &SnakeTailColor);
            Game_DrawSnake(&GameField, &Snake, &SnakeHeadColor);
            Game_DrawFruit(&GameField, &Fruit, &FruitColor);

            
            Win32_DrawDIBSectionToScreen(&GlobalDCHandle, 0, 0, GlobalClientRectWidth, GlobalClientRectHeight,
                                         0, 0, GlobalClientRectWidth, GlobalClientRectHeight,
                                         GlobalBackbufferMemory, &BitmapInfo, DIB_RGB_COLORS, SRCCOPY);
            //TEST AREA//
            /*
            RECT Rect;
            Rect.left = 100;
            Rect.top = 100;
            Rect.right = 300;
            Rect.bottom =200;

            LPCTSTR TestTextArea = "Using this text";
            int TextAreaSize = 20;

            DrawText(GlobalDCHandle, TestTextArea, TextAreaSize, &Rect, DT_CENTER);
            */
            //TEST AREA//
            
            Sleep(GlobalGameSpeedInMilliseconds);
        }
        ReleaseDC(GlobalWindowHandle, GlobalDCHandle);
        Win32_ReleaseBitmapMemory(GlobalBackbufferMemory); 
    }

    

    return 0;
}
