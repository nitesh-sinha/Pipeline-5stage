#include "common.h"
#include "sim.h"
#include "trace.h" 
#include <stdlib.h>
/*******************************************************************/
/* Simulator frame */ 
/*******************************************************************/

bool run_a_cycle();
void init_structures(); 



/* uop_pool related variables */ 

uint32_t free_op_num;
uint32_t active_op_num; 
Op *op_pool; 
Op *op_pool_free_head = NULL; 

/* simulator related local functions */ 

bool icache_access(uint32_t addr);
bool dcache_access(uint32_t addr);
void  init_latches(void);

#include "knob.h"
#include "all_knobs.h"

// knob variables
KnobsContainer *g_knobsContainer; /* < knob container > */
all_knobs_c    *g_knobs; /* < all knob variables > */

gzFile g_stream;

void init_knobs(int argc, char** argv)
{
  // Create the knob managing class
  g_knobsContainer = new KnobsContainer();

  // Get a reference to the actual knobs for this component instance
  g_knobs = g_knobsContainer->getAllKnobs();

  // apply the supplied command line switches
  char* pInvalidArgument = NULL;
  g_knobsContainer->applyComandLineArguments(argc, argv, &pInvalidArgument);

  g_knobs->display();
}

void read_trace_file(void)
{
  g_stream = gzopen((KNOB(KNOB_TRACE_FILE)->getValue()).c_str(), "r");
}

// simulator main function is called from outside of this file 

void simulator_main(int argc, char** argv) 
{
  init_knobs(argc, argv);

  // trace driven simulation 
  read_trace_file();
  init_structures();
  run_a_cycle();

}
int op_latency[NUM_OP_TYPE]; 

void init_op_latency(void)
{
  op_latency[OP_INV]   = 1; 
  op_latency[OP_NOP]   = 1; 
  op_latency[OP_CF]    = 1; 
  op_latency[OP_CMOV]  = 1; 
  op_latency[OP_LDA]   = 1;
  op_latency[OP_LD]    = 1; 
  op_latency[OP_ST]    = 1; 
  op_latency[OP_IADD]  = 1; 
  op_latency[OP_IMUL]  = 2; 
  op_latency[OP_IDIV]  = 4; 
  op_latency[OP_ICMP]  = 2; 
  op_latency[OP_LOGIC] = 1; 
  op_latency[OP_SHIFT] = 2; 
  op_latency[OP_BYTE]  = 1; 
  op_latency[OP_MM]    = 2; 
  op_latency[OP_FMEM]  = 2; 
  op_latency[OP_FCF]   = 1; 
  op_latency[OP_FCVT]  = 4; 
  op_latency[OP_FADD]  = 2; 
  op_latency[OP_FMUL]  = 4; 
  op_latency[OP_FDIV]  = 16; 
  op_latency[OP_FCMP]  = 2; 
  op_latency[OP_FBIT]  = 2; 
  op_latency[OP_FCMP]  = 2; 
}

void init_op(Op *op)
{
  op->num_src               = 0; 
  op->src[0]                = -1; 
  op->src[1]                = -1;
  op->dst                   = -1; 
  op->opcode                = 0; 
  op->is_fp                 = false;
  op->cf_type               = NOT_CF;
  op->mem_type              = NOT_MEM;
  op->write_flag             = 0;
  op->inst_size             = 0;
  op->ld_vaddr              = 0;
  op->st_vaddr              = 0;
  op->instruction_addr      = 0;
  op->branch_target         = 0;
  op->actually_taken        = 0;
  op->mem_read_size         = 0;
  op->mem_write_size        = 0;
  op->valid                 = FALSE; 
  op->last_op		    = FALSE; // initialize last_op to FALSE
  /* you might add more features here */ 
}


void init_op_pool(void)
{
  /* initialize op pool */ 
  op_pool = new Op [1024];
  free_op_num = 1024; 
  active_op_num = 0; 
  uint32_t op_pool_entries = 0; 
  int ii;
  for (ii = 0; ii < 1023; ii++) {

    op_pool[ii].op_pool_next = &op_pool[ii+1]; 
    op_pool[ii].op_pool_id   = op_pool_entries++; 
    init_op(&op_pool[ii]); 
  }
  op_pool[ii].op_pool_next = op_pool_free_head; 
  op_pool[ii].op_pool_id   = op_pool_entries++;
  init_op(&op_pool[ii]); 
  op_pool_free_head = &op_pool[0]; 
}





