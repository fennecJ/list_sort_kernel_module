#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/list.h>
#include <linux/slab.h> // kmalloc
#include <linux/list_sort.h>
#include <linux/ktime.h>
// #include "sort_impl.h"

#define TEST_FILE "perf_metric_list.h"
#define ITERS 4000

#include TEST_FILE

// extern int datas[];
// extern int data_len;
extern void list_impl_set_data_len(size_t);
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

typedef struct element_t {
    struct list_head list;
    int val;
    int seq;
} element_t;


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


static int cmp(void *priv, const struct list_head *a, const struct list_head *b) {
    element_t *a_data = list_entry(a, element_t, list);
    element_t *b_data = list_entry(b, element_t, list);
    int *cnt = (int*)priv;
    (*cnt)++;
    return a_data->val - b_data->val;
}

static void refill_list_data(struct list_head* head){
    element_t *ele_entry;
    int i = 0;
    list_for_each_entry(ele_entry, head, list){
        ele_entry->val = datas[i];
        ele_entry->seq = i;
        ++i;
    }
}

static int __init my_module_init(void) {
    ktime_t start_time, stop_time, elapsed_time;
    element_t *data;
    int i;
    int cmp_cnt = 0;
    test_t *test;
    size_t data_len = sizeof(datas) / sizeof(int);
    list_impl_set_data_len(data_len);
    pr_info("Start test on test_file = %s", TEST_FILE);
    // if(data_len != SAMPLES){
    //     pr_info("Data len is not match SAMPLES");
    // }

    test = tests;    
    // prepare data
    INIT_LIST_HEAD(&my_list);
    for(i = 0; i < data_len; i++){
        data = kmalloc(sizeof(*data), GFP_KERNEL);
        if(!data){
            pr_info("list_sort_tst_OOM!!");
            return -ENOMEM;
        }
        data->val = datas[i];
        data->seq = i;
        list_add_tail(&data->list, &my_list);
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