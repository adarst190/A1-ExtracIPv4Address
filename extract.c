#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int parseNumber(const char *s, int minDigits, int maxDigits,
                       int maxValue, int *value)
{
    int len = 0;
    int result = 0;

    while (s[len] != '\0')
    {
        if (!isdigit((unsigned char)s[len]))
            return 0;

        len++;
    }

    if (len < minDigits || len > maxDigits)
        return 0;

    if (len > 1 && s[0] == '0')
        return 0;

    for (int i = 0; i < len; i++)
    {
        result = result * 10 + (s[i] - '0');

        if (result > maxValue)
            return 0;
    }

    *value = result;
    return 1;
}

static int validateToken(const char *token,
                         unsigned long *outAddress,
                         int *outPort)
{
    unsigned long octets[4];
    int octetCount = 0;
    int portValue = -1;
    int i = 0;

    while (token[i] != '\0')
    {
        char buffer[6];
        int digits = 0;
        int value = 0;

        if (octetCount >= 4)
            return 0;

        while (isdigit((unsigned char)token[i]))
        {
            if (digits >= 3)
                return 0;

            buffer[digits++] = token[i++];
        }

        buffer[digits] = '\0';

        if (!parseNumber(buffer, 1, 3, 255, &value))
            return 0;

        octets[octetCount++] = (unsigned long)value;

        if (octetCount < 4)
        {
            if (token[i] != '.')
                return 0;

            i++;
        }
        else
        {
            break;
        }
    }

    if (octetCount != 4)
        return 0;

    if (token[i] == '\0')
    {
        *outPort = -1;
    }
    else
    {
        if (token[i] != ':')
            return 0;

        i++;

        char portBuf[8];
        int digits = 0;

        while (isdigit((unsigned char)token[i]))
        {
            if (digits >= 5)
                return 0;

            portBuf[digits++] = token[i++];
        }

        portBuf[digits] = '\0';

        if (!parseNumber(portBuf, 1, 5, 65535, &portValue))
            return 0;

        *outPort = portValue;
    }

    if (token[i] != '\0')
        return 0;

    *outAddress =
        (octets[0] << 24) |
        (octets[1] << 16) |
        (octets[2] << 8) |
        octets[3];

    return 1;
}

int extractIPv4(const char *str,
                unsigned long *outAddress,
                int *outPort)
{
    int i = 0;

    if (str == NULL || outAddress == NULL || outPort == NULL)
    {
        return 0;
    }

    *outAddress = 0;
    *outPort = -1;

    while (str[i] != '\0')
    {
        if (isdigit((unsigned char)str[i]) ||
            str[i] == '.' ||
            str[i] == ':')
        {
            char token[128];
            int j = 0;
            int overflow = 0;

            while (str[i] != '\0' &&
                   (isdigit((unsigned char)str[i]) ||
                    str[i] == '.' ||
                    str[i] == ':'))
            {
                if (j < (int)sizeof(token) - 1)
                {
                    token[j++] = str[i];
                }
                else
                {
                    overflow = 1;
                }

                i++;
            }

            if (overflow)
            {
                continue;
            }

            token[j] = '\0';

            if (validateToken(token, outAddress, outPort))
            {
                return 1;
            }
        } else
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
        printf("Enter a string (or 'END' to quit): ");

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

            printf(" (decimal value: %lu, port: ", address);

            if (port == -1)
                printf("none");
            else
                printf("%d", port);

            printf(")\n");
        }
        else
        {
            printf("Invalid input: no valid IPv4 address found\n");
        }
    }

    return 0;
}
