#include "gss.h"
#include "gss_tokenizer.h"
#include "gss_compiler.h"

#include "logging.h"

void GssEngine::compile(string code)
{
    instruction_pointer = 0;
    locals_stack_position = 0;
    
    try
    {
        GssTokenizer tokenizer(code);
        GssCompiler compiler(tokenizer);
        compiler.setNativeFunctions(native_functions);
        compiler.compile();
        LOG(INFO) << "----------------";
        int idx = 0;
        for(const GssInstruction& i : compiler.instructions)
        {
            LOG(INFO) << idx << ": " << i.toString();
            idx++;
        }
        LOG(INFO) << "----------------";
        instructions = compiler.instructions;
        string_table = compiler.string_table;
    }catch(GssTokenizerException e)
    {
        LOG(ERROR) << e.message;
        return;
    }catch(GssCompilerException e)
    {
        LOG(ERROR) << e.message;
        return;
    }
    
    memory = new GssMemory(1024*1024);
    
    for(unsigned int index=0; index<native_functions.size(); index++)
    {
        GssVariant* v = memory->getGlobal(index);
        v->type = GssVariant::Type::native_function;
        v->data.i = index;
    }
    
    try
    {
        while(instruction_pointer < instructions.size())
        {
            step();
        }
        LOG(INFO) << "Finished";
    }catch(GssRuntimeException e)
    {
        LOG(ERROR) << e.message;
    }catch(GssMemoryException e)
    {
        LOG(ERROR) << e.message;
    }
    LOG(INFO) << "Free memory: " << memory->getFreeMemoryAmount();
}

void GssEngine::addNativeFunction(string name, std::function<void(GssNativeFunctionCallData&)> function)
{
    native_functions.emplace_back(name, function);
}

