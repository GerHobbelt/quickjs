#ifndef DYNAMICARRAY_H
#define DYNAMICARRAY_H
#include <new>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <assert.h>
#include <atomic>
//#include "Memory.hpp"

template<typename T,size_t BLOCK_SIZE=50>
class dynamic_queue
{
private:
	typedef size_t key;
	struct blockStructure
	{
		blockStructure() = delete;
		static inline blockStructure * allocate()
		{
			static_assert(BLOCK_SIZE>0);
			blockStructure * const res=(blockStructure*)malloc(sizeof(blockStructure) + BLOCK_SIZE*sizeof(T));
			assert(res);
			//memset(res,0,sizeof(blockStructure));
			res->next=nullptr;
			return (blockStructure *)res;
		}
		static inline void free(blockStructure * ptr)
		{
			::free(ptr);
		}
		volatile blockStructure *next;
		volatile T value[];
	};

	blockStructure				*last_block;
	key							 last_pos;
	blockStructure				*current_block;
	key							 current_pos;


	#define unlikely(x)        __builtin_expect((x), 0)

	friend struct dynamic_queue_iterator; 
public:
	//,last_pos_lock(ATOMIC_FLAG_INIT)
	dynamic_queue():last_pos(0),current_pos(0)
	{
		static_assert(BLOCK_SIZE > 1 && "please choose a valide value");
		last_block = current_block = blockStructure::allocate();
	}
	~dynamic_queue()
	{
	}
	// emptiness does not means has no space allocated but has nothing to return (thread-safe)
	[[nodiscard]] bool empty(void)
	{
		bool res = false;
		
		if(last_block == current_block && last_pos == current_pos)
			res = true;
        
		return res;
	}

	// push_back: inserts element at the end (thread-safe)
	const T& push( T && value )
	{
		
		// it causes to not allocate memory with zero usage
		// for two reason:
		// 1- back() wont work, because if we allocate an empty block
		// back() have to return the last element of previos block
		// but we dont have access to it any more (it is a queue not doublequeue)
		// 2- empty() wont work, because may 
		// (last_block != current_block && last_pos != current_pos)
		// but it is actualy empty, which means (last_block == current_block->next && last_pos == 0)
		if(last_pos == BLOCK_SIZE)
		{
			last_block = (last_block->next = blockStructure::allocate());
			last_pos = 0;
		}
		void* element = (void*)(last_block->value+(last_pos++));
		
		new (element) T(value);
		return *(T*)element;
	}

	// constructs element in-place at the end (thread-safe)
	template< class... Args >
	const T& emplace( Args&&... args )
	{
		
		// it causes to not allocate memory with zero usage
		if(unlikely(last_pos == BLOCK_SIZE))
		{
			last_block = (last_block->next = blockStructure::allocate());
			last_pos = 0;
		}
		void * const element = (void*)(last_block->value+(last_pos++));
		
		new (element) T(args...);
		return *(T*)element;
	}

	// pop_front: destruct the first element if exists
	void pop()
	{
		// works like an union variable
		blockStructure *temp=0;
		/*
		 * hint: always current_block->value[current_pos] must return first value unless: 
		 * current_pos==last_pos && current_block==last_block
		*/
		
		// check if is not empty
		if(current_block == last_block)
		{
			// absolute empty
			if(current_pos == last_pos)
				return;
			// math details:we are sure that last_pos-1 wont be equal to -1 and last_pos is greater than 0
			//following line is same as: else if(current_pos == (last_pos-1))
			else if(last_pos == BLOCK_SIZE)
				// dont go to the next block, because it is null
				// we can do a little check instead: if(current_pos==BLOCK_SIZE && current_block->next!=null)
				// but I dont love that approach
				temp = (blockStructure*)0x1;
		}
		
		
		(current_block->value[current_pos++]).~T();
		
		// we must do this check, after the current_pos++ statement 
		// because calling front() will fail if current_pos==BLOCK_SIZE
		// we cant check if(current_pos==BLOCK_SIZE && last_pos!=current_pos)
		// so we store the result of compare in temporary variable
		if(current_pos==BLOCK_SIZE && temp==(blockStructure*)0x0)
		{
			temp = (blockStructure*)current_block->next;
			blockStructure::free(current_block);
			current_block = temp;
			current_pos=0;
		}
	}
	// access the first element (warn: no check if is empty)
	inline T& front()
		{return *(T*const)(current_block->value + current_pos);}
	// access the first element (warn: no check if is empty)
	const T& front() const
		{return *(T*const)(current_block->value + current_pos);}

	// access the last element
	T& back()
	{
		
		void * const element = (void*)(last_block->value+(last_pos-1));
		
		return *element;
	}
	const T& back() const;
protected:
	#undef unlikely
};

#endif // DYNAMICARRAY_H