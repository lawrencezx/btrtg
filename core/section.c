#include "compiler.h"

#include "x86pg.h"
#include "section.h"

void init_text_sec(struct section *data_sec)
{
    data_sec->type = TEXT_SEC;
}

void init_data_sec(struct section *data_sec)
{
    data_sec->type = DATA_SEC;
    data_sec->size = 0x1000;
}
