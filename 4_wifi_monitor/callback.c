#include "callback.h"
#include "frequency.h"
#include "ie.h"

int scan_callback(struct nl_msg *msg, void *arg)
{
    struct nlattr *tb[NL80211_ATTR_MAX + 1];
    struct genlmsghdr *gnlh = nlmsg_data(nlmsg_hdr(msg));
    struct nlattr *bss[NL80211_BSS_MAX + 1];

    static struct nla_policy bss_policy[NL80211_BSS_MAX + 1] =
        {
            [NL80211_BSS_TSF] = {.type = NLA_U64},
            [NL80211_BSS_FREQUENCY] = {.type = NLA_U32},
            [NL80211_BSS_BSSID] = {},
            [NL80211_BSS_BEACON_INTERVAL] = {.type = NLA_U16},
            [NL80211_BSS_CAPABILITY] = {.type = NLA_U16},
            [NL80211_BSS_INFORMATION_ELEMENTS] = {},
            [NL80211_BSS_SIGNAL_MBM] = {.type = NLA_U32},
            [NL80211_BSS_SIGNAL_UNSPEC] = {.type = NLA_U8},
            [NL80211_BSS_STATUS] = {.type = NLA_U32},
        };

    nla_parse(tb, NL80211_ATTR_MAX, genlmsg_attrdata(gnlh, 0), genlmsg_attrlen(gnlh, 0), NULL);

    if (!tb[NL80211_ATTR_BSS])
    {
        return NL_SKIP;
    }

    if (nla_parse_nested(bss, NL80211_BSS_MAX, tb[NL80211_ATTR_BSS], bss_policy))
    {
        return NL_SKIP;
    }

    /* Show SSID */
    if (bss[NL80211_BSS_INFORMATION_ELEMENTS])
    {
        char *ie = nla_data(bss[NL80211_BSS_INFORMATION_ELEMENTS]);
        int ie_len = nla_len(bss[NL80211_BSS_INFORMATION_ELEMENTS]);

        ie_handler(ie, ie_len);
    }

    /* Show BSSID */
    if (bss[NL80211_BSS_BSSID])
    {
        unsigned char mac_addr[20] = {0,};
        unsigned char *mac = nla_data(bss[NL80211_BSS_BSSID]);

        if (mac == NULL)
        {
            printf("BSSID:\t\tUnknown!\n");
        }
        else
        {
            snprintf(mac_addr, sizeof(mac_addr), "%02x:%02x:%02x:%02x:%02x:%02x",
                     mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
            printf("BSSID:\t\t%s\n", mac_addr);

	    set_mac_addr(mac_addr);
        }
    }

    /* Show Frequency and Channel */
    if (bss[NL80211_BSS_FREQUENCY])
    {
        uint32_t freq = nla_get_u32(bss[NL80211_BSS_FREQUENCY]);
        print_frequency(freq);
    }

    return NL_SKIP;
}

int get_iface_callback(struct nl_msg *msg, void *arg)
{
    struct nlattr *tb[NL80211_ATTR_MAX + 1];
    struct genlmsghdr *gnlh = nlmsg_data(nlmsg_hdr(msg));
    unsigned int *wiphy = arg;

    nla_parse(tb, NL80211_ATTR_MAX, genlmsg_attrdata(gnlh, 0), genlmsg_attrlen(gnlh, 0), NULL);

    if (wiphy && tb[NL80211_ATTR_WIPHY])
    {
        unsigned int thiswiphy = nla_get_u32(tb[NL80211_ATTR_WIPHY]);
        if (*wiphy != thiswiphy)
            printf("phy#%d\n", thiswiphy);
        *wiphy = thiswiphy;
    }

    if (tb[NL80211_ATTR_WIPHY_TX_POWER_LEVEL])
    {
        int32_t txp = nla_get_u32(tb[NL80211_ATTR_WIPHY_TX_POWER_LEVEL]);

        printf("txpower:\t%d.%.2d dBm\n",
               txp / 100, txp % 100);
    }

    return NL_SKIP;
}

int get_station_callback(struct nl_msg *msg, void *arg)
{
    struct nlattr *tb[NL80211_ATTR_MAX + 1];
    struct genlmsghdr *gnlh = nlmsg_data(nlmsg_hdr(msg));
    struct nlattr *sinfo[NL80211_STA_INFO_MAX + 1];
    struct nl80211_sta_flag_update *sta_flags;
    static struct nla_policy stats_policy[NL80211_STA_INFO_MAX + 1] =
    {
        [NL80211_STA_INFO_INACTIVE_TIME] = {.type = NLA_U32},
        [NL80211_STA_INFO_RX_BYTES] = {.type = NLA_U32},
        [NL80211_STA_INFO_TX_BYTES] = {.type = NLA_U32},
        [NL80211_STA_INFO_RX_BYTES64] = {.type = NLA_U64},
        [NL80211_STA_INFO_TX_BYTES64] = {.type = NLA_U64},
        [NL80211_STA_INFO_RX_PACKETS] = {.type = NLA_U32},
        [NL80211_STA_INFO_TX_PACKETS] = {.type = NLA_U32},
        [NL80211_STA_INFO_BEACON_RX] = {.type = NLA_U64},
        [NL80211_STA_INFO_SIGNAL] = {.type = NLA_U8},
        [NL80211_STA_INFO_T_OFFSET] = {.type = NLA_U64},
        [NL80211_STA_INFO_TX_BITRATE] = {.type = NLA_NESTED},
        [NL80211_STA_INFO_RX_BITRATE] = {.type = NLA_NESTED},
        [NL80211_STA_INFO_LLID] = {.type = NLA_U16},
        [NL80211_STA_INFO_PLID] = {.type = NLA_U16},
        [NL80211_STA_INFO_PLINK_STATE] = {.type = NLA_U8},
        [NL80211_STA_INFO_TX_RETRIES] = {.type = NLA_U32},
        [NL80211_STA_INFO_TX_FAILED] = {.type = NLA_U32},
        [NL80211_STA_INFO_BEACON_LOSS] = {.type = NLA_U32},
        [NL80211_STA_INFO_RX_DROP_MISC] = {.type = NLA_U64},
        [NL80211_STA_INFO_STA_FLAGS] =
            {.minlen = sizeof(struct nl80211_sta_flag_update)},
        [NL80211_STA_INFO_LOCAL_PM] = {.type = NLA_U32},
        [NL80211_STA_INFO_PEER_PM] = {.type = NLA_U32},
        [NL80211_STA_INFO_NONPEER_PM] = {.type = NLA_U32},
        [NL80211_STA_INFO_CHAIN_SIGNAL] = {.type = NLA_NESTED},
        [NL80211_STA_INFO_CHAIN_SIGNAL_AVG] = {.type = NLA_NESTED},
        [NL80211_STA_INFO_TID_STATS] = {.type = NLA_NESTED},
        [NL80211_STA_INFO_BSS_PARAM] = {.type = NLA_NESTED},
        [NL80211_STA_INFO_RX_DURATION] = {.type = NLA_U64},
        [NL80211_STA_INFO_TX_DURATION] = {.type = NLA_U64},
        [NL80211_STA_INFO_ACK_SIGNAL] = {.type = NLA_U8},
        [NL80211_STA_INFO_ACK_SIGNAL_AVG] = {.type = NLA_U8},
        [NL80211_STA_INFO_AIRTIME_LINK_METRIC] = {.type = NLA_U32},
        [NL80211_STA_INFO_CONNECTED_TO_GATE] = {.type = NLA_U8},
    };
    
    nla_parse(tb, NL80211_ATTR_MAX, genlmsg_attrdata(gnlh, 0),
              genlmsg_attrlen(gnlh, 0), NULL);

    if (!tb[NL80211_ATTR_STA_INFO])
    {
        fprintf(stderr, "sta stats missing!\n");
        return NL_SKIP;
    }
    if (nla_parse_nested(sinfo, NL80211_STA_INFO_MAX,
                         tb[NL80211_ATTR_STA_INFO],
                         stats_policy))
    {
        fprintf(stderr, "failed to parse nested attributes!\n");
        return NL_SKIP;
    }

    station_handler(sinfo);

    return NL_SKIP;
}
