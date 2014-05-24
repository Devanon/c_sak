# c\_sak

C Swiss Army Knife: Some random but useful tools for programming in C language.
Maybe in the future I will create a lib :)

Components:

## Hashtable

Hash table implementation. Allows to use as keys int, char, strings or raw memory data.
I would like to create a way of inserting all the different types of keys using only a
unique function in the form:
```c
void* ht\_add(struct hashtable *ht, void *key, void *value);
```
without requiring the user to indicate the type nor the length of the element being used
as key.

But I can't find a way of doing it. If you have any idea fork the repo and show me how :)

### Hashtable functions:
* struct hashtable\* init\_hashtable()
* int free\_hashtable()

* void* ht\_add\_int(ht, int key, void *value)
* void* ht\_find\_int(ht, int key)
* void* ht\_rm\_int(ht, int key)

* void* ht\_add\_char(ht, char key, void *value)
* void* ht\_find\_char(ht, char key)
* void* ht\_rm\_char(ht, char key)

* void* ht\_add\_str(ht, char *key, void *value)
* void* ht\_find\_str(ht, char *key)
* void* ht\_rm\_str(ht, char *key)

* void* ht\_add\_raw(ht, void *key, size\_t size, void *value)
* void* ht\_find\_raw(ht, void *key, size\_t size)
* void* ht\_rm\_raw(ht, void *key, size\_t size)
