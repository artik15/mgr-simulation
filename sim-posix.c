/*
 * sem - sim-posix.c
 *
 * Copyright (C) 2020  Krzysztof Mazur <krzysiek@podlesie.net>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <stdlib.h>
#include <stdbool.h>
#include <ngspice/sharedspice.h>
#include <linux/kernel.h>
#include <math.h>
#include "ssdi.h"

static double rand_gauss(void)
{
	double u, v, s;
	double n;

	do {
		u = 2.0 * drand48() - 1;
		v = 2.0 * drand48() - 1;
		s = u * u + v * v;
	} while ((s <= 0) || (s >= 1));
	return u * sqrt(-2 * log(s) / s);
}

int printk(const char *fmt, ...)
{
	va_list ap;
	int ret;

	va_start(ap, fmt);
	ret = vprintf(fmt, ap);
	va_end(ap);
	return ret;
}

int dac_sync;

int adc_get(void)
{
	return 0;
}

int __gpio_get_value(unsigned int gpio)
{
	return 0;
}

void __gpio_set_value(unsigned int gpio, int value)
{
}

int gpio_direction_input(unsigned int gpio)
{
	return 0;
}

int gpio_direction_output(unsigned int gpio, int value)
{
	return 0;
}

int gpio_set_mode(unsigned int gpio, int mode, int af)
{
	return 0;
}

int gpio_set_pullup(unsigned int gpio)
{
	return 0;
}

int uart_rx(void)
{
//	return 'T';
	return -1;
}

void uart_tx(int c)
{
	putchar(c);
}

int get_signals(void)
{
	return 0;
}

int set_signals(int enable)
{
	return 0;
}

void block_signals(void)
{
}

void unblock_signals(void)
{
}

void dac_set_frequency(int f)
{
}

void dac_set_amplitude(int f)
{
}

static int getchar_cb(char *outputreturn, int ident, void *userdata)
{
	printf("%s\n", outputreturn);
	/* setting a flag if an error message occurred */
//	if (ciprefix("stderr Error:", outputreturn))
//		error_ngspice = true;
	return 0;
}

static int getstat_cb(char *outputreturn, int ident, void *userdata)
{
	printf("%s\n", outputreturn);
	return 0;
}

static int sendinitdata_cb(pvecinfoall intdata, int ident, void *userdata)
{
	int vn = intdata->veccount;
	int i;

	for (i = 0; i < vn; i++) {
#if 0
		fprintf(stderr, "Vector: %s\n", intdata->vecs[i]->vecname);
		/* find the location of V(2) */
		if (cieq(intdata->vecs[i]->vecname, "V(2)"))
			vecgetnumber = i;
#endif
	}
	return 0;
}

static double velocity_2;
static unsigned long velocity_2n;

struct vcache {
	int initialized;
	unsigned int time;
	unsigned int velocity;
	unsigned int vmfc;
	unsigned int vmfcx;
	unsigned int vout;
	unsigned int pos;
	unsigned int neg;
	unsigned int vref;
	unsigned int coil_i;
	unsigned int v_plus;
	unsigned int v_minus;
};

