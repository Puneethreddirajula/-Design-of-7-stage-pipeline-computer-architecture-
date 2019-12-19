/*
 *  cpu.c
 *  Contains APEX cpu pipeline implementation
 *
 *  State University of New York, Binghamton
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "cpu.h"

/* Set this flag to 1 to enable debug messages */
#define ENABLE_DEBUG_MESSAGES 1

/*
 * This function creates and initializes APEX cpu.
 *
 * Note : You are free to edit this function according to your
 * 				implementation
 */
APEX_CPU*
APEX_cpu_init(const char* filename)
{
  if (!filename) {
    return NULL;
  }

  APEX_CPU* cpu = malloc(sizeof(*cpu));
  if (!cpu) {
    return NULL;
  }
  /* Initialize PC, Registers and all pipeline stages */
  cpu->pc = 4000;
  memset(cpu->regs, 0, sizeof(int) * 32);
  memset(cpu->regs_valid, 1, sizeof(int) * 32);
  memset(cpu->stage, 0, sizeof(CPU_Stage) * NUM_STAGES);
  memset(cpu->data_memory, 0, sizeof(int) * 4000);

  /* Parse input file and create code memory */
  cpu->code_memory = create_code_memory(filename, &cpu->code_memory_size);

  if (!cpu->code_memory) {
    free(cpu);
    return NULL;
  }

  if (ENABLE_DEBUG_MESSAGES) {
    fprintf(stderr,
            "APEX_CPU : Initialized APEX CPU, loaded %d instructions\n",
            cpu->code_memory_size);
    fprintf(stderr, "APEX_CPU : Printing Code Memory\n");
    printf("%-9s %-9s %-9s %-9s %-9s\n", "opcode", "rd", "rs1", "rs2", "imm");

    for (int i = 0; i < cpu->code_memory_size; ++i) {
      printf("%-9s %-9d %-9d %-9d %-9d\n",
             cpu->code_memory[i].opcode,
             cpu->code_memory[i].rd,
             cpu->code_memory[i].rs1,
             cpu->code_memory[i].rs2,
             cpu->code_memory[i].imm);
    }
  }

  /* Make all stages busy except Fetch stage, initally to start the pipeline */
  for (int i = 1; i < NUM_STAGES; ++i) {
    cpu->stage[i].busy = 1;
  }

  for(int i=0;i<=(sizeof(cpu->regs_valid)-113);i++)
  {
    cpu->regs_valid[i]=1;       //0 is INVALID & 1 is VALID
  }
  return cpu;
}

/*
 * This function de-allocates APEX cpu.
 *
 * Note : You are free to edit this function according to your
 * 				implementation
 */
void
APEX_cpu_stop(APEX_CPU* cpu)
{
  free(cpu->code_memory);
  free(cpu);
}


/* Converts the PC(4000 series) into
 * array index for code memory
 *
 * Note : You are not supposed to edit this function
 *
 */
int
get_code_index(int pc)
{
  return (pc - 4000) / 4;
}

static void
print_instruction(CPU_Stage* stage)
{
  if (strcmp(stage->opcode, "STORE") == 0) {
    printf(
      "%s,R%d,R%d,#%d ", stage->opcode, stage->rs1, stage->rs2, stage->imm);
  }

  if (strcmp(stage->opcode, "STR") == 0) {
    printf(
      "%s,R%d,R%d,R%d ", stage->opcode, stage->rs1, stage->rs2, stage->rs3);
  }

  if (strcmp(stage->opcode, "MOVC") == 0) {
    printf("%s,R%d,#%d ", stage->opcode, stage->rd, stage->imm);
  }

   if (strcmp(stage->opcode, "ADD") == 0) {
    printf("%s,R%d,R%d,R%d ", stage->opcode, stage->rd, stage->rs1, stage->rs2);
  }

  if (strcmp(stage->opcode, "ADDL") == 0) {
    printf("%s,R%d,R%d,#%d ", stage->opcode, stage->rd, stage->rs1, stage->imm);
  }

  if (strcmp(stage->opcode, "SUB") == 0) {
    printf("%s,R%d,R%d,R%d ", stage->opcode, stage->rd, stage->rs1, stage->rs2);
  }

  if (strcmp(stage->opcode, "SUBL") == 0) {
    printf("%s,R%d,R%d,#%d ", stage->opcode, stage->rd, stage->rs1, stage->imm);
  }

  if (strcmp(stage->opcode, "LOAD") == 0) {
    printf("%s,R%d,R%d,#%d ", stage->opcode, stage->rd, stage->rs1, stage->imm);
  }

  if (strcmp(stage->opcode, "LDR") == 0) {
    printf("%s,R%d,R%d,R%d ", stage->opcode, stage->rd, stage->rs1, stage->rs2);
  }

  if (strcmp(stage->opcode, "MUL") == 0) {
    printf("%s,R%d,R%d,R%d ", stage->opcode, stage->rd, stage->rs1, stage->rs2);
  }

  if (strcmp(stage->opcode, "AND") == 0) {
    printf("%s,R%d,R%d,R%d ", stage->opcode, stage->rd, stage->rs1, stage->rs2);
  }

  if (strcmp(stage->opcode, "OR") == 0) {
    printf("%s,R%d,R%d,R%d ", stage->opcode, stage->rd, stage->rs1, stage->rs2);
  }

  if (strcmp(stage->opcode, "XOR") == 0) {
    printf("%s,R%d,R%d,R%d ", stage->opcode, stage->rd, stage->rs1, stage->rs2);
  }

  if (strcmp(stage->opcode, "BZ") == 0) {
    printf("%s,#%d ", stage->opcode, stage->imm);
  }

  if (strcmp(stage->opcode, "BNZ") == 0) {
    printf("%s,#%d ", stage->opcode, stage->imm);
  }

  if (strcmp(stage->opcode, "JUMP") == 0) {
    printf("%s,R%d,#%d", stage->opcode, stage->rd, stage->imm);
  }

  if (strcmp(stage->opcode, "HALT") == 0) {
    printf("%s", stage->opcode);
  }

  if (strcmp(stage->opcode, "NOP") == 0) {
    printf("%s", stage->opcode);
  }
}

