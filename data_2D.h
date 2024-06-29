#ifndef DATA_2D_H
#define DATA_2D_H
#include <linux/list.h>
#include <linux/slab.h> // kmalloc

extern int datas[][2];
extern void list_impl_set_data_len(size_t);
extern size_t data_len;

typedef struct element_t {
    struct list_head list;
    int val1;
    int val2;
    int seq;
} element_t;

static int cmp(void *priv, const struct list_head *a, const struct list_head *b) {
    element_t *a_data = list_entry(a, element_t, list);
    element_t *b_data = list_entry(b, element_t, list);
    int *cnt = (int*)priv;
    (*cnt)++;
    int diff = a_data->val1 - b_data->val1;
    if(diff)
        return diff;
        
    return a_data->val2 - b_data->val2;
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
        data->val1 = datas[i][0];
        data->val2 = datas[i][1];
        data->seq = i;
        list_add_tail(&data->list, head);
    }
    return 0;
}

static void refill_list_data(struct list_head* head){
    element_t *ele_entry;
    int i = 0;
    list_for_each_entry(ele_entry, head, list){
        ele_entry->val1 = datas[i][0];
        ele_entry->val2 = datas[i][1];
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
            if (entry->val1 > safe->val1 || (entry->val1 == safe->val1 && entry->val2 > safe->val2)) {
                pr_info("ERROR: Wrong order");
                return false;
            }

            if ((entry->val1 == safe->val1) && (entry->val2 == safe->val2) && entry->seq > safe->seq)
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


#endif // !DATA_2D_H