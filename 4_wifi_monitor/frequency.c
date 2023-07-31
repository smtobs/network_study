#include "frequency.h"

static struct ch_freq
{
    uint8_t channel;
    char str_freqquency[8];
    uint32_t frequency;
}ch_freq;

static struct ch_freq ch_freq_map[] =
{
        {1, "2.412", 2412},
        {2, "2.417", 2417},
        {3, "2.422", 2422},
        {4, "2.427", 2427},
        {5, "2.432", 2432},
        {6, "2.437", 2437},
        {7, "2.442", 2442},
        {8, "2.447", 2447},
        {9, "2.452", 2452},
        {10, "2.457", 2457},
        {11, "2.462", 2462},
        {12, "2.467", 2467},
        {13, "2.472", 2472},
        {14, "2.484", 2484},
        /*  UNII */
        {36, "5.18", 5180},
        {40, "5.2", 5200},
        {44, "5.22", 5220},
        {48, "5.24", 5240},
        {52, "5.26", 5260},
        {56, "5.28", 5280},
        {60, "5.3", 5300},
        {64, "5.32", 5320},
        {149, "5.745", 5745},
        {153, "5.765", 5765},
        {157, "5.785", 5785},
        {161, "5.805", 5805},
        {165, "5.825", 5825},
        {167, "5.835", 5835},
        {169, "5.845", 5845},
        {171, "5.855", 5855},
        {173, "5.865", 5865},
        /* HiperLAN2 */
        {100, "5.5", 5500},
        {104, "5.52", 5520},
        {108, "5.54", 5540},
        {112, "5.56", 5560},
        {116, "5.58", 5580},
        {120, "5.6", 5600},
        {124, "5.62", 5620},
        {128, "5.64", 5640},
        {132, "5.66", 5660},
        {136, "5.68", 5680},
        {140, "5.7", 5700},
};

void print_frequency(uint32_t freq)
{
	static int ch_freq_map_num = (sizeof(ch_freq_map) / sizeof(struct ch_freq));
        uint8_t i, chan;
        char *p_str_freq = NULL;

        for (i = 0; i < ch_freq_map_num; i++)
        {
            if (freq == ch_freq_map[i].frequency)
            {
                chan = ch_freq_map[i].channel;
                p_str_freq = ch_freq_map[i].str_freqquency;
                break;
            }
        }

        if (p_str_freq == NULL || chan == 0)
        {
            printf("Frequency:\tUnknown!\n");
        }
        else
        {
            printf("Frequency:\t%s GHz (Channel %u)\t\n", p_str_freq, chan);
        }
}

