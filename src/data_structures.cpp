void* operator new(size_t size, void* memory) {
    return memory;
}

template<typename type>
void construct(type* it) {
    new (it) type;
}

typedef void* Alloc(u32 size);
typedef void  Dealloc(void* memory);

struct Allocator {
    Alloc*   alloc   = null;
    Dealloc* dealloc = null;
};

Allocator default_allocator;

Allocator make_allocator(Alloc* alloc, Dealloc* dealloc) {
    Allocator allocator;

    allocator.alloc   = alloc;
    allocator.dealloc = dealloc;

    return allocator;
}

#define for_each(element, in)                                   \
    for (auto iterator = make_iterator(in); iterator.has_next;) \
        if (element = get_next(&iterator))

template<typename type>
struct Array {
    Allocator* allocator = &default_allocator;
    
    u32 capacity = 0;
    u32 count    = 0;

    type* elements = null;
    type& operator [](u32 index);
};

template<typename type>
void allocate(Array<type>* array, u32 new_capacity) {
    assert(new_capacity >= array->count);

    type* old_elements = array->elements;
    array->capacity = new_capacity;

    array->elements = (type*) array->allocator->alloc(array->capacity * size_of(type));
    
    if (old_elements) {
        for (u32 i = 0; i < array->count; i++) {
            array->elements[i] = old_elements[i];
        }

        array->allocator->dealloc(old_elements);
    }
}

template<typename type>
void free(Array<type>* array) {
    array->capacity = 0;
    array->count    = 0;

    if (array->elements) {
        array->allocator->dealloc(array->elements);
    }
}

template<typename type>
u32 insert(Array<type>* array, type element, u32 index) {
    assert(index <= array->count);
    assert(array->count <= array->capacity);

    if (array->count == array->capacity) {
        allocate(array, array->capacity ? (array->capacity * 2) : 8);
    }
    
    for (u32 i = array->count; i > index; i--) {
        array->elements[i] = array->elements[i - 1];
    }

    array->elements[index] = element;
    array->count += 1;

    return index;
}

template<typename type>
u32 add(Array<type>* array, type element) {
    return insert(array, element, array->count);
}

template<typename type>
void remove(Array<type>* array, u32 index) {
    assert(index < array->count);
    
    for (u32 i = index; i < array->count; i++) {
        array->elements[i] = array->elements[i + 1];
    }

    array->count -= 1;
}

template<typename type>
type* get(Array<type>* array, u32 index) {
    assert(index < array->count);
    return &array->elements[index];
}

template<typename type>
type& Array<type>::operator [](u32 index) {
    assert(index < this->count);
    return this->elements[index];
}

template<typename type>
type* next(Array<type>* array) {
    type element;
    u32 index = add(array, element);

    return get(array, index);
}

template<typename type>
Array<type> copy(Array<type>* array, u32 start, u32 end) {
    assert(start < end);
    assert(end <= array->count);

    Array<type> the_copy;
    the_copy.allocator = array->allocator;

    allocate(&the_copy, array->count);

    for (u32 i = start; i < end; i++) {
        add(&the_copy, (*array)[i]);
    }

    return the_copy;
}

template<typename type>
Array<type> copy(Array<type>* array) {
    return copy(array, 0, array->count);
}

template<typename type>
struct Array_Iterator {
    Array<type>* array = null;

    u32 current = 0;
    u32 next    = 0;

    bool has_next = false;
};

template<typename type>
Array_Iterator<type> make_iterator(Array<type>* array) {
    Array_Iterator<type> iterator;
    iterator.array = array;

    if (array->count) {
        iterator.has_next = true;
    }

    return iterator;
}

template<typename type>
type* get_next(Array_Iterator<type>* iterator) {
    assert(iterator->has_next);

    type* element = &iterator->array->elements[iterator->next];
    iterator->current = iterator->next;

    iterator->has_next = false;

    if (iterator->current + 1 < iterator->array->count) {
        iterator->next = iterator->current + 1;
        iterator->has_next   = true;
    }

    return element;
}

template<typename type>
type* peek_next(Array_Iterator<type>* iterator) {
    assert(iterator->has_next);
    return &iterator->array->elements[iterator->next];
}

template<typename type, u32 size>
struct Bucket {
    type elements[size];
    bool occupied[size] = {};
};

template<typename type, u32 size>
struct Bucket_Array {
    Allocator* allocator = &default_allocator;

    Array<Bucket<type, size>*> buckets;
    u32 count = 0;
};

struct Bucket_Locator {
    u32 array_index  = 0;
    u32 bucket_index = 0;
};

