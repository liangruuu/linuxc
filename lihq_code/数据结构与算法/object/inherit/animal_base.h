#ifndef _ANIMAL_H_
#define _ANIMAL_H_

typedef struct animal_s_ animal_t;
typedef struct animal_ops_s_ animal_ops_t;


/* 动物类，是所有动物类的基类,也是抽象类 */
struct animal_s_ {
    char *name; /* 动物的名称 */
    animal_ops_t *animal_ops; /* 动物的基本行为 */
};

/* 动物的基本行为 */
struct animal_ops_s_ {
    /* 动物吃了什么食物 */
    void (*eat)(char *food);
    /* 动物走了多少步 */
    void (*walk)(int steps);
    /* 动物在说什么 */
    void (*talk)(char *msg);
};

/* 基类的构造函数，需要显示调用 */
extern animal_t * animal_init(char *name);

/* 基类的有关操作，如吃，走，说等等 */
extern void animal_eat(void *, char *);
extern void animal_walk(void *, int);
extern void animal_talk(void *, char *);

/* 基类的析构函数，需要显示调用 */
extern void animal_die(void *);

#endif