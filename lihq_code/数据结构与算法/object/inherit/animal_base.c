#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "animal_base.h"

/* 基类的构造函数，需要显示调用 */
animal_t * animal_init(char *name)
{
    assert(name != NULL);
    size_t name_len = strlen(name);

    animal_t *animal = (animal_t *)malloc(sizeof(animal_t)
            + sizeof(animal_ops_t) + name_len + 1);
    memset(animal, 0, (sizeof(animal_t) + sizeof(animal_ops_t)
                + name_len + 1));
    animal->name = (char *)animal + sizeof(animal_t);
    memcpy(animal->name, name, name_len);
    animal->animal_ops = (animal_ops_t *)((char *)animal
            + sizeof(animal_t) + name_len + 1);

    return animal;
}

/* 基类的有关操作，如吃，走，说等等 */
void animal_eat(void * p, char *food)
{
    animal_t *animal = p; 
    animal->animal_ops->eat(food);
    return;
}

void animal_walk(void * p, int steps)
{
    animal_t *animal = p; 
    animal->animal_ops->walk(steps);
    return;
}

void animal_talk(void * p, char *msg)
{
    animal_t *animal = p; 
    animal->animal_ops->talk(msg);
    return;
}

/* 基类的析构函数，需要显示调用 */
void animal_die(void * p)
{
    
    animal_t *animal = p; 
    assert(animal != NULL);

    free(animal);
    return;
}