static int senddata_cb(struct vecvaluesall *v, int num, int ident,
		void *userdata)
{
	static struct vcache vcache;
	int i;
	double time = 0;
	double velocity = 0;
	double vmfc = 0;
	double vmfcx = 0;
	double vout = 0;
	double pos = 0;
	double neg = 0;
	double vref = 0;
	double coil_i = 0;
	double v_plus = 0;
	double v_minus = 0;
	static double vref_old = 0.0;
	static double velocity_old = 0.0;
	static double old_time;

	if (!vcache.initialized) {
		vcache.initialized = 1;
		for (i = 0; i < v->veccount; i++) {
			struct vecvalues *vals = v->vecsa[i];

			if (!strcmp(vals->name, "velocity"))
				vcache.velocity = i;
			else if (!strcmp(vals->name, "vmfc"))
				vcache.vmfc = i;
			else if (!strcmp(vals->name, "vmfcx"))
				vcache.vmfcx = i;
			else if (!strcmp(vals->name, "vout"))
				vcache.vout = i;
			else if (!strcmp(vals->name, "time"))
				vcache.time = i;
			else if (!strcmp(vals->name, "pos"))
				vcache.pos = i;
			else if (!strcmp(vals->name, "neg"))
				vcache.neg = i;
			else if (!strcmp(vals->name, "vref"))
				vcache.vref = i;
			else if (!strcmp(vals->name, "v_plus"))
				vcache.v_plus = i;
			else if (!strcmp(vals->name, "v_minus"))
				vcache.v_minus = i;
			else if (!strcmp(vals->name, "v20#branch"))
				vcache.coil_i = i;
#if 1
			else
				fprintf(stderr, "vec: %s, %f %f\n", vals->name, vals->creal, vals->cimag);
#endif
//		printf("vec: %s, %f %f\n", vals->name, vals->creal, vals->cimag);
		}
	}
	velocity = v->vecsa[vcache.velocity]->creal;
	vmfc = v->vecsa[vcache.vmfc]->creal;
	vmfcx = v->vecsa[vcache.vmfcx]->creal;
	vout = v->vecsa[vcache.vout]->creal;
	time = v->vecsa[vcache.time]->creal;
	pos = v->vecsa[vcache.pos]->creal;
	neg = v->vecsa[vcache.neg]->creal;
	vref = v->vecsa[vcache.vref]->creal;
	coil_i = v->vecsa[vcache.coil_i]->creal;
	v_plus = v->vecsa[vcache.v_plus]->creal;
	v_minus = v->vecsa[vcache.v_minus]->creal;

	if (time == old_time)
		return 0;
	old_time = time;

	if (time >= 5) {
		velocity_2 += velocity * velocity;
		velocity_2n++;
	}

	if (time > -0.1) {
		static unsigned int u, u1 = -1;
		int vin;

		if (u != u1) {
			if (u & CONTROL_POS)
				ngSpice_Command((char *) "alter v5 dc 3.3");
			else
				ngSpice_Command((char *) "alter v5 dc 0.0");

			if (u & CONTROL_NEG)
				ngSpice_Command((char *) "alter v6 dc 0.0");
			else
				ngSpice_Command((char *) "alter v6 dc 3.3");
		}
		u1 = u;

#if 0
		vin = 65536.0 / 3.3 * (vref - 1.65);
		vin += round(20 * rand_gauss());
		u = ssdiv(vin);
		if (time < 1)
			u = 0;
#elif 1
		vin = 65536.0 / 3.3 * (vout - 1.65);
		vin += round(0.9 * rand_gauss());
		u = ssdi_simple(vin);
		if (time < 1)
			u = 0;
#else
		vin = 65536.0 / 3.3 * (vout - 1.65);
		vin += round(0.9 * rand_gauss());
		u = ssdim(vin);
		if (time < 1)
			u = 0;
#endif

		printf("XXX %.12e %.12e %.12e %.12e %.12e %.12e %.12e %d %d %d %.12e %e %e\n",
				time, velocity, vmfc, pos,
				neg, vmfcx, coil_i, vin,
				(u & CONTROL_POS) != 0,
				(u & CONTROL_NEG) != 0, vref,
				v_plus, v_minus);

//		printf("XXX %f %f %f %d %d\n", time, vmfc, vout, vin, u);
	}

#if 0
	int vn = intdata->veccount;
	int i;

	for (i = 0; i < vn; i++) {
		fprintf(stderr, "Vector: %s\n", intdata->vecs[i]->vecname);
#if 0
		/* find the location of V(2) */
		if (cieq(intdata->vecs[i]->vecname, "V(2)"))
			vecgetnumber = i;
#endif
	}
#endif
	return 0;
}

static int controlled_exit_cb(int status, bool immed, bool quit, int id,
		void *p)
{
	exit(0);
}

