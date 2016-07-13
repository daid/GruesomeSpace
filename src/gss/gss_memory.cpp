#include "gss_memory.h"
#include "gss_garbage_collector.h"

#include <string.h>

#include "logging.h"

GssMemory::GssMemory(unsigned int size)
{
    memory = calloc(size, 1);
    memory_size = size;
    
    allocation_point = 0;
    
    stack_location = createList(32);
    globals_location = createList(32);
}

GssMemory::~GssMemory()
{
    free(memory);
}

GssVariant* GssMemory::appendStack()
{
    GssList* list = (GssList*)get(stack_location);
    if (list->current_length == list->reserved_length)
    {
        unsigned int new_reserved_length = list->reserved_length + 16;
        unsigned int new_list_location = allocate(sizeof(GssVariant) * new_reserved_length);
        // list variable is now invalid, as allocate could have moved the [stack_location] when doing GC.
        list = (GssList*)get(stack_location);
        for(unsigned int n=0; n<list->current_length; n++)
        {
            GssVariant* old_var = (GssVariant*)get(list->position + sizeof(GssVariant) * n);
            GssVariant* new_var = (GssVariant*)get(new_list_location + sizeof(GssVariant) * n);
            *new_var = *old_var;
        }
        list->reserved_length = new_reserved_length;
        list->position = new_list_location;
    }
    list->current_length += 1;
    return (GssVariant*)get(list->position + sizeof(GssVariant) * (list->current_length - 1));
}

void GssMemory::setStackSize(unsigned int length)
{
    GssList* list = (GssList*)get(stack_location);
    if (list->current_length < length)
        throw GssMemoryException("Tried to increase stack with setStackSize, which can only decrease stack size.");
    list->current_length = length;
}

void GssMemory::popStack()
{
    GssList* list = (GssList*)get(stack_location);
    if (list->current_length == 0)
        throw GssMemoryException("Stack underrun!");
    list->current_length -= 1;
}

GssVariant* GssMemory::getStack(int position)
{
    GssList* list = (GssList*)get(stack_location);
    if (position < 0)
        position = list->current_length + position;
    return (GssVariant*)get(list->position + sizeof(GssVariant) * position);
}

unsigned int GssMemory::getStackSize()
{
    GssList* list = (GssList*)get(stack_location);
    return list->current_length;
}

GssVariant* GssMemory::getGlobal(unsigned int index)
{
    GssList* list = (GssList*)get(globals_location);
    if (list->reserved_length <= index)
    {
        unsigned int new_reserved_length = index + 1;
        unsigned int new_list_location = allocate(sizeof(GssVariant) * new_reserved_length);
        // list variable is now invalid, as allocate could have moved the [stack_location]
        GssList* list = (GssList*)get(globals_location);
        for(unsigned int n=0; n<list->current_length; n++)
        {
            GssVariant* old_var = (GssVariant*)get(list->position + sizeof(GssVariant) * n);
            GssVariant* new_var = (GssVariant*)get(new_list_location + sizeof(GssVariant) * n);
            *new_var = *old_var;
        }
        list->reserved_length = new_reserved_length;
        list->position = new_list_location;
    }
    while(list->current_length <= index)
    {
        GssVariant* var = (GssVariant*)get(list->position + sizeof(GssVariant) * list->current_length);
        var->type = GssVariant::Type::none;
        list->current_length += 1;
    }
    return (GssVariant*)get(list->position + sizeof(GssVariant) * index);
}

unsigned int GssMemory::createString(const string& str)
{
    unsigned int position = allocate(sizeof(uint32_t) + str.length() + 1);
    uint32_t* ptr = (uint32_t*)get(position);
    *ptr = str.length() + 1;
    ptr++;
    memcpy(ptr, str.c_str(), str.length() + 1);
    return position;
}

string GssMemory::getString(unsigned int position)
{
    uint32_t* ptr = (uint32_t*)get(position);
    string result = (char*)(ptr + 1);
    return result;
}

unsigned int GssMemory::createList(unsigned int reserved_length)
{
    unsigned int location = allocate(sizeof(GssList));
    GssList* list = (GssList*)get(location);
    list->current_length = 0;
    list->reserved_length = reserved_length;
    list->position = allocate(sizeof(GssVariant) * reserved_length);
    return location;
}

GssVariant* GssMemory::appendListOnStack()
{
    GssVariant* list_v = getStack(-1);
    if (list_v->type != GssVariant::Type::list)
        throw GssMemoryException("Tried to append on non-list item: " + list_v->toString());
    GssList* list = (GssList*)get(list_v->data.i);
    if (list->current_length < list->reserved_length)
    {
        list->current_length++;
        return ((GssVariant*)get(list->position)) + (list->current_length - 1);
    }
    
    //Increase the reserve
    list->reserved_length += 16;
    int new_buffer_position = allocate(sizeof(GssVariant) * list->reserved_length);
    //After this allocate all previous pointers are invalid, as GC could have happened.
    list_v = getStack(-1);
    list = (GssList*)get(list_v->data.i);
    GssVariant* old_ptr = (GssVariant*)get(list->position);
    GssVariant* new_ptr = (GssVariant*)get(new_buffer_position);
    for(unsigned int n=0; n<list->current_length; n++)
    {
        *(old_ptr + n) = *(old_ptr + n);
    }
    list->current_length++;
    list->position = new_buffer_position;
    return new_ptr + (list->current_length - 1);
}

GssVariant* GssMemory::getListEntry(unsigned int list_memory_position, int list_entry)
{
    GssList* list = (GssList*)get(list_memory_position);
    if (list_entry < 0)
        list_entry = list->current_length + list_entry;
    if (list_entry < 0 || list_entry >= int(list->current_length))
        throw GssMemoryException("Index out of range: " + string(list_entry));
    return ((GssVariant*)get(list->position)) + list_entry;
}

unsigned int GssMemory::getFreeMemoryAmount()
{
    runGarbageCollect();
    return memory_size - allocation_point;
}

unsigned int GssMemory::allocate(unsigned int size)
{
    size = ((size - 1) | 0x3) + 1;
    if (memory_size - allocation_point < size)
    {
        runGarbageCollect();
    }
    if (memory_size - allocation_point < size)
    {
        throw GssMemoryException("Out of memory");
    }
    unsigned int result = allocation_point;
    allocation_point += size;
    return result;
}

void GssMemory::runGarbageCollect()
{
    //LOG(DEBUG) << "runGarbageCollect pre: " << (memory_size - allocation_point);
    GssGarbageCollector garbage_collector(this);
    garbage_collector.run();
    //LOG(DEBUG) << "runGarbageCollect post:" << (memory_size - allocation_point);
}
