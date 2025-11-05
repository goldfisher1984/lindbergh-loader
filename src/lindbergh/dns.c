#include <stdio.h>
#include <unistd.h>
#include <string.h> /* for strncpy */
#include <stdlib.h>
#include <stdbool.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <net/if.h>
#include <arpa/inet.h>
#include <dlfcn.h>
#include <netdb.h>
#include "config.h"
#include "dns.h"

struct dns_hook_entry *dns_hook_entries;
int dns_hook_nentries;

int dns_hook_push(const char *from_src, const char *to_src)
{
    struct dns_hook_entry *newmem;
    struct dns_hook_entry *newitem;

    newmem = realloc(dns_hook_entries, (dns_hook_nentries + 1) * sizeof(struct dns_hook_entry));

    if (newmem == NULL)
    {
        return -1;
    }

    dns_hook_entries = newmem;
    newitem = &newmem[dns_hook_nentries++];
    strcpy(newitem->from, from_src);
    strcpy(newitem->to, to_src);

    return 1;
}

void dns_entry_init()
{
    int ret = 0;
    ret = dns_hook_push("tenporouter.loc", getConfig()->dns_router);
    if (ret == -1)
        return;

    ret = dns_hook_push("bbrouter.loc", getConfig()->dns_router);
    if (ret == -1)
        return;

    ret = dns_hook_push("naominet.jp", getConfig()->dns_default);
    if (ret == -1)
        return;

    ret = dns_hook_push("sc.naominet.jp", getConfig()->dns_default);
    if (ret == -1)
        return;

    ret = dns_hook_push("ib.naominet.jp", getConfig()->dns_default);
    if (ret == -1)
        return;
}

bool match_domain(const char *target, const char *pattern)
{
    if (strcmp(pattern, target) == 0)
    {
        return true;
    }

    int pattern_ptr_index = 0;
    int target_ptr_index = 0;

    while (pattern[pattern_ptr_index] != '\0' && target[target_ptr_index] != '\0')
    {
        if (pattern[pattern_ptr_index] == '*')
        {
            pattern_ptr_index++; // Check next character for wildcard match.

            while (pattern[pattern_ptr_index] != target[target_ptr_index])
            {
                target_ptr_index++;

                if (target[target_ptr_index] == '\0')
                    return false;
            }
        }
        else if (pattern[pattern_ptr_index] != target[target_ptr_index])
        {
            return false;
        }
        else
        {
            pattern_ptr_index++;
            target_ptr_index++;
        }
    }

    return pattern[pattern_ptr_index] == '\0' && target[target_ptr_index] == '\0';
}

char **StrToAscii(char *src, int *output_count)
{
    int length = strlen(src);
    if (length == 0)
        return NULL;

    *output_count = (length + 3) / 4;
    char **chunks = (char **)malloc(*output_count * sizeof(char *));
    if (!chunks)
        return NULL;

    for (int i = 0; i < *output_count; i++)
    {
        chunks[i] = (char *)malloc(9);
        if (!chunks[i])
        {
            for (int j = 0; j < i; j++)
                free(chunks[j]);
            free(chunks);
            return NULL;
        }

        int start = i * 4;
        int pos = 0;

        for (int j = 0; j < 4; j++)
        {
            if (start + j < length)
            {
                pos += sprintf(chunks[i] + pos, "%02x", (unsigned char)src[start + j]);
            }
            else
            {
                pos += sprintf(chunks[i] + pos, "00");
            }
        }
        chunks[i][pos] = '\0';
    }

    return chunks;
}

void free_ascii_chunks(char **chunks, int count)
{
    if (chunks == NULL) {
        return;
    }
    
    for (int i = 0; i < count; i++) {
        if (chunks[i] != NULL) {
            free(chunks[i]);
        }
    }
    free(chunks);
}