Op *get_free_op(void)
{
  /* return a free op from op pool */ 

  if (op_pool_free_head == NULL || (free_op_num == 1)) {
    std::cout <<"ERROR! OP_POOL SIZE is too small!! " << endl; 
    std::cout <<"please check free_op function " << endl; 
    assert(1); 
    exit(1);
  }

  free_op_num--;
  assert(free_op_num); 

  Op *new_op = op_pool_free_head; 
  op_pool_free_head = new_op->op_pool_next; 
  assert(!new_op->valid); 
  init_op(new_op);
  active_op_num++; 
  return new_op; 
}

void free_op(Op *op)
{
  free_op_num++;
  active_op_num--; 
  op->valid = FALSE; 
  op->op_pool_next = op_pool_free_head;
  op_pool_free_head = op; 
}



/*******************************************************************/
/*  Data structure */
/*******************************************************************/

typedef struct pipeline_latch_struct {
  Op *op; /* you must update this data structure. */
  bool op_valid; // it tells a stage in the pipeline whether there is a valid instruction to execute in that cycle
   /* you might add more data structures. But you should complete the above data elements */ 
}pipeline_latch; 


typedef struct Reg_element_struct{
  bool valid; // 8 bit register
  // data is not needed 
  /* you might add more data structures. But you should complete the above data elements */ 
}REG_element; 

REG_element register_file[NUM_REG]; //NUM_REG is 32

// To initialize the 32 registers in the processor

void init_registers()
{
  for(int i=0;i<NUM_REG;i++)
    register_file[i].valid=TRUE;
}


/*******************************************************************/
/* These are the functions you'll have to write.  */ 
/*******************************************************************/

void FE_stage();
void ID_stage();
void EX_stage(); 
void MEM_stage();
void WB_stage(); 

/*******************************************************************/
/*  These are the variables you'll have to write.  */ 
/*******************************************************************/

bool sim_end_condition = FALSE;     /* please complete the condition. */ 
UINT64 retired_instruction = 0;    /* number of retired instruction. (only correct instructions) */ 
UINT64 cycle_count = 0;            /* total number of cycles */ 
UINT64 data_hazard_count = 0;  
UINT64 control_hazard_count = 0; 
UINT64 icache_miss_count = 0;      /* total number of icache misses. for Lab #2 and Lab #3 */ 
UINT64 dcache_miss_count = 0;      /* total number of dcache  misses. for Lab #2 and Lab #3 */ 
UINT64 l2_cache_miss_count = 0;    /* total number of L2 cache  misses. for Lab #2 and Lab #3 */  



pipeline_latch *MEM_latch;  
pipeline_latch *EX_latch;
pipeline_latch *ID_latch;
pipeline_latch *FE_latch;
UINT64 ld_st_buffer[LD_ST_BUFFER_SIZE]; 
UINT64 next_pc; 

/*******************************************************************/
/*  Print messages  */
/*******************************************************************/

void print_stats() {
  std::ofstream out((KNOB(KNOB_OUTPUT_FILE)->getValue()).c_str());
  /* Do not modify this function. This messages will be used for grading */ 
  out << "Total instruction: " << retired_instruction << endl; 
  out << "Total cycles: " << cycle_count << endl; 
  float ipc = (cycle_count ? ((float)retired_instruction/(float)cycle_count): 0 );
  out << "IPC: " << ipc << endl; 
  out << "Total I-cache miss: " << icache_miss_count << endl; 
  out << "Total D-cache miss: " << dcache_miss_count << endl; 
  out << "Total L2-cache miss: " << l2_cache_miss_count << endl; 
  out << "Total data hazard: " << data_hazard_count << endl;
  out << "Total control hazard : " << control_hazard_count << endl; 
  out.close();
}

/*******************************************************************/
/*  Support Functions  */ 
/*******************************************************************/

bool get_op(Op *op)
{
  static UINT64 unique_count = 0; 
  Trace_op trace_op; 
  bool success = FALSE; 
  // read trace 
  // fill out op info 
  // return FALSE if the end of trace 
  success = (gzread(g_stream, &trace_op, sizeof(Trace_op)) >0 ); 
  if (KNOB(KNOB_PRINT_INST)->getValue()) dprint_trace(&trace_op); 

  /* copy trace structure to op */ 
  if (success) { 
    copy_trace_op(&trace_op, op); 

    op->inst_id  = unique_count++;
    op->valid    = TRUE; 
  }
  return success; 
}
/* return op execution cycle latency */ 

