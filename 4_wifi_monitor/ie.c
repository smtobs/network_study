#include "ie.h"

static unsigned char ieee80211_oui[3] = {0x00, 0x0f, 0xac};

static void print_cipher(const uint8_t *data)
{
	printf("Cipher:\t\t");
    if (memcmp(data, ieee80211_oui, 3) == 0)
    {
        switch (data[3])
        {
        case 0:
            printf("Use group cipher suite");
            break;
        case 1:
            printf("WEP-40");
            break;
        case 2:
            printf("TKIP");
            break;
        case 4:
            printf("CCMP");
            break;
        case 5:
            printf("WEP-104");
            break;
        case 6:
            printf("AES-128-CMAC");
            break;
        case 7:
            printf("NO-GROUP");
            break;
        case 8:
            printf("GCMP");
            break;
        default:
            printf("%.02x-%.02x-%.02x:%d",
                   data[0], data[1], data[2], data[3]);
            break;
        }
    }
    else
    {
        printf("%.02x-%.02x-%.02x:%d",
               data[0], data[1], data[2], data[3]);
    }
    printf("\n");
}

static void print_auth(const uint8_t *data)
{
	printf("Auth :\t\t");
    if (memcmp(data, ieee80211_oui, 3) == 0)
    {
        switch (data[3])
        {
        case 1:
            printf("IEEE 802.1X");
            break;
        case 2:
            printf("PSK");
            break;
        case 3:
            printf("FT/IEEE 802.1X");
            break;
        case 4:
            printf("FT/PSK");
            break;
        case 5:
            printf("IEEE 802.1X/SHA-256");
            break;
        case 6:
            printf("PSK/SHA-256");
            break;
        case 7:
            printf("TDLS/TPK");
            break;
        case 8:
            printf("SAE");
            break;
        case 9:
            printf("FT/SAE");
            break;
        case 11:
            printf("IEEE 802.1X/SUITE-B");
            break;
        case 12:
            printf("IEEE 802.1X/SUITE-B-192");
            break;
        case 13:
            printf("FT/IEEE 802.1X/SHA-384");
            break;
        case 14:
            printf("FILS/SHA-256");
            break;
        case 15:
	    printf("FILS/SHA-384");
            break;
        case 16:
            printf("FT/FILS/SHA-256");
            break;
        case 17:
            printf("FT/FILS/SHA-384");
            break;
        case 18:
            printf("OWE");
            break;
        default:
            printf("%.02x-%.02x-%.02x:%d",
                   data[0], data[1], data[2], data[3]);
            break;
        }
    }
    printf("\n");
}

static void rsn_ie_handler(char *ie, int ie_len)
{
    uint16_t count;
    int i;

    if (ie == NULL || ie_len <= 0)
    {
        printf("error parse_rsn_ie ie=[%p] ie_len=[%d]\n", ie, ie_len);
        return;
    }

    /* SKIP Version */
    ie += 2;
    ie_len -= 2;

    /* SKIP Group cipher */
    ie += 4;
    ie_len -= 4;

    /* Pairwise cipher count */
    if (ie_len < 2)
    {
        printf("\t * Pairwise ciphers: \n");
        return;
    }

    count = ie[0] | (ie[1] << 8);
    if (2 + (count * 4) > ie_len)
    {
        printf("Pairwise Error\n");
    }

    /* Pairwise ciphers */
    for (i = 0; i < count; i++)
    {
        print_cipher(ie + 2 + (i * 4));
    }

        /* AKM Count */
    ie += 2 + (count * 4);
    ie_len -= 2 + (count * 4);

    if (ie_len < 2)
    {
        printf("\t * Authentication suites: \n");
        return;
    }

    /* AKM */
    count = ie[0] | (ie[1] << 8);
    if (2 + (count * 4) > ie_len)
    {
        printf("AKM Error\n");
        return;
    }

    for (i = 0; i < count; i++)
    {
        print_auth(ie + 2 + (i * 4));
    }
}

void ie_handler(char *ie, int ie_len)
{
        while (ie_len > 2)
        {
            switch (ie[IE_TYPE])
            {
            case IE_SSID:
                printf("SSID:\t\t");
                fwrite(ie + 2, ie[IE_LEN], 1, stdout);
                printf("\n");
                break;

            case IE_SUPPORTED_RATES: // IE_SUPPORTED_RATES is 1
                printf("Supported Rates: ");
                for (int i = 0; i < ie[IE_LEN]; i++)
                {
                    printf("%.1f Mbps ", (ie[IE_VALUE + i] & 0x7F) / 2.0);
                }
                printf("\n");
                break;

            case IE_COUNTRY: // IE_COUNTRY is 7
                printf("Country Code:\t");
                fwrite(ie + 2, 2, 1, stdout); // Country code is the first 2 characters of IE value
                printf("\n");
                break;

            case IE_RSN:
                rsn_ie_handler(ie + 2, ie[IE_LEN]);
                break;

            case IE_DTIM:
                if (ie[IE_LEN] == 3)
                {
                    uint8_t dtim_count = ie[IE_VALUE];
                    uint8_t dtim_period = ie[IE_VALUE + 1];
                    uint8_t bitmap_ctrl = ie[IE_VALUE + 2];
                    printf("DTIM Count: %d\n", dtim_count);
                    printf("DTIM Period: %d\n", dtim_period);
                    printf("Bitmap Control: %d\n", bitmap_ctrl);
                }
                else
                {
                    printf("Invalid DTIM Information Element Length: %d\n", ie[IE_LEN]);
                }
                break;

            default:
		break;
            }
            ie_len -= ie[IE_LEN] + 2;
            ie += ie[IE_LEN] + 2;
        }
}
