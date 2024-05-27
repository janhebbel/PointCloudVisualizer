#include <stdint.h>

typedef int32_t b32;

static b32 KeyDown[256];
static b32 KeyWasDown[256];

static void
KeyEvent(int Key, b32 Down)
{
    KeyWasDown[Key] = KeyDown[Key];
    KeyDown[Key] = Down;
}

static void
UpdateKeyWasDown(void)
{
    for(int Key = 0; Key < 256; ++Key)
    {
        KeyEvent(Key, KeyDown[Key]);
    }
}

static void
ClearKeyboardState(void)
{
    for (int Key = 0; Key < 256; ++Key)
    {
        KeyDown[Key] = 0;
        KeyWasDown[Key] = 0;
    }
}

static b32
IsDown(int Key)
{
    return(KeyDown[Key]);
}

static b32
IsDownOnce(int Key)
{
    b32 Result = (KeyDown[Key] && !KeyWasDown[Key]);
    return(Result);
}
