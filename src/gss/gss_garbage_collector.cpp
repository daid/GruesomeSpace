#include "gss_garbage_collector.h"

#include <string.h>

#include "logging.h"

GssGarbageCollector::GssGarbageCollector(GssMemory* memory)
: memory(memory)
{
}

void GssGarbageCollector::run()
{
    old_memory = memory->memory;
    memory->memory = calloc(memory->memory_size, 1);
    memory->allocation_point = 0;

    memory->stack_location = processListAt(memory->stack_location);
    memory->globals_location = processListAt(memory->globals_location);
    
    free(old_memory);
}

unsigned int GssGarbageCollector::processListAt(unsigned int old_position)
{
    if (address_relocation_map.find(old_position) != address_relocation_map.end())
        return address_relocation_map[old_position];
    GssList* old_list = (GssList*)get(old_position);
    unsigned int new_position = memory->allocate(sizeof(GssList));
    GssList* new_list = (GssList*)memory->get(new_position);
    new_list->current_length = old_list->current_length;
    new_list->reserved_length = std::min(new_list->current_length + 16, old_list->reserved_length);
    new_list->position = memory->allocate(sizeof(GssVariant) * new_list->reserved_length);
    
    for(unsigned int n=0; n<old_list->current_length; n++)
    {
        copyVariant(old_list->position + sizeof(GssVariant) * n, new_list->position + sizeof(GssVariant) * n);
    }
    address_relocation_map[old_position] = new_position;
    return new_position;
}

void GssGarbageCollector::copyVariant(unsigned int old_position, unsigned int new_position)
{
    GssVariant* old_v = (GssVariant*)get(old_position);
    GssVariant* new_v = (GssVariant*)memory->get(new_position);
    *new_v = *old_v;
    if (old_v->type == GssVariant::Type::string)
    {
        if (address_relocation_map.find(old_v->data.i) != address_relocation_map.end())
        {
            new_v->data.i = address_relocation_map[old_v->data.i];
        }else{
            uint32_t* old_ptr = (uint32_t*)get(old_v->data.i);
            uint32_t str_len = *old_ptr;
            old_ptr++;
            new_v->data.i = memory->allocate(sizeof(uint32_t) + str_len);
            uint32_t* new_ptr = (uint32_t*)memory->get(new_v->data.i);
            *new_ptr = str_len;
            new_ptr++;
            memcpy(new_ptr, old_ptr, str_len);
            address_relocation_map[old_v->data.i] = new_v->data.i;
        }
    }
    if (old_v->type == GssVariant::Type::list)
        new_v->data.i = processListAt(old_v->data.i);
    if (old_v->type == GssVariant::Type::dictionary)
        LOG(ERROR) << "Copy missing for dictionary type";
}
