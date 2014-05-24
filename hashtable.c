#ifndef _HASHTABLE_C_
	#define _HASHTABLE_C_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <stdint.h> // for uint32_t usage

enum TYPES {
	INT, CHAR, STRING, RAW
};

struct kv_pair {
	enum TYPES key_type;
	size_t key_size;
	void *key;
	void *value;
	struct kv_pair *next;
};

struct hashtable {
	size_t size;
	size_t load_factor;
	struct kv_pair **buckets_ary;
};


/*
 * MurmurHash3 function, extracted from the qLibc Project:
 * https://github.com/wolkykim/qlibc/blob/master/src/utilities/qhash.c#L258
 */
uint32_t murmurhash3_32(const void *data, size_t nbytes)
{
	if (data == NULL || nbytes == 0)
		return 0;

	const uint32_t c1 = 0xcc9e2d51;
	const uint32_t c2 = 0x1b873593;

	const int nblocks = nbytes / 4;
	const uint32_t *blocks = (const uint32_t *) (data);
	const uint8_t *tail = (const uint8_t *) (data + (nblocks * 4));

	uint32_t h = 0;

	int i;
	uint32_t k;
	for (i = 0; i < nblocks; i++) {
		k = blocks[i];

		k *= c1;
		k = (k << 15) | (k >> (32 - 15));
		k *= c2;

		h ^= k;
		h = (h << 13) | (h >> (32 - 13));
		h = (h * 5) + 0xe6546b64;
	}

	k = 0;
	switch (nbytes & 3) {
	case 3:
		k ^= tail[2] << 16;
	case 2:
		k ^= tail[1] << 8;
	case 1:
		k ^= tail[0];
		k *= c1;
		k = (k << 15) | (k >> (32 - 15));
		k *= c2;
		h ^= k;
	};

	h ^= nbytes;

	h ^= h >> 16;
	h *= 0x85ebca6b;
	h ^= h >> 13;
	h *= 0xc2b2ae35;
	h ^= h >> 16;

	printf("%u\n", h);
	return h;
}


/*
 * Hash table init and free functions.
 */
struct hashtable* init_hashtable() {
	struct hashtable *ht = calloc(1, sizeof(struct hashtable));

	ht->size = 8;
	ht->load_factor = 4;
	ht->buckets_ary = calloc(8, sizeof(struct kv_pair*));

	size_t i;
	for (i = 0; i < 2; i++) {
		ht->buckets_ary[i] = NULL;
	}

	return ht;
}

int free_hashtable(struct hashtable *ht)
{
	struct kv_pair *curr, *tmp;

	size_t i;
	for (i = 0; i < ht->size; i++) {
		curr = ht->buckets_ary[i];

		while (curr != NULL) {
			tmp = curr;
			curr = curr->next;
			free(tmp->key);
			free(tmp);
		}
	}

	free(ht->buckets_ary);
	free(ht);

	return 0;
}


/*
 * Hash table internal functions.
 */
static int ht_resize(struct hashtable *ht, size_t new_size)
{
	#if DEBUG
	printf("[DEBUG] Resizing hashtable to size: %lu\n", new_size);
	#endif

	struct kv_pair **new_buckets_ary = calloc(new_size, sizeof(struct kv_pair*));

	int i = 0;
	while (i < ht->size) {
		struct kv_pair *curr, *tmp;
		curr = ht->buckets_ary[i];

		while (curr != NULL) {
			size_t new_bucket = murmurhash3_32(curr->key, curr->key_size) % new_size;

			tmp = curr;
			curr = curr->next;

			tmp->next = new_buckets_ary[new_bucket];
			new_buckets_ary[new_bucket] = tmp;

			#if DEBUG
			printf("[DEBUG]   Moving key(%i) from bucket %i to %lu\n", *(int*)tmp->key, i, new_bucket);
			#endif
		}

		i++;
	}

	free(ht->buckets_ary);

	ht->size = new_size;
	ht->buckets_ary = new_buckets_ary;

	return 0;
}

static void* ht_add_internal(struct hashtable *ht, void *key, void *value, enum TYPES type, size_t size)
{
	size_t bucket = murmurhash3_32(key, size) % ht->size;
	struct kv_pair *curr = ht->buckets_ary[bucket];

	int i = 0;
	while (curr != NULL) {
		if (type == RAW) {
			if (curr->key_size == size && memcmp(curr->key, key, size) == 0)
				break;
		} else if (curr->key_type == type && memcmp(curr->key, key, size) == 0)
			break;
		curr = curr->next;
		i++;
	}

	if (curr != NULL) { // Key already used
		curr->value = value;

	} else { // Key not used previously

		struct kv_pair *new_pair = calloc(1, sizeof(struct kv_pair));

		new_pair->key_type = type;
		new_pair->key_size = size;
		switch(type) {
		case INT:
		case CHAR:
		case RAW:
			new_pair->key = calloc(1, size);
			memmove(new_pair->key, key, size);
			break;
		case STRING:
			new_pair->key = calloc(1, size + 1);
			memmove(new_pair->key, key, size);
			*((char*)new_pair->key + size) = '\0';
			break;
		}

		new_pair->value = value;

		new_pair->next = ht->buckets_ary[bucket];
		ht->buckets_ary[bucket] = new_pair;
	}

	if (i >= ht->load_factor)
		ht_resize(ht, ht->size * 2);

	return value;
}

