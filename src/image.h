#ifndef IMAGE_H
#define IMAGE_H

#include "types.h"

#include "dat.h"
#include "iscript.h"
#include "list.h"
#include "offsets.h"

namespace ImageFlags
{
    const int Redraw = 0x1;
    const int Flipped = 0x2;
    const int FreezeY = 0x4;
    const int CanTurn = 0x8;
    const int FullIscript = 0x10;
    const int Clickable = 0x20;
    const int Hidden = 0x40;
    const int UseParentLo = 0x80;
}

#pragma pack(push)
#pragma pack(1)

struct CycleStruct
{
    uint8_t active;
    uint8_t speed; // Frames between updates
    uint8_t update_counter;
    uint8_t palette_entry_low;
    uint8_t adv_cycle_pos;
    uint8_t palette_entry_high;
    uint8_t _padding6[0x2];
    void *advanced_cycle_data;
    uint8_t adv_cycle_count;
    uint8_t _paddingd[0x3];
};

class GrpFrameHeader
{
    public:
        bool IsDecoded() const;
        uint8_t GetPixel(x32 x, y32 y) const;
        // Includes possible decoding padding
        int GetWidth() const;

        uint8_t x;
        uint8_t y;
        uint8_t w;
        uint8_t h;
        // If first two pixels of the frame are zero, it is considered decoded image
        uint8_t *frame;
};

struct ImgRenderFuncs
{
    uint32_t id;
    void (__fastcall *nonflipped)(int, int, GrpFrameHeader *, Rect32 *, void *);
    void (__fastcall *flipped)(int, int, GrpFrameHeader *, Rect32 *, void *);
};

struct ImgUpdateFunc
{
    uint32_t id;
    void (__fastcall *func)(Image *);
};

struct BlendPalette
{
    uint32_t id;
    uint8_t *data;
    char name[0xc];
};

struct GrpSprite
{
    uint16_t frame_count;
    x16u width;
    y16u height;
};

class Image
{
    public:
        RevListEntry<Image, 0x0> list;
        uint16_t image_id;
        uint8_t drawfunc;
        uint8_t direction;
        uint16_t flags;
        int8_t x_off;
        int8_t y_off;
        Iscript::Script iscript;
        uint16_t frameset;
        uint16_t frame;
        Point map_position;
        Point screen_position;
        Rect16 grp_bounds;
        GrpSprite *grp;
        void *drawfunc_param;
        void (__fastcall *Render)(int, int, GrpFrameHeader *, Rect32 *, void *);
        void (__fastcall *Update)(Image *); // Unused, used to do what DrawFunc_ProgressFrame does
        Sprite *parent; // 0x3c

        // ---------------

};

static_assert(sizeof(CycleStruct) == 0x10, "sizeof(CycleStruct)");

#pragma pack(pop)

#endif // IMAGE_H

