//Copyright (c) 2015-2016, UT-Battelle, LLC. See LICENSE file in the top-level directory
// This file contains code from NVSim, (c) 2012-2013,  Pennsylvania State University 
//and Hewlett-Packard Company. See LICENSE_NVSim file in the top-level directory.
//No part of DESTINY Project, including this file, may be copied,
//modified, propagated, or distributed except according to the terms
//contained in the LICENSE file.


#include "Result.h"
#include "global.h"
#include "formula.h"
#include "macros.h"

#include <iostream>
#include <fstream>

using namespace std;

Result::Result() {
	// TODO Auto-generated constructor stub
	if (inputParameter->routingMode == h_tree)
		bank = new BankWithHtree();
	else
		bank = new BankWithoutHtree();
	localWire = new Wire();
	globalWire = new Wire();

	/* initialize the worst case */
	bank->readLatency = invalid_value;
	bank->writeLatency = invalid_value;
	bank->readDynamicEnergy = invalid_value;
	bank->writeDynamicEnergy = invalid_value;
	bank->leakage = invalid_value;
	bank->height = invalid_value;
	bank->width = invalid_value;
	bank->area = invalid_value;

	/* No constraints */
	limitReadLatency = invalid_value;
	limitWriteLatency = invalid_value;
	limitReadDynamicEnergy = invalid_value;
	limitWriteDynamicEnergy = invalid_value;
	limitReadEdp = invalid_value;
	limitWriteEdp = invalid_value;
	limitArea = invalid_value;
	limitLeakage = invalid_value;

	/* Default read latency optimization */
	optimizationTarget = read_latency_optimized;

    cellTech = NULL;
}

Result::~Result() {
	// TODO Auto-generated destructor stub
	if (bank)
		delete bank;
	if (Result::localWire)
		delete Result::localWire;
	if (Result::globalWire)
		delete Result::globalWire;
}

void Result::reset() {
	bank->readLatency = invalid_value;
	bank->writeLatency = invalid_value;
	bank->readDynamicEnergy = invalid_value;
	bank->writeDynamicEnergy = invalid_value;
	bank->leakage = invalid_value;
	bank->height = invalid_value;
	bank->width = invalid_value;
	bank->area = invalid_value;
}

bool Result::compareAndUpdate(Result &newResult) {
    bool toUpdate = false;

	if (newResult.bank->readLatency <= limitReadLatency && newResult.bank->writeLatency <= limitWriteLatency
			&& newResult.bank->readDynamicEnergy <= limitReadDynamicEnergy && newResult.bank->writeDynamicEnergy <= limitWriteDynamicEnergy
			&& newResult.bank->readLatency * newResult.bank->readDynamicEnergy <= limitReadEdp
			&& newResult.bank->writeLatency * newResult.bank->writeDynamicEnergy <= limitWriteEdp
			&& newResult.bank->area <= limitArea && newResult.bank->leakage <= limitLeakage) {
		switch (optimizationTarget) {
		case read_latency_optimized:
			if 	(newResult.bank->readLatency < bank->readLatency)
				toUpdate = true;
			break;
		case write_latency_optimized:
			if 	(newResult.bank->writeLatency < bank->writeLatency)
				toUpdate = true;
			break;
		case read_energy_optimized:
			if 	(newResult.bank->readDynamicEnergy < bank->readDynamicEnergy)
				toUpdate = true;
			break;
		case write_energy_optimized:
			if 	(newResult.bank->writeDynamicEnergy < bank->writeDynamicEnergy)
				toUpdate = true;
			break;
		case read_edp_optimized:
			if 	(newResult.bank->readLatency * newResult.bank->readDynamicEnergy < bank->readLatency * bank->readDynamicEnergy)
				toUpdate = true;
			break;
		case write_edp_optimized:
			if 	(newResult.bank->writeLatency * newResult.bank->writeDynamicEnergy < bank->writeLatency * bank->writeDynamicEnergy)
				toUpdate = true;
			break;
		case area_optimized:
			if 	(newResult.bank->area < bank->area)
				toUpdate = true;
			break;
		case leakage_optimized:
			if 	(newResult.bank->leakage < bank->leakage)
				toUpdate = true;
			break;
		default:	/* Exploration */
			/* should not happen */
			;
		}
		if (toUpdate) {
			*bank = *(newResult.bank);
			*localWire = *(newResult.localWire);
			*globalWire = *(newResult.globalWire);
		}
	}

    return toUpdate;
}

string Result::printOptimizationTarget() {
    string rv;

    switch (optimizationTarget) {
    case read_latency_optimized:
        rv = "Read Latency";
        break;
    case write_latency_optimized:
        rv = "Write Latency";
        break;
    case read_energy_optimized:
        rv = "Read Energy";
        break;
    case write_energy_optimized:
        rv = "Write Energy";
        break;
    case read_edp_optimized:
        rv = "Read Energy-Delay-Product";
        break;
    case write_edp_optimized:
        rv = "Write Energy-Delay-Product";
        break;
    case area_optimized:
        rv = "Area";
        break;
    case leakage_optimized:
        rv = "Leakage";
        break;
    default:	/* Exploration */
        /* should not happen */
        ;
    }

    return rv;
}