static void* ht_find_internal(const struct hashtable *ht, void *key, enum TYPES type, size_t size)
{
	size_t bucket = murmurhash3_32(key, size) % ht->size;

	struct kv_pair *curr = ht->buckets_ary[bucket];
	while (curr != NULL) {
		if (type == RAW) {
			if (curr->key_size == size && memcmp(curr->key, key, size) == 0)
				return curr->value;
		} else if (curr->key_type == type && memcmp(curr->key, key, size) == 0)
			return curr->value;
		curr = curr->next;
	}
	return NULL;
}

static void* ht_rm_internal(struct hashtable *ht, void *key, enum TYPES type, size_t size)
{
	size_t bucket = murmurhash3_32(key, size) % ht->size;

	struct kv_pair *curr, *prev;
	curr = ht->buckets_ary[bucket];
	prev = NULL;

	while (curr != NULL) {
		if (type == RAW) {
			if (curr->key_size == size && memcmp(curr->key, key, size) == 0) {
				if (prev == NULL) {
					ht->buckets_ary[bucket] = curr->next;
				} else {
					prev->next = curr->next;
				}
				goto free_curr;
			}
		} else if (curr->key_type == type && memcmp(curr->key, key, size) == 0){
			if (prev == NULL) {
				ht->buckets_ary[bucket] = curr->next;
			} else {
				prev->next = curr->next;
			}
			goto free_curr;
		}

		prev = curr;
		curr = curr->next;
	}
	return NULL;

free_curr:
	free(curr->key);
	void *ret = curr->value;
	free(curr);
	return ret;
}


/*
 * Functions for integer keys.
 */
void* ht_add_int(struct hashtable *ht, int key, void *value)
{
	return ht_add_internal(ht, &key, value, INT, sizeof(int));
}

void* ht_find_int(const struct hashtable *ht, int key)
{
	return ht_find_internal(ht, &key, INT, sizeof(int));
}

void* ht_rm_int(struct hashtable *ht, int key)
{
	return ht_rm_internal(ht, &key, INT, sizeof(int));
}


/*
 * Functions for char keys.
 */
void* ht_add_char(struct hashtable *ht, char key, void *value)
{
	return ht_add_internal(ht, &key, value, CHAR, sizeof(char));
}

void* ht_find_char(const struct hashtable *ht, char key)
{
	return ht_find_internal(ht, &key, CHAR, sizeof(char));
}

void* ht_rm_char(struct hashtable *ht, char key)
{
	return ht_rm_internal(ht, &key, CHAR, sizeof(char));
}


/*
 * Functions for string keys.
 */
void* ht_add_str(struct hashtable *ht, char *key, void *value)
{
	return ht_add_internal(ht, key, value, STRING, strlen(key));
}

void* ht_find_str(const struct hashtable *ht, char *key)
{
	return ht_find_internal(ht, key, STRING, strlen(key));
}

void* ht_rm_str(struct hashtable *ht, char *key)
{
	return ht_rm_internal(ht, key, STRING, strlen(key));
}


/*
 * Functions for raw keys.
 */
void* ht_add_raw(struct hashtable *ht, void *key, size_t size, void *value)
{
	return ht_add_internal(ht, key, value, RAW, size);
}

void* ht_find_raw(const struct hashtable *ht, void *key, size_t size)
{
	return ht_find_internal(ht, key, RAW, size);
}

void* ht_rm_raw(struct hashtable *ht, void *key, size_t size)
{
	return ht_rm_internal(ht, key, RAW, size);
}


/*
 * Print debug info from the hash table
 * It will print information about the size of the hashtable, its load factor
 * and cycle over each of the buckets counting the number of items stored.
 */
int ht_debug_info()
{
	// TODO
	return 0;
}

#endif

/*
 * MAIN FUNCTION
 * Hash table testing commands.
 */
int main(int argc, char *argv[])
{
	struct hashtable *ht = init_hashtable();

	char *a = "First test string";
	char *b = "Second test string";
	char *c = "Third test string";
	char *d = "Fourth test string";
	char *e = "Fifth test string";
	char *f = "Sixth test string";
	char *g = "This really works! :)";

	ht_add_int(ht, 1, a);
	ht_add_char(ht, 'a', b);
	ht_add_str(ht, "string", c);
	ht_add_int(ht, 2, d);
	ht_add_int(ht, 3, e);
	ht_add_int(ht, 4, f);
	ht_add_raw(ht, ht, 16, g);
	ht_rm_int(ht, 2);


	printf("Found> \"%s\"\n", (char*)ht_find_int(ht, 1));
	printf("Found> \"%s\"\n", (char*)ht_find_char(ht, 'a'));
	printf("Found> \"%s\"\n", (char*)ht_find_str(ht, "string"));
	printf("Found> \"%s\"\n", (char*)ht_find_int(ht, 2));
	printf("Found> \"%s\"\n", (char*)ht_find_int(ht, 3));
	printf("Found> \"%s\"\n", (char*)ht_find_int(ht, 4));
	printf("Found> \"%s\"\n", (char*)ht_find_raw(ht, ht, 16));

	free_hashtable(ht);

	return 0;
}