/* Debug function which dumps the cpu stage
 * content
 *
 * Note : You are not supposed to edit this function
 *
 */
static void
print_stage_content(char* name, CPU_Stage* stage)
{
  printf("%-15s: pc(%d) ", name, stage->pc);
  print_instruction(stage);
  printf("\n");
}

/*
 *  Fetch Stage of APEX Pipeline
 *
 *  Note : You are free to edit this function according to your
 * 				 implementation
 */
int
fetch(APEX_CPU* cpu)
{
  CPU_Stage* stage = &cpu->stage[F];
  /* code */
  if (!stage->busy && !stage->stalled) 
  {  
    /* Store current PC in fetch latch */
    stage->pc = cpu->pc;
    /* Index into code memory using this pc and copy all instruction fields into
    * fetch latch
    */
    APEX_Instruction* current_ins = &cpu->code_memory[get_code_index(cpu->pc)];
    strcpy(stage->opcode, current_ins->opcode);
    stage->rd = current_ins->rd;
    stage->rs1 = current_ins->rs1;
    stage->rs2 = current_ins->rs2;
    stage->rs3 = current_ins->rs3;
    stage->imm = current_ins->imm;
    stage->rd = current_ins->rd;
    /* Update PC for next instruction */
    
    /* Copy data from fetch latch to decode latch*/
    if(cpu->stage[DRF].stalled==0)
    {
      cpu->pc += 4;
      cpu->stage[DRF] = cpu->stage[F];
    }

    if(strcmp(cpu->input,"display")==0)
    {
      if (ENABLE_DEBUG_MESSAGES) 
      {
        print_stage_content("Fetch", stage);
      }
    }
  }
  else
  {
    strcpy(stage->opcode,"");
    
    if(strcmp(cpu->input,"display")==0)
    {
      if (ENABLE_DEBUG_MESSAGES) 
      {
        print_stage_content("Fetch", stage);
      }
    }
  }
  return 0;
}
/*
 *  Decode Stage of APEX Pipeline
 *
 *  Note : You are free to edit this function according to your
 * 				 implementation
 */
