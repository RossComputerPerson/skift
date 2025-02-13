/* Copyright © 2018-2019 N. Van Bossuyt.                                      */
/* This code is licensed under the MIT License.                               */
/* See: LICENSE.md                                                            */

#include <libsystem/logger.h>
#include <libsystem/cstring.h>
#include <libmath/math.h>

#include "filesystem.h"

/* --- fsnode --------------------------------------------------------------- */

fsnode_t *fsnode(fsnode_type_t type)
{
    fsnode_t *node = MALLOC(fsnode_t);

    node->type = type;
    node->refcount = 0;
    lock_init(node->lock);

    switch (type)
    {
    case FSNODE_FILE:
    {
        file_t *file = &node->file;

        file->buffer = malloc(512);
        file->realsize = 512;
        file->size = 0;
        break;
    }
    case FSNODE_DIRECTORY:
    {
        directory_t *dir = &node->directory;

        dir->childs = list();
        break;
    }
    case FSNODE_FIFO:
    {
        fifo_t *fifo = &node->fifo;
        fifo->buffer = ringbuffer(4096);

        break;
    }
    default:
        break;
    }

    return node;
}

void fsnode_delete(fsnode_t *node)
{
    logger_info("Fsnode free: %08x", node);

    switch (node->type)
    {
    case FSNODE_FILE:
    {
        free(node->file.buffer);
        break;
    }
    case FSNODE_DIRECTORY:
    {
        list_foreach(item, node->directory.childs)
        {
            fsdirectory_entry_t *entry = item->value;
            fsnode_t *n = entry->node;

            n->refcount--;

            if (n->refcount == 0)
            {
                fsnode_delete(n);
            }
        }

        list_delete(node->directory.childs, LIST_FREE_VALUES);
        break;
    }
    case FSNODE_FIFO:
    {
        ringbuffer_delete(node->fifo.buffer);
        break;
    }
    default:
        break;
    }

    free(node);
}

int fsnode_size(fsnode_t *node)
{
    if (node->type == FSNODE_FILE)
    {
        return node->file.size;
    }

    return 0;
}

iostream_type_t fsnode_type(fsnode_t *this)
{
    switch (this->type)
    {
    case FSNODE_DIRECTORY:
        return IOSTREAM_TYPE_DIRECTORY;
    case FSNODE_FIFO:
        return IOSTREAM_TYPE_FIFO;
    case FSNODE_DEVICE:
        return IOSTREAM_TYPE_DEVICE;
    default:
        return IOSTREAM_TYPE_REGULAR;
    }
}

int fsnode_stat(fsnode_t *node, iostream_stat_t *stat)
{
    stat->type = fsnode_type(node);
    stat->size = fsnode_size(node);

    return 0;
}

/* --- Files ---------------------------------------------------------------- */

void file_trunc(fsnode_t *node)
{
    lock_acquire(node->lock);

    free(node->file.buffer);

    node->file.buffer = malloc(512);
    node->file.realsize = 512;
    node->file.size = 0;

    lock_release(node->lock);
}

int file_read(stream_t *stream, void *buffer, uint size)
{
    int result = 0;

    file_t *file = &stream->node->file;

    if (stream->offset <= file->size)
    {
        int readedsize = min(file->size - stream->offset, size);
        memcpy(buffer, (byte *)file->buffer + stream->offset, readedsize);

        result = readedsize;
        stream->offset += readedsize;
    }

    return result;
}

int file_write(stream_t *stream, const void *buffer, uint size)
{
    int result = 0;

    file_t *file = &stream->node->file;

    if (stream->flags & IOSTREAM_APPEND)
    {
        stream->offset = file->size;
    }

    if ((stream->offset + size) > file->realsize)
    {
        file->buffer = realloc(file->buffer, stream->offset + size);
        file->realsize = stream->offset + size;
    }

    file->size = max(stream->offset + size, file->size);
    memcpy((byte *)(file->buffer) + stream->offset, buffer, size);

    result = size;
    stream->offset += size;

    return result;
}

/* --- Fifo ----------------------------------------------------------------- */

int fifo_read(stream_t *stream, void *buffer, uint size)
{
    fifo_t *fifo = &stream->node->fifo;
    int result = ringbuffer_read(fifo->buffer, buffer, size);
    return result;
}

int fifo_write(stream_t *stream, const void *buffer, uint size)
{
    fifo_t *fifo = &stream->node->fifo;
    int result = ringbuffer_write(fifo->buffer, buffer, size);
    return result;
}

/* --- Directories ---------------------------------------------------------- */

// only call this method if you hold the directory lock.
fsdirectory_entry_t *directory_entry(fsnode_t *dir, const char *child)
{
    lock_assert(dir->lock);

    list_foreach(i, dir->directory.childs)
    {
        fsdirectory_entry_t *entry = (fsdirectory_entry_t *)i->value;

        if (strcmp(child, entry->name) == 0)
        {
            return entry;
        }
    }

    return NULL;
}

bool directory_has_entry(fsnode_t *dir, const char *child)
{
    lock_acquire(dir->lock);
    bool has_entry = directory_entry(dir, child) != NULL;
    lock_release(dir->lock);

    return has_entry;
}

directory_entries_t directory_entries(fsnode_t *dir)
{
    lock_acquire(dir->lock);

    int entries_count = dir->directory.childs->count;
    iostream_direntry_t *entries = malloc(sizeof(iostream_direntry_t) * entries_count);
    iostream_direntry_t *current = &entries[0];

    list_foreach(i, dir->directory.childs)
    {
        fsdirectory_entry_t *entry = (fsdirectory_entry_t *)i->value;

        strlcpy(current->name, entry->name, PATH_ELEMENT_LENGHT);
        fsnode_stat(entry->node, &current->stat);

        current++;
    }

    lock_release(dir->lock);

    return (directory_entries_t){.count = entries_count, .entries = entries};
}

bool directory_link(fsnode_t *dir, fsnode_t *child, const char *name)
{
    lock_acquire(dir->lock);

    if (directory_entry(dir, name) == NULL)
    {
        fsdirectory_entry_t *entry = MALLOC(fsdirectory_entry_t);

        strlcpy(entry->name, name, PATH_ELEMENT_LENGHT);
        child->refcount++;
        entry->node = child;

        list_pushback(dir->directory.childs, entry);

        lock_release(dir->lock);
        return true;
    }
    else
    {
        lock_release(dir->lock);
        return false;
    }
}

bool directory_unlink(fsnode_t *dir, const char *name)
{
    lock_acquire(dir->lock);
    fsdirectory_entry_t *entry = directory_entry(dir, name);

    if (entry != NULL)
    {
        fsnode_t *node = entry->node;

        list_remove(dir->directory.childs, entry);
        free(entry);

        node->refcount--;

        if (node->refcount == 0)
        {
            fsnode_delete(node);
        }

        lock_release(dir->lock);
        return true;
    }
    else
    {
        lock_release(dir->lock);
        return false;
    }
}

int directory_read(stream_t *stream, void *buffer, uint size)
{
    int index = stream->offset / sizeof(iostream_direntry_t);

    if (size == sizeof(iostream_direntry_t))
    {
        if (index < stream->direntries.count)
        {
            int entrysize = sizeof(iostream_direntry_t);

            memcpy(buffer, &stream->direntries.entries[index], entrysize);
            stream->offset += entrysize;

            return entrysize;
        }
        else
        {
            return 0;
        }
    }
    else
    {
        logger_warn("Directory read fail!");
        return -1;
    }
}