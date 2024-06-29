#ifndef DATA_1D_H
#define DATA_1D_H
#include <linux/list.h>
#include <linux/slab.h> // kmalloc

extern int datas[];
extern void list_impl_set_data_len(size_t);
extern size_t data_len;

typedef struct element_t {
    struct list_head list;
    int val;
    int seq;
} element_t;

static int cmp(void *priv, const struct list_head *a, const struct list_head *b) {
    element_t *a_data = list_entry(a, element_t, list);
    element_t *b_data = list_entry(b, element_t, list);
    int *cnt = (int*)priv;
    (*cnt)++;
    return a_data->val - b_data->val;
}

static inline int prepare_data(struct list_head *head){
    element_t *data;
    int i;
    list_impl_set_data_len(data_len);
    for(i = 0; i < data_len; i++){
        data = kmalloc(sizeof(*data), GFP_KERNEL);
        if(!data){
            pr_info("list_sort_tst_OOM!!");
            return -1;
        }
        data->val = datas[i];
        data->seq = i;
        list_add_tail(&data->list, head);
    }
    return 0;
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

static bool check_list(struct list_head *head, size_t statistic)
{
    if (list_empty(head))
        return 0 == statistic;

    element_t *entry, *safe;
    size_t ctr = 0;
    list_for_each_entry_safe (entry, safe, head, list) {
        ctr++;
    }
    int unstable = 0;
    list_for_each_entry_safe (entry, safe, head, list) {
        if (entry->list.next != head) {
            if (entry->val > safe->val) {
                pr_info("ERROR: Wrong order");
                return false;
            }
            if (entry->val == safe->val && entry->seq > safe->seq)
                unstable++;
        }
    }
    if (unstable) {
        pr_info("ERROR: unstable %d", unstable);
        return false;
    }

    if (ctr != statistic) {
        pr_info("ERROR: Inconsistent number of elements: %ld\n", ctr);
        return false;
    }
    return true;
}

#endif // !DATA_1D_H