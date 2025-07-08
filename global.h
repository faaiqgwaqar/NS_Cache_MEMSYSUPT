// This file contains code from NVSim, (c) 2012-2013,  Pennsylvania State University 
//and Hewlett-Packard Company. See LICENSE_NVSim file in the top-level directory.
//No part of DESTINY Project, including this file, may be copied,
//modified, propagated, or distributed except according to the terms
//contained in the LICENSE file.

#ifndef __DESTINY_GLOBAL_H__
#define __DESTINY_GLOBAL_H__


#include "InputParameter.h"
#include "Technology.h"
#include "MemCell.h"
#include "Wire.h"

extern InputParameter *inputParameter;
extern Technology *tech;
extern Technology *devtech;
extern MemCell *cell;
extern Technology *gtech;
extern Wire *localWire;		/* The wire type of local interconnects (for example, wire in mat) */
extern Wire *globalWire;	/* The wire type of global interconnects (for example, the ones that connect mats) */
extern MemCell **sweepCells;

const double invalid_value = 1e41;
const double invalid_value_min = 1e-41;
const double infinite_ramp = 1e41;

/* I am a Lazy POS and as a temp solution, I will add some gain cell parameters here */

// Enable/Disable functions
extern bool GCRAM_MUXEN; // Enable/Disable Use of Multiplexing on SenseAmps, although this might not be incredibly useful for us
extern bool GCRAM_FOLD; // If true, use a folded-bitline strategy. Otherwise, use an open bitline strategy
extern bool GCRAM_RACCU; // If true, use the traditional calculation for read latency calculation in DESTINY, else use from SPICE

// Device Parasitic Parameters
extern double GCRAM_GATECAP; // What is the gate capacitance of the device per micron
extern double GCRAM_CONTCAP; // What is the source/drain capacitance of the device per micron, this is parasitic
extern double GCRAM_CHANRES; // What is the channel resistnace at 1 micron. This has an inverse relationship to width when compared to cap

// Device Geometry Parameters
extern double GCRAM_RACCW; // What is the width of the read access transistor?
extern double GCRAM_WACCW; // What is the width of the write access transistor?

// Access Timing Parameters
extern double GCRAM_RACCT; // What is the read access latency of the cell
extern double GCRAM_WACCT; // What is the write access latency of the cell

// Voltage Operation Parameters
extern double GCRAM_WBVOLT; // What is the voltage used to activate the write bitlines?
extern double GCRAM_WWVOLT; // What is the voltage used to activate the write wordlines?
extern double GCRAM_RBVOLT; // What is the voltage used to activate the read bitlines?
extern double GCRAM_RWVOLT; // What is the voltage used to activate the read wordlines?

extern double GCRAM_DECLAYER; // How many layers are using the 3D decoding scheme?
extern bool GCRAM_2TGC; // Are we using a 2 transistor gain cell?
extern bool GCRAM_3TGC; // are we using a 3 transistor gain cell?

extern int numRowSubarray, numColumnSubarray;

#endif

