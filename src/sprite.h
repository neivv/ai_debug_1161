#ifndef SPRITE_H
#define SPRITE_H

#include "types.h"

#include <tuple>

#include "common/iter.h"
#include "dat.h"
#include "image.h"
#include "list.h"
#include "rng.h"
#include "offsets.h"
#include "unsorted_list.h"

namespace SpriteFlags
{
    const int HasSelectionCircle = 0x1;
    const int DashedSelectionMask = 0x6; // Team can have max 4 players, so max 0..3 dashed circles
    const int HasHealthBar = 0x8;
    const int Unk10 = 0x10; // Draw sort?
    const int Hidden = 0x20;
    const int Unk40 = 0x40; // Became uncloaked?
    const int Nobrkcodestart = 0x80;
}

#pragma pack(push)
#pragma pack(1)

class Sprite
{
    friend class LoneSpriteSystem;
    public:
        RevListEntry<Sprite, 0x0> list;
        uint16_t sprite_id;
        uint8_t player;
        uint8_t selectionIndex;
        uint8_t visibility_mask;
        uint8_t elevation;
        uint8_t flags;

        uint8_t selection_flash_timer;
        uint16_t index;
        uint8_t width;
        uint8_t height;
        Point position;
        Image *main_image;
        RevListHead<Image, 0x0> first_overlay;
        ListHead<Image, 0x0> last_overlay;
};


#pragma pack(pop)

#endif // SPRITE_H