void Result::print(int indent) {
	cout << string(indent, ' ') << endl;
    cout << string(indent, ' ') << "=============" << endl;
    cout << string(indent, ' ') << "   SUMMARY   " << endl;
    cout << string(indent, ' ') << "=============" << endl;
    cout << string(indent, ' ') << "Optimized for: " << printOptimizationTarget() << endl;
    cellTech->PrintCell(indent);
	cout << string(indent, ' ') << endl;
    cout << string(indent, ' ') << "=============" << endl;
    cout << string(indent, ' ') << "CONFIGURATION" << endl;
    cout << string(indent, ' ') << "=============" << endl;
    if (bank->stackedDieCount > 1) {
        cout << string(indent, ' ') << "Bank Organization: " << bank->numRowMat << " x " << bank->numColumnMat << " x " << bank->stackedDieCount << endl;
        cout << string(indent, ' ') << " - Row Activation   : " << bank->numActiveMatPerColumn << " / " << bank->numRowMat << " x 1" << endl;
        cout << string(indent, ' ') << " - Column Activation: " << bank->numActiveMatPerRow << " / " << bank->numColumnMat << " x 1" << endl;
    } else {
        cout << string(indent, ' ') << "Bank Organization: " << bank->numRowMat << " x " << bank->numColumnMat << endl;
        cout << string(indent, ' ') << " - Row Activation   : " << bank->numActiveMatPerColumn << " / " << bank->numRowMat << endl;
        cout << string(indent, ' ') << " - Column Activation: " << bank->numActiveMatPerRow << " / " << bank->numColumnMat << endl;
    }
	cout << string(indent, ' ') << "Mat Organization: " << bank->numRowSubarray << " x " << bank->numColumnSubarray << endl;
	cout << string(indent, ' ') << " - Row Activation   : " << bank->numActiveSubarrayPerColumn << " / " << bank->numRowSubarray << endl;
	cout << string(indent, ' ') << " - Column Activation: " << bank->numActiveSubarrayPerRow << " / " << bank->numColumnSubarray << endl;
	cout << string(indent, ' ') << " - Subarray Size    : " << bank->mat.subarray.numRow << " Rows x " << bank->mat.subarray.numColumn << " Columns" << endl;
	cout << string(indent, ' ') << "Mux Level:" << endl;
	cout << string(indent, ' ') << " - Senseamp Mux      : " << bank->muxSenseAmp << endl;
	cout << string(indent, ' ') << " - Output Level-1 Mux: " << bank->muxOutputLev1 << endl;
	cout << string(indent, ' ') << " - Output Level-2 Mux: " << bank->muxOutputLev2 << endl;
	if (inputParameter->designTarget == cache)
		cout << string(indent, ' ') << " - One set is partitioned into " << bank->numRowPerSet << " rows" << endl;
	cout << string(indent, ' ') << "Local Wire:" << endl;
	cout << string(indent, ' ') << " - Wire Type : ";
	switch (localWire->wireType) {
	case local_aggressive:
		cout << string(indent, ' ') << "Local Aggressive" << endl;
		break;
	case local_conservative:
		cout << string(indent, ' ') << "Local Conservative" << endl;
		break;
	case semi_aggressive:
		cout << string(indent, ' ') << "Semi-Global Aggressive" << endl;
		break;
	case semi_conservative:
		cout << string(indent, ' ') << "Semi-Global Conservative" << endl;
		break;
	case global_aggressive:
		cout << string(indent, ' ') << "Global Aggressive" << endl;
		break;
	case global_conservative:
		cout << string(indent, ' ') << "Global Conservative" << endl;
		break;
	default:
		cout << string(indent, ' ') << "DRAM Wire" << endl;
	}
	cout << string(indent, ' ') << " - Repeater Type: ";
	switch (localWire->wireRepeaterType) {
	case repeated_none:
		cout << string(indent, ' ') << "No Repeaters" << endl;
		break;
	case repeated_opt:
		cout << string(indent, ' ') << "Fully-Optimized Repeaters" << endl;
		break;
	case repeated_5:
		cout << string(indent, ' ') << "Repeaters with 5% Overhead" << endl;
		break;
	case repeated_10:
		cout << string(indent, ' ') << "Repeaters with 10% Overhead" << endl;
		break;
	case repeated_20:
		cout << string(indent, ' ') << "Repeaters with 20% Overhead" << endl;
		break;
	case repeated_30:
		cout << string(indent, ' ') << "Repeaters with 30% Overhead" << endl;
		break;
	case repeated_40:
		cout << string(indent, ' ') << "Repeaters with 40% Overhead" << endl;
		break;
	case repeated_50:
		cout << string(indent, ' ') << "Repeaters with 50% Overhead" << endl;
		break;
	default:
		cout << string(indent, ' ') << "Unknown" << endl;
	}
	cout << string(indent, ' ') << " - Low Swing : ";
	if (localWire->isLowSwing)
		cout << string(indent, ' ') << "Yes" << endl;
	else
		cout << string(indent, ' ') << "No" << endl;
	cout << string(indent, ' ') << "Global Wire:" << endl;
	cout << string(indent, ' ') << " - Wire Type : ";
	switch (globalWire->wireType) {
	case local_aggressive:
		cout << string(indent, ' ') << "Local Aggressive" << endl;
		break;
	case local_conservative:
		cout << string(indent, ' ') << "Local Conservative" << endl;
		break;
	case semi_aggressive:
		cout << string(indent, ' ') << "Semi-Global Aggressive" << endl;
		break;
	case semi_conservative:
		cout << string(indent, ' ') << "Semi-Global Conservative" << endl;
		break;
	case global_aggressive:
		cout << string(indent, ' ') << "Global Aggressive" << endl;
		break;
	case global_conservative:
		cout << string(indent, ' ') << "Global Conservative" << endl;
		break;
	default:
		cout << string(indent, ' ') << "DRAM Wire" << endl;
	}
	cout << string(indent, ' ') << " - Repeater Type: ";
	switch (globalWire->wireRepeaterType) {
	case repeated_none:
		cout << string(indent, ' ') << "No Repeaters" << endl;
		break;
	case repeated_opt:
		cout << string(indent, ' ') << "Fully-Optimized Repeaters" << endl;
		break;
	case repeated_5:
		cout << string(indent, ' ') << "Repeaters with 5% Overhead" << endl;
		break;
	case repeated_10:
		cout << string(indent, ' ') << "Repeaters with 10% Overhead" << endl;
		break;
	case repeated_20:
		cout << string(indent, ' ') << "Repeaters with 20% Overhead" << endl;
		break;
	case repeated_30:
		cout << string(indent, ' ') << "Repeaters with 30% Overhead" << endl;
		break;
	case repeated_40:
		cout << string(indent, ' ') << "Repeaters with 40% Overhead" << endl;
		break;
	case repeated_50:
		cout << string(indent, ' ') << "Repeaters with 50% Overhead" << endl;
		break;
	default:
		cout << string(indent, ' ') << "Unknown" << endl;
	}
	cout << string(indent, ' ') << " - Low Swing : ";
	if (globalWire->isLowSwing)
		cout << string(indent, ' ') << "Yes" << endl;
	else
		cout << string(indent, ' ') << "No" << endl;
	cout << string(indent, ' ') << "Buffer Design Style: ";
	switch (bank->areaOptimizationLevel) {
	case latency_first:
		cout << string(indent, ' ') << "Latency-Optimized" << endl;
		break;
	case area_first:
		cout << string(indent, ' ') << "Area-Optimized" << endl;
		break;
	default:	/* balance */
		cout << string(indent, ' ') << "Balanced" << endl;
	}

	cout << string(indent, ' ') << "=============" << endl;
    cout << string(indent, ' ') << "   RESULT" << endl;
    cout << string(indent, ' ') << "=============" << endl;

	cout << string(indent, ' ') << "Area:" << endl;

	cout << string(indent, ' ') << " - Total Area = " << TO_METER(bank->height) << " x " << TO_METER(bank->width)
			<< " = " << TO_SQM(bank->area) << endl;
	cout << string(indent, ' ') << " |--- Mat Area      = " << TO_METER(bank->mat.height) << " x " << TO_METER(bank->mat.width)
			<< " = " << TO_SQM(bank->mat.area) << "   (" << cell->area * tech->featureSize * tech->featureSize
			* bank->capacity / bank->numRowMat / bank->numColumnMat / bank->mat.area * 100 << "%)" << endl;
	cout << string(indent, ' ') << " |--- Subarray Area = " << TO_METER(bank->mat.subarray.height) << " x "
			<< TO_METER(bank->mat.subarray.width) << " = " << TO_SQM(bank->mat.subarray.area) << "   ("
			<< cell->area * tech->featureSize * tech->featureSize * bank->capacity / bank->numRowMat
			/ bank->numColumnMat / bank->numRowSubarray / bank->numColumnSubarray
			/ bank->mat.subarray.area * 100 << "%)" <<endl;
	
	/* SubArray Area BreakDown */

	cout << string(indent, ' ') << " |--- Subarray rowDecoder Area (BothDirs) = " << TO_SQM(2*bank->mat.subarray.rowDecoder.area) << endl;
	cout << string(indent, ' ') << " |--- Subarray WWL Decoder Area Dir1 = " << TO_SQM(bank->mat.subarray.gcRowDecoder.area) << endl;
	cout << string(indent, ' ') << " |--- Subarray WWL Decoder Area Dir2 = " << TO_SQM(bank->mat.subarray.gcRowDecoder_bidir.area) << endl;
	cout << string(indent, ' ') << " |--- Subarray bitlineMuxDecoder Area = " << TO_SQM(2*bank->mat.subarray.bitlineMuxDecoder.area) << endl;
	cout << string(indent, ' ') << " |--- Subarray bitlineMux Area = " << TO_SQM(bank->mat.subarray.bitlineMux.area) << endl;
	cout << string(indent, ' ') << " |--- Subarray senseAmpMuxLev1 Area = " << TO_SQM(bank->mat.subarray.senseAmpMuxLev1.area) << endl;
	cout << string(indent, ' ') << " |--- Subarray senseAmpMuxLev2 Area = " << TO_SQM(bank->mat.subarray.senseAmpMuxLev2.area) << endl;
	cout << string(indent, ' ') << " |--- Subarray senseAmpMuxLev1Decoder Area = " << TO_SQM(bank->mat.subarray.senseAmpMuxLev1Decoder.area) << endl;
	cout << string(indent, ' ') << " |--- Subarray senseAmpMuxLev2Decoder Area = " << TO_SQM(bank->mat.subarray.senseAmpMuxLev2Decoder.area) << endl;
	cout << string(indent, ' ') << " |--- Subarray precharger Area Dir1 = " << TO_SQM(2*bank->mat.subarray.precharger.area) << endl;
	cout << string(indent, ' ') << " |--- Subarray writeDriver Area Dir1 = " << TO_SQM(2*bank->mat.subarray.writecharger.area) << endl;
	cout << string(indent, ' ') << " |--- Subarray precharger Area Dir2 = " << TO_SQM(2*bank->mat.subarray.precharger_bidir.area) << endl;
	cout << string(indent, ' ') << " |--- Subarray writeDriver Area Dir2 = " << TO_SQM(2*bank->mat.subarray.writecharger_bidir.area) << endl;
	cout << string(indent, ' ') << " |--- Subarray senseAmp Area Dir1= " << TO_SQM(bank->mat.subarray.senseAmp.area) << endl;
	cout << string(indent, ' ') << " |--- Subarray senseAmp Area Dir2 = " << TO_SQM(bank->mat.subarray.senseAmp_bidir.area) << endl;
	cout << string(indent, ' ') << " |--- Subarray LevelShifter Area (BothDirs) = " << TO_SQM(bank->mat.subarray.write_levelshifter.area + bank->mat.subarray.read_levelshifter.area) << endl;
	cout << string(indent, ' ') << " |--- Subarray MIV Area = " << TO_SQM(bank->mat.subarray.tsvArray.area) << endl;


	/*SubArray Area BreakDown End*/
	
	if (cell->memCellType == gcDRAM){
		cout << string(indent, ' ') << " |--- Subarray Memory Tiers = " << bank->mat.subarray.stackedMemTiers << endl;
	}
    if (bank->stackedDieCount > 1 && bank->partitionGranularity == 0) {
        cout << string(indent, ' ') << " |--- TSV Area      = " << TO_SQM(bank->tsvArray.area) << endl;
    } else if (bank->stackedDieCount > 1 && bank->partitionGranularity == 1) {
        double totalTSVArea = bank->tsvArray.area + bank->mat.tsvArray.area
                              * bank->numColumnMat * bank->numRowMat;
        double areaLogicLayer = bank->mat.areaAllLogicBlocks * bank->numColumnMat
                              * bank->numRowMat;

        cout << string(indent, ' ') << " |--- TSV Area      = " << TO_SQM(totalTSVArea) << endl;
        cout << string(indent, ' ') << " |--- Logic Layer Area = " << TO_SQM(areaLogicLayer) << endl;
    }
	cout << string(indent, ' ') << " - Area Efficiency = " << cell->area * tech->featureSize * tech->featureSize
			* bank->capacity / bank->area * 100 << "%" << endl;

	cout << string(indent, ' ') << "Timing:" << endl;

	cout << string(indent, ' ') << " -  Read Latency = " << TO_SECOND(bank->readLatency) << endl;
    if (bank->stackedDieCount > 1 && bank->partitionGranularity == 0) {
        double totalTSVLatency = (bank->tsvArray.readLatency + bank->tsvArray.writeLatency) * (bank->stackedDieCount-1);
        cout << string(indent, ' ') << " |--- TSV Latency    = " << TO_SECOND(totalTSVLatency) << endl;
    } else if (bank->stackedDieCount > 1 && bank->partitionGranularity == 1) {
        double totalTSVLatency = bank->tsvArray.readLatency * (bank->stackedDieCount-1)
                               + bank->mat.tsvArray.writeLatency * (bank->stackedDieCount-1);

        cout << string(indent, ' ') << " |--- TSV Latency    = " << TO_SECOND(totalTSVLatency) << endl;
    }
	if (inputParameter->routingMode == h_tree)
		cout << string(indent, ' ') << " |--- H-Tree Latency = " << TO_SECOND(bank->routingReadLatency) << endl;
	else
		cout << string(indent, ' ') << " |--- Non-H-Tree Latency = " << TO_SECOND(bank->routingReadLatency) << endl;
	cout << string(indent, ' ') << " |--- Mat Latency    = " << TO_SECOND(bank->mat.readLatency) << endl;
	cout << string(indent, ' ') << "    |--- Predecoder Latency = " << TO_SECOND(bank->mat.predecoderLatency) << endl;
	cout << string(indent, ' ') << "    |--- Subarray Latency   = " << TO_SECOND(bank->mat.subarray.readLatency) << endl;
	if(cell->memCellType != gcDRAM) { 
		cout << string(indent, ' ') << "       |--- Row Decoder Latency = " << TO_SECOND(bank->mat.subarray.rowDecoder.readLatency) << endl;
		cout << string(indent, ' ') << "       |--- Bitline Latency     = " << TO_SECOND(bank->mat.subarray.bitlineDelay) << endl;
		if (inputParameter->internalSensing)
			cout << string(indent, ' ') << "       |--- Senseamp Latency    = " << TO_SECOND(bank->mat.subarray.senseAmp.readLatency) << endl;
		cout << string(indent, ' ') << "       |--- Precharge Latency   = " << TO_SECOND(bank->mat.subarray.precharger.readLatency) << endl;
	}
	else {
		cout << string(indent, ' ') << "    |--- Subarray Dir2 Latency   = " << TO_SECOND(bank->mat.subarray.readLatency_bidir) << endl;
		cout << string(indent, ' ') << "       |--- Read Row Decoder Latency = " << TO_SECOND(bank->mat.subarray.gcRowDecoder.readLatency) << endl;
		cout << string(indent, ' ') << "       |--- Write Row Decoder Latency = " << TO_SECOND(bank->mat.subarray.rowDecoder.readLatency) << endl;
		cout << string(indent, ' ') << "       |--- Write Level Shifter Latency = " << TO_SECOND(bank->mat.subarray.write_levelshifter.readLatency) << endl;
		cout << string(indent, ' ') << "       |--- Bitline Latency     = " << TO_SECOND(bank->mat.subarray.bitlineDelay) << endl;
		cout << string(indent, ' ') << "       |--- Bitline Dir2 Latency     = " << TO_SECOND(bank->mat.subarray.bitlineDelay_dir2) << endl;
		if (inputParameter->internalSensing)
			cout << string(indent, ' ') << "       |--- Senseamp Latency    = " << TO_SECOND(bank->mat.subarray.senseAmp.readLatency) << endl;
		cout << string(indent, ' ') << "       |--- Senseamp Dir2 Latency    = " << TO_SECOND(bank->mat.subarray.senseAmp_bidir.readLatency) << endl;
		cout << string(indent, ' ') << "       |--- Precharge Latency   = " << TO_SECOND(bank->mat.subarray.precharger.readLatency) << endl;
		cout << string(indent, ' ') << "       |--- Precharge Dir2 Latency   = " << TO_SECOND(bank->mat.subarray.precharger_bidir.readLatency) << endl;
		cout << string(indent, ' ') << "       |--- Write Bitline Latency     = " << TO_SECOND(bank->mat.subarray.writeBitlineDelay_dir1) << endl;
		cout << string(indent, ' ') << "       |--- Write Bitline Dir2 Latency     = " << TO_SECOND(bank->mat.subarray.writeBitlineDelay_dir2) << endl;
		cout << string(indent, ' ') << "       |--- Writecharge Latency   = " << TO_SECOND(bank->mat.subarray.writecharger.readLatency) << endl;
		cout << string(indent, ' ') << "       |--- Writecharge Dir2 Latency   = " << TO_SECOND(bank->mat.subarray.writecharger_bidir.readLatency) << endl;
	}

	cout << string(indent, ' ') << "       |--- Mux Latency         = " << TO_SECOND(bank->mat.subarray.bitlineMux.readLatency
													+ bank->mat.subarray.senseAmpMuxLev1.readLatency
													+ bank->mat.subarray.senseAmpMuxLev2.readLatency) << endl;
	if (bank->mat.memoryType == tag && bank->mat.internalSenseAmp)
		cout << string(indent, ' ') << "    |--- Comparator Latency  = " << TO_SECOND(bank->mat.comparator.readLatency) << endl;

	if (cell->memCellType == PCRAM || cell->memCellType == FBRAM ||
			(cell->memCellType == memristor && (cell->accessType == CMOS_access || cell->accessType == BJT_access))) {
		cout << string(indent, ' ') << " - RESET Latency = " << TO_SECOND(bank->resetLatency) << endl;
        if (bank->stackedDieCount > 1 && bank->partitionGranularity == 0) {
            cout << string(indent, ' ') << " |--- TSV Latency    = " << TO_SECOND(bank->tsvArray.resetLatency * (bank->stackedDieCount-1)) << endl;
        } else if (bank->stackedDieCount > 1 && bank->partitionGranularity == 1) {
            cout << string(indent, ' ') << " |--- TSV Latency    = " << TO_SECOND(bank->tsvArray.resetLatency * (bank->stackedDieCount-1)) << endl;
        }
		if (inputParameter->routingMode == h_tree)
			cout << string(indent, ' ') << " |--- H-Tree Latency = " << TO_SECOND(bank->routingResetLatency) << endl;
		else
			cout << string(indent, ' ') << " |--- Non-H-Tree Latency = " << TO_SECOND(bank->routingResetLatency) << endl;
		cout << string(indent, ' ') << " |--- Mat Latency    = " << TO_SECOND(bank->mat.resetLatency) << endl;
		cout << string(indent, ' ') << "    |--- Predecoder Latency = " << TO_SECOND(bank->mat.predecoderLatency) << endl;
		cout << string(indent, ' ') << "    |--- Subarray Latency   = " << TO_SECOND(bank->mat.subarray.resetLatency) << endl;
		cout << string(indent, ' ') << "       |--- RESET Pulse Duration = " << TO_SECOND(cell->resetPulse) << endl;
		cout << string(indent, ' ') << "       |--- Row Decoder Latency  = " << TO_SECOND(bank->mat.subarray.rowDecoder.writeLatency) << endl;
		cout << string(indent, ' ') << "       |--- Charge Latency   = " << TO_SECOND(bank->mat.subarray.chargeLatency) << endl;
		cout << string(indent, ' ') << " - SET Latency   = " << TO_SECOND(bank->setLatency) << endl;
        if (bank->stackedDieCount > 1 && bank->partitionGranularity == 0) {
            cout << string(indent, ' ') << " |--- TSV Latency    = " << TO_SECOND(bank->tsvArray.setLatency * (bank->stackedDieCount-1)) << endl;
        } else if (bank->stackedDieCount > 1 && bank->partitionGranularity == 1) {
            cout << string(indent, ' ') << " |--- TSV Latency    = " << TO_SECOND(bank->tsvArray.setLatency * (bank->stackedDieCount-1)) << endl;
        }
		if (inputParameter->routingMode == h_tree)
			cout << string(indent, ' ') << " |--- H-Tree Latency = " << TO_SECOND(bank->routingSetLatency) << endl;
		else
			cout << string(indent, ' ') << " |--- Non-H-Tree Latency = " << TO_SECOND(bank->routingSetLatency) << endl;
		cout << string(indent, ' ') << " |--- Mat Latency    = " << TO_SECOND(bank->mat.setLatency) << endl;
		cout << string(indent, ' ') << "    |--- Predecoder Latency = " << TO_SECOND(bank->mat.predecoderLatency) << endl;
		cout << string(indent, ' ') << "    |--- Subarray Latency   = " << TO_SECOND(bank->mat.subarray.setLatency) << endl;
		cout << string(indent, ' ') << "       |--- SET Pulse Duration   = " << TO_SECOND(cell->setPulse) << endl;
		cout << string(indent, ' ') << "       |--- Row Decoder Latency  = " << TO_SECOND(bank->mat.subarray.rowDecoder.writeLatency) << endl;
		cout << string(indent, ' ') << "       |--- Charger Latency      = " << TO_SECOND(bank->mat.subarray.chargeLatency) << endl;
	} else if (cell->memCellType == SLCNAND) {
		cout << string(indent, ' ') << " - Erase Latency = " << TO_SECOND(bank->resetLatency) << endl;
        if (bank->stackedDieCount > 1 && bank->partitionGranularity == 0) {
            cout << string(indent, ' ') << " |--- TSV Latency    = " << TO_SECOND(bank->tsvArray.resetLatency * (bank->stackedDieCount-1)) << endl;
        } else if (bank->stackedDieCount > 1 && bank->partitionGranularity == 1) {
            cout << string(indent, ' ') << " |--- TSV Latency    = " << TO_SECOND(bank->tsvArray.resetLatency * (bank->stackedDieCount-1)) << endl;
        }
		if (inputParameter->routingMode == h_tree)
			cout << string(indent, ' ') << " |--- H-Tree Latency = " << TO_SECOND(bank->routingResetLatency) << endl;
		else
			cout << string(indent, ' ') << " |--- Non-H-Tree Latency = " << TO_SECOND(bank->routingResetLatency) << endl;
		cout << string(indent, ' ') << " |--- Mat Latency    = " << TO_SECOND(bank->mat.resetLatency) << endl;
		cout << string(indent, ' ') << " - Programming Latency   = " << TO_SECOND(bank->setLatency) << endl;
		if (inputParameter->routingMode == h_tree)
			cout << string(indent, ' ') << " |--- H-Tree Latency = " << TO_SECOND(bank->routingSetLatency) << endl;
		else
			cout << string(indent, ' ') << " |--- Non-H-Tree Latency = " << TO_SECOND(bank->routingSetLatency) << endl;
		cout << string(indent, ' ') << " |--- Mat Latency    = " << TO_SECOND(bank->mat.setLatency) << endl;
	} else {
		cout << string(indent, ' ') << " - Write Latency = " << TO_SECOND(bank->writeLatency) << endl;
        if (bank->stackedDieCount > 1 && bank->partitionGranularity == 0) {
            cout << string(indent, ' ') << " |--- TSV Latency    = " << TO_SECOND(bank->tsvArray.writeLatency * (bank->stackedDieCount-1)) << endl;
        } else if (bank->stackedDieCount > 1 && bank->partitionGranularity == 1) {
            cout << string(indent, ' ') << " |--- TSV Latency    = " << TO_SECOND(bank->tsvArray.writeLatency * (bank->stackedDieCount-1)) << endl;
        }
		if (inputParameter->routingMode == h_tree)
			cout << string(indent, ' ') << " |--- H-Tree Latency = " << TO_SECOND(bank->routingWriteLatency) << endl;
		else
			cout << string(indent, ' ') << " |--- Non-H-Tree Latency = " << TO_SECOND(bank->routingWriteLatency) << endl;
		cout << string(indent, ' ') << " |--- Mat Latency    = " << TO_SECOND(bank->mat.writeLatency) << endl;
		cout << string(indent, ' ') << "    |--- Predecoder Latency = " << TO_SECOND(bank->mat.predecoderLatency) << endl;
		cout << string(indent, ' ') << "    |--- Subarray Latency   = " << TO_SECOND(bank->mat.subarray.writeLatency) << endl;
		if (cell->memCellType == gcDRAM) cout << string(indent, ' ')<< "    |--- Subarray Dir2 Latency   = " << TO_SECOND(bank->mat.subarray.writeLatency_bidir) << endl;
		if (cell->memCellType == MRAM)
			cout << string(indent, ' ') << "       |--- Write Pulse Duration = " << TO_SECOND(cell->resetPulse) << endl;	// MRAM reset/set is equal
		cout << string(indent, ' ') << "       |--- Row Decoder Latency = " << TO_SECOND(bank->mat.subarray.rowDecoder.writeLatency) << endl;
		if(cell->memCellType == gcDRAM) cout << string(indent, ' ') << "       |--- Level Shifter Latency = " << TO_SECOND(bank->mat.subarray.write_levelshifter.readLatency) << endl;
		cout << string(indent, ' ') << "       |--- Charge Latency      = " << TO_SECOND(bank->mat.subarray.chargeLatency) << endl;
	}
    if (cell->memCellType == eDRAM || cell->memCellType == gcDRAM) {
        cout << string(indent, ' ') << " - Refresh Latency = " << TO_SECOND(bank->refreshLatency) << endl;
        if ((bank->stackedDieCount > 1 && bank->partitionGranularity == 0)) {
            cout << string(indent, ' ') << " |--- TSV Latency    = " << TO_SECOND(bank->tsvArray.writeLatency * (bank->stackedDieCount-1)) << endl;
        } else if (bank->stackedDieCount > 1 && bank->partitionGranularity == 1) {
            cout << string(indent, ' ') << " |--- TSV Latency    = " << TO_SECOND(bank->tsvArray.writeLatency * (bank->stackedDieCount-1)) << endl;
        } if (bank->mat.subarray.stackedMemTiers) {
			cout << string(indent, ' ') << " |--- MIV Latency    = " << TO_SECOND(bank->mat.subarray.tsvArray.writeLatency * (bank->mat.subarray.stackedMemTiers)) << endl;
			//cout << string(indent, ' ') << " |--- BL Res    = " << TO_SECOND(bank->mat.subarray.resBitline) << endl;
			//cout << string(indent, ' ') << " |--- BL Cap    = " << TO_SECOND(bank->mat.subarray.capBitline) << endl;
		}
		if (inputParameter->routingMode == h_tree)
			cout << string(indent, ' ') << " |--- H-Tree Latency = " << TO_SECOND(bank->routingRefreshLatency) << endl;
		else
			cout << string(indent, ' ') << " |--- Non-H-Tree Latency = " << TO_SECOND(bank->routingRefreshLatency) << endl;
		cout << string(indent, ' ') << " |--- Mat Latency    = " << TO_SECOND(bank->mat.refreshLatency) << endl;
		cout << string(indent, ' ') << "    |--- Predecoder Latency = " << TO_SECOND(bank->mat.predecoderLatency) << endl;
		cout << string(indent, ' ') << "    |--- Subarray Latency   = " << TO_SECOND(bank->mat.subarray.refreshLatency) << endl;
    }

	double readBandwidth = (double)bank->blockSize /
			(bank->mat.subarray.readLatency - bank->mat.subarray.rowDecoder.readLatency
			+ bank->mat.subarray.precharger.readLatency) / 8;
	if (cell->memCellType == gcDRAM) {
		readBandwidth = (double)bank->blockSize /
			(bank->mat.subarray.readLatency - bank->mat.subarray.gcRowDecoder.readLatency
			+ bank->mat.subarray.precharger.readLatency) / 8;
	}
	//cout << "DEBUGGING: bank blockSize - " << bank->blockSize << endl;
	//cout << "DEBUGGING: subarray readLatency - " << TO_SECOND(bank->mat.subarray.readLatency) << ", decoder readLatency - " << TO_SECOND(bank->mat.subarray.rowDecoder.readLatency) << ", precharger latency - " << TO_SECOND(bank->mat.subarray.precharger.readLatency) << endl;
	cout << string(indent, ' ') << " - Read Bandwidth  = " << TO_BPS(readBandwidth) << endl;
	double writeBandwidth = (double)bank->blockSize /
			(bank->mat.subarray.writeLatency) / 8;
	cout << string(indent, ' ') << " - Write Bandwidth = " << TO_BPS(writeBandwidth) << endl;

	cout << string(indent, ' ') << "Power:" << endl;

	cout << string(indent, ' ') << " -  Read Dynamic Energy = " << TO_JOULE(bank->readDynamicEnergy) << endl;
    if (bank->stackedDieCount > 1 && bank->partitionGranularity == 0) {
        // address and control bit dynamics + read data output dynamic
        cout << string(indent, ' ') << " |--- TSV Dynamic Energy    = " << TO_JOULE(bank->tsvArray.writeDynamicEnergy * (bank->stackedDieCount-1) * bank->tsvArray.numReadBits + bank->tsvArray.readDynamicEnergy * bank->tsvArray.numDataBits * (bank->stackedDieCount-1)) << endl;
    } else if (bank->stackedDieCount > 1 && bank->partitionGranularity == 1) {
        // address bit dynamic + control bit dynamic + read data dynamic
        // TODO: revisit this
        cout << string(indent, ' ') << " |--- TSV Dynamic Energy    = " << TO_JOULE(bank->mat.tsvArray.writeDynamicEnergy * (bank->stackedDieCount-1) * (bank->mat.tsvArray.numAccessBits) + bank->tsvArray.writeDynamicEnergy * (bank->stackedDieCount-1) * bank->stackedDieCount + bank->tsvArray.readDynamicEnergy * bank->blockSize * (bank->stackedDieCount-1)) << endl;
    }
	if (inputParameter->routingMode == h_tree)
		cout << string(indent, ' ') << " |--- H-Tree Dynamic Energy = " << TO_JOULE(bank->routingReadDynamicEnergy) << endl;
	else
		cout << string(indent, ' ') << " |--- Non-H-Tree Dynamic Energy = " << TO_JOULE(bank->routingReadDynamicEnergy) << endl;
	cout << string(indent, ' ') << " |--- Mat Dynamic Energy    = " << TO_JOULE(bank->mat.readDynamicEnergy) << " per mat" << endl;
	cout << string(indent, ' ') << "    |--- Predecoder Dynamic Energy = " << TO_JOULE(bank->mat.readDynamicEnergy - bank->mat.subarray.readDynamicEnergy
														* bank->numActiveSubarrayPerRow * bank->numActiveSubarrayPerColumn)
														<< endl;
	cout << string(indent, ' ') << "    |--- Subarray Dynamic Energy   = " << TO_JOULE(bank->mat.subarray.readDynamicEnergy) << " per active subarray" << endl;\
	if (cell->memCellType != gcDRAM)
		cout << string(indent, ' ') << "       |--- Row Decoder Dynamic Energy = " << TO_JOULE(bank->mat.subarray.rowDecoder.readDynamicEnergy) << endl;
	else
		cout << string(indent, ' ') << "       |--- Row Decoder Dynamic Energy = " << TO_JOULE(bank->mat.subarray.gcRowDecoder.readDynamicEnergy) << endl;
	cout << string(indent, ' ') << "       |--- Mux Decoder Dynamic Energy = " << TO_JOULE(bank->mat.subarray.bitlineMuxDecoder.readDynamicEnergy
													+ bank->mat.subarray.senseAmpMuxLev1Decoder.readDynamicEnergy
													+ bank->mat.subarray.senseAmpMuxLev2Decoder.readDynamicEnergy) << endl;
	if (cell->memCellType == PCRAM || cell->memCellType == FBRAM || cell->memCellType == MRAM || cell->memCellType == memristor ) {
		cout << string(indent, ' ') << "       |--- Bitline & Cell Read Energy = " << TO_JOULE(bank->mat.subarray.cellReadEnergy) << endl;
	}
	if (inputParameter->internalSensing)
		cout << string(indent, ' ') << "       |--- Senseamp Dynamic Energy    = " << TO_JOULE(bank->mat.subarray.senseAmp.readDynamicEnergy) << endl;
	cout << string(indent, ' ') << "       |--- Mux Dynamic Energy         = " << TO_JOULE(bank->mat.subarray.bitlineMux.readDynamicEnergy
													+ bank->mat.subarray.senseAmpMuxLev1.readDynamicEnergy
													+ bank->mat.subarray.senseAmpMuxLev2.readDynamicEnergy) << endl;
	cout << string(indent, ' ') << "       |--- Precharge Dynamic Energy   = " << TO_JOULE(bank->mat.subarray.precharger.readDynamicEnergy) << endl;
	//if(cell->memCellType == gcDRAM){
	//	cout << string(indent, ' ') << "       |--- write_levelshifter Dynamic Energy   = " << TO_JOULE(bank->mat.subarray.write_levelshifter.readDynamicEnergy) << endl;
	//}

	if (cell->memCellType == PCRAM || cell->memCellType == FBRAM ||
			(cell->memCellType == memristor && (cell->accessType == CMOS_access || cell->accessType == BJT_access))) {
		cout << string(indent, ' ') << " - RESET Dynamic Energy = " << TO_JOULE(bank->resetDynamicEnergy) << endl;
        if (bank->stackedDieCount > 1 && bank->partitionGranularity == 0) {
            cout << string(indent, ' ') << " |--- TSV Dynamic Energy    = " << TO_JOULE(bank->tsvArray.resetDynamicEnergy * (bank->stackedDieCount-1) * bank->tsvArray.numAccessBits) << endl;
        } else if (bank->stackedDieCount > 1 && bank->partitionGranularity == 1) {
            cout << string(indent, ' ') << " |--- TSV Dynamic Energy    = " << TO_JOULE(bank->tsvArray.resetDynamicEnergy * (bank->stackedDieCount-1) * bank->tsvArray.numAccessBits) << endl;
        }
		if (inputParameter->routingMode == h_tree)
			cout << string(indent, ' ') << " |--- H-Tree Dynamic Energy = " << TO_JOULE(bank->routingResetDynamicEnergy) << endl;
		else
			cout << string(indent, ' ') << " |--- H-Tree Dynamic Energy = " << TO_JOULE(bank->routingResetDynamicEnergy) << endl;
		cout << string(indent, ' ') << " |--- Mat Dynamic Energy    = " << TO_JOULE(bank->mat.resetDynamicEnergy) << " per mat" << endl;
		cout << string(indent, ' ') << "    |--- Predecoder Dynamic Energy = " << TO_JOULE(bank->mat.writeDynamicEnergy - bank->mat.subarray.writeDynamicEnergy
															* bank->numActiveSubarrayPerRow * bank->numActiveSubarrayPerColumn)
															<< endl;
		cout << string(indent, ' ') << "    |--- Subarray Dynamic Energy   = " << TO_JOULE(bank->mat.subarray.writeDynamicEnergy) << " per active subarray" << endl;
		cout << string(indent, ' ') << "       |--- Row Decoder Dynamic Energy = " << TO_JOULE(bank->mat.subarray.rowDecoder.writeDynamicEnergy) << endl;
		cout << string(indent, ' ') << "       |--- Mux Decoder Dynamic Energy = " << TO_JOULE(bank->mat.subarray.bitlineMuxDecoder.writeDynamicEnergy
														+ bank->mat.subarray.senseAmpMuxLev1Decoder.writeDynamicEnergy
														+ bank->mat.subarray.senseAmpMuxLev2Decoder.writeDynamicEnergy) << endl;
		cout << string(indent, ' ') << "       |--- Mux Dynamic Energy         = " << TO_JOULE(bank->mat.subarray.bitlineMux.writeDynamicEnergy
														+ bank->mat.subarray.senseAmpMuxLev1.writeDynamicEnergy
														+ bank->mat.subarray.senseAmpMuxLev2.writeDynamicEnergy) << endl;
		cout << string(indent, ' ') << "       |--- Cell RESET Dynamic Energy  = " << TO_JOULE(bank->mat.subarray.cellResetEnergy) << endl;
		cout << string(indent, ' ') << " - SET Dynamic Energy = " << TO_JOULE(bank->setDynamicEnergy) << endl;
		if (inputParameter->routingMode == h_tree)
			cout << string(indent, ' ') << " |--- H-Tree Dynamic Energy = " << TO_JOULE(bank->routingSetDynamicEnergy) << endl;
		else
			cout << string(indent, ' ') << " |--- Non-H-Tree Dynamic Energy = " << TO_JOULE(bank->routingSetDynamicEnergy) << endl;
		cout << string(indent, ' ') << " |--- Mat Dynamic Energy    = " << TO_JOULE(bank->mat.setDynamicEnergy) << " per mat" << endl;
		cout << string(indent, ' ') << "    |--- Predecoder Dynamic Energy = " << TO_JOULE(bank->mat.writeDynamicEnergy - bank->mat.subarray.writeDynamicEnergy
															* bank->numActiveSubarrayPerRow * bank->numActiveSubarrayPerColumn)
															<< endl;
		cout << string(indent, ' ') << "    |--- Subarray Dynamic Energy   = " << TO_JOULE(bank->mat.subarray.writeDynamicEnergy) << " per active subarray" << endl;
		cout << string(indent, ' ') << "       |--- Row Decoder Dynamic Energy = " << TO_JOULE(bank->mat.subarray.rowDecoder.writeDynamicEnergy) << endl;
		cout << string(indent, ' ') << "       |--- Mux Decoder Dynamic Energy = " << TO_JOULE(bank->mat.subarray.bitlineMuxDecoder.writeDynamicEnergy
														+ bank->mat.subarray.senseAmpMuxLev1Decoder.writeDynamicEnergy
														+ bank->mat.subarray.senseAmpMuxLev2Decoder.writeDynamicEnergy) << endl;
		cout << string(indent, ' ') << "       |--- Mux Dynamic Energy         = " << TO_JOULE(bank->mat.subarray.bitlineMux.writeDynamicEnergy
														+ bank->mat.subarray.senseAmpMuxLev1.writeDynamicEnergy
														+ bank->mat.subarray.senseAmpMuxLev2.writeDynamicEnergy) << endl;
		cout << string(indent, ' ') << "       |--- Cell SET Dynamic Energy    = " << TO_JOULE(bank->mat.subarray.cellSetEnergy) << endl;
	} else if (cell->memCellType == SLCNAND) {
		cout << string(indent, ' ') << " - Erase Dynamic Energy = " << TO_JOULE(bank->resetDynamicEnergy) << " per block" << endl;
        if (bank->stackedDieCount > 1 && bank->partitionGranularity == 0) {
            cout << string(indent, ' ') << " |--- TSV Dynamic Energy    = " << TO_JOULE(bank->tsvArray.resetDynamicEnergy * (bank->stackedDieCount-1) * bank->tsvArray.numAccessBits) << endl;
        } else if (bank->stackedDieCount > 1 && bank->partitionGranularity == 1) {
            cout << string(indent, ' ') << " |--- TSV Dynamic Energy    = " << TO_JOULE(bank->tsvArray.resetDynamicEnergy * (bank->stackedDieCount-1) * bank->tsvArray.numAccessBits) << endl;
        }
		if (inputParameter->routingMode == h_tree)
			cout << string(indent, ' ') << " |--- H-Tree Dynamic Energy = " << TO_JOULE(bank->routingResetDynamicEnergy) << endl;
		else
			cout << string(indent, ' ') << " |--- Non-H-Tree Dynamic Energy = " << TO_JOULE(bank->routingResetDynamicEnergy) << endl;
		cout << string(indent, ' ') << " |--- Mat Dynamic Energy    = " << TO_JOULE(bank->mat.resetDynamicEnergy) << " per mat" << endl;
		cout << string(indent, ' ') << "    |--- Predecoder Dynamic Energy = " << TO_JOULE(bank->mat.writeDynamicEnergy - bank->mat.subarray.writeDynamicEnergy
															* bank->numActiveSubarrayPerRow * bank->numActiveSubarrayPerColumn)
															<< endl;
		cout << string(indent, ' ') << "    |--- Subarray Dynamic Energy   = " << TO_JOULE(bank->mat.subarray.writeDynamicEnergy) << " per active subarray" << endl;
		cout << string(indent, ' ') << "       |--- Row Decoder Dynamic Energy = " << TO_JOULE(bank->mat.subarray.rowDecoder.writeDynamicEnergy) << endl;
		cout << string(indent, ' ') << "       |--- Mux Decoder Dynamic Energy = " << TO_JOULE(bank->mat.subarray.bitlineMuxDecoder.writeDynamicEnergy
														+ bank->mat.subarray.senseAmpMuxLev1Decoder.writeDynamicEnergy
														+ bank->mat.subarray.senseAmpMuxLev2Decoder.writeDynamicEnergy) << endl;
		cout << string(indent, ' ') << "       |--- Mux Dynamic Energy         = " << TO_JOULE(bank->mat.subarray.bitlineMux.writeDynamicEnergy
														+ bank->mat.subarray.senseAmpMuxLev1.writeDynamicEnergy
														+ bank->mat.subarray.senseAmpMuxLev2.writeDynamicEnergy) << endl;
		cout << string(indent, ' ') << " - Programming Dynamic Energy = " << TO_JOULE(bank->setDynamicEnergy) << " per page" << endl;
		if (inputParameter->routingMode == h_tree)
			cout << string(indent, ' ') << " |--- H-Tree Dynamic Energy = " << TO_JOULE(bank->routingSetDynamicEnergy) << endl;
		else
			cout << string(indent, ' ') << " |--- Non-H-Tree Dynamic Energy = " << TO_JOULE(bank->routingSetDynamicEnergy) << endl;
		cout << string(indent, ' ') << " |--- Mat Dynamic Energy    = " << TO_JOULE(bank->mat.setDynamicEnergy) << " per mat" << endl;
		cout << string(indent, ' ') << "    |--- Predecoder Dynamic Energy = " << TO_JOULE(bank->mat.writeDynamicEnergy - bank->mat.subarray.writeDynamicEnergy
															* bank->numActiveSubarrayPerRow * bank->numActiveSubarrayPerColumn)
															<< endl;
		cout << string(indent, ' ') << "    |--- Subarray Dynamic Energy   = " << TO_JOULE(bank->mat.subarray.writeDynamicEnergy) << " per active subarray" << endl;
		cout << string(indent, ' ') << "       |--- Row Decoder Dynamic Energy = " << TO_JOULE(bank->mat.subarray.rowDecoder.writeDynamicEnergy) << endl;
		cout << string(indent, ' ') << "       |--- Mux Decoder Dynamic Energy = " << TO_JOULE(bank->mat.subarray.bitlineMuxDecoder.writeDynamicEnergy
														+ bank->mat.subarray.senseAmpMuxLev1Decoder.writeDynamicEnergy
														+ bank->mat.subarray.senseAmpMuxLev2Decoder.writeDynamicEnergy) << endl;
		cout << string(indent, ' ') << "       |--- Mux Dynamic Energy         = " << TO_JOULE(bank->mat.subarray.bitlineMux.writeDynamicEnergy
														+ bank->mat.subarray.senseAmpMuxLev1.writeDynamicEnergy
														+ bank->mat.subarray.senseAmpMuxLev2.writeDynamicEnergy) << endl;
	} else {
		cout << string(indent, ' ') << " - Write Dynamic Energy = " << TO_JOULE(bank->writeDynamicEnergy) << endl;
        if (bank->stackedDieCount > 1 && bank->partitionGranularity == 0) {
            cout << string(indent, ' ') << " |--- TSV Dynamic Energy    = " << TO_JOULE(bank->tsvArray.writeDynamicEnergy * (bank->stackedDieCount-1) * bank->tsvArray.numAccessBits) << endl;
        } else if (bank->stackedDieCount > 1 && bank->partitionGranularity == 1) {
            cout << string(indent, ' ') << " |--- TSV Dynamic Energy    = " << TO_JOULE(bank->tsvArray.writeDynamicEnergy * (bank->stackedDieCount-1) * bank->tsvArray.numAccessBits) << endl;
        }
		if (inputParameter->routingMode == h_tree)
			cout << string(indent, ' ') << " |--- H-Tree Dynamic Energy = " << TO_JOULE(bank->routingWriteDynamicEnergy) << endl;
		else
			cout << string(indent, ' ') << " |--- Non-H-Tree Dynamic Energy = " << TO_JOULE(bank->routingWriteDynamicEnergy) << endl;
		cout << string(indent, ' ') << " |--- Mat Dynamic Energy    = " << TO_JOULE(bank->mat.writeDynamicEnergy) << " per mat" << endl;
		cout << string(indent, ' ') << "    |--- Predecoder Dynamic Energy = " << TO_JOULE(bank->mat.writeDynamicEnergy - bank->mat.subarray.writeDynamicEnergy
															* bank->numActiveSubarrayPerRow * bank->numActiveSubarrayPerColumn)
															<< endl;
		cout << string(indent, ' ') << "    |--- Subarray Dynamic Energy   = " << TO_JOULE(bank->mat.subarray.writeDynamicEnergy) << " per active subarray" << endl;
		cout << string(indent, ' ') << "       |--- Row Decoder Dynamic Energy = " << TO_JOULE(bank->mat.subarray.rowDecoder.writeDynamicEnergy) << endl;
		if(cell->memCellType == gcDRAM){
			cout << string(indent, ' ') << "       |--- write_levelshifter Dynamic Energy   = " << TO_JOULE(bank->mat.subarray.write_levelshifter.readDynamicEnergy) << endl;
		}
		cout << string(indent, ' ') << "       |--- Mux Decoder Dynamic Energy = " << TO_JOULE(bank->mat.subarray.bitlineMuxDecoder.writeDynamicEnergy
														+ bank->mat.subarray.senseAmpMuxLev1Decoder.writeDynamicEnergy
														+ bank->mat.subarray.senseAmpMuxLev2Decoder.writeDynamicEnergy) << endl;
		cout << string(indent, ' ') << "       |--- Mux Dynamic Energy         = " << TO_JOULE(bank->mat.subarray.bitlineMux.writeDynamicEnergy
														+ bank->mat.subarray.senseAmpMuxLev1.writeDynamicEnergy
														+ bank->mat.subarray.senseAmpMuxLev2.writeDynamicEnergy) << endl;
		if (cell->memCellType == MRAM) {
			cout << string(indent, ' ') << "       |--- Bitline & Cell Write Energy= " << TO_JOULE(bank->mat.subarray.cellResetEnergy) << endl;
		}
	}
    if (cell->memCellType == eDRAM || cell->memCellType == gcDRAM) {
        cout << string(indent, ' ') << " - Refresh Dynamic Energy = " << TO_JOULE(bank->refreshDynamicEnergy) << endl;
        if (bank->stackedDieCount > 1 && bank->partitionGranularity == 0) {
            cout << string(indent, ' ') << " |--- TSV Dynamic Energy    = " << TO_JOULE(bank->tsvArray.writeDynamicEnergy * (bank->stackedDieCount-1) * bank->tsvArray.numReadBits) << endl;
        } else if (bank->stackedDieCount > 1 && bank->partitionGranularity == 1) {
            cout << string(indent, ' ') << " |--- TSV Dynamic Energy    = " << TO_JOULE(bank->tsvArray.writeDynamicEnergy * (bank->stackedDieCount-1) * bank->tsvArray.numReadBits) << endl;
        }
		if (inputParameter->routingMode == h_tree)
			cout << string(indent, ' ') << " |--- H-Tree Dynamic Energy = " << TO_JOULE(bank->routingRefreshDynamicEnergy) << endl;
		else
			cout << string(indent, ' ') << " |--- Non-H-Tree Dynamic Energy = " << TO_JOULE(bank->routingRefreshDynamicEnergy) << endl;
        cout << string(indent, ' ') << " |--- Mat Dynamic Energy    = " << TO_JOULE(bank->mat.refreshDynamicEnergy) << " per mat" << endl;
        cout << string(indent, ' ') << "    |--- Predecoder Dynamic Energy = " << 
                TO_JOULE(bank->mat.refreshDynamicEnergy - bank->mat.subarray.refreshDynamicEnergy
                         * bank->numActiveSubarrayPerRow * bank->numActiveSubarrayPerColumn) << endl;
        cout << string(indent, ' ') << "    |--- Subarray Dynamic Energy   = " << TO_JOULE(bank->mat.subarray.refreshDynamicEnergy) << " per active subarray" << endl;
        cout << string(indent, ' ') << "       |--- Row Decoder Dynamic Energy = " << TO_JOULE(bank->mat.subarray.rowDecoder.readDynamicEnergy) << endl;
        if (inputParameter->internalSensing)
            cout << string(indent, ' ') << "       |--- Senseamp Dynamic Energy    = " << TO_JOULE(bank->mat.subarray.senseAmp.refreshDynamicEnergy) << endl;
        cout << string(indent, ' ') << "       |--- Precharge Dynamic Energy   = " << TO_JOULE(bank->mat.subarray.precharger.refreshDynamicEnergy) << endl;
    }

	cout << string(indent, ' ') << " - Leakage Power = " << TO_WATT(bank->leakage) << endl;
	cout << string(indent, ' ') << " -- Subarray Leakage Power = " << TO_WATT(bank->mat.subarray.leakage) << endl;
	/* Subarray Component Leakage Analysis */
	cout << string(indent, ' ') << " -- Subarray Leakage rowDecoder Power = " << TO_WATT(bank->mat.subarray.rowDecoder.leakage) << endl;
	cout << string(indent, ' ') << " -- Subarray Leakage bitlineMuxDecoder Power = " << TO_WATT(bank->mat.subarray.bitlineMuxDecoder.leakage) << endl;
	cout << string(indent, ' ') << " -- Subarray Leakage bitlineMux Power = " << TO_WATT(bank->mat.subarray.bitlineMux.leakage) << endl;
	cout << string(indent, ' ') << " -- Subarray Leakage senseAmpMuxLev1 Power = " << TO_WATT(bank->mat.subarray.senseAmpMuxLev1.leakage) << endl;
	cout << string(indent, ' ') << " -- Subarray Leakage senseAmpMuxLev2 Power = " << TO_WATT(bank->mat.subarray.senseAmpMuxLev2.leakage) << endl;
	cout << string(indent, ' ') << " -- Subarray Leakage senseAmpMuxLev1Decoder Power = " << TO_WATT(bank->mat.subarray.senseAmpMuxLev1Decoder.leakage) << endl;
	cout << string(indent, ' ') << " -- Subarray Leakage senseAmpMuxLev2Decoder Power = " << TO_WATT(bank->mat.subarray.senseAmpMuxLev2Decoder.leakage) << endl;
	cout << string(indent, ' ') << " -- Subarray Leakage precharger Power = " << TO_WATT(bank->mat.subarray.precharger.leakage) << endl;
	cout << string(indent, ' ') << " -- Subarray Leakage senseAmp Power = " << TO_WATT(bank->mat.subarray.senseAmp.leakage) << endl;
	cout << string(indent, ' ') << " END SUBARRAY LEAKAGE ANALYSIS "<< endl;
	/* Subarray Component Leakage Analysis END*/
    if (bank->stackedDieCount > 1 && bank->partitionGranularity == 0) {
        cout << string(indent, ' ') << " |--- TSV Leakage              = " << TO_WATT(bank->tsvArray.leakage * (bank->stackedDieCount-1) * bank->tsvArray.numTotalBits) << endl;
    } else if (bank->stackedDieCount > 1 && bank->partitionGranularity == 1) {
        cout << string(indent, ' ') << " |--- TSV Leakage              = " << TO_WATT(bank->tsvArray.leakage * (bank->stackedDieCount-1) * bank->tsvArray.numTotalBits + bank->mat.tsvArray.leakage * bank->numColumnMat * bank->numRowMat * bank->mat.tsvArray.numTotalBits) << endl;
    }
	if (inputParameter->routingMode == h_tree){
		cout << string(indent, ' ') << " |--- H-Tree Leakage Power     = " << TO_WATT(bank->routingLeakage) << endl;
	} else
		cout << string(indent, ' ') << " |--- Non-H-Tree Leakage Power = " << TO_WATT(bank->routingLeakage) << endl;
	cout << string(indent, ' ') << " |--- Mat Leakage Power        = " << TO_WATT(bank->mat.leakage) << " per mat" << endl;
    if (cell->memCellType == eDRAM || cell->memCellType == gcDRAM) {
        cout << string(indent, ' ') << " - Refresh Power = " << TO_WATT(bank->refreshDynamicEnergy / (cell->retentionTime)) << endl;
    }
}