int
decode(APEX_CPU* cpu)
{
  CPU_Stage* stage = &cpu->stage[DRF];
  
  
    /* Read data from register file for store */
      if (strcmp(stage->opcode, "STORE") == 0)
      {
        if(cpu->regs_valid[stage->rs1]==0 || cpu->regs_valid[stage->rs2]==0)
        {
          stage->stalled=1;
        }
        else
        {
          stage->rs1_value = cpu->regs[stage->rs1];
          stage->rs2_value = cpu->regs[stage->rs2];
          stage->stalled=0;
        }
      }
  
      if (strcmp(stage->opcode, "STR") == 0) 
      {
        if(cpu->regs_valid[stage->rs1]==0 || cpu->regs_valid[stage->rs2]==0 || cpu->regs_valid[stage->rs3]==0)
        {
          stage->stalled=1;
        }
        else
        {
          stage->rs1_value = cpu->regs[stage->rs1];
          stage->rs2_value = cpu->regs[stage->rs2];
          stage->rs3_value = cpu->regs[stage->rs3];
          stage->stalled=0;
        }

      }
    
      /* No Register file read needed for MOVC */
      if (strcmp(stage->opcode, "MOVC") == 0) 
      {
      }
      if (strcmp(stage->opcode, "ADD") == 0) 
      {
        if(cpu->regs_valid[stage->rs1]==0 || cpu->regs_valid[stage->rs2]==0)
        {
          stage->stalled=1;
        }
        else
        {
          stage->rs1_value = cpu->regs[stage->rs1];
          stage->rs2_value = cpu->regs[stage->rs2];
          stage->stalled=0;
        }
      }

      if(strcmp(stage->opcode, "ADDL") == 0)
      {
        if(cpu->regs_valid[stage->rs1]==0)
        {
          stage->stalled=1;
        }
        else
        {
          stage->rs1_value = cpu->regs[stage->rs1];
          stage->stalled=0;
        }
      }

      if(strcmp(stage->opcode, "SUB") == 0)
      {
        if(cpu->regs_valid[stage->rs1]==0 || cpu->regs_valid[stage->rs2]==0)
        {
          stage->stalled=1;
        }
        else
        {
          stage->rs1_value = cpu->regs[stage->rs1];
          stage->rs2_value = cpu->regs[stage->rs2];
          stage->stalled=0;
        }
      }

      if(strcmp(stage->opcode, "SUBL") == 0)
      {
        if(cpu->regs_valid[stage->rs1]==0)
        {
          stage->stalled=1;
        }
        else
        {
          stage->rs1_value = cpu->regs[stage->rs1];
          stage->stalled=0;
        }
      }

      if(strcmp(stage->opcode, "MUL") == 0)
      {
        if(cpu->regs_valid[stage->rs1]==0 || cpu->regs_valid[stage->rs2]==0)
        {
          stage->stalled=1;
        }
        else
        {
          stage->rs1_value = cpu->regs[stage->rs1];
          stage->rs2_value = cpu->regs[stage->rs2];
          stage->stalled=0;
        }
      } 
  
      if(strcmp(stage->opcode, "LOAD") == 0)
      {
        if(cpu->regs_valid[stage->rs1]==0)
        {
          stage->stalled=1;
        }
        else
        {
          stage->rs1_value = cpu->regs[stage->rs1];
          stage->stalled=0;
        }
      } 

      if(strcmp(stage->opcode, "LDR") == 0)
      {
        if(cpu->regs_valid[stage->rs1]==0 || cpu->regs_valid[stage->rs2]==0)
        {
          stage->stalled=1;
        }
        else
        {
          stage->rs1_value = cpu->regs[stage->rs1];
          stage->rs2_value = cpu->regs[stage->rs2];
          stage->stalled=0;
        }
      } 

      if(strcmp(stage->opcode, "AND") == 0)
      {
        if(cpu->regs_valid[stage->rs1]==0 || cpu->regs_valid[stage->rs2]==0)
        {
          stage->stalled=1;
        }
        else
        {
          stage->rs1_value = cpu->regs[stage->rs1];
          stage->rs2_value = cpu->regs[stage->rs2];
          stage->stalled=0;
        }
      }

      if(strcmp(stage->opcode, "OR") == 0)
      {
        if(cpu->regs_valid[stage->rs1]==0 || cpu->regs_valid[stage->rs2]==0)
        {
          stage->stalled=1;
        }
        else
        {
          stage->rs1_value = cpu->regs[stage->rs1];
          stage->rs2_value = cpu->regs[stage->rs2];
          stage->stalled=0;
        }
      }

      if(strcmp(stage->opcode, "XOR") == 0)
      {
        if(cpu->regs_valid[stage->rs1]==0 || cpu->regs_valid[stage->rs2]==0)
        {
          stage->stalled=1;
        }
        else
        {
          stage->rs1_value = cpu->regs[stage->rs1];
          stage->rs2_value = cpu->regs[stage->rs2];
          stage->stalled=0;
        }
      }

      /*if(strcmp(stage->opcode, "BZ") == 0)
      {
        if(strcmp(cpu->stage[WB].opcode,"ADD")!=0 && strcmp(cpu->stage[WB].opcode,"SUB")!=0 && strcmp(cpu->stage[WB].opcode,"MUL")!=0) 
          //&& strcmp(cpu->stage[MEM2],"ADD")!=0 && strcmp(cpu->stage[MEM2],"ADD")!=0 && strcmp(cpu->stage[MEM2],"ADD")!=0 &&)
        {
          stage->stalled=0;
        }
      }*/
      
      if(strcmp(stage->opcode, "BZ") == 0)
      {
          if(!cpu->zero_flag && (strcmp(cpu->stage[EX1].opcode, "ADD") == 0 || strcmp(cpu->stage[EX1].opcode, "NOP") == 0 || strcmp(cpu->stage[EX1].opcode, "SUB") == 0 || strcmp(cpu->stage[EX1].opcode, "MUL") == 0))
          { 
            if(cpu->str!=4)
            {
              stage->stalled = 1;
              cpu->str++;
            }
            else
            {
              stage->stalled=0;
            }
          }
          else
          {
             stage->stalled=0;
          }
      }

      if(strcmp(stage->opcode, "BNZ") == 0)
      {
          if(!cpu->zero_flag && (strcmp(cpu->stage[EX1].opcode, "ADD") == 0 || strcmp(cpu->stage[EX1].opcode, "NOP") == 0 || strcmp(cpu->stage[EX1].opcode, "SUB") == 0 || strcmp(cpu->stage[EX1].opcode, "MUL") == 0))
          { 
            if(cpu->str!=4)
            {
              stage->stalled = 1;
              cpu->str++;
            }
            else
            {
              stage->stalled=0;
            }
          }
          else
          {
          stage->stalled=0;
          }
      }

      if(strcmp(stage->opcode, "JUMP") == 0)
      {
        if(cpu->regs_valid[stage->rd]==0)
        {
          stage->stalled=1;
        }
        else
        {
          //printf("Requested DATA of RS1 is: %d\n",cpu->regs[stage->rd]);
          stage->rs1_value = cpu->regs[stage->rd];
          stage->stalled=0;
        }
      }

      if(strcmp(stage->opcode, "HALT") == 0)
      {
        cpu->stage[F].busy=1;
        cpu->sp=1;
        //cpu->stage[DRF].busy=1;
      }

      /* Copy data from decode latch to execute latch*/
      if(stage->stalled==0 && stage->busy == 0)
      {
        cpu->stage[EX1] = cpu->stage[DRF];
      }
      else
      {
        strcpy(cpu->stage[EX1].opcode,"NOP");
        //cpu->stage[EX1] = cpu->stage[DRF];
      }
      
      if(cpu->sp==1)
      {
        if(strcmp(cpu->input,"display")==0)
        {
          if (ENABLE_DEBUG_MESSAGES) 
          {
            print_stage_content("Decode/RF", stage);
          }
        }
        strcpy(stage->opcode,"");
      }
      else
      {
        if(strcmp(cpu->input,"display")==0)
        {
          if (ENABLE_DEBUG_MESSAGES) 
          {
            print_stage_content("Decode/RF", stage);
          }
        }
      }
      
    
return 0;
}

