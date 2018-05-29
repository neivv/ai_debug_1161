#include "strings.h"

const char *Tbl::GetTblString(int index) const
{
    if (index == 0 || index > entries)
        return "(No string)";

    uint16_t offset = *(&first_offset + index - 1);
    return ((const char *)&entries) + offset;
}
