# c_sak

C Swiss Army Knife: Some random but useful tools for programming in C language.
Maybe in the future I will create a lib :)

Components:

## Hashtable

Hash table implementation. Allows to use as keys int, char, strings or raw memory data.
I would like to create a way of inserting all the different types of keys using only a
unique function in the form:
```c
void* ht_add(struct hashtable *ht, void *key, void *value);
```
without requiring the user to indicate the type nor the length of the element being used
as key.

But I can't find a way of doing it. If you have any idea fork the repo and show me how :)

### Hashtable functions:
```c
struct hashtable* init_hashtable()
int free_hashtable()
void* ht_add_int(ht, int key, void *value)
void* ht_find_int(ht, int key)
void* ht_rm_int(ht, int key)

void* ht_add_char(ht, char key, void *value)
void* ht_find_char(ht, char key)
void* ht_rm_char(ht, char key)

void* ht_add_str(ht, char *key, void *value)
void* ht_find_str(ht, char *key)
void* ht_rm_str(ht, char *key)

void* ht_add_raw(ht, void *key, size_t size, void *value)
void* ht_find_raw(ht, void *key, size_t size)
void* ht_rm_raw(ht, void *key, size_t size)
```
