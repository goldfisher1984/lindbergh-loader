#include <netdb.h>
void dns_entry_init();

#define MAX_DNS_LENGTH 256
struct dns_hook_entry
{
    char from[MAX_DNS_LENGTH];
    char to[MAX_DNS_LENGTH];
};

bool match_domain(const char *target, const char *pattern);
char **StrToAscii(char *src, int *output_count);