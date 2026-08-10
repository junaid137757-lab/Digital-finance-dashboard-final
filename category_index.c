#include <string.h>

#include "category_index.h"
#include "common.h"

/* Table size: a fixed multiple of MAX_BUDGETS so the load
   factor stays well under 100%, keeping average probe
   chains short even when every budget slot is in use.
   Still a compile-time constant - no malloc. */
#define CATEGORY_INDEX_CAPACITY (MAX_BUDGETS * 2)

/* Each slot holds either -1 (empty) or a valid index into
   budgets[]. There is no separate chaining structure - on
   a collision we probe the next slot, wrapping around,
   until we find an empty one or the matching category. */
static int slots[CATEGORY_INDEX_CAPACITY];

/* djb2 string hash. Not cryptographic - just needs to
   spread category names evenly across the table so probe
   chains stay short. */
static unsigned int hashCategory(const char *category)
{
    unsigned int hash = 5381u;

    while(*category != '\0')
    {
        hash = ((hash << 5) + hash) + (unsigned int)(unsigned char)(*category);
        category++;
    }

    return hash;
}

void categoryIndexRebuild(void)
{
    int i;

    for(i = 0; i < CATEGORY_INDEX_CAPACITY; i++)
        slots[i] = -1;

    for(i = 0; i < budgetCount; i++)
    {
        unsigned int h = hashCategory(budgets[i].category) % CATEGORY_INDEX_CAPACITY;

        while(slots[h] != -1)
            h = (h + 1u) % CATEGORY_INDEX_CAPACITY;

        slots[h] = i;
    }
}

int categoryIndexLookup(const char *category)
{
    unsigned int h = hashCategory(category) % CATEGORY_INDEX_CAPACITY;
    int probes = 0;

    while(slots[h] != -1 && probes < CATEGORY_INDEX_CAPACITY)
    {
        if(strcmp(budgets[slots[h]].category, category) == 0)
            return slots[h];

        h = (h + 1u) % CATEGORY_INDEX_CAPACITY;
        probes++;
    }

    return -1;
}