int get_op_latency(Op *op) 
{
  assert (op->opcode < NUM_OP_TYPE); 
  return op_latency[op->opcode];
}

/* Print out all the register values */ 
void dump_reg() {
  for (int ii = 0; ii < NUM_REG; ii++) {
    std::cout << cycle_count << ":register[" << ii  << "]: V:" << register_file[ii].valid << endl; 
  }
}

void print_pipeline() {
  std::cout << "--------------------------------------------" << endl; 
  std::cout <<"cycle count : " << dec << cycle_count << " retired_instruction : " << retired_instruction << endl; 
  std::cout << (int)cycle_count << " FE: " ;
  if (FE_latch->op_valid) {
    Op *op = FE_latch->op; 
    cout << (int)op->inst_id ;
  }
  else {
    cout <<"####";
  }
  std::cout << " ID: " ;
  if (ID_latch->op_valid) {
    Op *op = ID_latch->op; 
    cout << (int)op->inst_id ;
  }
  else {
    cout <<"####";
  }
  std::cout << " EX: " ;
  if (EX_latch->op_valid) {
    Op *op = EX_latch->op; 
    cout << (int)op->inst_id ;
  }
  else {
    cout <<"####";
  }


  std::cout << " MEM: " ;
  if (MEM_latch->op_valid) {
    Op *op = MEM_latch->op; 
    cout << (int)op->inst_id ;
  }
  else {
    cout <<"####";
  }
  cout << endl; 
  //  dump_reg();   
  std::cout << "--------------------------------------------" << endl; 
}

void print_heartbeat()
{
  static uint64_t last_cycle ;
  static uint64_t last_inst_count; 
  float temp_ipc = float(retired_instruction - last_inst_count) /(float)(cycle_count-last_cycle) ;
  float ipc = float(retired_instruction) /(float)(cycle_count) ;
  /* Do not modify this function. This messages will be used for grading */ 
  cout <<"**Heartbeat** cycle_count: " << cycle_count << " inst:" << retired_instruction << " IPC: " << temp_ipc << " Overall IPC: " << ipc << endl; 
  last_cycle = cycle_count;
  last_inst_count = retired_instruction; 
}
/*******************************************************************/
/*                                                                 */
/*******************************************************************/

bool run_a_cycle(){

  for (;;) { 
    if (((KNOB(KNOB_MAX_SIM_COUNT)->getValue() && (cycle_count >= KNOB(KNOB_MAX_SIM_COUNT)->getValue())) || 
      (KNOB(KNOB_MAX_INST_COUNT)->getValue() && (retired_instruction >= KNOB(KNOB_MAX_INST_COUNT)->getValue())) ||  (sim_end_condition))) { 
        // please complete sim_end_condition 
        print_heartbeat(); 
        print_stats();
        return TRUE; 
    }
    cycle_count++; 
    if (!(cycle_count%5000)) { //print_heartbeat every 5000 cycles
      print_heartbeat(); 
    }
    WB_stage(); 
    MEM_stage();
    EX_stage(); 
    ID_stage(); 
    FE_stage(); 
    if (KNOB(KNOB_PRINT_PIPE_FREQ)->getValue() && !(cycle_count%KNOB(KNOB_PRINT_PIPE_FREQ)->getValue())) print_pipeline();
  }
  return TRUE; 
}


/*******************************************************************/
/* Complete the following functions.  */
/* You can add new data structures and also new elements to Op, Pipeline_latch data structure */ 
/*******************************************************************/

void init_structures(void)
{
  init_op_pool(); 
  init_op_latency();
  init_latches();
  init_registers(); // initializing processor registers to TRUE since no data dependency when processor is powered on
}


int latency=0; // to keep track of instrn latency

bool cd_stall=FALSE; // stall due to control dependency
bool dd_stall=FALSE; // stall due to data dependency
bool ex_stall=FALSE; // stall due to execution cycles


