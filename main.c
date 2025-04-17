#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#define PAGE_SIZE 512

typedef uint8_t u8;
typedef uint16_t u16;

typedef struct {
	u16 pos;
	u16 ksize;
	u16 vsize;
} page_element;

typedef struct {
	u16 count;
	page_element elems[];
} page;

void add_page_element(page *p, u8 *k, u16 ksize, u8 *v, u16 vsize) {
	u16 new_element_pos = PAGE_SIZE;
	for (u16 i = 0; i < p->count; i++) {
		if (p->elems[i].pos < new_element_pos) {
			new_element_pos = p->elems[i].pos;
		}
	}

	new_element_pos -= ksize;
	new_element_pos -= vsize;

	p->elems[p->count].pos = new_element_pos;
	p->elems[p->count].ksize = ksize;
	p->elems[p->count].vsize = vsize;

	u8 *kdst = (u8 *)p + new_element_pos;
	u8 *vdst = (u8 *)p + new_element_pos + ksize;

	memcpy(kdst, k, ksize);
	memcpy(vdst, v, vsize);

	p->count++;
}

u16 get_page_element_key_size(page *p, u16 elnum) {
	return p->elems[elnum].ksize;
}

u8 *get_page_element_key_ptr(page *p, u16 elnum) {
	return (u8 *)p + p->elems[elnum].pos;
}

u8 *get_page_element_val_ptr(page *p, u16 elnum) {
	return (u8 *)p + p->elems[elnum].pos + p->elems[elnum].ksize;
}

u8 *get_page_element_val_ptr_by_key(page *p, u8 *k, u16 ksize) {
	for (u16 i = 0; i < p->count; i++) {
		u16 ksrcsize = get_page_element_key_size(p, i);
		if (ksrcsize != ksize) {
			continue;
		}

		u8 *ksrc = get_page_element_key_ptr(p, i);
		if (memcmp(k, ksrc, ksize) == 0) {
			return get_page_element_val_ptr(p, i);
		};
	}

	return NULL;
}

u16 get_page_space_left(page *p) {
	u16 new_element_pos = PAGE_SIZE;
	for (u8 i = 0; i < p->count; i++) {
		if (p->elems[i].pos < new_element_pos) {
			new_element_pos = p->elems[i].pos;
		}
	}

	return new_element_pos - sizeof(p->count) - (sizeof(page_element) * p->count);
}

int main(void) {
	u8 m[512] = {0};
	page *p = (page *)m;

	{
		u16 space_left = get_page_space_left(p);
		fprintf(stderr, "space left: %hu\n", space_left);
	}

	{
		u8 key[] = "Hello";
		u8 val[] = "World";

		add_page_element(p, key, sizeof(key), val, sizeof(val));

		u16 space_left = get_page_space_left(p);
		fprintf(stderr, "space left: %hu\n", space_left);
	}

	{
		u8 key[] = "Foo";
		u8 val[] = "Bar";

		add_page_element(p, key, sizeof(key), val, sizeof(val));

		u16 space_left = get_page_space_left(p);
		fprintf(stderr, "space left: %hu\n", space_left);
	}

	{
		u8 *k = get_page_element_key_ptr(p, 1);
		u8 *v = get_page_element_val_ptr(p, 1);

		fprintf(stderr, "%s\n", k);
		fprintf(stderr, "%s\n", v);
	}

	{
		u8 search[] = "Hello";
		u8 *val = get_page_element_val_ptr_by_key(p, search, sizeof(search));
		if (val == NULL) {
			fprintf(stderr, "Key (%s) not found\n", search);
		} else {
			fprintf(stderr, "Key (%s) found: %s\n", search, val);
		}
	}

	write(1, m, 512);
}