void Result::printToFile(int indent, const string &FileName) {
    // Attempt to open the file
    std::ofstream outFile(FileName.c_str());
    if (!outFile.is_open()) {
        std::cerr << "Error opening file: " << FileName << std::endl;
        return;
    }

    	outFile << string(indent, ' ') << endl;
    outFile << string(indent, ' ') << "=============" << endl;
    outFile << string(indent, ' ') << "   SUMMARY   " << endl;
    outFile << string(indent, ' ') << "=============" << endl;
    outFile << string(indent, ' ') << "Optimized for: " << printOptimizationTarget() << endl;
    // cellTech->PrintCell(indent);
	outFile << string(indent, ' ') << endl;
    outFile << string(indent, ' ') << "=============" << endl;
    outFile << string(indent, ' ') << "CONFIGURATION" << endl;
    outFile << string(indent, ' ') << "=============" << endl;
    if (bank->stackedDieCount > 1) {
        outFile << string(indent, ' ') << "Bank Organization: " << bank->numRowMat << " x " << bank->numColumnMat << " x " << bank->stackedDieCount << endl;
        outFile << string(indent, ' ') << " - Row Activation   : " << bank->numActiveMatPerColumn << " / " << bank->numRowMat << " x 1" << endl;
        outFile << string(indent, ' ') << " - Column Activation: " << bank->numActiveMatPerRow << " / " << bank->numColumnMat << " x 1" << endl;
    } else {
        outFile << string(indent, ' ') << "Bank Organization: " << bank->numRowMat << " x " << bank->numColumnMat << endl;
        outFile << string(indent, ' ') << " - Row Activation   : " << bank->numActiveMatPerColumn << " / " << bank->numRowMat << endl;
        outFile << string(indent, ' ') << " - Column Activation: " << bank->numActiveMatPerRow << " / " << bank->numColumnMat << endl;
    }
	outFile << string(indent, ' ') << "Mat Organization: " << bank->numRowSubarray << " x " << bank->numColumnSubarray << endl;
	outFile << string(indent, ' ') << " - Row Activation   : " << bank->numActiveSubarrayPerColumn << " / " << bank->numRowSubarray << endl;
	outFile << string(indent, ' ') << " - Column Activation: " << bank->numActiveSubarrayPerRow << " / " << bank->numColumnSubarray << endl;
	outFile << string(indent, ' ') << " - Subarray Size    : " << bank->mat.subarray.numRow << " Rows x " << bank->mat.subarray.numColumn << " Columns" << endl;
	outFile << string(indent, ' ') << "Mux Level:" << endl;
	outFile << string(indent, ' ') << " - Senseamp Mux      : " << bank->muxSenseAmp << endl;
	outFile << string(indent, ' ') << " - Output Level-1 Mux: " << bank->muxOutputLev1 << endl;
	outFile << string(indent, ' ') << " - Output Level-2 Mux: " << bank->muxOutputLev2 << endl;
	if (inputParameter->designTarget == cache)
		outFile << string(indent, ' ') << " - One set is partitioned into " << bank->numRowPerSet << " rows" << endl;
	outFile << string(indent, ' ') << "Local Wire:" << endl;
	outFile << string(indent, ' ') << " - Wire Type : ";
	switch (localWire->wireType) {
	case local_aggressive:
		outFile << string(indent, ' ') << "Local Aggressive" << endl;
		break;
	case local_conservative:
		outFile << string(indent, ' ') << "Local Conservative" << endl;
		break;
	case semi_aggressive:
		outFile << string(indent, ' ') << "Semi-Global Aggressive" << endl;
		break;
	case semi_conservative:
		outFile << string(indent, ' ') << "Semi-Global Conservative" << endl;
		break;
	case global_aggressive:
		outFile << string(indent, ' ') << "Global Aggressive" << endl;
		break;
	case global_conservative:
		outFile << string(indent, ' ') << "Global Conservative" << endl;
		break;
	default:
		outFile << string(indent, ' ') << "DRAM Wire" << endl;
	}
	outFile << string(indent, ' ') << " - Repeater Type: ";
	switch (localWire->wireRepeaterType) {
	case repeated_none:
		outFile << string(indent, ' ') << "No Repeaters" << endl;
		break;
	case repeated_opt:
		outFile << string(indent, ' ') << "Fully-Optimized Repeaters" << endl;
		break;
	case repeated_5:
		outFile << string(indent, ' ') << "Repeaters with 5% Overhead" << endl;
		break;
	case repeated_10:
		outFile << string(indent, ' ') << "Repeaters with 10% Overhead" << endl;
		break;
	case repeated_20:
		outFile << string(indent, ' ') << "Repeaters with 20% Overhead" << endl;
		break;
	case repeated_30:
		outFile << string(indent, ' ') << "Repeaters with 30% Overhead" << endl;
		break;
	case repeated_40:
		outFile << string(indent, ' ') << "Repeaters with 40% Overhead" << endl;
		break;
	case repeated_50:
		outFile << string(indent, ' ') << "Repeaters with 50% Overhead" << endl;
		break;
	default:
		outFile << string(indent, ' ') << "Unknown" << endl;
	}
	outFile << string(indent, ' ') << " - Low Swing : ";
	if (localWire->isLowSwing)
		outFile << string(indent, ' ') << "Yes" << endl;
	else
		outFile << string(indent, ' ') << "No" << endl;
	outFile << string(indent, ' ') << "Global Wire:" << endl;
	outFile << string(indent, ' ') << " - Wire Type : ";
	switch (globalWire->wireType) {
	case local_aggressive:
		outFile << string(indent, ' ') << "Local Aggressive" << endl;
		break;
	case local_conservative:
		outFile << string(indent, ' ') << "Local Conservative" << endl;
		break;
	case semi_aggressive:
		outFile << string(indent, ' ') << "Semi-Global Aggressive" << endl;
		break;
	case semi_conservative:
		outFile << string(indent, ' ') << "Semi-Global Conservative" << endl;
		break;
	case global_aggressive:
		outFile << string(indent, ' ') << "Global Aggressive" << endl;
		break;
	case global_conservative:
		outFile << string(indent, ' ') << "Global Conservative" << endl;
		break;
	default:
		outFile << string(indent, ' ') << "DRAM Wire" << endl;
	}
	outFile << string(indent, ' ') << " - Repeater Type: ";
	switch (globalWire->wireRepeaterType) {
	case repeated_none:
		outFile << string(indent, ' ') << "No Repeaters" << endl;
		break;
	case repeated_opt:
		outFile << string(indent, ' ') << "Fully-Optimized Repeaters" << endl;
		break;
	case repeated_5:
		outFile << string(indent, ' ') << "Repeaters with 5% Overhead" << endl;
		break;
	case repeated_10:
		outFile << string(indent, ' ') << "Repeaters with 10% Overhead" << endl;
		break;
	case repeated_20:
		outFile << string(indent, ' ') << "Repeaters with 20% Overhead" << endl;
		break;
	case repeated_30:
		outFile << string(indent, ' ') << "Repeaters with 30% Overhead" << endl;
		break;
	case repeated_40:
		outFile << string(indent, ' ') << "Repeaters with 40% Overhead" << endl;
		break;
	case repeated_50:
		outFile << string(indent, ' ') << "Repeaters with 50% Overhead" << endl;
		break;
	default:
		outFile << string(indent, ' ') << "Unknown" << endl;
	}
	outFile << string(indent, ' ') << " - Low Swing : ";
	if (globalWire->isLowSwing)
		outFile << string(indent, ' ') << "Yes" << endl;
	else
		outFile << string(indent, ' ') << "No" << endl;
	outFile << string(indent, ' ') << "Buffer Design Style: ";
	switch (bank->areaOptimizationLevel) {
	case latency_first:
		outFile << string(indent, ' ') << "Latency-Optimized" << endl;
		break;
	case area_first:
		outFile << string(indent, ' ') << "Area-Optimized" << endl;
		break;
	default:	/* balance */
		outFile << string(indent, ' ') << "Balanced" << endl;
	}

	outFile << string(indent, ' ') << "=============" << endl;
    outFile << string(indent, ' ') << "   RESULT" << endl;
    outFile << string(indent, ' ') << "=============" << endl;

	outFile << string(indent, ' ') << "Area:" << endl;

	outFile << string(indent, ' ') << " - Total Area = " << TO_METER(bank->height) << " x " << TO_METER(bank->width)
			<< " = " << TO_SQM(bank->area) << endl;
	outFile << string(indent, ' ') << " |--- Mat Area      = " << TO_METER(bank->mat.height) << " x " << TO_METER(bank->mat.width)
			<< " = " << TO_SQM(bank->mat.area) << "   (" << cell->area * tech->featureSize * tech->featureSize
			* bank->capacity / bank->numRowMat / bank->numColumnMat / bank->mat.area * 100 << "%)" << endl;
	outFile << string(indent, ' ') << " |--- Mat BEOL:FEOL Area = " << bank->mat.subarray.memoryArea / bank->mat.subarray.area <<  endl;
	outFile << string(indent, ' ') << " |--- Subarray Area = " << TO_METER(bank->mat.subarray.height) << " x "
			<< TO_METER(bank->mat.subarray.width) << " = " << TO_SQM(bank->mat.subarray.area) << "   ("
			<< cell->area * tech->featureSize * tech->featureSize * bank->capacity / bank->numRowMat
			/ bank->numColumnMat / bank->numRowSubarray / bank->numColumnSubarray
			/ bank->mat.subarray.area * 100 << "%)" <<endl;
	
	/* SubArray Area BreakDown */

	outFile << string(indent, ' ') << " |--- Subarray rowDecoder Area (BothDirs) = " << TO_SQM(2*bank->mat.subarray.rowDecoder.area) << endl;
	outFile << string(indent, ' ') << " |--- Subarray WWL Decoder Area Dir1 = " << TO_SQM(bank->mat.subarray.gcRowDecoder.area) << endl;
	outFile << string(indent, ' ') << " |--- Subarray WWL Decoder Area Dir2 = " << TO_SQM(bank->mat.subarray.gcRowDecoder_bidir.area) << endl;
	outFile << string(indent, ' ') << " |--- Subarray bitlineMuxDecoder Area = " << TO_SQM(2*bank->mat.subarray.bitlineMuxDecoder.area) << endl;
	outFile << string(indent, ' ') << " |--- Subarray bitlineMux Area = " << TO_SQM(bank->mat.subarray.bitlineMux.area) << endl;
	outFile << string(indent, ' ') << " |--- Subarray senseAmpMuxLev1 Area = " << TO_SQM(bank->mat.subarray.senseAmpMuxLev1.area) << endl;
	outFile << string(indent, ' ') << " |--- Subarray senseAmpMuxLev2 Area = " << TO_SQM(bank->mat.subarray.senseAmpMuxLev2.area) << endl;
	outFile << string(indent, ' ') << " |--- Subarray senseAmpMuxLev1Decoder Area = " << TO_SQM(bank->mat.subarray.senseAmpMuxLev1Decoder.area) << endl;
	outFile << string(indent, ' ') << " |--- Subarray senseAmpMuxLev2Decoder Area = " << TO_SQM(bank->mat.subarray.senseAmpMuxLev2Decoder.area) << endl;
	outFile << string(indent, ' ') << " |--- Subarray precharger Area Dir1 = " << TO_SQM(2*bank->mat.subarray.precharger.area) << endl;
	outFile << string(indent, ' ') << " |--- Subarray writeDriver Area Dir1 = " << TO_SQM(2*bank->mat.subarray.writecharger.area) << endl;
	outFile << string(indent, ' ') << " |--- Subarray precharger Area Dir2 = " << TO_SQM(2*bank->mat.subarray.precharger_bidir.area) << endl;
	outFile << string(indent, ' ') << " |--- Subarray writeDriver Area Dir2 = " << TO_SQM(2*bank->mat.subarray.writecharger_bidir.area) << endl;
	outFile << string(indent, ' ') << " |--- Subarray senseAmp Area Dir1= " << TO_SQM(bank->mat.subarray.senseAmp.area) << endl;
	outFile << string(indent, ' ') << " |--- Subarray senseAmp Area Dir2 = " << TO_SQM(bank->mat.subarray.senseAmp_bidir.area) << endl;
	outFile << string(indent, ' ') << " |--- Subarray LevelShifter Area (BothDirs) = " << TO_SQM(bank->mat.subarray.write_levelshifter.area + bank->mat.subarray.read_levelshifter.area) << endl;
	outFile << string(indent, ' ') << " |--- Subarray MIV Area = " << TO_SQM(bank->mat.subarray.tsvArray.area) << endl;


	/*SubArray Area BreakDown End*/
	
	if (cell->memCellType == gcDRAM){
		outFile << string(indent, ' ') << " |--- Subarray Memory Tiers = " << bank->mat.subarray.stackedMemTiers << endl;
	}
    if (bank->stackedDieCount > 1 && bank->partitionGranularity == 0) {
        outFile << string(indent, ' ') << " |--- TSV Area      = " << TO_SQM(bank->tsvArray.area) << endl;
    } else if (bank->stackedDieCount > 1 && bank->partitionGranularity == 1) {
        double totalTSVArea = bank->tsvArray.area + bank->mat.tsvArray.area
                              * bank->numColumnMat * bank->numRowMat;
        double areaLogicLayer = bank->mat.areaAllLogicBlocks * bank->numColumnMat
                              * bank->numRowMat;

        outFile << string(indent, ' ') << " |--- TSV Area      = " << TO_SQM(totalTSVArea) << endl;
        outFile << string(indent, ' ') << " |--- Logic Layer Area = " << TO_SQM(areaLogicLayer) << endl;
    }
	outFile << string(indent, ' ') << " - Area Efficiency = " << cell->area * tech->featureSize * tech->featureSize
			* bank->capacity / bank->area * 100 << "%" << endl;

	outFile << string(indent, ' ') << "Timing:" << endl;

	outFile << string(indent, ' ') << " -  Read Latency = " << TO_SECOND(bank->readLatency) << endl;
    if (bank->stackedDieCount > 1 && bank->partitionGranularity == 0) {
        double totalTSVLatency = (bank->tsvArray.readLatency + bank->tsvArray.writeLatency) * (bank->stackedDieCount-1);
        outFile << string(indent, ' ') << " |--- TSV Latency    = " << TO_SECOND(totalTSVLatency) << endl;
    } else if (bank->stackedDieCount > 1 && bank->partitionGranularity == 1) {
        double totalTSVLatency = bank->tsvArray.readLatency * (bank->stackedDieCount-1)
                               + bank->mat.tsvArray.writeLatency * (bank->stackedDieCount-1);

        outFile << string(indent, ' ') << " |--- TSV Latency    = " << TO_SECOND(totalTSVLatency) << endl;
    }
	if (inputParameter->routingMode == h_tree)
		outFile << string(indent, ' ') << " |--- H-Tree Latency = " << TO_SECOND(bank->routingReadLatency) << endl;
	else
		outFile << string(indent, ' ') << " |--- Non-H-Tree Latency = " << TO_SECOND(bank->routingReadLatency) << endl;
	outFile << string(indent, ' ') << " |--- Mat Latency    = " << TO_SECOND(bank->mat.readLatency) << endl;
	outFile << string(indent, ' ') << "    |--- Predecoder Latency = " << TO_SECOND(bank->mat.predecoderLatency) << endl;
	outFile << string(indent, ' ') << "    |--- Subarray Latency   = " << TO_SECOND(bank->mat.subarray.readLatency) << endl;
	if(cell->memCellType != gcDRAM) { 
		outFile << string(indent, ' ') << "       |--- Row Decoder Latency = " << TO_SECOND(bank->mat.subarray.rowDecoder.readLatency) << endl;
		outFile << string(indent, ' ') << "       |--- Bitline Latency     = " << TO_SECOND(bank->mat.subarray.bitlineDelay) << endl;
		if (inputParameter->internalSensing)
			outFile << string(indent, ' ') << "       |--- Senseamp Latency    = " << TO_SECOND(bank->mat.subarray.senseAmp.readLatency) << endl;
		outFile << string(indent, ' ') << "       |--- Precharge Latency   = " << TO_SECOND(bank->mat.subarray.precharger.readLatency) << endl;
	}
	else {
		outFile << string(indent, ' ') << "    |--- Subarray Dir2 Latency   = " << TO_SECOND(bank->mat.subarray.readLatency_bidir) << endl;
		outFile << string(indent, ' ') << "       |--- Read Row Decoder Latency = " << TO_SECOND(bank->mat.subarray.gcRowDecoder.readLatency) << endl;
		outFile << string(indent, ' ') << "       |--- Write Row Decoder Latency = " << TO_SECOND(bank->mat.subarray.rowDecoder.readLatency) << endl;
		outFile << string(indent, ' ') << "       |--- Write Level Shifter Latency = " << TO_SECOND(bank->mat.subarray.write_levelshifter.readLatency) << endl;
		outFile << string(indent, ' ') << "       |--- Bitline Latency     = " << TO_SECOND(bank->mat.subarray.bitlineDelay_dir1) << endl;
		outFile << string(indent, ' ') << "       |--- Bitline Dir2 Latency     = " << TO_SECOND(bank->mat.subarray.bitlineDelay_dir2) << endl;
		if (inputParameter->internalSensing)
			outFile << string(indent, ' ') << "       |--- Senseamp Latency    = " << TO_SECOND(bank->mat.subarray.senseAmp.readLatency) << endl;
		outFile << string(indent, ' ') << "       |--- Senseamp Dir2 Latency    = " << TO_SECOND(bank->mat.subarray.senseAmp_bidir.readLatency) << endl;
		outFile << string(indent, ' ') << "       |--- Precharge Latency   = " << TO_SECOND(bank->mat.subarray.precharger.readLatency) << endl;
		outFile << string(indent, ' ') << "       |--- Precharge Dir2 Latency   = " << TO_SECOND(bank->mat.subarray.precharger_bidir.readLatency) << endl;
		outFile << string(indent, ' ') << "       |--- Write Bitline Latency     = " << TO_SECOND(bank->mat.subarray.writeBitlineDelay_dir1) << endl;
		outFile << string(indent, ' ') << "       |--- Write Bitline Dir2 Latency     = " << TO_SECOND(bank->mat.subarray.writeBitlineDelay_dir2) << endl;
		outFile << string(indent, ' ') << "       |--- Writecharge Latency   = " << TO_SECOND(bank->mat.subarray.writecharger.readLatency) << endl;
		outFile << string(indent, ' ') << "       |--- Writecharge Dir2 Latency   = " << TO_SECOND(bank->mat.subarray.writecharger_bidir.readLatency) << endl;
	}

	outFile << string(indent, ' ') << "       |--- Mux Latency         = " << TO_SECOND(bank->mat.subarray.bitlineMux.readLatency
													+ bank->mat.subarray.senseAmpMuxLev1.readLatency
													+ bank->mat.subarray.senseAmpMuxLev2.readLatency) << endl;
	if (bank->mat.memoryType == tag && bank->mat.internalSenseAmp)
		outFile << string(indent, ' ') << "    |--- Comparator Latency  = " << TO_SECOND(bank->mat.comparator.readLatency) << endl;

	if (cell->memCellType == PCRAM || cell->memCellType == FBRAM ||
			(cell->memCellType == memristor && (cell->accessType == CMOS_access || cell->accessType == BJT_access))) {
		outFile << string(indent, ' ') << " - RESET Latency = " << TO_SECOND(bank->resetLatency) << endl;
        if (bank->stackedDieCount > 1 && bank->partitionGranularity == 0) {
            outFile << string(indent, ' ') << " |--- TSV Latency    = " << TO_SECOND(bank->tsvArray.resetLatency * (bank->stackedDieCount-1)) << endl;
        } else if (bank->stackedDieCount > 1 && bank->partitionGranularity == 1) {
            outFile << string(indent, ' ') << " |--- TSV Latency    = " << TO_SECOND(bank->tsvArray.resetLatency * (bank->stackedDieCount-1)) << endl;
        }
		if (inputParameter->routingMode == h_tree)
			outFile << string(indent, ' ') << " |--- H-Tree Latency = " << TO_SECOND(bank->routingResetLatency) << endl;
		else
			outFile << string(indent, ' ') << " |--- Non-H-Tree Latency = " << TO_SECOND(bank->routingResetLatency) << endl;
		outFile << string(indent, ' ') << " |--- Mat Latency    = " << TO_SECOND(bank->mat.resetLatency) << endl;
		outFile << string(indent, ' ') << "    |--- Predecoder Latency = " << TO_SECOND(bank->mat.predecoderLatency) << endl;
		outFile << string(indent, ' ') << "    |--- Subarray Latency   = " << TO_SECOND(bank->mat.subarray.resetLatency) << endl;
		outFile << string(indent, ' ') << "       |--- RESET Pulse Duration = " << TO_SECOND(cell->resetPulse) << endl;
		outFile << string(indent, ' ') << "       |--- Row Decoder Latency  = " << TO_SECOND(bank->mat.subarray.rowDecoder.writeLatency) << endl;
		outFile << string(indent, ' ') << "       |--- Charge Latency   = " << TO_SECOND(bank->mat.subarray.chargeLatency) << endl;
		outFile << string(indent, ' ') << " - SET Latency   = " << TO_SECOND(bank->setLatency) << endl;
        if (bank->stackedDieCount > 1 && bank->partitionGranularity == 0) {
            outFile << string(indent, ' ') << " |--- TSV Latency    = " << TO_SECOND(bank->tsvArray.setLatency * (bank->stackedDieCount-1)) << endl;
        } else if (bank->stackedDieCount > 1 && bank->partitionGranularity == 1) {
            outFile << string(indent, ' ') << " |--- TSV Latency    = " << TO_SECOND(bank->tsvArray.setLatency * (bank->stackedDieCount-1)) << endl;
        }
		if (inputParameter->routingMode == h_tree)
			outFile << string(indent, ' ') << " |--- H-Tree Latency = " << TO_SECOND(bank->routingSetLatency) << endl;
		else
			outFile << string(indent, ' ') << " |--- Non-H-Tree Latency = " << TO_SECOND(bank->routingSetLatency) << endl;
		outFile << string(indent, ' ') << " |--- Mat Latency    = " << TO_SECOND(bank->mat.setLatency) << endl;
		outFile << string(indent, ' ') << "    |--- Predecoder Latency = " << TO_SECOND(bank->mat.predecoderLatency) << endl;
		outFile << string(indent, ' ') << "    |--- Subarray Latency   = " << TO_SECOND(bank->mat.subarray.setLatency) << endl;
		outFile << string(indent, ' ') << "       |--- SET Pulse Duration   = " << TO_SECOND(cell->setPulse) << endl;
		outFile << string(indent, ' ') << "       |--- Row Decoder Latency  = " << TO_SECOND(bank->mat.subarray.rowDecoder.writeLatency) << endl;
		outFile << string(indent, ' ') << "       |--- Charger Latency      = " << TO_SECOND(bank->mat.subarray.chargeLatency) << endl;
	} else if (cell->memCellType == SLCNAND) {
		outFile << string(indent, ' ') << " - Erase Latency = " << TO_SECOND(bank->resetLatency) << endl;
        if (bank->stackedDieCount > 1 && bank->partitionGranularity == 0) {
            outFile << string(indent, ' ') << " |--- TSV Latency    = " << TO_SECOND(bank->tsvArray.resetLatency * (bank->stackedDieCount-1)) << endl;
        } else if (bank->stackedDieCount > 1 && bank->partitionGranularity == 1) {
            outFile << string(indent, ' ') << " |--- TSV Latency    = " << TO_SECOND(bank->tsvArray.resetLatency * (bank->stackedDieCount-1)) << endl;
        }
		if (inputParameter->routingMode == h_tree)
			outFile << string(indent, ' ') << " |--- H-Tree Latency = " << TO_SECOND(bank->routingResetLatency) << endl;
		else
			outFile << string(indent, ' ') << " |--- Non-H-Tree Latency = " << TO_SECOND(bank->routingResetLatency) << endl;
		outFile << string(indent, ' ') << " |--- Mat Latency    = " << TO_SECOND(bank->mat.resetLatency) << endl;
		outFile << string(indent, ' ') << " - Programming Latency   = " << TO_SECOND(bank->setLatency) << endl;
		if (inputParameter->routingMode == h_tree)
			outFile << string(indent, ' ') << " |--- H-Tree Latency = " << TO_SECOND(bank->routingSetLatency) << endl;
		else
			outFile << string(indent, ' ') << " |--- Non-H-Tree Latency = " << TO_SECOND(bank->routingSetLatency) << endl;
		outFile << string(indent, ' ') << " |--- Mat Latency    = " << TO_SECOND(bank->mat.setLatency) << endl;
	} else {
		outFile << string(indent, ' ') << " - Write Latency = " << TO_SECOND(bank->writeLatency) << endl;
        if (bank->stackedDieCount > 1 && bank->partitionGranularity == 0) {
            outFile << string(indent, ' ') << " |--- TSV Latency    = " << TO_SECOND(bank->tsvArray.writeLatency * (bank->stackedDieCount-1)) << endl;
        } else if (bank->stackedDieCount > 1 && bank->partitionGranularity == 1) {
            outFile << string(indent, ' ') << " |--- TSV Latency    = " << TO_SECOND(bank->tsvArray.writeLatency * (bank->stackedDieCount-1)) << endl;
        }
		if (inputParameter->routingMode == h_tree)
			outFile << string(indent, ' ') << " |--- H-Tree Latency = " << TO_SECOND(bank->routingWriteLatency) << endl;
		else
			outFile << string(indent, ' ') << " |--- Non-H-Tree Latency = " << TO_SECOND(bank->routingWriteLatency) << endl;
		outFile << string(indent, ' ') << " |--- Mat Latency    = " << TO_SECOND(bank->mat.writeLatency) << endl;
		outFile << string(indent, ' ') << "    |--- Predecoder Latency = " << TO_SECOND(bank->mat.predecoderLatency) << endl;
		outFile << string(indent, ' ') << "    |--- Subarray Latency   = " << TO_SECOND(bank->mat.subarray.writeLatency) << endl;
		if (cell->memCellType == gcDRAM) outFile << string(indent, ' ')<< "    |--- Subarray Dir2 Latency   = " << TO_SECOND(bank->mat.subarray.writeLatency_bidir) << endl;
		if (cell->memCellType == MRAM)
			outFile << string(indent, ' ') << "       |--- Write Pulse Duration = " << TO_SECOND(cell->resetPulse) << endl;	// MRAM reset/set is equal
		outFile << string(indent, ' ') << "       |--- Row Decoder Latency = " << TO_SECOND(bank->mat.subarray.rowDecoder.writeLatency) << endl;
		if(cell->memCellType == gcDRAM) outFile << string(indent, ' ') << "       |--- Level Shifter Latency = " << TO_SECOND(bank->mat.subarray.write_levelshifter.readLatency) << endl;
		outFile << string(indent, ' ') << "       |--- Charge Latency      = " << TO_SECOND(bank->mat.subarray.chargeLatency) << endl;
	}
    if (cell->memCellType == eDRAM || cell->memCellType == gcDRAM) {
        outFile << string(indent, ' ') << " - Refresh Latency = " << TO_SECOND(bank->refreshLatency) << endl;
        if ((bank->stackedDieCount > 1 && bank->partitionGranularity == 0)) {
            outFile << string(indent, ' ') << " |--- TSV Latency    = " << TO_SECOND(bank->tsvArray.writeLatency * (bank->stackedDieCount-1)) << endl;
        } else if (bank->stackedDieCount > 1 && bank->partitionGranularity == 1) {
            outFile << string(indent, ' ') << " |--- TSV Latency    = " << TO_SECOND(bank->tsvArray.writeLatency * (bank->stackedDieCount-1)) << endl;
        } if (bank->mat.subarray.stackedMemTiers) {
			outFile << string(indent, ' ') << " |--- MIV Latency    = " << TO_SECOND(bank->mat.subarray.tsvArray.writeLatency * (bank->mat.subarray.stackedMemTiers)) << endl;
			//outFile << string(indent, ' ') << " |--- BL Res    = " << TO_SECOND(bank->mat.subarray.resBitline) << endl;
			//outFile << string(indent, ' ') << " |--- BL Cap    = " << TO_SECOND(bank->mat.subarray.capBitline) << endl;
		}
		if (inputParameter->routingMode == h_tree)
			outFile << string(indent, ' ') << " |--- H-Tree Latency = " << TO_SECOND(bank->routingRefreshLatency) << endl;
		else
			outFile << string(indent, ' ') << " |--- Non-H-Tree Latency = " << TO_SECOND(bank->routingRefreshLatency) << endl;
		outFile << string(indent, ' ') << " |--- Mat Latency    = " << TO_SECOND(bank->mat.refreshLatency) << endl;
		outFile << string(indent, ' ') << "    |--- Predecoder Latency = " << TO_SECOND(bank->mat.predecoderLatency) << endl;
		outFile << string(indent, ' ') << "    |--- Subarray Latency   = " << TO_SECOND(bank->mat.subarray.refreshLatency) << endl;
    }

	double readBandwidth = (double)bank->blockSize /
			(bank->mat.subarray.readLatency - bank->mat.subarray.rowDecoder.readLatency
			+ bank->mat.subarray.precharger.readLatency) / 8;
	if (cell->memCellType == gcDRAM) {
		readBandwidth = (double)bank->blockSize /
			(bank->mat.subarray.readLatency - bank->mat.subarray.gcRowDecoder.readLatency
			+ bank->mat.subarray.precharger.readLatency) / 8;
	}
	//outFile << "DEBUGGING: bank blockSize - " << bank->blockSize << endl;
	//outFile << "DEBUGGING: subarray readLatency - " << TO_SECOND(bank->mat.subarray.readLatency) << ", decoder readLatency - " << TO_SECOND(bank->mat.subarray.rowDecoder.readLatency) << ", precharger latency - " << TO_SECOND(bank->mat.subarray.precharger.readLatency) << endl;
	outFile << string(indent, ' ') << " - Read Bandwidth  = " << TO_BPS(readBandwidth) << endl;
	double writeBandwidth = (double)bank->blockSize /
			(bank->mat.subarray.writeLatency) / 8;
	outFile << string(indent, ' ') << " - Write Bandwidth = " << TO_BPS(writeBandwidth) << endl;

	outFile << string(indent, ' ') << "Power:" << endl;

	outFile << string(indent, ' ') << " -  Read Dynamic Energy = " << TO_JOULE(bank->readDynamicEnergy) << endl;
    if (bank->stackedDieCount > 1 && bank->partitionGranularity == 0) {
        // address and control bit dynamics + read data output dynamic
        outFile << string(indent, ' ') << " |--- TSV Dynamic Energy    = " << TO_JOULE(bank->tsvArray.writeDynamicEnergy * (bank->stackedDieCount-1) * bank->tsvArray.numReadBits + bank->tsvArray.readDynamicEnergy * bank->tsvArray.numDataBits * (bank->stackedDieCount-1)) << endl;
    } else if (bank->stackedDieCount > 1 && bank->partitionGranularity == 1) {
        // address bit dynamic + control bit dynamic + read data dynamic
        // TODO: revisit this
        outFile << string(indent, ' ') << " |--- TSV Dynamic Energy    = " << TO_JOULE(bank->mat.tsvArray.writeDynamicEnergy * (bank->stackedDieCount-1) * (bank->mat.tsvArray.numAccessBits) + bank->tsvArray.writeDynamicEnergy * (bank->stackedDieCount-1) * bank->stackedDieCount + bank->tsvArray.readDynamicEnergy * bank->blockSize * (bank->stackedDieCount-1)) << endl;
    }
	if (inputParameter->routingMode == h_tree)
		outFile << string(indent, ' ') << " |--- H-Tree Dynamic Energy = " << TO_JOULE(bank->routingReadDynamicEnergy) << endl;
	else
		outFile << string(indent, ' ') << " |--- Non-H-Tree Dynamic Energy = " << TO_JOULE(bank->routingReadDynamicEnergy) << endl;
	outFile << string(indent, ' ') << " |--- Mat Dynamic Energy    = " << TO_JOULE(bank->mat.readDynamicEnergy) << " per mat" << endl;
	outFile << string(indent, ' ') << "    |--- Predecoder Dynamic Energy = " << TO_JOULE(bank->mat.readDynamicEnergy - bank->mat.subarray.readDynamicEnergy
														* bank->numActiveSubarrayPerRow * bank->numActiveSubarrayPerColumn)
														<< endl;
	outFile << string(indent, ' ') << "    |--- Subarray Dynamic Energy   = " << TO_JOULE(bank->mat.subarray.readDynamicEnergy) << " per active subarray" << endl;\
	if (cell->memCellType != gcDRAM)
		outFile << string(indent, ' ') << "       |--- Row Decoder Dynamic Energy = " << TO_JOULE(bank->mat.subarray.rowDecoder.readDynamicEnergy) << endl;
	else
		outFile << string(indent, ' ') << "       |--- Row Decoder Dynamic Energy = " << TO_JOULE(bank->mat.subarray.gcRowDecoder.readDynamicEnergy) << endl;
	outFile << string(indent, ' ') << "       |--- Mux Decoder Dynamic Energy = " << TO_JOULE(bank->mat.subarray.bitlineMuxDecoder.readDynamicEnergy
													+ bank->mat.subarray.senseAmpMuxLev1Decoder.readDynamicEnergy
													+ bank->mat.subarray.senseAmpMuxLev2Decoder.readDynamicEnergy) << endl;
	if (cell->memCellType == PCRAM || cell->memCellType == FBRAM || cell->memCellType == MRAM || cell->memCellType == memristor ) {
		outFile << string(indent, ' ') << "       |--- Bitline & Cell Read Energy = " << TO_JOULE(bank->mat.subarray.cellReadEnergy) << endl;
	}
	if (inputParameter->internalSensing)
		outFile << string(indent, ' ') << "       |--- Senseamp Dynamic Energy    = " << TO_JOULE(bank->mat.subarray.senseAmp.readDynamicEnergy) << endl;
	outFile << string(indent, ' ') << "       |--- Mux Dynamic Energy         = " << TO_JOULE(bank->mat.subarray.bitlineMux.readDynamicEnergy
													+ bank->mat.subarray.senseAmpMuxLev1.readDynamicEnergy
													+ bank->mat.subarray.senseAmpMuxLev2.readDynamicEnergy) << endl;
	outFile << string(indent, ' ') << "       |--- Precharge Dynamic Energy   = " << TO_JOULE(bank->mat.subarray.precharger.readDynamicEnergy) << endl;
	//if(cell->memCellType == gcDRAM){
	//	outFile << string(indent, ' ') << "       |--- write_levelshifter Dynamic Energy   = " << TO_JOULE(bank->mat.subarray.write_levelshifter.readDynamicEnergy) << endl;
	//}

	if (cell->memCellType == PCRAM || cell->memCellType == FBRAM ||
			(cell->memCellType == memristor && (cell->accessType == CMOS_access || cell->accessType == BJT_access))) {
		outFile << string(indent, ' ') << " - RESET Dynamic Energy = " << TO_JOULE(bank->resetDynamicEnergy) << endl;
        if (bank->stackedDieCount > 1 && bank->partitionGranularity == 0) {
            outFile << string(indent, ' ') << " |--- TSV Dynamic Energy    = " << TO_JOULE(bank->tsvArray.resetDynamicEnergy * (bank->stackedDieCount-1) * bank->tsvArray.numAccessBits) << endl;
        } else if (bank->stackedDieCount > 1 && bank->partitionGranularity == 1) {
            outFile << string(indent, ' ') << " |--- TSV Dynamic Energy    = " << TO_JOULE(bank->tsvArray.resetDynamicEnergy * (bank->stackedDieCount-1) * bank->tsvArray.numAccessBits) << endl;
        }
		if (inputParameter->routingMode == h_tree)
			outFile << string(indent, ' ') << " |--- H-Tree Dynamic Energy = " << TO_JOULE(bank->routingResetDynamicEnergy) << endl;
		else
			outFile << string(indent, ' ') << " |--- H-Tree Dynamic Energy = " << TO_JOULE(bank->routingResetDynamicEnergy) << endl;
		outFile << string(indent, ' ') << " |--- Mat Dynamic Energy    = " << TO_JOULE(bank->mat.resetDynamicEnergy) << " per mat" << endl;
		outFile << string(indent, ' ') << "    |--- Predecoder Dynamic Energy = " << TO_JOULE(bank->mat.writeDynamicEnergy - bank->mat.subarray.writeDynamicEnergy
															* bank->numActiveSubarrayPerRow * bank->numActiveSubarrayPerColumn)
															<< endl;
		outFile << string(indent, ' ') << "    |--- Subarray Dynamic Energy   = " << TO_JOULE(bank->mat.subarray.writeDynamicEnergy) << " per active subarray" << endl;
		outFile << string(indent, ' ') << "       |--- Row Decoder Dynamic Energy = " << TO_JOULE(bank->mat.subarray.rowDecoder.writeDynamicEnergy) << endl;
		outFile << string(indent, ' ') << "       |--- Mux Decoder Dynamic Energy = " << TO_JOULE(bank->mat.subarray.bitlineMuxDecoder.writeDynamicEnergy
														+ bank->mat.subarray.senseAmpMuxLev1Decoder.writeDynamicEnergy
														+ bank->mat.subarray.senseAmpMuxLev2Decoder.writeDynamicEnergy) << endl;
		outFile << string(indent, ' ') << "       |--- Mux Dynamic Energy         = " << TO_JOULE(bank->mat.subarray.bitlineMux.writeDynamicEnergy
														+ bank->mat.subarray.senseAmpMuxLev1.writeDynamicEnergy
														+ bank->mat.subarray.senseAmpMuxLev2.writeDynamicEnergy) << endl;
		outFile << string(indent, ' ') << "       |--- Cell RESET Dynamic Energy  = " << TO_JOULE(bank->mat.subarray.cellResetEnergy) << endl;
		outFile << string(indent, ' ') << " - SET Dynamic Energy = " << TO_JOULE(bank->setDynamicEnergy) << endl;
		if (inputParameter->routingMode == h_tree)
			outFile << string(indent, ' ') << " |--- H-Tree Dynamic Energy = " << TO_JOULE(bank->routingSetDynamicEnergy) << endl;
		else
			outFile << string(indent, ' ') << " |--- Non-H-Tree Dynamic Energy = " << TO_JOULE(bank->routingSetDynamicEnergy) << endl;
		outFile << string(indent, ' ') << " |--- Mat Dynamic Energy    = " << TO_JOULE(bank->mat.setDynamicEnergy) << " per mat" << endl;
		outFile << string(indent, ' ') << "    |--- Predecoder Dynamic Energy = " << TO_JOULE(bank->mat.writeDynamicEnergy - bank->mat.subarray.writeDynamicEnergy
															* bank->numActiveSubarrayPerRow * bank->numActiveSubarrayPerColumn)
															<< endl;
		outFile << string(indent, ' ') << "    |--- Subarray Dynamic Energy   = " << TO_JOULE(bank->mat.subarray.writeDynamicEnergy) << " per active subarray" << endl;
		outFile << string(indent, ' ') << "       |--- Row Decoder Dynamic Energy = " << TO_JOULE(bank->mat.subarray.rowDecoder.writeDynamicEnergy) << endl;
		outFile << string(indent, ' ') << "       |--- Mux Decoder Dynamic Energy = " << TO_JOULE(bank->mat.subarray.bitlineMuxDecoder.writeDynamicEnergy
														+ bank->mat.subarray.senseAmpMuxLev1Decoder.writeDynamicEnergy
														+ bank->mat.subarray.senseAmpMuxLev2Decoder.writeDynamicEnergy) << endl;
		outFile << string(indent, ' ') << "       |--- Mux Dynamic Energy         = " << TO_JOULE(bank->mat.subarray.bitlineMux.writeDynamicEnergy
														+ bank->mat.subarray.senseAmpMuxLev1.writeDynamicEnergy
														+ bank->mat.subarray.senseAmpMuxLev2.writeDynamicEnergy) << endl;
		outFile << string(indent, ' ') << "       |--- Cell SET Dynamic Energy    = " << TO_JOULE(bank->mat.subarray.cellSetEnergy) << endl;
	} else if (cell->memCellType == SLCNAND) {
		outFile << string(indent, ' ') << " - Erase Dynamic Energy = " << TO_JOULE(bank->resetDynamicEnergy) << " per block" << endl;
        if (bank->stackedDieCount > 1 && bank->partitionGranularity == 0) {
            outFile << string(indent, ' ') << " |--- TSV Dynamic Energy    = " << TO_JOULE(bank->tsvArray.resetDynamicEnergy * (bank->stackedDieCount-1) * bank->tsvArray.numAccessBits) << endl;
        } else if (bank->stackedDieCount > 1 && bank->partitionGranularity == 1) {
            outFile << string(indent, ' ') << " |--- TSV Dynamic Energy    = " << TO_JOULE(bank->tsvArray.resetDynamicEnergy * (bank->stackedDieCount-1) * bank->tsvArray.numAccessBits) << endl;
        }
		if (inputParameter->routingMode == h_tree)
			outFile << string(indent, ' ') << " |--- H-Tree Dynamic Energy = " << TO_JOULE(bank->routingResetDynamicEnergy) << endl;
		else
			outFile << string(indent, ' ') << " |--- Non-H-Tree Dynamic Energy = " << TO_JOULE(bank->routingResetDynamicEnergy) << endl;
		outFile << string(indent, ' ') << " |--- Mat Dynamic Energy    = " << TO_JOULE(bank->mat.resetDynamicEnergy) << " per mat" << endl;
		outFile << string(indent, ' ') << "    |--- Predecoder Dynamic Energy = " << TO_JOULE(bank->mat.writeDynamicEnergy - bank->mat.subarray.writeDynamicEnergy
															* bank->numActiveSubarrayPerRow * bank->numActiveSubarrayPerColumn)
															<< endl;
		outFile << string(indent, ' ') << "    |--- Subarray Dynamic Energy   = " << TO_JOULE(bank->mat.subarray.writeDynamicEnergy) << " per active subarray" << endl;
		outFile << string(indent, ' ') << "       |--- Row Decoder Dynamic Energy = " << TO_JOULE(bank->mat.subarray.rowDecoder.writeDynamicEnergy) << endl;
		outFile << string(indent, ' ') << "       |--- Mux Decoder Dynamic Energy = " << TO_JOULE(bank->mat.subarray.bitlineMuxDecoder.writeDynamicEnergy
														+ bank->mat.subarray.senseAmpMuxLev1Decoder.writeDynamicEnergy
														+ bank->mat.subarray.senseAmpMuxLev2Decoder.writeDynamicEnergy) << endl;
		outFile << string(indent, ' ') << "       |--- Mux Dynamic Energy         = " << TO_JOULE(bank->mat.subarray.bitlineMux.writeDynamicEnergy
														+ bank->mat.subarray.senseAmpMuxLev1.writeDynamicEnergy
														+ bank->mat.subarray.senseAmpMuxLev2.writeDynamicEnergy) << endl;
		outFile << string(indent, ' ') << " - Programming Dynamic Energy = " << TO_JOULE(bank->setDynamicEnergy) << " per page" << endl;
		if (inputParameter->routingMode == h_tree)
			outFile << string(indent, ' ') << " |--- H-Tree Dynamic Energy = " << TO_JOULE(bank->routingSetDynamicEnergy) << endl;
		else
			outFile << string(indent, ' ') << " |--- Non-H-Tree Dynamic Energy = " << TO_JOULE(bank->routingSetDynamicEnergy) << endl;
		outFile << string(indent, ' ') << " |--- Mat Dynamic Energy    = " << TO_JOULE(bank->mat.setDynamicEnergy) << " per mat" << endl;
		outFile << string(indent, ' ') << "    |--- Predecoder Dynamic Energy = " << TO_JOULE(bank->mat.writeDynamicEnergy - bank->mat.subarray.writeDynamicEnergy
															* bank->numActiveSubarrayPerRow * bank->numActiveSubarrayPerColumn)
															<< endl;
		outFile << string(indent, ' ') << "    |--- Subarray Dynamic Energy   = " << TO_JOULE(bank->mat.subarray.writeDynamicEnergy) << " per active subarray" << endl;
		outFile << string(indent, ' ') << "       |--- Row Decoder Dynamic Energy = " << TO_JOULE(bank->mat.subarray.rowDecoder.writeDynamicEnergy) << endl;
		outFile << string(indent, ' ') << "       |--- Mux Decoder Dynamic Energy = " << TO_JOULE(bank->mat.subarray.bitlineMuxDecoder.writeDynamicEnergy
														+ bank->mat.subarray.senseAmpMuxLev1Decoder.writeDynamicEnergy
														+ bank->mat.subarray.senseAmpMuxLev2Decoder.writeDynamicEnergy) << endl;
		outFile << string(indent, ' ') << "       |--- Mux Dynamic Energy         = " << TO_JOULE(bank->mat.subarray.bitlineMux.writeDynamicEnergy
														+ bank->mat.subarray.senseAmpMuxLev1.writeDynamicEnergy
														+ bank->mat.subarray.senseAmpMuxLev2.writeDynamicEnergy) << endl;
	} else {
		outFile << string(indent, ' ') << " - Write Dynamic Energy = " << TO_JOULE(bank->writeDynamicEnergy) << endl;
        if (bank->stackedDieCount > 1 && bank->partitionGranularity == 0) {
            outFile << string(indent, ' ') << " |--- TSV Dynamic Energy    = " << TO_JOULE(bank->tsvArray.writeDynamicEnergy * (bank->stackedDieCount-1) * bank->tsvArray.numAccessBits) << endl;
        } else if (bank->stackedDieCount > 1 && bank->partitionGranularity == 1) {
            outFile << string(indent, ' ') << " |--- TSV Dynamic Energy    = " << TO_JOULE(bank->tsvArray.writeDynamicEnergy * (bank->stackedDieCount-1) * bank->tsvArray.numAccessBits) << endl;
        }
		if (inputParameter->routingMode == h_tree)
			outFile << string(indent, ' ') << " |--- H-Tree Dynamic Energy = " << TO_JOULE(bank->routingWriteDynamicEnergy) << endl;
		else
			outFile << string(indent, ' ') << " |--- Non-H-Tree Dynamic Energy = " << TO_JOULE(bank->routingWriteDynamicEnergy) << endl;
		outFile << string(indent, ' ') << " |--- Mat Dynamic Energy    = " << TO_JOULE(bank->mat.writeDynamicEnergy) << " per mat" << endl;
		outFile << string(indent, ' ') << "    |--- Predecoder Dynamic Energy = " << TO_JOULE(bank->mat.writeDynamicEnergy - bank->mat.subarray.writeDynamicEnergy
															* bank->numActiveSubarrayPerRow * bank->numActiveSubarrayPerColumn)
															<< endl;
		outFile << string(indent, ' ') << "    |--- Subarray Dynamic Energy   = " << TO_JOULE(bank->mat.subarray.writeDynamicEnergy) << " per active subarray" << endl;
		outFile << string(indent, ' ') << "       |--- Row Decoder Dynamic Energy = " << TO_JOULE(bank->mat.subarray.rowDecoder.writeDynamicEnergy) << endl;
		if(cell->memCellType == gcDRAM){
			outFile << string(indent, ' ') << "       |--- write_levelshifter Dynamic Energy   = " << TO_JOULE(bank->mat.subarray.write_levelshifter.readDynamicEnergy) << endl;
		}
		outFile << string(indent, ' ') << "       |--- Mux Decoder Dynamic Energy = " << TO_JOULE(bank->mat.subarray.bitlineMuxDecoder.writeDynamicEnergy
														+ bank->mat.subarray.senseAmpMuxLev1Decoder.writeDynamicEnergy
														+ bank->mat.subarray.senseAmpMuxLev2Decoder.writeDynamicEnergy) << endl;
		outFile << string(indent, ' ') << "       |--- Mux Dynamic Energy         = " << TO_JOULE(bank->mat.subarray.bitlineMux.writeDynamicEnergy
														+ bank->mat.subarray.senseAmpMuxLev1.writeDynamicEnergy
														+ bank->mat.subarray.senseAmpMuxLev2.writeDynamicEnergy) << endl;
		if (cell->memCellType == MRAM) {
			outFile << string(indent, ' ') << "       |--- Bitline & Cell Write Energy= " << TO_JOULE(bank->mat.subarray.cellResetEnergy) << endl;
		}
	}
    if (cell->memCellType == eDRAM || cell->memCellType == gcDRAM) {
        outFile << string(indent, ' ') << " - Refresh Dynamic Energy = " << TO_JOULE(bank->refreshDynamicEnergy) << endl;
        if (bank->stackedDieCount > 1 && bank->partitionGranularity == 0) {
            outFile << string(indent, ' ') << " |--- TSV Dynamic Energy    = " << TO_JOULE(bank->tsvArray.writeDynamicEnergy * (bank->stackedDieCount-1) * bank->tsvArray.numReadBits) << endl;
        } else if (bank->stackedDieCount > 1 && bank->partitionGranularity == 1) {
            outFile << string(indent, ' ') << " |--- TSV Dynamic Energy    = " << TO_JOULE(bank->tsvArray.writeDynamicEnergy * (bank->stackedDieCount-1) * bank->tsvArray.numReadBits) << endl;
        }
		if (inputParameter->routingMode == h_tree)
			outFile << string(indent, ' ') << " |--- H-Tree Dynamic Energy = " << TO_JOULE(bank->routingRefreshDynamicEnergy) << endl;
		else
			outFile << string(indent, ' ') << " |--- Non-H-Tree Dynamic Energy = " << TO_JOULE(bank->routingRefreshDynamicEnergy) << endl;
        outFile << string(indent, ' ') << " |--- Mat Dynamic Energy    = " << TO_JOULE(bank->mat.refreshDynamicEnergy) << " per mat" << endl;
        outFile << string(indent, ' ') << "    |--- Predecoder Dynamic Energy = " << 
                TO_JOULE(bank->mat.refreshDynamicEnergy - bank->mat.subarray.refreshDynamicEnergy
                         * bank->numActiveSubarrayPerRow * bank->numActiveSubarrayPerColumn) << endl;
        outFile << string(indent, ' ') << "    |--- Subarray Dynamic Energy   = " << TO_JOULE(bank->mat.subarray.refreshDynamicEnergy) << " per active subarray" << endl;
        outFile << string(indent, ' ') << "       |--- Row Decoder Dynamic Energy = " << TO_JOULE(bank->mat.subarray.rowDecoder.readDynamicEnergy) << endl;
        if (inputParameter->internalSensing)
            outFile << string(indent, ' ') << "       |--- Senseamp Dynamic Energy    = " << TO_JOULE(bank->mat.subarray.senseAmp.refreshDynamicEnergy) << endl;
        outFile << string(indent, ' ') << "       |--- Precharge Dynamic Energy   = " << TO_JOULE(bank->mat.subarray.precharger.refreshDynamicEnergy) << endl; }

		outFile << string(indent, ' ') << " - Leakage Power = " << TO_WATT(bank->leakage) << endl;
		outFile << string(indent, ' ') << " -- Subarray Leakage Power = " << TO_WATT(bank->mat.subarray.leakage) << endl;
    outFile.close();
}

