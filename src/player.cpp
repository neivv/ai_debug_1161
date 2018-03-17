#include "player.h"

#include "constants/order.h"
#include "ai.h"
#include "limits.h"
#include "offsets.h"
#include "order.h"
#include "unit.h"
#include "yms.h"

bool IsHumanPlayer(int player)
{
    if (player >= 0 && player < Limits::ActivePlayers)
        return bw::players[player].type == 2;
    else
        return false;
}

bool IsComputerPlayer(int player)
{
    if (player >= 0 && player < Limits::ActivePlayers)
        return bw::players[player].type == 1;
    else
        return false;
}

bool HasEnemies(int player)
{
    Assert(player >= 0 && player < Limits::Players);
    for (int i = 0; i < Limits::ActivePlayers; i++)
    {
        if (bw::alliances[player][i] == 0)
            return true;
    }
    return false;
}

static bool IsActive(int player)
{
    int type = bw::players[player].type;
    switch (type)
    {
        case 1:
        case 2:
            return bw::victory_status[player] == 0;
        case 3:
        case 4:
        case 7:
            return true;
        default:
            return false;
    }
}

int ActivePlayerIterator::NextActivePlayer(int beg)
{
    for (int i = beg + 1; i < 8; i++)
    {
        if (IsActive(i))
            return i;
    }
    return -1;
}

int NetPlayerToGame(int net_player)
{
    for (int i = 0; i < Limits::ActivePlayers; i++)
    {
        if (bw::players[i].storm_id == net_player)
            return i;
    }
    return -1;
}

