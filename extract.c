#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

static int validNumber(const char *s, int minDigits, int maxDigits,
                       int minValue, int maxValue, long *value)
{
    int len = strlen(s);

    if (len < minDigits || len > maxDigits)
        return 0;

    for (int i = 0; i < len; i++)
    {
        if (!isdigit((unsigned char)s[i]))
            return 0;
    }

    /* No leading zeros unless the value is exactly 0 */
    if (len > 1 && s[0] == '0')
        return 0;

    long v = strtol(s, NULL, 10);

    if (v < minValue || v > maxValue)
        return 0;

    *value = v;
    return 1;
}

static int validateToken(const char *token,
                         unsigned long *outAddress,
                         int *outPort)
{
    char copy[128];
    char addressPart[128];
    char *colonPos;
    long portValue;
    int octetCount = 0;
    unsigned long octets[4];

    if (strlen(token) >= sizeof(copy))
        return 0;

    strcpy(copy, token);

    /* Check for optional port */
    colonPos = strchr(copy, ':');

    if (colonPos)
    {
        /* Only one colon allowed */
        if (strchr(colonPos + 1, ':'))
            return 0;

        *colonPos = '\0';

        if (!validNumber(colonPos + 1,
                         1, 5,
                         0, 65535,
                         &portValue))
            return 0;

        *outPort = (int)portValue;
    }
    else
    {
        *outPort = -1;
    }

    strcpy(addressPart, copy);

    char *saveptr;
    char *part = strtok_r(addressPart, ".", &saveptr);

    while (part)
    {
        long value;

        if (octetCount >= 4)
            return 0;

        if (!validNumber(part,
                         1, 3,
                         0, 255,
                         &value))
            return 0;

        octets[octetCount++] = (unsigned long)value;
        part = strtok_r(NULL, ".", &saveptr);
    }

    if (octetCount != 4)
        return 0;

    /* Ensure exactly 3 periods */
    int dots = 0;
    for (int i = 0; copy[i]; i++)
    {
        if (copy[i] == '.')
            dots++;
    }

    if (dots != 3)
        return 0;

    *outAddress =
        (octets[0] << 24) |
        (octets[1] << 16) |
        (octets[2] << 8)  |
         octets[3];

    return 1;
}

int extractIPv4(const char *str,
                unsigned long *outAddress,
                int *outPort)
{
    int i = 0;

    *outAddress = 0;
    *outPort = -1;

    while (str[i])
    {
        if (isdigit((unsigned char)str[i]) ||
            str[i] == '.' ||
            str[i] == ':')
        {
            char token[128];
            int j = 0;
            int start = i;

            while (str[i] &&
                   (isdigit((unsigned char)str[i]) ||
                    str[i] == '.' ||
                    str[i] == ':'))
            {
                if (j < (int)sizeof(token) - 1)
                    token[j++] = str[i];

                i++;
            }

            token[j] = '\0';

            if (validateToken(token, outAddress, outPort))
                return 1;
        }
        else
        {
            i++;
        }
    }

    return 0;
}

int main(void)
{
    char input[1024];
    unsigned long address;
    int port;

    while (1)
    {
        printf("Enter text: ");
        if (!fgets(input, sizeof(input), stdin))
            break;

        input[strcspn(input, "\n")] = '\0';

        if (strcmp(input, "END") == 0)
        {
            printf("Program terminated\n");
            break;
        }

        if (extractIPv4(input, &address, &port))
        {
            printf("Extracted IPv4 address: ");

            printf("%lu.%lu.%lu.%lu",
                   (address >> 24) & 255,
                   (address >> 16) & 255,
                   (address >> 8) & 255,
                   address & 255);

            printf(" (decimal value: %lu, port: ",
                   address);

            if (port == -1)
                printf("NONE");
            else
                printf("%d", port);

            printf(")\n");
        }
        else
        {
            printf("No valid IPv4 address found\n");
        }
    }

    return 0;
}