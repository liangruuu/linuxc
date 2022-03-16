#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "dog.h"

static void eat(char *food);

static void walk(int steps);

static void talk(char *msg);

dog_t * dog_init()
{
    dog_t *dog = (dog_t *)malloc(sizeof(dog_t));
    animal_t *animal = (animal_t *)animal_init("dog");
    memcpy(&(dog->base), animal, sizeof(animal_t));

    dog->base.animal_ops->eat = eat;
    dog->base.animal_ops->walk = walk;
    dog->base.animal_ops->talk = talk;

    return dog;
}

void dog_die(dog_t *dog)
{
    /* nothing to do here. */
}

static void eat(char *food)
{
    printf("I'm a dog, I eat %s.\n", food);
}

static void walk(int steps)
{
    printf("I'm a dog, I can jump %d steps one time.\n", steps);
}

static void talk(char *msg)
{
    printf("I'm a dog, I talk my language %s.\n", msg);
}