#include "ai.h"

#include "offsets.h"

using std::get;

namespace Ai
{

Region *GetRegion(int player, int region)
{
    return bw::player_ai_regions[player] + region;
}
} // namespace ai