void Result::printAsCache(Result &tagResult, CacheAccessMode cacheAccessMode) {
	if (bank->memoryType != data || tagResult.bank->memoryType != tag) {
		cout << "This is not a valid cache configuration." << endl;
		return;
	} else {
		double cacheHitLatency, cacheMissLatency, cacheWriteLatency;
		double cacheHitDynamicEnergy, cacheMissDynamicEnergy, cacheWriteDynamicEnergy;
		double cacheLeakage;
		double cacheArea;
		if (cacheAccessMode == normal_access_mode) {
			/* Calculate latencies */
			cacheMissLatency = tagResult.bank->readLatency;		/* only the tag access latency */
			cacheHitLatency = MAX(tagResult.bank->readLatency, bank->mat.readLatency);	/* access tag and activate data row in parallel */
			cacheHitLatency += bank->mat.subarray.columnDecoderLatency;		/* add column decoder latency after hit signal arrives */
			cacheHitLatency += bank->readLatency - bank->mat.readLatency;	/* H-tree in and out latency */
			cacheWriteLatency = MAX(tagResult.bank->writeLatency, bank->writeLatency);	/* Data and tag are written in parallel */
			/* Calculate power */
			cacheMissDynamicEnergy = tagResult.bank->readDynamicEnergy;	/* no matter what tag is always accessed */
			cacheMissDynamicEnergy += bank->readDynamicEnergy;	/* data is also partially accessed, TO-DO: not accurate here */
			cacheHitDynamicEnergy = tagResult.bank->readDynamicEnergy + bank->readDynamicEnergy;
			cacheWriteDynamicEnergy = tagResult.bank->writeDynamicEnergy + bank->writeDynamicEnergy;
		} else if (cacheAccessMode == fast_access_mode) {
			/* Calculate latencies */
			cacheMissLatency = tagResult.bank->readLatency;
			cacheHitLatency = MAX(tagResult.bank->readLatency, bank->readLatency);
			cacheWriteLatency = MAX(tagResult.bank->writeLatency, bank->writeLatency);
			/* Calculate power */
			cacheMissDynamicEnergy = tagResult.bank->readDynamicEnergy;	/* no matter what tag is always accessed */
			cacheMissDynamicEnergy += bank->readDynamicEnergy;	/* data is also partially accessed, TO-DO: not accurate here */
			cacheHitDynamicEnergy = tagResult.bank->readDynamicEnergy + bank->readDynamicEnergy;
			cacheWriteDynamicEnergy = tagResult.bank->writeDynamicEnergy + bank->writeDynamicEnergy;
		} else {		/* sequential access */
			/* Calculate latencies */
			cacheMissLatency = tagResult.bank->readLatency;
			cacheHitLatency = tagResult.bank->readLatency + bank->readLatency;
			cacheWriteLatency = MAX(tagResult.bank->writeLatency, bank->writeLatency);
			/* Calculate power */
			cacheMissDynamicEnergy = tagResult.bank->readDynamicEnergy;	/* no matter what tag is always accessed */
			cacheHitDynamicEnergy = tagResult.bank->readDynamicEnergy + bank->readDynamicEnergy;
			cacheWriteDynamicEnergy = tagResult.bank->writeDynamicEnergy + bank->writeDynamicEnergy;
		}
		/* Calculate leakage */
		cacheLeakage = tagResult.bank->leakage + bank->leakage;
		/* Calculate area */
		cacheArea = tagResult.bank->area + bank->area;	/* TO-DO: simply add them together here */

		/* start printing */
		cout << endl << "=======================" << endl << "CACHE DESIGN -- SUMMARY" << endl << "=======================" << endl;
		cout << "Access Mode: ";
		switch (cacheAccessMode) {
		case normal_access_mode:
			cout << "Normal" << endl;
			break;
		case fast_access_mode:
			cout << "Fast" << endl;
			break;
		default:	/* sequential */
			cout << "Sequential" << endl;
		}
		cout << "Area:" << endl;
		cout << " - Total Area = " << cacheArea * 1e6 << "mm^2" << endl;
		cout << " |--- Data Array Area = " << bank->height * 1e6 << "um x " << bank->width * 1e6 << "um = " << bank->area * 1e6 << "mm^2" << endl;
		cout << " |--- Tag Array Area  = " << tagResult.bank->height * 1e6 << "um x " << tagResult.bank->width * 1e6 << "um = " << tagResult.bank->area * 1e6 << "mm^2" << endl;
		cout << "Timing:" << endl;
		cout << " - Cache Hit Latency   = " << cacheHitLatency * 1e9 << "ns" << endl;
		cout << " - Cache Miss Latency  = " << cacheMissLatency * 1e9 << "ns" << endl;
		cout << " - Cache Write Latency = " << cacheWriteLatency * 1e9 << "ns" << endl;
        if (cell->memCellType == eDRAM || cell->memCellType == gcDRAM) {
            cout << " - Cache Refresh Latency = " << MAX(tagResult.bank->refreshLatency, bank->refreshLatency) * 1e6 << "us per bank" << endl;
            cout << " - Cache Availability = " << ((cell->retentionTime - MAX(tagResult.bank->refreshLatency, bank->refreshLatency)) / cell->retentionTime) * 100.0 << "%" << endl;
        }
		cout << "Power:" << endl;
		cout << " - Cache Hit Dynamic Energy   = " << cacheHitDynamicEnergy * 1e9 << "nJ per access" << endl;
		cout << " - Cache Miss Dynamic Energy  = " << cacheMissDynamicEnergy * 1e9 << "nJ per access" << endl;
		cout << " - Cache Write Dynamic Energy = " << cacheWriteDynamicEnergy * 1e9 << "nJ per access" << endl;
        if (cell->memCellType == eDRAM || cell->memCellType == gcDRAM) {
            cout << " - Cache Refresh Dynamic Energy = " << (tagResult.bank->refreshDynamicEnergy + bank->refreshDynamicEnergy) * 1e9 << "nJ per bank" << endl;
        }
		cout << " - Cache Total Leakage Power  = " << cacheLeakage * 1e3 << "mW" << endl;
		cout << " |--- Cache Data Array Leakage Power = " << bank->leakage * 1e3 << "mW" << endl;
		cout << " |--- Cache Tag Array Leakage Power  = " << tagResult.bank->leakage * 1e3 << "mW" << endl;
        if (cell->memCellType == eDRAM || cell->memCellType == gcDRAM) {
            cout << " - Cache Refresh Power = " << TO_WATT(bank->refreshDynamicEnergy / (cell->retentionTime)) << " per bank" << endl;
        }
        if (inputParameter->printLevel > 0) {
            cout << endl << "CACHE DATA ARRAY DETAILS";
            print(4);
            cout << endl << "CACHE TAG ARRAY DETAILS";
            tagResult.print(4);
        }
	}
}

