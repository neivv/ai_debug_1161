#include "selection.h"

Selection client_select(bw::client_selection_group);
Selection client_select3(bw::client_selection_group3);
Selection selections[8] =
{
    {bw::selection_groups[0]}, {bw::selection_groups[1]},
    {bw::selection_groups[2]}, {bw::selection_groups[3]},
    {bw::selection_groups[4]}, {bw::selection_groups[5]},
    {bw::selection_groups[6]}, {bw::selection_groups[7]},
};

Unit *Selection::Get(int pos)
{
    return *(Unit **)(addr + 4 * pos);
}

int Selection::Find(Unit *unit)
{
    for (unsigned i = 0; i < Limits::Selection; i++)
    {
        if (unit == addr[i])
            return i;
    }
    return -1;
}
