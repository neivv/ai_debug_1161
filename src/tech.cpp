#include "tech.h"

#include "constants/tech.h"
#include "offsets.h"


int GetTechLevel(TechType tech_, int player)
{
    int tech = tech_.Raw();
    Assert(tech >= 0 && tech < TechId::None.Raw());
    if (tech < 0x18)
        return bw::tech_level_sc[player][tech];
    else
        return bw::tech_level_bw[player][(tech - 0x18)];
}

void SetTechLevel(TechType tech_, int player, int amount)
{
    int tech = tech_.Raw();
    Assert(tech >= 0 && tech < TechId::None.Raw());
    if (tech < 0x18)
        bw::tech_level_sc[player][tech] = amount;
    else
        bw::tech_level_bw[player][(tech - 0x18)] = amount;
}