void WB_stage()
{    
  if(MEM_latch->op_valid) // if instrn in MEM_latch has to be executed in current cycle
  { 
    Op *wb_op;
    wb_op=MEM_latch->op; // copying MEM_latch to op structure
    if(wb_op->last_op==TRUE) // taking care of the sim_end_condition for the last instrn
      sim_end_condition=TRUE;

    if(wb_op->cf_type>=CF_BR) // Branch instrn at WB stage removes control dependency from all preceeding stages
	cd_stall=FALSE;
 
    if((int)wb_op->dst==-1) // instruction doesn't write to register
     {
      retired_instruction++;
      free_op(wb_op);
     }	
    else
     {  
      register_file[(int)wb_op->dst].valid=TRUE; // to remove register(data) dependency at ID stage
      dd_stall=FALSE;  
      retired_instruction++;
      free_op(wb_op);
     }
    
   }
}

void MEM_stage()
{  
   
   if(EX_latch->op_valid) // if instrn in EX_latch can be executed in current cycle
   {
     Op *mem_op;

     mem_op=EX_latch->op; // copying EX_latch to mem_op structure
     MEM_latch->op=mem_op; // move instruction to MEM_latch
     MEM_latch->op_valid=TRUE;
	 
   }
   else
     MEM_latch->op_valid=FALSE; // since MEM_latch was not updated in this cycle, set its valid bit to false 
}


void EX_stage()
{
   /* The very first execution(for any instrn) in EX_stage would always match the 2nd condition.
      If latency>1 for an instrn, then 1st condition is matched in subsequent cycles in EX stage(so decrement latency)
      Instrn moves to EX latch when latency becomes 0 */
    

   if(ex_stall==TRUE)
     {
      Op *ex_op=ID_latch->op; // read from ID_latch
      latency--;
      if(latency==0)
       {
	ex_stall=FALSE;
	EX_latch->op=ex_op; 
	EX_latch->op_valid=TRUE;
       }
       return;
     }
    
   if(ex_stall==FALSE && ID_latch->op_valid==TRUE)
    {
      Op *ex_op=ID_latch->op; // read from ID_latch
      latency=get_op_latency(ex_op);
      
      if(latency>1)
	{
 	 ex_stall=TRUE;
	 latency--;
	 EX_latch->op_valid=FALSE;
	 return;
	}
       else // single cycle instruction, so pass it to EX latch
	{
	 EX_latch->op=ex_op; 
	 EX_latch->op_valid=TRUE;
	}
       return;
     }

    if(ID_latch->op_valid==FALSE) // If ID latch instrn is invalid, then EX stage does not do any processing
     {
      EX_latch->op_valid=FALSE;
      return;
     }
}


// flags to check if the current instruction in ID stage has control or data dependency
bool cd=FALSE;
bool dd=FALSE; 

  
void ID_stage()
{  
  /* If there is an ex_stall or dd_stall in the current cycle, ID stage should not do any work. 
     If none of those 2 stalls are in place, then enter ID stage only if FE latch's valid flag is TRUE. */

  if(ex_stall) 
   {
    ID_latch->op_valid=FALSE;
    return;
   }

  if(dd_stall) 
   { 
    data_hazard_count++;
    ID_latch->op_valid=FALSE;
    return;
   }
    
    if(FE_latch->op_valid==TRUE)
    {

     Op *id_op;
     id_op=FE_latch->op;


   // check for control dependency

   if(id_op->cf_type >= CF_BR)
      cd=TRUE;  
   else
      cd=FALSE;     
    
  
  // check for data dependency
  int src0_reg=(int)id_op->src[0];
  int src1_reg=(int)id_op->src[1];
  
  int dst_reg=(int)id_op->dst; 
  
  

  if(src0_reg==-1 && src1_reg!=-1)  //instruction has src1 register only
   {
     if(register_file[src1_reg].valid==FALSE) // src1 register is not ready 
     dd=TRUE;
     else
     dd=FALSE;
   }

   

   if(src1_reg==-1 && src0_reg!=-1)  //instruction has src0 register only
   { 
     if(register_file[src0_reg].valid==FALSE) // src0 register is not ready 
      dd=TRUE;
     else
      dd=FALSE;
     
   }


   if(src1_reg!=-1 && src0_reg!=-1) ////instruction has both src0 and src1 registers
   {
     if(register_file[src0_reg].valid==FALSE || register_file[src1_reg].valid==FALSE) // either register is not ready 
     dd=TRUE;
     else
     dd=FALSE;
   }
  
  

   
   if(src1_reg==-1 && src0_reg==-1)  // if there are no src registers, then no data dependency
      dd=FALSE;

    
    /* if CD and DD => stall FE for current cycle. Dont pass current instruction to ID latch. CH++ for every cycle. DH++
    if CD but no DD=> stall FE for current cycle. Pass current instruction to ID latch. CH++ for just one cycle. 
    if no CD but DD=> stall FE. DH++. dont pass current instruction to ID latch
    if no CD and no DD=> no stalls. pass the current instruction to ID latch     */

    if(cd==TRUE && dd==TRUE)
     { 
       cd_stall=TRUE;
       dd_stall=TRUE;
       control_hazard_count++;
       data_hazard_count++;
       ID_latch->op_valid=FALSE;
       return;
     }

    if(cd==TRUE && dd==FALSE)
     { 
       dd_stall=FALSE;
       cd_stall=TRUE;
       control_hazard_count++;
       ID_latch->op=id_op;
       ID_latch->op_valid=TRUE;
       if(dst_reg!=-1) // instruction writes to destination register, so change valid flag of dst to FALSE
         register_file[dst_reg].valid=FALSE;
       return;
     }

    if(cd==FALSE && dd==TRUE)
     { 
       cd_stall=FALSE;
       dd_stall=TRUE;
       data_hazard_count++;
       ID_latch->op_valid=FALSE;
       return;
     }
    
    if(cd==FALSE && dd==FALSE)
     { 
       cd_stall=FALSE;
       dd_stall=FALSE;
       ID_latch->op=id_op;
       ID_latch->op_valid=TRUE;
       if(dst_reg!=-1) // instruction writes to destination register, so change valid flag to FALSE
         register_file[dst_reg].valid=FALSE;
       return;
     }
   
  }
  else
   {
    ID_latch->op_valid=FALSE;
    //cd_stall=TRUE; // for simulating infinite loop
    return;
   }
    
}
 


