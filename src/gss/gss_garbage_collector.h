#ifndef GSS_GARBAGE_COLLECTOR_H
#define GSS_GARBAGE_COLLECTOR_H

#include "gss_memory.h"

#include <map>

class GssGarbageCollector : sf::NonCopyable
{
public:
    GssGarbageCollector(GssMemory* memory);

    void run();
private:
    GssMemory* memory;
    void* old_memory;
    
    std::map<unsigned int, unsigned int> address_relocation_map;
    
    void* get(unsigned int location) { return ((char*)old_memory) + location; }
    
    unsigned int processListAt(unsigned int old_position);
    void copyVariant(unsigned int old_position, unsigned int new_position);
};

#endif//GSS_GARBAGE_COLLECTOR_H