void Result::printAsCacheToFile(CacheAccessMode cacheAccessMode, const string &FileName) {
    // Attempt to open the file
    ofstream outFile(FileName.c_str());
    if (!outFile.is_open()) {
        cerr << "Error opening file: " << FileName << endl;
        return;
    }

    if (bank->memoryType != data) {
        outFile << "This is not a valid cache configuration." << endl;
        outFile.close();
        return;
    } else {
        double cacheHitLatency, cacheMissLatency, cacheWriteLatency;
        double cacheHitDynamicEnergy, cacheMissDynamicEnergy, cacheWriteDynamicEnergy;
        double cacheLeakage;
        double cacheArea;

        if (cacheAccessMode == normal_access_mode) {
            // Calculate latencies
            // cacheMissLatency = tagResult.bank->readLatency;  // only the tag access latency
            cacheHitLatency =
                bank->mat.readLatency; // access tag and activate data row in parallel
            cacheHitLatency += bank->mat.subarray.columnDecoderLatency;  // add column decoder latency after hit
            cacheHitLatency += bank->readLatency - bank->mat.readLatency; // H-tree in and out latency
            cacheWriteLatency = bank->writeLatency; // Data and tag are written in parallel

            // Calculate power
            //cacheMissDynamicEnergy = tagResult.bank->readDynamicEnergy; // no matter what tag is always accessed
            cacheMissDynamicEnergy += bank->readDynamicEnergy;          // data is also partially accessed
            cacheHitDynamicEnergy = bank->readDynamicEnergy;
            cacheWriteDynamicEnergy = bank->writeDynamicEnergy;
        } else if (cacheAccessMode == fast_access_mode) {
            // Calculate latencies
            // cacheMissLatency = tagResult.bank->readLatency;
            cacheHitLatency = bank->readLatency;
            cacheWriteLatency = bank->writeLatency;

            // Calculate power
            // cacheMissDynamicEnergy = tagResult.bank->readDynamicEnergy; // no matter what tag is always accessed
            cacheMissDynamicEnergy += bank->readDynamicEnergy;          // data is also partially accessed
            cacheHitDynamicEnergy = bank->readDynamicEnergy;
            cacheWriteDynamicEnergy = bank->writeDynamicEnergy;
        } else { // sequential access
            // Calculate latencies
            // cacheMissLatency = tagResult.bank->readLatency;
            cacheHitLatency = bank->readLatency;
            cacheWriteLatency =bank->writeLatency;

            // Calculate power
            //cacheMissDynamicEnergy = tagResult.bank->readDynamicEnergy; // no matter what tag is always accessed
            cacheHitDynamicEnergy = bank->readDynamicEnergy;
            cacheWriteDynamicEnergy = bank->writeDynamicEnergy;
        }

        // Calculate leakage
        // cacheLeakage = tagResult.bank->leakage + bank->leakage;
        // Calculate area
        cacheArea = bank->area;  // Just add them

        // Now write to the file instead of the console
        outFile << endl
                << "=======================" << endl
                << "CACHE DESIGN -- SUMMARY" << endl
                << "=======================" << endl;
        outFile << "Access Mode: ";
        switch (cacheAccessMode) {
            case normal_access_mode:
                outFile << "Normal" << endl;
                break;
            case fast_access_mode:
                outFile << "Fast" << endl;
                break;
            default: // sequential
                outFile << "Sequential" << endl;
        }

        // Area
        outFile << "Area:" << endl;
        outFile << " - Total Area = " << cacheArea * 1e6 << "mm^2" << endl;
        outFile << " |--- Data Array Area = " << bank->height * 1e6 << "um x "
                << bank->width * 1e6 << "um = " << bank->area * 1e6 << "mm^2" << endl;
        //outFile << " |--- Tag Array Area  = " << tagResult.bank->height * 1e6 << "um x "
        //        << tagResult.bank->width * 1e6 << "um = " << tagResult.bank->area * 1e6 << "mm^2" << endl;

        // Timing
        outFile << "Timing:" << endl;
        outFile << " - Cache Hit Latency   = " << cacheHitLatency * 1e9 << "ns" << endl;
        outFile << " - Cache Miss Latency  = " << cacheMissLatency * 1e9 << "ns" << endl;
        outFile << " - Cache Write Latency = " << cacheWriteLatency * 1e9 << "ns" << endl;
        if (cell->memCellType == eDRAM || cell->memCellType == gcDRAM) {
            outFile << " - Cache Refresh Latency = "
                    << bank->refreshLatency * 1e6
                    << "us per bank" << endl;
            outFile << " - Cache Availability = "
                    << ((cell->retentionTime -
                         bank->refreshLatency) /
                        cell->retentionTime) *
                           100.0
                    << "%" << endl;
        }

        // Power
        outFile << "Power:" << endl;
        outFile << " - Cache Hit Dynamic Energy   = " << cacheHitDynamicEnergy * 1e9
                << "nJ per access" << endl;
        outFile << " - Cache Miss Dynamic Energy  = " << cacheMissDynamicEnergy * 1e9
                << "nJ per access" << endl;
        outFile << " - Cache Write Dynamic Energy = " << cacheWriteDynamicEnergy * 1e9
                << "nJ per access" << endl;
        if (cell->memCellType == eDRAM || cell->memCellType == gcDRAM) {
            outFile << " - Cache Refresh Dynamic Energy = "
                    << (bank->refreshDynamicEnergy) * 1e9
                    << "nJ per bank" << endl;
        }
        outFile << " - Cache Total Leakage Power  = " << cacheLeakage * 1e3 << "mW" << endl;
        outFile << " |--- Cache Data Array Leakage Power = " << bank->leakage * 1e3 << "mW" << endl;
        //outFile << " |--- Cache Tag Array Leakage Power  = "
        //        << tagResult.bank->leakage * 1e3 << "mW" << endl;
        if (cell->memCellType == eDRAM || cell->memCellType == gcDRAM) {
            outFile << " - Cache Refresh Power = "
                    << TO_WATT(bank->refreshDynamicEnergy / (cell->retentionTime))
                    << " per bank" << endl;
        }

        // Print details if needed
        if (inputParameter->printLevel > 0) {
            outFile << endl << "CACHE DATA ARRAY DETAILS";
            // Instead of 'print(4)', you might have to implement a similar method that takes
            // an output stream reference if it prints. Or you can adapt it similarly
            // to this approach. For demonstration, it's shown as is:
            printToFile(4, FileName); // You would need to modify 'print' to accept an ofstream.
            
            //outFile << endl << "CACHE TAG ARRAY DETAILS";
            //tagResult.printToFile(4, FileName); // Similarly, modify to accept an ofstream.
        }
    }

    // Close the file after writing
    outFile.close();
}