int fe_ctr=0; // to detect infinte loops in program

void FE_stage()
{
  /* FE latch valid flag has to be FALSE only when there is a cd_stall AND not other stalls. 
     Because only in a cd_stall, ID stage has to continue processing the instructions and move them
     to the next latch. Whereas when there are other stalls(ex_stall, dd_stall etc.) alongwith cd_stall, 
     ID stage is also stalled. */
   fe_ctr++;
   if(fe_ctr>1000) // If simulator waits for an op for > 1000 cycles, terminate the program
    {
     //std::cout<<"****************Detected a loop. Terminating the simulator***************"<<endl;
     exit(1);
    }

   if(cd_stall==TRUE)
    {
     if(dd_stall==TRUE) 
       FE_latch->op_valid=TRUE;
     else
       FE_latch->op_valid=FALSE; 
    return;
    }	
  
  /* After EX or DD stall is removed, ID should continue working in that cycle and 
     so the FE_latch bit has to be TRUE for that to happen */
  
   if(ex_stall==TRUE || dd_stall==TRUE)
    {
    FE_latch->op_valid=TRUE; 
    return;
    } 

  
  Op *fe_op = get_free_op(); // op currently in FE_stage
  bool success=get_op(fe_op); 
  
  if(success==FALSE) //last instruction
      fe_op->last_op=TRUE;
  
  FE_latch->op=fe_op;
  FE_latch->op_valid=TRUE; 
  fe_ctr=0; // Instruction was successfully read, so reset the loop detection counter
  return;
  
  //   next_pc = pc + op->inst_size;  // you need this code for building a branch predictor 

}


void  init_latches()
{
  MEM_latch = new pipeline_latch();
  EX_latch = new pipeline_latch();
  ID_latch = new pipeline_latch();
  FE_latch = new pipeline_latch();

  MEM_latch->op = NULL;
  EX_latch->op = NULL;
  ID_latch->op = NULL;
  FE_latch->op = NULL;

  /* you must set valid value correctly  */ 
  MEM_latch->op_valid = false;
  EX_latch->op_valid = false;
  ID_latch->op_valid = false;
  FE_latch->op_valid = false;

}


bool icache_access(uint32_t addr) {

  /* For Lab #1, you assume that all I-cache hit */     
  bool hit = FALSE; 
  if (KNOB(KNOB_PERFECT_ICACHE)->getValue()) hit = TRUE; 
  return hit; 
}



bool dcache_access(uint32_t addr) {
  /* For Lab #1, you assume that all D-cache hit */     
  bool hit = FALSE;
  if (KNOB(KNOB_PERFECT_DCACHE)->getValue()) hit = TRUE; 
  return hit; 
}
