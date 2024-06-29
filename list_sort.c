#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/list.h>
#include <linux/slab.h> // kmalloc
#include <linux/list_sort.h>
#include <linux/ktime.h>

#define TEST_FILE "datasets/xfs_ext_busy_1.h"
#include TEST_FILE
#define ITERS 4000
// #include "data_1D.h"
#include "data_2D.h"
size_t data_len = sizeof(datas) / sizeof(datas[0]);


extern void adaptive_ShiversSort(void*, struct list_head*, list_cmp_func_t);
extern void power_sort(void*, struct list_head*, list_cmp_func_t);
extern void timsort(void*, struct list_head*, list_cmp_func_t);

typedef void (*test_func_t)(void *priv,
                            struct list_head *head,
                            list_cmp_func_t cmp);
typedef struct {
    char *name;
    test_func_t impl;
} test_t;

typedef void (*sample_func_t)(struct list_head *head, element_t *space, int samples);
typedef struct {
    char *name;
    sample_func_t impl;
} sample_t;

static struct list_head my_list;
test_t tests[] = {
    {.name = "timsort", .impl = timsort},
    {.name = "adaptive_ShiversSort", .impl = adaptive_ShiversSort},
    {.name = "powerSort", .impl = power_sort},
    {.name = "listSort", .impl = list_sort},
    {NULL, NULL},
};

static int __init my_module_init(void) {
    ktime_t start_time, stop_time, elapsed_time;
    int i;
    int cmp_cnt = 0;
    test_t *test;
    pr_info("Start test on test_file = %s", TEST_FILE);

    test = tests;    
    // prepare data
    INIT_LIST_HEAD(&my_list);
    int err = prepare_data(&my_list);
    if(err){
        return -ENOMEM;
    }

    elapsed_time = 0;
    local_irq_disable(); // disable interrupt
    get_cpu();          // disable preemption
    while (test->impl)
    {
        for(i = 0; i < ITERS; i++){
            start_time = ktime_get();
            test->impl(&cmp_cnt, &my_list, cmp);
            stop_time = ktime_get();
            elapsed_time += stop_time - start_time;
            refill_list_data(&my_list);
        }
        printk(KERN_INFO "List done sort with %s, iter = %d, data_len = %lu:\n", test->name, ITERS, data_len);
        pr_info("Comparison cnt: %d", cmp_cnt);
        pr_info("elapsed time: %llu ns", elapsed_time);
        refill_list_data(&my_list);
        cmp_cnt = 0;
        elapsed_time = 0;
        test++;
    }
    local_irq_enable();
    put_cpu();
    return 0;
}

static void __exit my_module_exit(void) {
    struct element_t *data, *tmp;
    list_for_each_entry_safe(data, tmp, &my_list, list) {
        list_del(&data->list);
        kfree(data);
    }
    printk(KERN_INFO "Module exit, memory freed.\n");
}

module_init(my_module_init);
module_exit(my_module_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("GPT");
MODULE_DESCRIPTION("A simple kernel module to demonstrate list sorting using list_sort.c");