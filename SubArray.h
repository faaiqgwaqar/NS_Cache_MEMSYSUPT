//Copyright (c) 2015-2016, UT-Battelle, LLC. See LICENSE file in the top-level directory
// This file contains code from NVSim, (c) 2012-2013,  Pennsylvania State University 
//and Hewlett-Packard Company. See LICENSE_NVSim file in the top-level directory.
//No part of DESTINY Project, including this file, may be copied,
//modified, propagated, or distributed except according to the terms
//contained in the LICENSE file.


#ifndef SUBARRAY_H_
#define SUBARRAY_H_

#include "FunctionUnit.h"
#include "RowDecoder.h"
#include "Precharger.h"
#include "SenseAmp.h"
#include "Mux.h"
#include "LevelShifter.h"
#include "LogicLevelShifter.h"
#include "TSV.h"
#include "typedef.h"

class SubArray: public FunctionUnit {
public:
	SubArray();
	virtual ~SubArray();

	/* Functions */
	void PrintProperty();
	void Initialize(long long _numRow, long long _numColumn, bool _multipleRowPerSet, bool _split,
			int _muxSenseAmp, bool _internalSenseAmp, int _muxOutputLev1, int _muxOutputLev2,
			BufferDesignTarget _areaOptimizationLevel, int _num3DLevels);
	void CalculateArea();
	//void CalculateRC();
	void CalculateLatency(double _rampInput);
	void CalculatePower();
	void CalculateRepeater(int numCol);
	SubArray & operator=(const SubArray &);

	/* Properties */
	bool initialized;	/* Initialization flag */
	bool invalid;		/* Indicate that the current configuration is not valid, pass down to all the sub-components */
	bool internalSenseAmp; /* Indicate whether sense amp is within subarray */
	long long numRow;			/* Number of rows */
	long long numColumn;		/* Number of columns */
	bool multipleRowPerSet;		/* For cache design, whether a set is partitioned into multiple wordlines */
	bool split;			/* Whether the row decoder is at the middle of subarrays */
	int muxSenseAmp;	/* How many bitlines connect to one sense amplifier */
	int muxOutputLev1;	/* How many sense amplifiers connect to one output bit, level-1 */
	int muxOutputLev2;	/* How many sense amplifiers connect to one output bit, level-2 */
	BufferDesignTarget areaOptimizationLevel;
	TSV_type tsvType;
    int num3DLevels; /* Number of monolithic 3D levels in the subarray. */

