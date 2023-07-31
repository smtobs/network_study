#include "station_info.h"

#define BIT(x)                 (1ULL << (x))

static void parse_bss_param(struct nlattr *bss_param_attr)
{
    struct nlattr *bss_param_info[NL80211_STA_BSS_PARAM_MAX + 1], *info;
    static struct nla_policy bss_poilcy[NL80211_STA_BSS_PARAM_MAX + 1] =
        {
            [NL80211_STA_BSS_PARAM_CTS_PROT] = {.type = NLA_FLAG},
            [NL80211_STA_BSS_PARAM_SHORT_PREAMBLE] = {.type = NLA_FLAG},
            [NL80211_STA_BSS_PARAM_SHORT_SLOT_TIME] = {.type = NLA_FLAG},
            [NL80211_STA_BSS_PARAM_DTIM_PERIOD] = {.type = NLA_U8},
            [NL80211_STA_BSS_PARAM_BEACON_INTERVAL] = {.type = NLA_U16},
        };

    if (nla_parse_nested(bss_param_info, NL80211_STA_BSS_PARAM_MAX,
                         bss_param_attr, bss_poilcy))
    {
        printf("failed to parse nested bss param attributes!\n");
    }

    info = bss_param_info[NL80211_STA_BSS_PARAM_DTIM_PERIOD];
    if (info)
    {
        printf("DTIM period: %u\n", nla_get_u8(info));
    }

    info = bss_param_info[NL80211_STA_BSS_PARAM_BEACON_INTERVAL];
    if (info)
    {
        printf("beacon interval: %u\n", nla_get_u16(info));
    }

    info = bss_param_info[NL80211_STA_BSS_PARAM_SHORT_PREAMBLE];
    if (info)
    {
        printf("short preamble: ");
        if (nla_get_u16(info))
        {
            printf("yes\n");
        }
        else
        {
            printf("no\n");
        }
    }

    info = bss_param_info[NL80211_STA_BSS_PARAM_SHORT_SLOT_TIME];
    if (info)
    {
	 printf("short slot time: ");
        if (nla_get_u16(info))
        {
            printf("yes\n");
        }
        else
        {
            printf("no\n");
        }
    }
}

static void parse_bitrate(struct nlattr *bitrate_attr, char *buf, int buflen)
{
    int rate = 0;
    char *pos = buf;
    struct nlattr *rinfo[NL80211_RATE_INFO_MAX + 1];
    static struct nla_policy rate_policy[NL80211_RATE_INFO_MAX + 1] = {
        [NL80211_RATE_INFO_BITRATE] = {.type = NLA_U16},
        [NL80211_RATE_INFO_BITRATE32] = {.type = NLA_U32},
        [NL80211_RATE_INFO_MCS] = {.type = NLA_U8},
        [NL80211_RATE_INFO_40_MHZ_WIDTH] = {.type = NLA_FLAG},
        [NL80211_RATE_INFO_SHORT_GI] = {.type = NLA_FLAG},
    };

    if (nla_parse_nested(rinfo, NL80211_RATE_INFO_MAX,
                         bitrate_attr, rate_policy))
    {
        snprintf(buf, buflen, "failed to parse nested rate attributes!");
        return;
    }

    if (rinfo[NL80211_RATE_INFO_BITRATE32])
        rate = nla_get_u32(rinfo[NL80211_RATE_INFO_BITRATE32]);
    else if (rinfo[NL80211_RATE_INFO_BITRATE])
        rate = nla_get_u16(rinfo[NL80211_RATE_INFO_BITRATE]);
    if (rate > 0)
        pos += snprintf(pos, buflen - (pos - buf),
                        "%d.%d MBit/s", rate / 10, rate % 10);
    else
        pos += snprintf(pos, buflen - (pos - buf), "(unknown)");

    if (rinfo[NL80211_RATE_INFO_MCS])
        pos += snprintf(pos, buflen - (pos - buf),
                        " MCS %d", nla_get_u8(rinfo[NL80211_RATE_INFO_MCS]));
    if (rinfo[NL80211_RATE_INFO_VHT_MCS])
        pos += snprintf(pos, buflen - (pos - buf),
                        " VHT-MCS %d", nla_get_u8(rinfo[NL80211_RATE_INFO_VHT_MCS]));
    if (rinfo[NL80211_RATE_INFO_40_MHZ_WIDTH])
        pos += snprintf(pos, buflen - (pos - buf), " 40MHz");
    if (rinfo[NL80211_RATE_INFO_80_MHZ_WIDTH])
        pos += snprintf(pos, buflen - (pos - buf), " 80MHz");
    if (rinfo[NL80211_RATE_INFO_80P80_MHZ_WIDTH])
        pos += snprintf(pos, buflen - (pos - buf), " 80P80MHz");
    if (rinfo[NL80211_RATE_INFO_160_MHZ_WIDTH])
        pos += snprintf(pos, buflen - (pos - buf), " 160MHz");
    if (rinfo[NL80211_RATE_INFO_SHORT_GI])
        pos += snprintf(pos, buflen - (pos - buf), " short GI");
    if (rinfo[NL80211_RATE_INFO_VHT_NSS])
        pos += snprintf(pos, buflen - (pos - buf),
                        " VHT-NSS %d", nla_get_u8(rinfo[NL80211_RATE_INFO_VHT_NSS]));
    if (rinfo[NL80211_RATE_INFO_HE_MCS])
        pos += snprintf(pos, buflen - (pos - buf),
                        " HE-MCS %d", nla_get_u8(rinfo[NL80211_RATE_INFO_HE_MCS]));
    if (rinfo[NL80211_RATE_INFO_HE_NSS])
        pos += snprintf(pos, buflen - (pos - buf),
                        " HE-NSS %d", nla_get_u8(rinfo[NL80211_RATE_INFO_HE_NSS]));
    if (rinfo[NL80211_RATE_INFO_HE_GI])
        pos += snprintf(pos, buflen - (pos - buf),
                        " HE-GI %d", nla_get_u8(rinfo[NL80211_RATE_INFO_HE_GI]));
    if (rinfo[NL80211_RATE_INFO_HE_DCM])
        pos += snprintf(pos, buflen - (pos - buf),
                        " HE-DCM %d", nla_get_u8(rinfo[NL80211_RATE_INFO_HE_DCM]));
    if (rinfo[NL80211_RATE_INFO_HE_RU_ALLOC])
        pos += snprintf(pos, buflen - (pos - buf),
                        " HE-RU-ALLOC %d", nla_get_u8(rinfo[NL80211_RATE_INFO_HE_RU_ALLOC]));
}