void Result::printToCsvFile(ofstream &outputFile) {
	cout << "in print output csv loop" << endl;
/*
	outputFile << bank->readDynamicEnergy * 1e12 << ",";
	outputFile << (bank->readDynamicEnergy - bank->mat.readDynamicEnergy
					* bank->numActiveMatPerColumn * bank->numActiveMatPerRow)
					* 1e12 << ",";
	outputFile << bank->mat.readDynamicEnergy * 1e12 << ",";
	outputFile << (bank->mat.readDynamicEnergy - bank->mat.subarray.readDynamicEnergy
					* bank->numActiveSubarrayPerRow * bank->numActiveSubarrayPerColumn)
					* 1e12 << ",";
	outputFile << bank->mat.subarray.readDynamicEnergy * 1e12 << ",";
	outputFile << bank->mat.subarray.rowDecoder.readDynamicEnergy * 1e12 << ",";
	outputFile << (bank->mat.subarray.bitlineMuxDecoder.readDynamicEnergy
					+ bank->mat.subarray.senseAmpMuxLev1Decoder.readDynamicEnergy
					+ bank->mat.subarray.senseAmpMuxLev2Decoder.readDynamicEnergy) * 1e12 << ",";
	outputFile << bank->mat.subarray.senseAmp.readDynamicEnergy * 1e12 << ",";
	outputFile << (bank->mat.subarray.bitlineMux.readDynamicEnergy
					+ bank->mat.subarray.senseAmpMuxLev1.readDynamicEnergy
					+ bank->mat.subarray.senseAmpMuxLev2.readDynamicEnergy) * 1e12 << ",";
	outputFile << bank->mat.subarray.precharger.readDynamicEnergy * 1e12 << ",";
	outputFile << bank->writeDynamicEnergy * 1e12 << ",";
	outputFile << (bank->writeDynamicEnergy - bank->mat.writeDynamicEnergy
					* bank->numActiveMatPerColumn * bank->numActiveMatPerRow)
					* 1e12 << ",";
	outputFile << bank->mat.writeDynamicEnergy * 1e12 << ",";
	outputFile << (bank->mat.writeDynamicEnergy - bank->mat.subarray.writeDynamicEnergy
					* bank->numActiveSubarrayPerRow * bank->numActiveSubarrayPerColumn)
					* 1e12 << ",";
	outputFile << bank->mat.subarray.writeDynamicEnergy * 1e12 << ",";
	outputFile << bank->mat.subarray.rowDecoder.writeDynamicEnergy * 1e12 << ",";
	outputFile << (bank->mat.subarray.bitlineMuxDecoder.writeDynamicEnergy
					+ bank->mat.subarray.senseAmpMuxLev1Decoder.writeDynamicEnergy
					+ bank->mat.subarray.senseAmpMuxLev2Decoder.writeDynamicEnergy) * 1e12 << ",";
	outputFile << (bank->mat.subarray.bitlineMux.writeDynamicEnergy
					+ bank->mat.subarray.senseAmpMuxLev1.writeDynamicEnergy
					+ bank->mat.subarray.senseAmpMuxLev2.writeDynamicEnergy) * 1e12 << endl;
*/
	outputFile << bank->numRowMat << "," << bank->numColumnMat << "," << bank->stackedDieCount << "," << bank->numActiveMatPerColumn << "," << bank->numActiveMatPerRow << ",";
	outputFile << bank->numRowSubarray << "," << bank->numColumnSubarray << "," << bank->numActiveSubarrayPerColumn << "," << bank->numActiveSubarrayPerRow << ",";
	outputFile << bank->mat.subarray.numRow << "," << bank->mat.subarray.numColumn << ",";
	outputFile << bank->muxSenseAmp << "," << bank->muxOutputLev1 << "," << bank->muxOutputLev2 << ",";
	if (inputParameter->designTarget == cache)
		outputFile << bank->numRowPerSet << ",";
	else
		outputFile << "N/A,";
	switch (localWire->wireType) {
	case local_aggressive:
		outputFile << "Local Aggressive" << ",";
		break;
	case local_conservative:
		outputFile << "Local Conservative" << ",";
		break;
	case semi_aggressive:
		outputFile << "Semi-Global Aggressive" << ",";
		break;
	case semi_conservative:
		outputFile << "Semi-Global Conservative" << ",";
		break;
	case global_aggressive:
		outputFile << "Global Aggressive" << ",";
		break;
	case global_conservative:
		outputFile << "Global Conservative" << ",";
		break;
	default:
		outputFile << "DRAM Wire" << ",";
	}
	switch (localWire->wireRepeaterType) {
	case repeated_none:
		outputFile << "No Repeaters" << ",";
		break;
	case repeated_opt:
		outputFile << "Fully-Optimized Repeaters" << ",";
		break;
	case repeated_5:
		outputFile << "Repeaters with 5% Overhead" << ",";
		break;
	case repeated_10:
		outputFile << "Repeaters with 10% Overhead" << ",";
		break;
	case repeated_20:
		outputFile << "Repeaters with 20% Overhead" << ",";
		break;
	case repeated_30:
		outputFile << "Repeaters with 30% Overhead" << ",";
		break;
	case repeated_40:
		outputFile << "Repeaters with 40% Overhead" << ",";
		break;
	case repeated_50:
		outputFile << "Repeaters with 50% Overhead" << ",";
		break;
	default:
		outputFile << "N/A" << ",";
	}
	if (localWire->isLowSwing)
		outputFile << "Yes" << ",";
	else
		outputFile << "No" << ",";
	switch (globalWire->wireType) {
	case local_aggressive:
		outputFile << "Local Aggressive" << ",";
		break;
	case local_conservative:
		outputFile << "Local Conservative" << ",";
		break;
	case semi_aggressive:
		outputFile << "Semi-Global Aggressive" << ",";
		break;
	case semi_conservative:
		outputFile << "Semi-Global Conservative" << ",";
		break;
	case global_aggressive:
		outputFile << "Global Aggressive" << ",";
		break;
	case global_conservative:
		outputFile << "Global Conservative" << ",";
		break;
	default:
		outputFile << "DRAM Wire" << ",";
	}
	switch (globalWire->wireRepeaterType) {
	case repeated_none:
		outputFile << "No Repeaters" << ",";
		break;
	case repeated_opt:
		outputFile << "Fully-Optimized Repeaters" << ",";
		break;
	case repeated_5:
		outputFile << "Repeaters with 5% Overhead" << ",";
		break;
	case repeated_10:
		outputFile << "Repeaters with 10% Overhead" << ",";
		break;
	case repeated_20:
		outputFile << "Repeaters with 20% Overhead" << ",";
		break;
	case repeated_30:
		outputFile << "Repeaters with 30% Overhead" << ",";
		break;
	case repeated_40:
		outputFile << "Repeaters with 40% Overhead" << ",";
		break;
	case repeated_50:
		outputFile << "Repeaters with 50% Overhead" << ",";
		break;
	default:
		outputFile << "N/A" << ",";
	}
	if (globalWire->isLowSwing)
		outputFile << "Yes" << ",";
	else
		outputFile << "No" << ",";
	switch (bank->areaOptimizationLevel) {
	case latency_first:
		outputFile << "Latency-Optimized" << ",";
		break;
	case area_first:
		outputFile << "Area-Optimized" << ",";
		break;
	default:	/* balance */
		outputFile << "Balanced" << ",";
	}
	outputFile << bank->height * 1e6 << "," << bank->width * 1e6 << "," << bank->area * 1e6 << ",";
	outputFile << bank->mat.height * 1e6 << "," << bank->mat.width * 1e6 << "," << bank->mat.area * 1e6 << ",";
	outputFile << bank->mat.subarray.height * 1e6 << "," << bank->mat.subarray.width * 1e6 << "," << bank->mat.subarray.area * 1e6 << ",";
	outputFile << cell->area * tech->featureSize * tech->featureSize * bank->capacity / bank->area * 100 << ",";
	outputFile << bank->readLatency * 1e9 << "," << bank->writeLatency * 1e9 << ",";
    if (cell->memCellType == eDRAM || cell->memCellType == gcDRAM) {
        outputFile << bank->refreshLatency * 1e9 << ",";
    } else {
        outputFile << "0,";
    }
	outputFile << bank->readDynamicEnergy * 1e12 << "," << bank->writeDynamicEnergy * 1e12 << ",";
    if (cell->memCellType == eDRAM || cell->memCellType == gcDRAM) {
        outputFile << bank->refreshDynamicEnergy * 1e12 << ",";
    } else {
        outputFile << "0,";
    }
	outputFile << bank->leakage * 1e3 << ",";
    if (cell->memCellType == eDRAM || cell->memCellType == gcDRAM) {
        outputFile << TO_WATT(bank->refreshDynamicEnergy / (cell->retentionTime)) << ",";
    } else {
        outputFile << "0,";
    }

	if (cell->memCellType == gcDRAM) {
        outputFile << bank->mat.subarray.stackedMemTiers << ",";
    } else {
        outputFile << "0,";
    }

	outputFile << "\n";
}