/*
 *  Execute Stage of APEX Pipeline
 *
 *  Note : You are free to edit this function according to your
 * 				 implementation
 */
int
execute(APEX_CPU* cpu)
{

  CPU_Stage* stage = &cpu->stage[EX1];
  if(cpu->stage[DRF].stalled==1)
  {
    stage->stalled=1;
  }
  if (!stage->busy && !stage->stalled) 
  {
    /* Store */
    if (strcmp(stage->opcode, "STORE") == 0) 
    {
      stage->mem_address = stage->rs2_value + stage->imm;
      //cpu->regs_valid[stage->rs1]=0;
    }

    if (strcmp(stage->opcode, "STR") == 0) 
    {
      stage->mem_address = stage->rs2_value + stage->rs3_value;
      //cpu->regs_valid[stage->rs1]=0;
    }

    /* MOVC */
    if (strcmp(stage->opcode, "MOVC") == 0) 
    {
      stage->buffer = stage->imm + 0;
      cpu->regs_valid[stage->rd]=0;
    }
	
    if (strcmp(stage->opcode, "ADD") == 0) 
    {
      stage->buffer = stage->rs1_value + stage->rs2_value;

      cpu->regs_valid[stage->rd]=0;
    }

    if (strcmp(stage->opcode, "ADDL") == 0) 
    {
      stage->buffer = stage->rs1_value + stage->imm;
      cpu->regs_valid[stage->rd]=0;
    }

    if (strcmp(stage->opcode, "SUB") == 0) 
    {
      stage->buffer = stage->rs1_value - stage->rs2_value;
      cpu->regs_valid[stage->rd]=0;
    }

    if (strcmp(stage->opcode, "SUBL") == 0) 
    {
      stage->buffer = stage->rs1_value - stage->imm;
      cpu->regs_valid[stage->rd]=0;
    }

    if (strcmp(stage->opcode, "MUL") == 0) 
    {
      stage->buffer = stage->rs1_value * stage->rs2_value;
      cpu->regs_valid[stage->rd]=0;      
    }

    if (strcmp(stage->opcode, "LOAD") == 0) 
    {
      stage->mem_address = stage->rs1_value + stage->imm;
      cpu->regs_valid[stage->rd]=0;
    }

    if (strcmp(stage->opcode, "LDR") == 0) 
    {
      stage->mem_address = stage->rs1_value + stage->rs2_value;
      cpu->regs_valid[stage->rd]=0;
    }

    /*if (strcmp(stage->opcode, "BZ") == 0) 
    {
        if(cpu->zero_flag == 1)
        {
          int temp;
          temp = abs(stage->pc + stage->imm);
          cpu->ins_completed = temp/4;
          cpu->pc = temp;
          temp = temp % 4;
          cpu->pc = cpu->pc - temp;
          cpu->zero_flag=0;
          cpu->stage[DRF].busy=1;
        }
    }

    if (strcmp(stage->opcode, "BNZ") == 0) 
    {
      if(cpu->zero_flag == 0)
      {
        int temp;
        temp = abs(stage->pc + stage->imm);
        cpu->pc = temp;
        temp = temp % 4;
        cpu->pc = cpu->pc - temp;
        cpu->stage[DRF].busy=1;
      }
    }*/

    if (strcmp(stage->opcode, "JUMP") == 0) 
    {
      stage->buffer = stage->rs1_value + stage->imm;
      stage->next_addr = stage->pc + 4;
      cpu->pc = stage->buffer;
      stage->buffer = stage->buffer % 4;
      cpu->pc = cpu->pc - stage->buffer;
      stage->busy=1;
      cpu->stage[DRF].busy=1;
      cpu->stp=1;
      cpu->ins_completed++;
    }
    if (strcmp(stage->opcode, "AND") == 0) 
    {
      cpu->regs_valid[stage->rd]=0;
    }
    if (strcmp(stage->opcode, "OR") == 0) 
    {
      cpu->regs_valid[stage->rd]=0;
    }
    if (strcmp(stage->opcode, "XOR") == 0) 
    {
      cpu->regs_valid[stage->rd]=0;
    }
    if (strcmp(stage->opcode, "HALT") == 0) 
    {

    }
    /* Copy data from Execute latch to Memory latch*/
    cpu->stage[EX2] = cpu->stage[EX1];

    
  }
  else
  {
    strcpy(cpu->stage[EX2].opcode,"NOP");
  }
  if(strcmp(cpu->input,"display")==0)
  {
    if (ENABLE_DEBUG_MESSAGES) 
    {
      print_stage_content("Execute 1", stage);
    }
  }
  return 0;
}

