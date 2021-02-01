#include "compiler.h"

#include "parseLib.h"

int getElemsSize(xmlNodePtr node)
{
    int num = 0;
    for (; node != NULL; node = node->next) {
        if (node->type == XML_ELEMENT_NODE) {
            num++;
        }
    }
    return num;
}