	bool voltageSense;	/* Whether the sense amplifier is voltage-sensing */
	double senseVoltage;/* Minimum sensible voltage */
	double voltagePrecharge;
	long long numSenseAmp;	/* Number of sense amplifiers */
	long long numSenseAmp_bidir;
	double lenWordline;	/* Length of wordlines, Unit: m */
	double lenBitline;	/* Length of bitlines, Unit: m */
	double capWordline;	/* Wordline capacitance, Unit: F */
	double capWordline_bidir;	/* Wordline capacitance, Unit: F */
	double capReadWordline;	/* Wordline capacitance, Gain Cell, Unit: F */
	double capReadWordline_bidir;	/* Wordline capacitance, Gain Cell, Unit: F */
	double capBitline;	/* Bitline capacitance, Unit: F */
	double capPlateline;
	double capBitline_bidir;	/* Bitline capacitance, Unit: F */
	double capWriteBitline;	/* Bitline capacitance, Unit: F */
	double capWriteBitline_bidir;	/* Bitline capacitance, Unit: F */
	double resWordline;	/* Wordline resistance, Unit: ohm */
	double resBitline;	/* Bitline resistance, Unit: ohm */
	double resPlateline;
	double resWordline_bidir;	/* Wordline resistance, Unit: ohm */
	double resReadWordline;	/* Wordline resistance, Unit: ohm */
	double resReadWordline_bidir;	/* Wordline resistance, Unit: ohm */
	double resWriteBitline;	/* Bitline resistance, Unit: ohm */
	double resBitline_bidir;	/* Bitline resistance, Unit: ohm */
	double resWriteBitline_bidir;	/* Bitline resistance, Unit: ohm */
	double resCellAccess; /* Resistance of access device, Unit: ohm */
	double capCellAccess; /* Capacitance of access device, Unit: ohm */
	double resMemCellOff;  /* HRS resistance, Unit: ohm */
	double resMemCellOn;   /* LRS resistance, Unit: ohm */
	double voltageMemCellOff; /* Voltage drop on HRS during read operation, Unit: V */
	double voltageMemCellOn;   /* Voltage drop on LRS druing read operation, Unit: V */
	double resInSerialForSenseAmp; /* Serial resistance of voltage-in voltage sensing as a voltage divider, Unit: ohm */
	double resEquivalentOn;          /* resInSerialForSenseAmp in parallel with resMemCellOn, Unit: ohm */
	double resEquivalentOff;          /* resInSerialForSenseAmp in parallel with resMemCellOn, Unit: ohm */
	double bitlineDelay;	/* Bitline delay, Unit: s */
	double bitlineDelay_dir1;	/* Bitline delay, Unit: s */
	double bitlineDelay_dir2;	/* Bitline delay, Unit: s */
	double writeBitlineDelay_dir1;	/* Bitline delay, Unit: s */
	double writeBitlineDelay_dir2;	/* Bitline delay, Unit: s */
	double chargeLatency;	/* The bitline charge delay during write operations, Unit: s */
	double chargeReadLatency;	/* The bitline charge delay during Read operations, Unit: s */
	double columnDecoderLatency;	/* The worst-case mux latency, Unit: s */
	double bitlineDelayOn;  /* Bitline delay of LRS, Unit: s */
	double bitlineDelayOff; /* Bitline delay of HRS, Unit: s */
	double logicArea;		/* Seperation of FEOL logic from BEOL Subarray */
	double logicWidth;		/* Seperation of FEOL logic from BEOL Subarray */
	double logicHeight;		/* Seperation of FEOL logic from BEOL Subarray */
	double memoryArea;		/* Seperation of FEOL logic from BEOL Subarray */
	double memoryWidth;		/* Seperation of FEOL logic from BEOL Subarray */
	double memoryHeight;
	double areaRatio;		/* Seperation of FEOL logic from BEOL Subarray */
	int stackedMemTiers;
	double readLatency_bidir;
	double writeLatency_bidir;
	double resReadCellAccess; /* Resistance of access device, Unit: ohm */ //gcDRAM bidir only
	double capReadCellAccess; /* Capacitance of access device, Unit: ohm */ //gcDRAM bidir only
	double resWriteCellAccess; /* Resistance of access device, Unit: ohm */ //gcDRAM bidir only
	double capWriteCellAccess; /* Capacitance of access device, Unit: ohm */ //gcDRAM bidir only
	int largerLine;


	// 1.4 update : parameters for buffer insertion
	double widthInvN, widthInvP;
	double wInv, hInv, drivecapin, drivecapout, targetdriveres;
	double sectionres, sectioncap, sectionresMux, sectioncapMux;
	double activityRowRead, activityRowWrite;		// Activity for # of rows
	double gateCapRep;
	int numRepeaters;
	double bufferSizeRatio;

	RowDecoder	rowDecoder;
	RowDecoder	rowDecoder_bidir;
	RowDecoder	gcRowDecoder;
	RowDecoder	plateLineDecoder;
	RowDecoder	gcRowDecoder_bidir;
	RowDecoder	bitlineMuxDecoder;
	RowDecoder	bitlineMuxDecoder_bidir;
	Mux			bitlineMux;
	RowDecoder	senseAmpMuxLev1Decoder;
	Mux			senseAmpMuxLev1;
	RowDecoder	senseAmpMuxLev2Decoder;
	Mux			senseAmpMuxLev2;
	Precharger	precharger;
	Precharger	writecharger;
	Precharger	precharger_bidir;
	Precharger	writecharger_bidir;
	SenseAmp	senseAmp;
	SenseAmp	senseAmp_bidir;

	// 3D Decoder - Need to use NumRow x Cgg x 2(?) xnumLayer decoding to numLayers
	RowDecoder 	threeDimDecoder;
	Mux			threeDimMux;

	// Gain Cell Update : Add Level Shifters for Hold/Boost Voltages
	LogicLevelShifter read_levelshifter;
	LevelShifter write_levelshifter;
	TSV tsvArray;
};

#endif /* SUBARRAY_H_ */