int 
execute2(APEX_CPU* cpu)
{
  CPU_Stage* stage = &cpu->stage[EX2];
  if(cpu->stage[EX1].stalled == 1)
  {
    stage->stalled = 1;
  }
  if(cpu->stp==1)
  {

    cpu->pc = stage->next_addr;
    cpu->stp=0;
  }
  if (!stage->busy && !stage->stalled) 
  {
    if (strcmp(stage->opcode, "BZ") == 0) 
    {
        if(cpu->zero_flag == 1)
        {
          int temp;
          temp = abs(stage->pc + stage->imm);
          cpu->ins_completed = temp/4;
          cpu->buffer = temp;
          temp = temp % 4;
          cpu->buffer = cpu->buffer - temp;
        }
    }

    if (strcmp(stage->opcode, "BNZ") == 0) 
    {
        if(cpu->zero_flag == 0)
        {
          int temp;
          temp = abs(stage->pc + stage->imm);
          cpu->ins_completed = temp/4;
          cpu->buffer = temp;
          temp = temp % 4;
          cpu->buffer = cpu->buffer - temp;
        }
    }

    if (strcmp(stage->opcode, "AND") == 0) {

      stage->buffer = stage->rs1_value & stage->rs2_value;
      
      //printf("AND output is:%d\n",stage->buffer);
    }
    if (strcmp(stage->opcode, "MOVC") == 0)
    {
      cpu->stage[EX2].busy=1;
    }

    if (strcmp(stage->opcode, "OR") == 0) 
    {
      stage->buffer = stage->rs1_value | stage->imm;
    }

    if (strcmp(stage->opcode, "XOR") == 0) 
    {
      stage->buffer = stage->rs1_value ^ stage->imm;
    }

    if (strcmp(stage->opcode, "JUMP") == 0) 
    {
      stage->stalled=1;
    }
    cpu->stage[EX2].busy=0;
    cpu->stage[MEM1] = cpu->stage[EX2];
    
    
    
  }
  else
  {
    strcpy(cpu->stage[MEM1].opcode,"NOP");
  }
  if(strcmp(cpu->input,"display")==0)
  {
    if (ENABLE_DEBUG_MESSAGES) 
    {
      print_stage_content("Execute 2", stage);
    }
  }
  return 0;
}
/*
 *  Memory Stage of APEX Pipeline
 *
 *  Note : You are free to edit this function according to your
 * 				 implementation
 */