template<typename type, u32 size>
Bucket_Locator add(Bucket_Array<type, size>* bucket_array, type element) {
    i32 array_index  = -1;
    i32 bucket_index = -1;

    for (u32 i = 0; i < bucket_array->buckets.count; i++) {
        Bucket<type, size>* bucket = bucket_array->buckets[i];
        
        for (u32 j = 0; j < size; j++) {
            if (bucket->occupied[j]) continue;

            bucket_index = j;
            break;
        }

        if (bucket_index != -1) {
            array_index = i;
            break;
        }
    }

    if (array_index == -1) {
        // @note: There is a preprocessor bug passing generic types through the macro size_of...

        Bucket<type, size>* bucket = (Bucket<type, size>*) bucket_array->allocator->alloc(sizeof(Bucket<type, size>));
        construct(bucket);

        array_index  = add(&bucket_array->buckets, bucket);
        bucket_index = 0;
    }

    assert(array_index  >= 0);
    assert(bucket_index >= 0);

    Bucket<type, size>* bucket = bucket_array->buckets[array_index];

    bucket->elements[bucket_index] = element;
    bucket->occupied[bucket_index] = true;

    Bucket_Locator bucket_locator;

    bucket_locator.array_index  = array_index;
    bucket_locator.bucket_index = bucket_index;

    bucket_array->count += 1;

    return bucket_locator;
}

template<typename type, u32 size>
void remove(Bucket_Array<type, size>* bucket_array, Bucket_Locator locator) {
    Bucket<type, size>* bucket = bucket_array->buckets[locator.array_index];
    assert(bucket->occupied[locator.bucket_index]);

    // @todo: Drop a bucket if there is a good amount of room in another bucket?

    bucket->occupied[locator.bucket_index] = false;
    bucket_array->count -= 1;
}

template<typename type, u32 size>
type* get(Bucket_Array<type, size>* bucket_array, Bucket_Locator locator) {
    Bucket<type, size>* bucket = bucket_array->buckets[locator.array_index];
    assert(bucket->occupied[locator.bucket_index]);

    return &bucket->elements[locator.bucket_index];
}

template<typename type, u32 size>
type* next(Bucket_Array<type, size>* bucket_array) {
    type element;
    Bucket_Locator locator = add(bucket_array, element);

    return get(bucket_array, locator);
}

template<typename type, u32 size>
void free(Bucket_Array<type, size>* bucket_array) {
    for (u32 i = 0; i < bucket_array->buckets.count; i++) {
        Bucket<type, size>* bucket = bucket_array->buckets[i];
        bucket_array->allocator->dealloc(bucket);
    }

    free(&bucket_array->buckets);
}

template<typename type, u32 size>
struct Bucket_Iterator {
    Bucket_Array<type, size>* bucket_array = null;

    Bucket_Locator current;
    Bucket_Locator next;

    bool has_next = false;
};

template<typename type, u32 size>
Bucket_Iterator<type, size> make_iterator(Bucket_Array<type, size>* bucket_array) {
    Bucket_Iterator<type, size> iterator;
    iterator.bucket_array = bucket_array;

    for (u32 i = 0; i < bucket_array->buckets.count; i++) {
        Bucket<type, size>* bucket = bucket_array->buckets[i];
        
        for (u32 j = 0; j < size; j++) {
            if (!bucket->occupied[j]) continue;

            iterator.next.array_index  = i;
            iterator.next.bucket_index = j;
            
            iterator.has_next = true;
            break;
        }

        if (iterator.has_next) break;
    }

    return iterator;
}

template<typename type, u32 size>
type* get_next(Bucket_Iterator<type, size>* iterator) {
    assert(iterator->has_next);

    type* element = get(iterator->bucket_array, iterator->next);
    iterator->current = iterator->next;

    iterator->has_next = false;

    for (u32 i = iterator->current.array_index; i < iterator->bucket_array->buckets.count; i++) {
        Bucket<type, size>* bucket = iterator->bucket_array->buckets[i];
        
        u32 bucket_start = 0;
        if (i == iterator->current.array_index) {
            bucket_start = iterator->current.bucket_index + 1;
        }

        for (u32 j = bucket_start; j < size; j++) {
            if (!bucket->occupied[j]) continue;

            iterator->next.array_index  = i;
            iterator->next.bucket_index = j;
            
            iterator->has_next = true;
            break;
        }

        if (iterator->has_next) break;
    }

    return element;
}

template<typename type, u32 size>
type* peek_next(Bucket_Iterator<type, size>* iterator) {
    assert(iterator->has_next);
    return get(iterator->bucket_array, iterator->next);
}

template<typename type, u32 size>
void remove(Bucket_Array<type, size>* bucket_array, type* element) {
    bool was_removed = false;

    for_each (type* it, bucket_array) {
        if (it != element) continue;

        remove(bucket_array, iterator.current);
        was_removed = true;

        break;
    }

    assert(was_removed);
}