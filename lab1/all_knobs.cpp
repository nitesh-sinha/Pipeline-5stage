/**********************************************************************************************
 * File         : all_knob.cc
 * Description  : knob template 
 * This file comes from MacSim Simulator 
 * All knobs have to be added in this file 
 *********************************************************************************************/


#include "all_knobs.h"

#include <string>

all_knobs_c::all_knobs_c() {
	KNOB_MAX_SIM_COUNT = new KnobTemplate< unsigned long long > ("max_sim_count", 0);
	KNOB_MAX_INST_COUNT = new KnobTemplate< unsigned long long > ("max_inst_count", 10);
	KNOB_OUTPUT_FILE = new KnobTemplate< string > ("output_file", "sim.out");
	KNOB_PRINT_INST = new KnobTemplate< bool > ("print_inst", 1);
	KNOB_PERFECT_ICACHE = new KnobTemplate< bool > ("perfect_icache", 1);
	KNOB_PERFECT_DCACHE = new KnobTemplate< bool > ("perfect_dcache", 1);
	KNOB_PERFECT_BR = new KnobTemplate< bool > ("perfect_br", 1);
	KNOB_PERFECT_L2 = new KnobTemplate< bool > ("perfect_l2", 1);
	KNOB_READ_TRACE = new KnobTemplate< bool > ("read_trace", 1);
	KNOB_TRACE_FILE = new KnobTemplate< string > ("trace_file", "trace.pzip");
	KNOB_ISSUE_WIDTH = new KnobTemplate< unsigned > ("issue_width", 1);
	KNOB_ICACHE_LATENCY = new KnobTemplate< unsigned > ("icache_latency", 1);
	KNOB_DCACHE_LATENCY = new KnobTemplate< unsigned > ("dcache_latency", 1);
	KNOB_MEM_LATENCY = new KnobTemplate< unsigned > ("mem_latency", 1);
	KNOB_L2CACHE_LATENCY = new KnobTemplate< unsigned > ("l2cache_latency", 1);
	KNOB_PRINT_PIPE_FREQ = new KnobTemplate< unsigned long long > ("print_pipe_freq", 1);
}

all_knobs_c::~all_knobs_c() {
	delete KNOB_MAX_SIM_COUNT;
	delete KNOB_MAX_INST_COUNT;
	delete KNOB_OUTPUT_FILE;
	delete KNOB_PRINT_INST;
	delete KNOB_PERFECT_ICACHE;
	delete KNOB_PERFECT_DCACHE;
	delete KNOB_PERFECT_BR;
	delete KNOB_PERFECT_L2;
	delete KNOB_READ_TRACE;
	delete KNOB_TRACE_FILE;
	delete KNOB_ISSUE_WIDTH;
	delete KNOB_ICACHE_LATENCY;
	delete KNOB_DCACHE_LATENCY;
	delete KNOB_MEM_LATENCY;
	delete KNOB_L2CACHE_LATENCY;
	delete KNOB_PRINT_PIPE_FREQ;
}

void all_knobs_c::registerKnobs(KnobsContainer *container) {
	container->insertKnob( KNOB_MAX_SIM_COUNT );
	container->insertKnob( KNOB_MAX_INST_COUNT );
	container->insertKnob( KNOB_OUTPUT_FILE );
	container->insertKnob( KNOB_PRINT_INST );
	container->insertKnob( KNOB_PERFECT_ICACHE );
	container->insertKnob( KNOB_PERFECT_DCACHE );
	container->insertKnob( KNOB_PERFECT_BR );
	container->insertKnob( KNOB_PERFECT_L2 );
	container->insertKnob( KNOB_READ_TRACE );
	container->insertKnob( KNOB_TRACE_FILE );
	container->insertKnob( KNOB_ISSUE_WIDTH );
	container->insertKnob( KNOB_ICACHE_LATENCY );
	container->insertKnob( KNOB_DCACHE_LATENCY );
	container->insertKnob( KNOB_MEM_LATENCY );
	container->insertKnob( KNOB_L2CACHE_LATENCY );
	container->insertKnob( KNOB_PRINT_PIPE_FREQ );
}

void all_knobs_c::display() {
	KNOB_MAX_SIM_COUNT->display(cout); cout << endl;
	KNOB_MAX_INST_COUNT->display(cout); cout << endl;
	KNOB_OUTPUT_FILE->display(cout); cout << endl;
	KNOB_PRINT_INST->display(cout); cout << endl;
	KNOB_PERFECT_ICACHE->display(cout); cout << endl;
	KNOB_PERFECT_DCACHE->display(cout); cout << endl;
	KNOB_PERFECT_BR->display(cout); cout << endl;
	KNOB_PERFECT_L2->display(cout); cout << endl;
	KNOB_READ_TRACE->display(cout); cout << endl;
	KNOB_TRACE_FILE->display(cout); cout << endl;
	KNOB_ISSUE_WIDTH->display(cout); cout << endl;
	KNOB_ICACHE_LATENCY->display(cout); cout << endl;
	KNOB_DCACHE_LATENCY->display(cout); cout << endl;
	KNOB_MEM_LATENCY->display(cout); cout << endl;
	KNOB_L2CACHE_LATENCY->display(cout); cout << endl;
	KNOB_PRINT_PIPE_FREQ->display(cout); cout << endl;
}

