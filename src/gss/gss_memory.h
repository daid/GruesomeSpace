#ifndef GSS_MEMORY_H
#define GSS_MEMORY_H

#include <SFML/System.hpp>
#include <limits>

#include "gss_variant.h"

class GssMemory : sf::NonCopyable
{
public:
    static constexpr unsigned int NO_MEMORY = std::numeric_limits<unsigned int>::max();

    GssMemory(unsigned int size);
    ~GssMemory();

    GssVariant* appendStack(); //Warning: appendStack might run the GC and thus invalidates previous GssVariants.
    GssVariant* getStack(int position);
    void popStack();
    void setStackSize(unsigned int length);
    unsigned int getStackSize();

    GssVariant* getGlobal(unsigned int index); //Warning: getGlobal might run the GC and thus invalidates previous GssVariants.
    
    unsigned int createString(const string& str);
    string getString(unsigned int position);

    unsigned int createList(unsigned int reserved_length);
    GssVariant* appendListOnStack();
    GssVariant* getListEntry(unsigned int list_memory_position, int list_entry);
    
    unsigned int getFreeMemoryAmount();

private:
    void* memory;
    unsigned int memory_size;

    unsigned int stack_location;
    unsigned int globals_location;
    
    unsigned int allocation_point;

    unsigned int allocate(unsigned int size);

    void* get(unsigned int location) { return ((char*)memory) + location; }
    
    void runGarbageCollect();
    
    friend class GssGarbageCollector;
};

class GssList
{
public:
    uint32_t current_length;
    uint32_t reserved_length;
    uint32_t position;
};

class GssMemoryException : public std::exception
{
public:
    string message;
    
    GssMemoryException(string message) : message(message) {}
};

#endif//GSS_MEMORY_H
