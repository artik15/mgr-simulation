#include "ssdi.h"
#include "stdio.h"

/*
 * Assumptions:
 *
 * Sampling frequency: 100 kHz, 10 us period
 * Frequency: 100 - 200 Hz, 5-10 ms period, 500 to 1000
 */

#define V_MED		32
#define V_SMALL		16


#define SW_TIME		100
#define POST_TIME	50

enum ssdim_state {
	SSDIM_INIT,
	SSDIM_P,
	SSDIM_SWITCH_PN,
	SSDIM_POST_PN,
	SSDIM_N,
	SSDIM_SWITCH_NP,
	SSDIM_POST_NP,
};

struct ssdim {
	enum ssdim_state state;
	int v_max;
	int v_min;
	unsigned int c;
};

unsigned int ssdiv(int v)
{
	// extern unsigned int adc_offset;
	static struct ssdim p;
	unsigned int u = 0;
    
	// v -= adc_offset;
	switch (p.state) {
	case SSDIM_INIT:
		u = 0;
		p.v_max = p.v_min = v;
		if (v > V_MED)
			p.state = SSDIM_P;
		else if (v < -V_MED)
			p.state = SSDIM_N;
		break;
	case SSDIM_P:
		if (v > p.v_max) {
			p.v_max = v;
		} else if (v < p.v_max - V_SMALL) {
			p.state = SSDIM_SWITCH_PN;
			p.c = SW_TIME - 1;
			u = CONTROL_NEG;
		}
		break;
	case SSDIM_SWITCH_PN:
		if (!p.c) {
			p.state = SSDIM_POST_PN;
			p.c = POST_TIME - 1;
			break;
		}
		p.c--;
		u = CONTROL_NEG;
		break;
	case SSDIM_POST_PN:
		if (!p.c) {
			p.state = SSDIM_N;
			p.v_min = v;
		}
		p.c--;
		break;
	case SSDIM_N:
		if (v < p.v_min) {
			p.v_min = v;
		} else if (v > p.v_min + V_SMALL) {
			p.state = SSDIM_SWITCH_NP;
			p.c = SW_TIME - 1;
			u = CONTROL_POS;
		}
		break;
	case SSDIM_SWITCH_NP:
		if (!p.c) {
			p.state = SSDIM_POST_NP;
			p.c = POST_TIME - 1;
			break;
		}
		p.c--;
		u = CONTROL_POS;
		break;
	case SSDIM_POST_NP:
		if (!p.c) {
			p.state = SSDIM_P;
			p.v_max = v;
		}
		p.c--;
		break;
	}
    // printf("STAT %u v %d u %u\n", p.state, v, u);
	return u;
}



#include "ssdi.h"

#define V_MID 30
#define DEAD_ZONE 15
#define ZERO_COUNTER 3
#define SWITCH_PERIOD 40
#define POST_SWITCH_PERIOD 50

enum SSDI_STATE {
    SSDI_INIT,
    SSDI_POSITIVE,
    SSDI_POSITIVE_SWITCH,
    SSDI_POSITIVE_SWITCH_OFF_STAB,
    SSDI_NEGATIVE,
    SSDI_NEGATIVE_SWITCH,
    SSDI_NEGATIVE_SWITCH_OFF_STAB,
};

struct ssdi_var
{
    enum SSDI_STATE state;
    int v_max;
    int v_min;
    unsigned int ctr;
};


unsigned int switch_period = 40;

unsigned int ssdi_simple(int v)
{
    // extern unsigned int adc_offset;
    // v -= adc_offset;
    static struct ssdi_var object = {0};
    unsigned int u = 0;
    // static unsigned int ctr = 0;

    switch (object.state)
    {
    case SSDI_INIT:
        object.v_max = object.v_min = v;

        if (v > V_MID)
        {
            object.state = SSDI_POSITIVE;
        } else if (v < -V_MID)
        {
            object.state = SSDI_NEGATIVE;
        }
        break;

    case SSDI_POSITIVE:
        if (v > object.v_max)
        {
            object.v_max = v;
        }

        if (v < object.v_max - DEAD_ZONE)
        {
            object.state = SSDI_POSITIVE_SWITCH;
            object.ctr = switch_period;
        }

        break;

    case SSDI_POSITIVE_SWITCH:
        u = CONTROL_NEG;

        if (!object.ctr)
        {
            object.state = SSDI_POSITIVE_SWITCH_OFF_STAB;
            object.ctr = POST_SWITCH_PERIOD;
        } else
        {
            object.ctr--;
        }
        break;
    
    case SSDI_POSITIVE_SWITCH_OFF_STAB:
        if (!object.ctr)
        {
            object.state = SSDI_NEGATIVE;
            object.v_min = v;
        } else
        {
            object.ctr--;
        }
        break;

    case SSDI_NEGATIVE:
        if (v < object.v_min)
        {
            object.v_min = v;
        }

        if (v > object.v_min + DEAD_ZONE)
        {
            object.state = SSDI_NEGATIVE_SWITCH;
            object.ctr = switch_period;
        }
        break;

    case SSDI_NEGATIVE_SWITCH:
        u = CONTROL_POS;
        if (!object.ctr)
        {
            object.state = SSDI_NEGATIVE_SWITCH_OFF_STAB;
            object.ctr = POST_SWITCH_PERIOD;
        } else
        {
            object.ctr--;
        }
        break;

    case SSDI_NEGATIVE_SWITCH_OFF_STAB:
        if (!object.ctr)
        {
            object.state = SSDI_POSITIVE;
            object.v_max = v;
        } else
        {
            object.ctr--;
        }
        break;

    default:
        break;
    }
    
    return u;
}