void Result::printAsCacheToCsvFile(Result &tagResult, CacheAccessMode cacheAccessMode, ofstream &outputFile) {
	if (bank->memoryType != data || tagResult.bank->memoryType != tag) {
		cout << "This is not a valid cache configuration." << endl;
		return;
	} else {
		double cacheHitLatency, cacheMissLatency, cacheWriteLatency;
		double cacheHitDynamicEnergy, cacheMissDynamicEnergy, cacheWriteDynamicEnergy;
		double cacheLeakage;
		double cacheArea;
		if (cacheAccessMode == normal_access_mode) {
			/* Calculate latencies */
			cacheMissLatency = tagResult.bank->readLatency;		/* only the tag access latency */
			cacheHitLatency = MAX(tagResult.bank->readLatency, bank->mat.readLatency);	/* access tag and activate data row in parallel */
			cacheHitLatency += bank->mat.subarray.columnDecoderLatency;		/* add column decoder latency after hit signal arrives */
			cacheHitLatency += bank->readLatency - bank->mat.readLatency;	/* H-tree in and out latency */
			cacheWriteLatency = MAX(tagResult.bank->writeLatency, bank->writeLatency);	/* Data and tag are written in parallel */
			/* Calculate power */
			cacheMissDynamicEnergy = tagResult.bank->readDynamicEnergy;	/* no matter what tag is always accessed */
			cacheMissDynamicEnergy += bank->readDynamicEnergy;	/* data is also partially accessed, TO-DO: not accurate here */
			cacheHitDynamicEnergy = tagResult.bank->readDynamicEnergy + bank->readDynamicEnergy;
			cacheWriteDynamicEnergy = tagResult.bank->writeDynamicEnergy + bank->writeDynamicEnergy;
		} else if (cacheAccessMode == fast_access_mode) {
			/* Calculate latencies */
			cacheMissLatency = tagResult.bank->readLatency;
			cacheHitLatency = MAX(tagResult.bank->readLatency, bank->readLatency);
			cacheWriteLatency = MAX(tagResult.bank->writeLatency, bank->writeLatency);
			/* Calculate power */
			cacheMissDynamicEnergy = tagResult.bank->readDynamicEnergy;	/* no matter what tag is always accessed */
			cacheMissDynamicEnergy += bank->readDynamicEnergy;	/* data is also partially accessed, TO-DO: not accurate here */
			cacheHitDynamicEnergy = tagResult.bank->readDynamicEnergy + bank->readDynamicEnergy;
			cacheWriteDynamicEnergy = tagResult.bank->writeDynamicEnergy + bank->writeDynamicEnergy;
		} else {		/* sequential access */
			/* Calculate latencies */
			cacheMissLatency = tagResult.bank->readLatency;
			cacheHitLatency = tagResult.bank->readLatency + bank->readLatency;
			cacheWriteLatency = MAX(tagResult.bank->writeLatency, bank->writeLatency);
			/* Calculate power */
			cacheMissDynamicEnergy = tagResult.bank->readDynamicEnergy;	/* no matter what tag is always accessed */
			cacheHitDynamicEnergy = tagResult.bank->readDynamicEnergy + bank->readDynamicEnergy;
			cacheWriteDynamicEnergy = tagResult.bank->writeDynamicEnergy + bank->writeDynamicEnergy;
		}
		/* Calculate leakage */
		cacheLeakage = tagResult.bank->leakage + bank->leakage;
		/* Calculate area */
		cacheArea = tagResult.bank->area + bank->area;	/* TO-DO: simply add them together here */

		/* start printing */
		switch (cacheAccessMode) {
		case normal_access_mode:
			outputFile << "Normal" << ",";
			break;
		case fast_access_mode:
			outputFile << "Fast" << ",";
			break;
		default:	/* sequential */
			outputFile << "Sequential" << ",";
		}
		outputFile << cacheArea * 1e6 << ",";
		outputFile << cacheHitLatency * 1e9 << ",";
		outputFile << cacheMissLatency * 1e9 << ",";
		outputFile << cacheWriteLatency * 1e9 << ",";
        if (cell->memCellType == eDRAM || cell->memCellType == gcDRAM) {
            outputFile << MAX(tagResult.bank->refreshLatency, bank->refreshLatency) * 1e9 << ",";
        } else {
            outputFile << "0,";
        }
		outputFile << cacheHitDynamicEnergy * 1e9 << ",";
		outputFile << cacheMissDynamicEnergy * 1e9 << ",";
		outputFile << cacheWriteDynamicEnergy * 1e9 << ",";
        if (cell->memCellType == eDRAM || cell->memCellType == gcDRAM) {
            outputFile << (tagResult.bank->refreshDynamicEnergy + bank->refreshDynamicEnergy) * 1e9 << ",";
        } else {
            outputFile << "0,";
        }
		outputFile << cacheLeakage * 1e3 << ",";
        if (cell->memCellType == eDRAM || cell->memCellType == gcDRAM) {
            outputFile << TO_WATT(bank->refreshDynamicEnergy / (cell->retentionTime)) << ",";
        } else {
            outputFile << "0,";
        }
		printToCsvFile(outputFile);
		tagResult.printToCsvFile(outputFile);
        outputFile << bank->mat.subarray.leakage + tagResult.bank->mat.subarray.leakage << ",";
        outputFile << (bank->mat.subarray.area + tagResult.bank->mat.subarray.area) * 1e6 << ",";
		outputFile << endl;
	}
}
