#include <stdio.h>
#include <stdlib.h>

#include "animal_base.h"
#include "dog.h"
#include "cat.h"

// 实现类的继承和多态

int main()
{
    dog_t *dog = dog_init();
    cat_t *cat = cat_init();

    /* dog 类测试 */
    animal_eat(dog, "bones");
    animal_walk(dog, 5);
    animal_talk(dog, "wang wang wang...");

    /* cat 类测试 */
    animal_eat(cat, "fish");
    animal_walk(cat, 3);
    animal_talk(cat, "miao miao miao...");

    exit(0);
}