int
memory(APEX_CPU* cpu)
{
  CPU_Stage* stage = &cpu->stage[MEM1];
    if(cpu->stage[EX2].stalled==1){
    stage->stalled=1;
  }
  if (!stage->busy && !stage->stalled) {

    /* Store */
    if (strcmp(stage->opcode, "STORE") == 0) 
    {
        cpu->data_memory[stage->mem_address] = stage->rs1_value;
        //printf("In Store ins value at MEM[%d] is %d\n", stage->buffer,cpu->data_memory[stage->buffer]);
    }

    if (strcmp(stage->opcode, "STR") == 0) 
    {
        cpu->data_memory[stage->mem_address] = stage->rs1_value;
        //printf("In Store ins value at MEM[%d] is %d\n", stage->buffer,cpu->data_memory[stage->buffer]);
    }

    /* MOVC */
    if (strcmp(stage->opcode, "MOVC") == 0) 
    {
      cpu->stage[MEM1].busy=1;
    }

    if (strcmp(stage->opcode, "BZ") == 0) 
    {
      if(cpu->zero_flag==1)
      {
          cpu->pc=cpu->buffer;
          cpu->zero_flag=0;
          strcpy(cpu->stage[F].opcode,"NOP");
          strcpy(cpu->stage[DRF].opcode,"NOP");
          strcpy(cpu->stage[EX1].opcode,"NOP");
          strcpy(cpu->stage[EX2].opcode,"NOP");
        }
    }

    if (strcmp(stage->opcode, "BNZ") == 0) 
    {
      if(cpu->zero_flag==0)
      {
          cpu->pc=cpu->buffer;
          cpu->zero_flag=0;
          strcpy(cpu->stage[F].opcode,"NOP");
          strcpy(cpu->stage[DRF].opcode,"NOP");
          strcpy(cpu->stage[EX1].opcode,"NOP");
          strcpy(cpu->stage[EX2].opcode,"NOP");
        }
    }

    cpu->stage[MEM1].busy=0;
    /* Copy data from decode latch to execute latch*/
    cpu->stage[MEM2] = cpu->stage[MEM1];
  }
  else
  {
    strcpy(cpu->stage[MEM2].opcode,"NOP");
  }
  if(strcmp(cpu->input,"display")==0)
  {
    if (ENABLE_DEBUG_MESSAGES) 
    {
      print_stage_content("Memory 1", stage);
    }
  }
  return 0;
}

int memory2(APEX_CPU *cpu)
{
  CPU_Stage* stage = &cpu->stage[MEM2];
    if(cpu->stage[MEM1].stalled==1){
    stage->stalled=1;
  }
  if (!stage->busy && !stage->stalled)
  {

    if (strcmp(stage->opcode, "LOAD") == 0) 
    {
      stage->rs1_value = cpu->data_memory[stage->mem_address];
    }

    if (strcmp(stage->opcode, "LDR") == 0) 
    {
      stage->rs1_value = cpu->data_memory[stage->mem_address];
    }

    if (strcmp(stage->opcode, "MOVC") == 0)
    {
      cpu->stage[MEM2].busy=1;
    }
    cpu->stage[MEM2].busy=0;
    cpu->stage[WB] = cpu->stage[MEM2];
  }

  else
  {
    strcpy(cpu->stage[WB].opcode,"NOP");
  }

  if(strcmp(cpu->input,"display")==0)
  {
    if (ENABLE_DEBUG_MESSAGES) 
    { 
      print_stage_content("Memory 2", stage);
    }
  }
  return 0;
}
/*
 *  Writeback Stage of APEX Pipeline
 *
 *  Note : You are free to edit this function according to your
 * 				 implementation
 */

