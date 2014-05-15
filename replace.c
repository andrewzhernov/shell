#include "replace.h"

int replace(char** text, char* pattern, char* target) {
    char* new_text;
    size_t capacity = REPLACE_PAGE_SIZE;
    size_t target_size = strlen(target);
    size_t last = 0;
    size_t i = 0;

    if (pattern == NULL || target == NULL || text == NULL || *text == NULL)
        return REPLACE_INVALID_POINTER;
    if (*pattern == '\0')
        return REPLACE_EMPTY_PATTERN;

    new_text = (char*)malloc(capacity * sizeof(char));
    if (new_text == NULL)
        return REPLACE_NOT_ENOUGH_MEMORY;

    while ((*text)[i] != '\0') 
    {
        size_t j = 0;
        size_t count = target_size;
        while (pattern[j] != '\0' && (*text)[i + j] != '\0' && (*text)[i + j] == pattern[j])
        {
            ++j;
        }
        if (pattern[j] != '\0')
             count = j;

        if (last + count >= capacity)
        {
            char *ptr;
            while (last + count >= capacity)
                capacity += REPLACE_PAGE_SIZE;
            ptr = (char*)realloc(new_text, capacity);
            if (ptr = NULL)
                return REPLACE_NOT_ENOUGH_MEMORY;
            new_text = ptr;
        }
        if (j == 0)
            new_text[last++] = (*text)[i++];
        else if (pattern[j] == '\0')
        {
            size_t index = 0;
            while (target[index] != '\0')
                new_text[last++] = target[index++];
            i += j;
        }
        else
        {
            size_t index;
            for (index = 0; index < j; ++index)
                new_text[last++] = (*text)[i++];
        }
    }
    
    free(*text);

    new_text[last] = '\0';
    *text = new_text;
    return REPLACE_SUCCESS_RETVAL;
}
