/* Copyright © 2018-2019 N. Van Bossuyt.                                      */
/* This code is licensed under the MIT License.                               */
/* See: LICENSE.md                                                            */

#include <libsystem/list.h>

list_t *list()
{
    list_t *l = MALLOC(list_t);

    l->count = 0;
    l->head = NULL;
    l->tail = NULL;

    return l;
}

void list_delete(list_t *l, list_delete_action_t free_items)
{
    list_clear(l, free_items);
    free(l);
}

void list_clear(list_t *list, list_delete_action_t free_items)
{
    list_item_t *current = list->head;

    while (current)
    {
        list_item_t *next = current->next;
        
        if (free_items == LIST_FREE_VALUES) 
        {
            free(current->value);
        }
        else if (free_items == LIST_RELEASE_VALUES) 
        {
            object_release(current->value);
        }

        free(current);
        current = next;
    }

    list->count = 0;
    list->head = NULL;
    list->tail = NULL;
}

void list_insert_sorted(list_t *list, void *value, list_comparator_t comparator)
{
    if (list->head == NULL || comparator(value, list->head->value))
    {
        list_push(list, value);
    }
    else
    {
        list_item_t *current = list->head;

        while (current->next != NULL && comparator(current->next->value, value))
        {
            current = current->next;
        }

        list_item_t *item = MALLOC(list_item_t);

        item->prev = current;
        item->next = current->next;
        item->value = value;

        if (current->next == NULL)
        {
            list->tail = item;
        }
        else
        {
            current->next->prev = item;
        }

        current->next = item;

        list->count++;
    }
}

bool list_peek(list_t *list, void **value)
{
    if (list->head != NULL)
    {
        *value = list->head->value;
        return true;
    }
    else
    {
        *value = NULL;
        return false;
    }
}

bool list_peek_and_pushback(list_t* l, void** value)
{
    if (list_peek(l, value))
    {
        list_item_t *item = l->head;

        // Pop
        if (l->count == 1)
        {
            l->count = 0;
            l->head = NULL;
            l->tail = NULL;
        }
        else if (l->count > 1)
        {
            item->next->prev = NULL;
            l->head = item->next;

            l->count--;
        }

        // Push back
        item->prev = NULL;
        item->next = NULL;

        l->count++;

        if (l->tail == NULL)
        {
            l->tail = item;
            l->head = item;
        }
        else
        {
            l->tail->next = item;
            item->prev = l->tail;
            l->tail = item;
        }

        return true;
    }
    else
    {
        return false;
    }
}

bool list_peekback(list_t *list, void **value)
{
    if (list->tail != NULL)
    {
        *value = list->tail->value;
        return true;
    }
    else
    {
        return false;
    }
}

bool list_peekat(list_t *list, int index, void **value)
{
    if (index < list->count)
    {
        list_item_t* current = list->head;

        for(int i = 0; i < index; i++)
        {
            current = current->next;
        }
        
        *value = current->value;

        return true;
    }
    else
    {
        return false;
    }
}

void list_push(list_t *l, void *value)
{
    list_item_t *item = MALLOC(list_item_t);

    item->prev = NULL;
    item->next = NULL;
    item->value = value;

    l->count++;

    if (l->head == NULL)
    {
        l->head = item;
        l->tail = item;
    }
    else
    {
        l->head->prev = item;
        item->next = l->head;
        l->head = item;
    }
}

bool list_pop(list_t *l, void **value)
{
    list_item_t *item = l->head;

    if (l->count == 0)
    {
        return false;
    }
    else if (l->count == 1)
    {
        l->count = 0;
        l->head = NULL;
        l->tail = NULL;
    }
    else if (l->count > 1)
    {
        item->next->prev = NULL;
        l->head = item->next;

        l->count--;
    }

    if (value != NULL)
        *(value) = item->value;

    free(item);

    return true;
}

void list_pushback(list_t *l, void *value)
{
    list_item_t *item = MALLOC(list_item_t);

    item->prev = NULL;
    item->next = NULL;
    item->value = value;

    l->count++;

    if (l->tail == NULL)
    {
        l->tail = item;
        l->head = item;
    }
    else
    {
        l->tail->next = item;
        item->prev = l->tail;
        l->tail = item;
    }
}

bool list_popback(list_t *l, void **value)
{
    list_item_t *item = l->tail;

    if (l->count == 0)
    {
        return NULL;
    }
    else if (l->count == 1)
    {
        l->count = 0;
        l->head = NULL;
        l->tail = NULL;
    }
    else if (l->count > 1)
    {
        item->prev->next = NULL;
        l->tail = item->prev;

        l->count--;
    }

    if (value != NULL)
        *(value) = item->value;
        
    free(item);

    return true;
}

bool list_remove(list_t *l, void *value)
{
    list_foreach(item, l)
    {
        if (item->value == value)
        {
            if (item->prev != NULL)
            {
                item->prev->next = item->next;
            }
            else
            {
                l->head = item->next;
            }

            if (item->next != NULL)
            {
                item->next->prev = item->prev;
            }
            else
            {
                l->tail = item->prev;
            }

            l->count--;
            free(item);

            return true;
        }
    }

    return false;
}

bool list_containe(list_t *l, void *value)
{
    list_foreach(item, l)
    {
        if (item->value == value)
        {
            return true;
        }
    }

    return false;
}