int
writeback(APEX_CPU* cpu)
{
  CPU_Stage* stage = &cpu->stage[WB];
  if(cpu->stage[MEM2].stalled==1)
  {
    stage->stalled=1;
  }
  if (!stage->busy && !stage->stalled) {

    /* Update register file */
    if (strcmp(stage->opcode, "MOVC") == 0) 
    {
      cpu->regs[stage->rd] = stage->buffer;
      if(cpu->stage[MEM2].rd == stage->rd && strcmp(cpu->stage[MEM2].opcode,"NOP")!=0)
      {
        cpu->regs_valid[stage->rd]=0;
      }
      else
      {
        cpu->regs_valid[stage->rd]=1;
      }
      //printf("MOVC data is: %d\n",cpu->regs[stage->rd]);
    }

    if (strcmp(stage->opcode, "ADD") == 0) {
      cpu->regs[stage->rd] = stage->buffer;
      if(cpu->regs[stage->rd] == 0)
      {
        cpu->zero_flag=1;
      }
      else
      {
        cpu->ad=1;
        cpu->zero_flag=0;
      }
      if(cpu->stage[MEM2].rd == stage->rd && strcmp(cpu->stage[MEM2].opcode,"NOP")!=0)
      {
        cpu->regs_valid[stage->rd]=0;
      }
      else
      {
        cpu->regs_valid[stage->rd]=1;
      }
      //printf("ADD in WB is: %d\n",cpu->regs[stage->rd]);
    }

    if (strcmp(stage->opcode, "ADDL") == 0) {

      cpu->regs[stage->rd] = stage->buffer;
      if(cpu->regs[stage->rd] == 0)
      {
        cpu->zero_flag=1;
      }
      else
      {
        cpu->zero_flag=0;
      }
      if(cpu->stage[MEM2].rd == stage->rd && strcmp(cpu->stage[MEM2].opcode,"NOP")!=0)
      {
        cpu->regs_valid[stage->rd]=0;
      }
      else
      {
        cpu->regs_valid[stage->rd]=1;
      }
    }

    if (strcmp(stage->opcode, "SUB") == 0) {

      cpu->regs[stage->rd] = stage->buffer;
      //printf("SUB DATA is :%d\n",cpu->regs[stage->rd]);
      if(cpu->regs[stage->rd] == 0)
      {
        cpu->zero_flag=1;
      }
      else
      {
        cpu->sb=1;
        cpu->zero_flag=0;
      }
      if(cpu->stage[MEM2].rd == stage->rd && strcmp(cpu->stage[MEM2].opcode,"NOP")!=0)
      {
        cpu->regs_valid[stage->rd]=0;
      }
      else
      {
        cpu->regs_valid[stage->rd]=1;
      }
    }

    if (strcmp(stage->opcode, "SUBL") == 0) 
    {
      cpu->regs[stage->rd] = stage->buffer;
      if(cpu->regs[stage->rd] == 0)
      {
        cpu->zero_flag=1;
      }
      else
      {
        cpu->zero_flag=0;

      }
      //printf("ZERO FLAG is: %d\n",cpu->zero_flag);
      if(cpu->stage[MEM2].rd == stage->rd && strcmp(cpu->stage[MEM2].opcode,"NOP")!=0)
      {
        cpu->regs_valid[stage->rd]=0;
      }
      else
      {
        cpu->regs_valid[stage->rd]=1;
      }
      //printf("SUBL data is: %d\n",cpu->regs[stage->rd]);
    }

    if (strcmp(stage->opcode, "MUL") == 0) {

      cpu->regs[stage->rd] = stage->buffer;
      if(cpu->regs[stage->rd] == 0)
      {
        cpu->zero_flag=1;
      }
      else
      {
        cpu->ml=1;
        cpu->zero_flag=0;
      }
      if(cpu->stage[MEM2].rd == stage->rd && strcmp(cpu->stage[MEM2].opcode,"NOP")!=0)
      {
        cpu->regs_valid[stage->rd]=0;
      }
      else
      {
        cpu->regs_valid[stage->rd]=1;
      }
      //printf("MUL data is: %d\n",cpu->regs[stage->rd]);
    }

    if (strcmp(stage->opcode, "AND") == 0) {

      cpu->regs[stage->rd] = stage->buffer;
      
      if(cpu->stage[MEM2].rd == stage->rd && strcmp(cpu->stage[MEM2].opcode,"NOP")!=0)
      {
        cpu->regs_valid[stage->rd]=0;
      }
      else
      {
        cpu->regs_valid[stage->rd]=1;
      }
      //printf("AND data is: %d\n",cpu->regs[stage->rd]);
    }

    if (strcmp(stage->opcode, "OR") == 0) {

      cpu->regs[stage->rd] = stage->buffer;
      if(cpu->stage[MEM2].rd == stage->rd && strcmp(cpu->stage[MEM2].opcode,"NOP")!=0)
      {
        cpu->regs_valid[stage->rd]=0;
      }
      else
      {
        cpu->regs_valid[stage->rd]=1;
      }
      //printf("OR data is: %d\n",cpu->regs[stage->rd]);
    }

    if (strcmp(stage->opcode, "XOR") == 0) {

      cpu->regs[stage->rd] = stage->buffer;
      if(cpu->stage[MEM2].rd == stage->rd && strcmp(cpu->stage[MEM2].opcode,"NOP")!=0)
      {
        cpu->regs_valid[stage->rd]=0;
      }
      else
      {
        cpu->regs_valid[stage->rd]=1;
      }
      //printf("XOR data is: %d\n",cpu->regs[stage->rd]);
    }

    if (strcmp(stage->opcode, "LOAD") == 0) {

      cpu->regs[stage->rd] = stage->rs1_value;
      if(cpu->stage[MEM2].rd == stage->rd && strcmp(cpu->stage[MEM2].opcode,"NOP")!=0)
      {
        cpu->regs_valid[stage->rd]=0;
      }
      else
      {
        cpu->regs_valid[stage->rd]=1;
      }
      //printf("LOAD data is: %d\n",cpu->regs[stage->rd]);
    }

    if (strcmp(stage->opcode, "LDR") == 0) {

      cpu->regs[stage->rd] = stage->buffer;
      if(cpu->stage[MEM2].rd == stage->rd && strcmp(cpu->stage[MEM2].opcode,"NOP")!=0)
      {
        cpu->regs_valid[stage->rd]=0;
      }
      else
      {
        cpu->regs_valid[stage->rd]=1;
      }
      //printf("LDR data is: %d\n",cpu->regs[stage->rd]);
    }

    if (strcmp(stage->opcode, "BZ") == 0) {
      cpu->zero_flag=0;
      cpu->str=0;
    }

    if (strcmp(stage->opcode, "BNZ") == 0) {
      cpu->zero_flag=0;
      cpu->str=0;
    }

    if (strcmp(stage->opcode, "JUMP") == 0) {
      
    }

    if (strcmp(stage->opcode, "HALT") == 0) 
    {
      cpu->code_memory_size = cpu->ins_completed+1;
    }
    //cpu->stage[WB].busy=0;
    cpu->ins_completed++;
  }
  if(strcmp(cpu->input,"display")==0)
  {
    if (ENABLE_DEBUG_MESSAGES) 
    {
      print_stage_content("Writeback", stage);
    }
  }
  return 0;
}

