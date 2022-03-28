#ifndef _CAT_H_
#define _CAT_H_
 
#include "animal_base.h"

typedef struct cat_s_ cat_t;

struct cat_s_ {
    animal_t base; /* 继承自 animal 基类 */

    /* 以下还可以添加与 cat 相关的属性和方法(函数指针), 如: */
    /* char *owner; // cat 的主人 */
    /* void (*hunt)(const char *rabbit); // 猎兔犬 */
};

extern cat_t * cat_init();
extern void cat_die(cat_t *);
 
#endif