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
	u8 count;
	page_element elems[];
} page;

void add_page_element(page *p, u8 *k, u16 ksize, u8 *v, u16 vsize) {
	u16 new_element_pos = PAGE_SIZE;
	for (u8 i = 0; i < p->count; i++) {
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

u8 *get_page_element_key_ptr(page *p, u8 elnum) {
	return (u8 *)p + p->elems[elnum].pos;
}

u8 *get_page_element_val_ptr(page *p, u8 elnum) {
	return (u8 *)p + p->elems[elnum].pos + p->elems[elnum].ksize;
}

int main(void) {
	u8 m[512] = {0};
	page *p = (page *)m;

	{
		u8 key[] = "Hello";
		u8 val[] = "World";

		add_page_element(p, key, sizeof(key), val, sizeof(val));
	}

	{
		u8 key[] = "Foo";
		u8 val[] = "Bar";

		add_page_element(p, key, sizeof(key), val, sizeof(val));
	}

	u8 *k = get_page_element_key_ptr(p, 1);
	u8 *v = get_page_element_val_ptr(p, 1);

	fprintf(stderr, "%s\n", k);
	fprintf(stderr, "%s\n", v);

	write(1, m, 512);
}
