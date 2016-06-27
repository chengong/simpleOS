#include "debug.h"
#include "pmm.h"
#include "vmm.h"
#include "heap.h"

static void alloc_chunk(uint32_t start, uint32_t len);

static void free_chunk(header_t *chunk);

static void split_chunk(header_t *chunk, uint32_t len);

static void glue_chunk(header_t *chunk);

static uint32_t heap_max = HEAP_START;

static header_t *heap_first;

void init_heap()
{
    heap_first = 0;
}

void *kmalloc(uint32_t len)
{
    len += sizeof(header_t);

    header_t *cur_header = heap_first;
    header_t *pre_header = 0;

    while (cur_header) {
        if (cur_header->allocated == 0 && cur_header->length >= len) {
            split_chunk(cur_header, len);
            cur_header->allocated = 1;
            return (void *)((uint32_t)cur_header + sizeof(header_t));
        }

        pre_header = cur_header;
        cur_header = cur_header->next;
    }

    uint32_t chunk_start;
    
    if (pre_header) {
        chunk_start = (uint32_t)pre_header + pre_header->length;
    } else {
        chunk_start = HEAP_START;
        heap_first  = (header_t *)chunk_start;
    }

    alloc_chunk(chunk_start, len);
    cur_header = (header_t *) chunk_start;
    cur_header->prev = pre_header;
    cur_header->next = 0;
    cur_header->allocated = 1;
    cur_header->length = len;

    if (pre_header) {
        pre_header->next = cur_header;
    }
    
    return (void *)(chunk_start+ sizeof(header_t));
}

void kfree(void *p)
{
    header_t *header = (header_t*)((uint32_t)p - sizeof(header_t));
    header->allocated = 0;

    glue_chunk(header);
}

void alloc_chunk(uint32_t start, uint32_t len)
{
    while (start + len > heap_max) {
        uint32_t page = pmm_alloc_page();
        map(pgd_kern, heap_max, page, PAGE_PRESENT | PAGE_WRITE);
        heap_max += PAGE_SIZE;
    }
}

void free_chunk(header_t *chunk)
{
    if (chunk->prev == 0) {
        heap_first = 0;
    } else {
        chunk->prev->next = 0;
    }

    while ((heap_max - PAGE_SIZE) >= (uint32_t)chunk) {
        heap_max -= PAGE_SIZE;
        uint32_t page;
        get_mapping(pgd_kern, heap_max, &page);
        unmap(pgd_kern, heap_max);
        pmm_free_page(page);
    }
}

void split_chunk(header_t *chunk, uint32_t len)
{
    if (chunk->length - len > sizeof (header_t)) {
        header_t *new_chunk = (header_t *)((uint32_t)chunk + chunk + len);
        new_chunk->prev = chunk;
        new_chunk->next = chunk->next;
        new_chunk->allocated = 0;
        new_chunk->length = chunk->length - len - sizeof(header_t);

        chunk->next = new_chunk;
        chunk->length = len ;
    }
}

void glue_chunk(header_t *chunk)
{
    if (chunk->next && chunk->next->allocated == 0) {
        chunk->length = chunk->length + chunk->next->length;
        if (chunk->next->next) {
            chunk->next->next->prev = chunk;
        }
        chunk->next->next = chunk->next;
    }

    if (chunk->prev && chunk->prev->allocated == 0) {
        chunk->prev->length = chunk->prev->length + chunk->length;
        chunk->prev->next = chunk->next;
        if (chunk->next) {
            chunk->next->prev = chunk->prev;
        }
        chunk = chunk->prev;
    }

    if(chunk->next == 0) {
        free_chunk(chunk);
    }
}

void test_heap()
{
    printk_color(rc_black, rc_magenta, "Test kmalloc() and kfree() now \n");

    void *addr1 = kmalloc(50);
    printk("kmalloc    50 bytes in 0x%X\n", addr1);
    void *addr2 = kmalloc(500);
    printk("kmalloc   500 bytes in 0x%X\n", addr2);
    void *addr3 = kmalloc(5000);
    printk("kmalloc  5000 bytes in 0x%X\n", addr3);
    void *addr4 = kmalloc(50000);
    printk("kmalloc 50000 bytes in 0x%X\n", addr4);

    printk("free mem in 0x%X\n", addr1);
    kfree(addr1);
    printk("free mem in 0x%X\n", addr2);
    kfree(addr2);
    printk("free mem in 0x%X\n", addr3);
    kfree(addr3);
    printk("free mem in 0x%X\n", addr4);
    kfree(addr4);
}