void GssEngine::step()
{
    GssInstruction& instruction = instructions[instruction_pointer];
    //LOG(DEBUG) << memory->getStackSize() << ":" << locals_stack_position << ":" << instruction.toString();
    switch(instruction.type)
    {
    case GssInstruction::Type::nop:
        break;
    case GssInstruction::Type::push_none:
        {
            GssVariant* v = memory->appendStack();
            v->type = GssVariant::Type::none;
        }
        break;
    case GssInstruction::Type::push_int:
        {
            GssVariant* v = memory->appendStack();
            v->type = GssVariant::Type::integer;
            v->data.i = instruction.data.i;
        }
        break;
    case GssInstruction::Type::push_float:
        {
            GssVariant* v = memory->appendStack();
            v->type = GssVariant::Type::float_value;
            v->data.f = instruction.data.f;
        }
        break;
    case GssInstruction::Type::push_empty_list:
        {
            memory->appendStack();
            unsigned int list_position = memory->createList(16);
            GssVariant* v = memory->getStack(-1);
            v->type = GssVariant::Type::list;
            v->data.i = list_position;
        }
        break;
    case GssInstruction::Type::push_string_from_string_table:
        {
            GssVariant* v = memory->appendStack();
            v->type = GssVariant::Type::string;
            v->data.i = memory->createString(string_table[instruction.data.i]);
        }
        break;
    case GssInstruction::Type::push_script_function:
        {
            GssVariant* v = memory->appendStack();
            v->type = GssVariant::Type::script_function;
            v->data.i = instruction.data.i;
        }
        break;
    case GssInstruction::Type::jump:
        instruction_pointer = instruction.data.i;
        return;
    case GssInstruction::Type::jump_if_zero:
        if (memory->getStack(-1)->isZero())
        {
            memory->popStack();
            instruction_pointer = instruction.data.i;
            return;
        }
        memory->popStack();
        break;
    case GssInstruction::Type::jump_if_not_zero:
        if (!memory->getStack(-1)->isZero())
        {
            memory->popStack();
            instruction_pointer = instruction.data.i;
            return;
        }
        memory->popStack();
        break;
    case GssInstruction::Type::pop:
        memory->popStack();
        break;

    case GssInstruction::Type::push_global_by_index:
        {
            //First increase the stack, but ignore this reference, as it could be invalidated by getGlobal. So get a new reference after this which is garanteed to be valid, as getStack does not trigger gc.
            memory->appendStack();
            GssVariant* source = memory->getGlobal(instruction.data.i);
            GssVariant* target = memory->getStack(-1);
            *target = *source;
        }
        break;
    case GssInstruction::Type::assign_global_by_index:
        {
            GssVariant* global = memory->getGlobal(instruction.data.i);
            *global = *memory->getStack(-1);
            memory->popStack();
        }
        break;

    case GssInstruction::Type::push_local_by_index:
        {
            GssVariant* top = memory->appendStack();
            GssVariant* local = memory->getStack(locals_stack_position + instruction.data.i);
            *top = *local;
        }
        break;
    case GssInstruction::Type::assign_local_by_index:
        {
            GssVariant* top = memory->getStack(-1);
            GssVariant* local = memory->getStack(locals_stack_position + instruction.data.i);
            *local = *top;
            memory->popStack();
        }
        break;
        
    case GssInstruction::Type::get_from_table:
        {
            GssVariant* position = memory->getStack(-1);
            GssVariant* list_v = memory->getStack(-2);
            if (list_v->type != GssVariant::Type::list)
                throw GssRuntimeException("Tried to index non-list type: " + list_v->toString());
            if (position->type != GssVariant::Type::integer)
                throw GssRuntimeException("Tried to index with non-integer type: " + position->toString());
            GssVariant* list_ptr = memory->getListEntry(list_v->data.i, position->data.i);
            *list_v = *list_ptr;
            memory->popStack();
        }
        break;
    case GssInstruction::Type::assign_to_table:
        {
            GssVariant* var = memory->getStack(-1);
            GssVariant* position = memory->getStack(-2);
            GssVariant* list_v = memory->getStack(-3);
            if (list_v->type != GssVariant::Type::list)
                throw GssRuntimeException("Tried to index non-list type: " + list_v->toString());
            if (position->type != GssVariant::Type::integer)
                throw GssRuntimeException("Tried to index with non-integer type: " + position->toString());
            GssVariant* list_ptr = memory->getListEntry(list_v->data.i, position->data.i);
            *list_ptr = *var;
            memory->popStack();
            memory->popStack();
            memory->popStack();
        }
        break;
    case GssInstruction::Type::add_to_table:
        {
            GssVariant var = *memory->getStack(-1);
            memory->popStack();
            if (memory->getStack(-1)->type != GssVariant::Type::list)
                throw GssRuntimeException("Tried to append to non-list data.");
            *memory->appendListOnStack() = var;
        }
        break;
    case GssInstruction::Type::get_from_table_by_string_table:
    case GssInstruction::Type::assign_to_table_by_string_table:
        throw GssRuntimeException("Unknown instruction: " + instruction.toString());
        break;

    case GssInstruction::Type::call_function:
        {
            GssVariant* func_info = memory->getStack(-instruction.data.i - 1);
            if (func_info->type == GssVariant::Type::script_function)
            {
                unsigned int new_instruction_pointer = func_info->data.i;
                func_info->type = GssVariant::Type::function_return_data;
                if (instruction_pointer + 1 >= std::numeric_limits<uint16_t>::max())
                    throw GssRuntimeException("Stack overflow (instruction out of range on call)");
                if (locals_stack_position >= std::numeric_limits<uint16_t>::max())
                    throw GssRuntimeException("Stack overflow (local position out of range on call)");
                func_info->data.s[0] = instruction_pointer + 1;
                func_info->data.s[1] = locals_stack_position;
                locals_stack_position = memory->getStackSize() - instruction.data.i;
                instruction_pointer = new_instruction_pointer;
                return;
            }else if (func_info->type == GssVariant::Type::native_function)
            {
                GssNativeFunctionCallData function_call_data(memory->getStackSize() - instruction.data.i, instruction.data.i, memory);
                func_info->type = GssVariant::Type::none; //The func_info stack location will be used to store the return value. So set this to None in case the native function does not set a return value.
                native_functions[func_info->data.i].function(function_call_data);
                memory->setStackSize(memory->getStackSize() - instruction.data.i);
            }else{
                throw GssRuntimeException("Tried to call function on non-function variable: " + func_info->toString());
            }
        }
        break;
    case GssInstruction::Type::ensure_locals:
        while(memory->getStackSize() < locals_stack_position + instruction.data.i)
            memory->appendStack()->type = GssVariant::Type::none;
        break;
    case GssInstruction::Type::return_from_function:
        {
            if (locals_stack_position == 0)
                throw GssRuntimeException("Return while no longer in a function.");
            GssVariant* return_value = memory->getStack(-1);
            GssVariant* return_info = memory->getStack(locals_stack_position - 1);
            
            memory->setStackSize(locals_stack_position);
            
            instruction_pointer = return_info->data.s[0];
            locals_stack_position = return_info->data.s[1];
            //The [return_value] variable is now outside of normal stack range due to the setStackSize, but no GC can be triggered yet at this point, so this is safe.
            *return_info = *return_value;
        }
        return;

    case GssInstruction::Type::boolean_not:
        {
            GssVariant* v0 = memory->getStack(-1);
            if (v0->isZero())
            {
                v0->data.i = 1;
                v0->type = GssVariant::Type::integer;
            }else{
                v0->data.i = 0;
                v0->type = GssVariant::Type::integer;
            }
        }
        break;
    case GssInstruction::Type::binary_not:
        throw GssRuntimeException("Unknown instruction: " + instruction.toString());
        break;
    case GssInstruction::Type::negative:
        {
            GssVariant* v = memory->getStack(-1);
            if (v->type == GssVariant::Type::integer)
                v->data.i = -v->data.i;
            else if (v->type == GssVariant::Type::float_value)
                v->data.f = -v->data.f;
            else
                throw GssRuntimeException("Tried to negate non-number type: " + v->toString());
        }
        break;

    case GssInstruction::Type::boolean_or:
    case GssInstruction::Type::boolean_and:
    case GssInstruction::Type::binary_or:
    case GssInstruction::Type::binary_not_2:
    case GssInstruction::Type::binary_and:
    case GssInstruction::Type::boolean_equal:
    case GssInstruction::Type::boolean_not_equal:
        throw GssRuntimeException("Unknown instruction: " + instruction.toString());
        break;
    case GssInstruction::Type::boolean_less:
        {
            GssVariant* v0 = memory->getStack(-2);
            GssVariant* v1 = memory->getStack(-1);
            if (v0->canBeFloat() && v1->canBeFloat())
            {
                v0->data.i = v0->toFloat() < v1->toFloat();
                v0->type = GssVariant::Type::integer;
            }else{
                throw GssRuntimeException("Bad operation '<' on types: " + v0->toString() + " " + v1->toString());
            }
            memory->popStack();
        }
        break;
    case GssInstruction::Type::boolean_less_equal:
        {
            GssVariant* v0 = memory->getStack(-2);
            GssVariant* v1 = memory->getStack(-1);
            if (v0->canBeFloat() && v1->canBeFloat())
            {
                v0->data.i = v0->toFloat() <= v1->toFloat();
                v0->type = GssVariant::Type::integer;
            }else{
                throw GssRuntimeException("Bad operation '<' on types: " + v0->toString() + " " + v1->toString());
            }
            memory->popStack();
        }
        break;
    case GssInstruction::Type::boolean_greater:
        {
            GssVariant* v0 = memory->getStack(-2);
            GssVariant* v1 = memory->getStack(-1);
            if (v0->canBeFloat() && v1->canBeFloat())
            {
                v0->data.i = v0->toFloat() > v1->toFloat();
                v0->type = GssVariant::Type::integer;
            }else{
                throw GssRuntimeException("Bad operation '<' on types: " + v0->toString() + " " + v1->toString());
            }
            memory->popStack();
        }
        break;
    case GssInstruction::Type::boolean_greater_equal:
        {
            GssVariant* v0 = memory->getStack(-2);
            GssVariant* v1 = memory->getStack(-1);
            if (v0->canBeFloat() && v1->canBeFloat())
            {
                v0->data.i = v0->toFloat() >= v1->toFloat();
                v0->type = GssVariant::Type::integer;
            }else{
                throw GssRuntimeException("Bad operation '<' on types: " + v0->toString() + " " + v1->toString());
            }
            memory->popStack();
        }
        break;
    case GssInstruction::Type::left_shift:
    case GssInstruction::Type::right_shift:
        throw GssRuntimeException("Unknown instruction: " + instruction.toString());
        break;
    case GssInstruction::Type::add:
        {
            GssVariant* v0 = memory->getStack(-2);
            GssVariant* v1 = memory->getStack(-1);
            if (v0->type == GssVariant::Type::integer && v1->type == GssVariant::Type::integer)
            {
                v0->data.i = v0->data.i + v1->data.i;
            }else if (v0->canBeFloat() && v1->canBeFloat())
            {
                v0->data.f = v0->toFloat() + v1->toFloat();
                v0->type = GssVariant::Type::float_value;
            }else if (v0->type == GssVariant::Type::string && v1->type == GssVariant::Type::string)
            {
                string s0 = memory->getString(v0->data.i);
                string s1 = memory->getString(v1->data.i);
                unsigned int new_string_position = memory->createString(s0 + s1);//createString can GC, so we v0 and v1 are invalid at this point.
                v0 = memory->getStack(-2);
                v0->data.i = new_string_position;
            }else{
                throw GssRuntimeException("Bad operation '+' on types: " + v0->toString() + " " + v1->toString());
            }
            memory->popStack();
        }
        break;
    case GssInstruction::Type::substract:
        {
            GssVariant* v0 = memory->getStack(-2);
            GssVariant* v1 = memory->getStack(-1);
            if (v0->type == GssVariant::Type::integer && v1->type == GssVariant::Type::integer)
            {
                v0->data.i = v0->data.i - v1->data.i;
            }else if (v0->canBeFloat() && v1->canBeFloat())
            {
                v0->data.f = v0->toFloat() - v1->toFloat();
                v0->type = GssVariant::Type::float_value;
            }else{
                throw GssRuntimeException("Bad operation '-' on types: " + v0->toString() + " " + v1->toString());
            }
            memory->popStack();
        }
        break;
    case GssInstruction::Type::multiply:
        {
            GssVariant* v0 = memory->getStack(-2);
            GssVariant* v1 = memory->getStack(-1);
            if (v0->type == GssVariant::Type::integer && v1->type == GssVariant::Type::integer)
            {
                v0->data.i = v0->data.i * v1->data.i;
            }else if (v0->canBeFloat() && v1->canBeFloat())
            {
                v0->data.f = v0->toFloat() * v1->toFloat();
                v0->type = GssVariant::Type::float_value;
            }else{
                throw GssRuntimeException("Bad operation '-' on types: " + v0->toString() + " " + v1->toString());
            }
            memory->popStack();
        }
        break;
    case GssInstruction::Type::division:
        {
            GssVariant* v0 = memory->getStack(-2);
            GssVariant* v1 = memory->getStack(-1);
            if (v0->type == GssVariant::Type::integer && v1->type == GssVariant::Type::integer)
            {
                v0->data.i = v0->data.i / v1->data.i;
            }else if (v0->canBeFloat() && v1->canBeFloat())
            {
                v0->data.f = v0->toFloat() / v1->toFloat();
                v0->type = GssVariant::Type::float_value;
            }else{
                throw GssRuntimeException("Bad operation '-' on types: " + v0->toString() + " " + v1->toString());
            }
            memory->popStack();
        }
        break;
    case GssInstruction::Type::modulo:
        {
            GssVariant* v0 = memory->getStack(-2);
            GssVariant* v1 = memory->getStack(-1);
            if (v0->type == GssVariant::Type::integer && v1->type == GssVariant::Type::integer)
            {
                v0->data.i = v0->data.i % v1->data.i;
            }else if (v0->canBeFloat() && v1->canBeFloat())
            {
                v0->data.f = fmodf(v0->toFloat(), v1->toFloat());
                v0->type = GssVariant::Type::float_value;
            }else{
                throw GssRuntimeException("Bad operation '-' on types: " + v0->toString() + " " + v1->toString());
            }
            memory->popStack();
        }
        break;
    default:
        throw GssRuntimeException("Unknown instruction: " + instruction.toString());
    }
    instruction_pointer++;
}