static char *get_chain_signal(struct nlattr *attr_list)
{
    struct nlattr *attr;
    static char buf[64];
    char *cur = buf;
    int i = 0, rem;
    const char *prefix;

    if (!attr_list)
        return "";

    nla_for_each_nested(attr, attr_list, rem)
    {
        if (i++ > 0)
        {
            prefix = ", ";
        }
        else
        {
            prefix = "[";
        }

        cur += snprintf(cur, sizeof(buf) - (cur - buf), "%s%d", prefix,
                        (int8_t)nla_get_u8(attr));
    }

    if (i)
    {
        snprintf(cur, sizeof(buf) - (cur - buf), "] ");
    }

    return buf;
}

void station_handler(struct nlattr **sinfo)
{

    struct nl80211_sta_flag_update *sta_flags;

    if (sinfo[NL80211_STA_INFO_RX_BYTES64])
        printf("rx bytes:\t%llu\n",
               (unsigned long long)nla_get_u64(sinfo[NL80211_STA_INFO_RX_BYTES64]));
    else if (sinfo[NL80211_STA_INFO_RX_BYTES])
        printf("rx bytes:\t%u\n",
               nla_get_u32(sinfo[NL80211_STA_INFO_RX_BYTES]));

    if (sinfo[NL80211_STA_INFO_RX_PACKETS])
        printf("rx packets:\t%u\n",
               nla_get_u32(sinfo[NL80211_STA_INFO_RX_PACKETS]));

    if (sinfo[NL80211_STA_INFO_TX_BYTES64])
        printf("tx bytes:\t%llu\n",
               (unsigned long long)nla_get_u64(sinfo[NL80211_STA_INFO_TX_BYTES64]));
    else if (sinfo[NL80211_STA_INFO_TX_BYTES])
        printf("tx bytes:\t%u\n",
               nla_get_u32(sinfo[NL80211_STA_INFO_TX_BYTES]));

    if (sinfo[NL80211_STA_INFO_TX_PACKETS])
        printf("tx packets:\t%u\n",
               nla_get_u32(sinfo[NL80211_STA_INFO_TX_PACKETS]));

    if (sinfo[NL80211_STA_INFO_TX_RETRIES])
        printf("tx retries:\t%u\n",
               nla_get_u32(sinfo[NL80211_STA_INFO_TX_RETRIES]));
        if (sinfo[NL80211_STA_INFO_TX_FAILED])
        printf("tx failed:\t%u\n",
               nla_get_u32(sinfo[NL80211_STA_INFO_TX_FAILED]));

    if (sinfo[NL80211_STA_INFO_BEACON_LOSS])
        printf("beacon loss:\t%u\n",
               nla_get_u32(sinfo[NL80211_STA_INFO_BEACON_LOSS]));

    if (sinfo[NL80211_STA_INFO_BEACON_RX])
        printf("beacon rx:\t%llu\n",
               (unsigned long long)nla_get_u64(sinfo[NL80211_STA_INFO_BEACON_RX]));

    if (sinfo[NL80211_STA_INFO_RX_DROP_MISC])
        printf("rx drop misc:\t%llu\n",
               (unsigned long long)nla_get_u64(sinfo[NL80211_STA_INFO_RX_DROP_MISC]));

    char *chain = get_chain_signal(sinfo[NL80211_STA_INFO_CHAIN_SIGNAL]);
    if (sinfo[NL80211_STA_INFO_SIGNAL])
        printf("signal:  \t%d %sdBm\n",
               (int8_t)nla_get_u8(sinfo[NL80211_STA_INFO_SIGNAL]),
               chain);

    if (sinfo[NL80211_STA_INFO_TX_BITRATE])
    {
        char buf[100];

        parse_bitrate(sinfo[NL80211_STA_INFO_TX_BITRATE], buf, sizeof(buf));
        printf("tx bitrate:\t%s\n", buf);
    }

    if (sinfo[NL80211_STA_INFO_RX_BITRATE])
    {
        char buf[100];

        parse_bitrate(sinfo[NL80211_STA_INFO_RX_BITRATE], buf, sizeof(buf));
        printf("rx bitrate:\t%s\n", buf);
    }
       if (sinfo[NL80211_STA_INFO_EXPECTED_THROUGHPUT])
    {
        uint32_t thr;

        thr = nla_get_u32(sinfo[NL80211_STA_INFO_EXPECTED_THROUGHPUT]);
        /* convert in Mbps but scale by 1000 to save kbps units */
        thr = thr * 1000 / 1024;

        printf("expected throughput:\t%u.%uMbps\n",
               thr / 1000, thr % 1000);
    }

    if (sinfo[NL80211_STA_INFO_STA_FLAGS])
    {
        sta_flags = (struct nl80211_sta_flag_update *)
            nla_data(sinfo[NL80211_STA_INFO_STA_FLAGS]);

        if (sta_flags->mask & BIT(NL80211_STA_FLAG_SHORT_PREAMBLE))
        {
            printf("preamble: \t");
            if (sta_flags->set & BIT(NL80211_STA_FLAG_SHORT_PREAMBLE))
            {
                printf("short\n");
            }
            else
            {
                printf("long\n");
            }
        }

        if (sta_flags->mask & BIT(NL80211_STA_FLAG_WME))
        {
            printf("WMM/WME:\t");
            if (sta_flags->set & BIT(NL80211_STA_FLAG_WME))
            {
                printf("yes\n");
            }
	               else
            {
                printf("no\n");
            }
        }

        if (sta_flags->mask & BIT(NL80211_STA_FLAG_MFP))
        {
            printf("MFP:\t\t");
            if (sta_flags->set & BIT(NL80211_STA_FLAG_MFP))
            {
                printf("yes\n");
            }
            else
            {
                printf("no\n");
            }
        }

        if (sta_flags->mask & BIT(NL80211_STA_FLAG_TDLS_PEER))
        {
            printf("TDLS peer:\t");
            if (sta_flags->set & BIT(NL80211_STA_FLAG_TDLS_PEER))
            {
                printf("yes\n");
            }
            else
            {
                printf("no\n");
            }
        }
    }

    if (sinfo[NL80211_STA_INFO_CONNECTED_TIME])
    {
        printf("connected time:\t%u seconds\n",
               nla_get_u32(sinfo[NL80211_STA_INFO_CONNECTED_TIME]));
    }
}


static unsigned char mac[20];
void set_mac_addr(const unsigned char *mac_addr)
{
    if (mac_addr)
    	snprintf(mac, sizeof(mac), "%s", mac_addr);
}

unsigned char *get_mac_addr()
{
    static unsigned char err_mac[] = "00:11:22:33:44:55";

    if (mac && (strlen(mac) == 17))
    {
	return mac ?  mac : err_mac;
    }
    else
    {
	return err_mac;
    }
}