static void spice_init(void)
{
	static char coil_r[64], coil_l[64], coil_c[64], out_sw[128];
	static char ref_src[64];
	static const char *circuit[] = {
		"* foo",
		".include lib.spice",
		".include mech-model.spice",
		".include fogor.spice",
		".SUBCKT COIL 1 3",
		coil_r,
		coil_l,
		coil_c,
		".ENDS",
		ref_src,
		"i1 velocity 0 SINE(0 0.04 149.77 0 0 0)",
//		"i1 velocity 0 SINE(0 0.01 149.77 0 0 0)",
		"X1 velocity 0 PLATE",
		"v2000 velocity2 0 SINE(0 0.06 149.77 0 0 0)",
//		"X2 velocity2 0 PLATE",
		"X10 velocity 0 VMFC1 0 MFC",
		"X11 velocity 0 VMFC2 VMFC1 MFC",
		"X12 velocity 0 VMFC3 VMFC2 MFC",
		"X13 velocity 0 VMFC4 VMFC3 MFC",
		"X14 velocity 0 VMFC5 VMFC4 MFC",
		"X15 velocity 0 VMFC6 VMFC5 MFC",
		"X16 velocity 0 VMFC7 VMFC6 MFC",
		"X17 velocity 0 VMFC8 VMFC7 MFC",
		"X18 velocity 0 VREF_MFC 0 MFC",
		"X19 velocity2 0 VREC 0 MFC",
		"X20 velocity2 0 VREC 0 MFC",
		"X21 velocity2 0 VREC 0 MFC",
		"X22 velocity2 0 VREC 0 MFC",
		"v20 VMFC8 VMFCX dc 0",
		"X23 VMFCX VMFC COIL",
		out_sw,
//		"X30 VMFC POS NEG V_PLUS V_MINUS V33 VP VN 0 OUTSW",
		"X31 VMFC VOUT V33 VOFFSET VP VN 0 A_IN",
		"X32 VREF_MFC VREF V33 VOFFSET VP VN 0 VEL_IN",
		"X33 VREC V_PLUS V_MINUS 0 RECOVERY",
		"v1 V33 0 dc 3.3",
		"v2 VOFFSET 0 dc 1.65",
		"v3 VP 0 dc 15",
		"v4 VN 0 dc -15",
		"v5 POS 0 dc 0.0",
		"v6 NEG 0 dc 3.3",
		".options interp ABSTOL=10n VNTOL=10m ITL3=10 ITL4=500 ITL5=10000 RAMPTIME=1u CHGTOL=1e-8 TRTOL=7 XTRTOL=2",
		".control",
		"set num_threads=4",
		".endc",
		".tran 10u 2000m 0m 2u",
		".end",
		NULL,
	};
	const char *mode;

	mode = getenv("MODE");
	if (!mode)
		mode = "SSDI";
	if (!strcmp(mode, "SSDI")) {
		snprintf(out_sw, sizeof(out_sw),
				"X30 VMFC POS NEG 0 0 V33 VP VN 0 OUTSW");
	} else if (!strcmp(mode, "SSDIV")) {
		snprintf(out_sw, sizeof(out_sw),
				"X30 VMFC POS NEG V_PLUS V_MINUS V33 VP VN 0 OUTSW");
	} else {
		snprintf(out_sw, sizeof(out_sw), " ");
	}


	fprintf(stderr, "Coil: %s\n", getenv("COIL_NAME"));
	snprintf(coil_r, sizeof(coil_r), "R1 1 2 %s",
			getenv("COIL_R") ? : "0.75");
	snprintf(coil_l, sizeof(coil_l), "L1 2 3 %s",
			getenv("COIL_L") ? : "25m");
	snprintf(coil_c, sizeof(coil_c), "C1 1 3 %s",
			getenv("COIL_C") ? : "23p");
	snprintf(ref_src, sizeof(ref_src),
			"v100 vgen 0 SINE(0 0.04 149.77 0 0 %s)",
			getenv("PHASE") ? : "205.9");
	fprintf(stderr, "	%s\n", coil_r);
	fprintf(stderr, "	%s\n", coil_l);
	fprintf(stderr, "	%s\n", coil_c);

	fprintf(stderr, "Initializing ngspice....\n");
	ngSpice_Init(getchar_cb, getstat_cb, controlled_exit_cb, senddata_cb,
			sendinitdata_cb, NULL, NULL);
	fprintf(stderr, "Loading circuit...\n");
	ngSpice_Circ((char **) circuit);
	fprintf(stderr, "starting simulation...\n");
	ngSpice_Command((char *) "run");
	fprintf(stderr, "Velocity: %.2f dB\n",
			10 * log10(velocity_2 / velocity_2n));
#if 0
	printf("%s %.2f\n",
			getenv("PHASE") ? : "0",
			10 * log10(velocity_2 / velocity_2n));
#endif
	exit(0);
//	ngSpice_Command((char *) "alter v1 dc 2.5");
//	ngSpice_Command((char *) "run");
//	ngSpice_Command((char *) "bg_run");
}

void adc_init(void)
{
}

void uart_init(void)
{
	spice_init();
}

void adc_select_linein(void)
{
}

void adc_select_voltage(void)
{
}

void adc_select_velocity(void)
{
}

int dac_get_A(void)
{
	return 1000;
}


int main(int argc, void **argv)
{
	spice_init();
	return 0;
}