int APEX_simulate(APEX_CPU* cpu)
{
  printf("--------------------------------\n");
    printf("===============STATE OF REGISTER FILE===============\n");
    for(int i=0;i<=(sizeof(cpu->regs)-113);i++)
    {
      printf("|     REG[%02d]     |    VALUE = %-5d|   STATUS = %s  |\n",i,cpu->regs[i],cpu->regs_valid[i]?"VALID":"INVALID");
    }
    display_mem(cpu);
  return 0;
}
int display_mem(APEX_CPU* cpu)
{
  printf("--------------------------------\n");
    printf("===============STATE OF DATA MEMORY===============\n");
    for(int i=0;i<=(sizeof(cpu->data_memory)-16285);i++)
    {
      printf("|     MEM[%02d]     |    DATA VALUE = %-5d|\n",i,cpu->data_memory[i]);
    }
  return 0;
}
/*
 *  APEX CPU simulation loop
 *
 *  Note : You are free to edit this function according to your
 * 				 implementation
 */
int
APEX_cpu_run(APEX_CPU* cpu)
{
  while (1) 
  {
    /* All the instructions committed, so exit */
    if(strcmp(cpu->input,"simulate")==0)
    {
      if (cpu->ins_completed == cpu->code_memory_size || cpu->clock == cpu->clk) 
      {
        printf("(apex) >> Simulation Complete");
        APEX_simulate(cpu);
        break;
      }
    }

    if(strcmp(cpu->input,"display")==0)
    {
      if (cpu->ins_completed == cpu->code_memory_size || cpu->clock == cpu->clk) 
      {
        printf("(apex) >> Simulation Complete");
        break;
      }
      if (ENABLE_DEBUG_MESSAGES) 
      {
        printf("--------------------------------\n");
        printf("Clock Cycle #: %d\n", cpu->clock);
        printf("--------------------------------\n");
      }
    }
    writeback(cpu);
    memory2(cpu);
    memory(cpu);
    execute2(cpu);
    execute(cpu);
    decode(cpu);
    fetch(cpu);
    cpu->clock++;
  }
  return 0;
}
