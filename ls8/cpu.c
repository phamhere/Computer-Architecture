#include "cpu.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define DATA_LEN 6

unsigned char cpu_ram_read(struct cpu *cpu, unsigned char address)
{
  return cpu->ram[address];
}

void cpu_ram_write(struct cpu *cpu, unsigned char address, unsigned char value)
{
  cpu->ram[address] = value;
}

/**
 * Load the binary bytes from a .ls8 source file into a RAM array
 */
void cpu_load(struct cpu *cpu, int argc, char *file)
{
  // char data[DATA_LEN] = {
  //     // From print8.ls8
  //     0b10000010, // LDI R0,8
  //     0b00000000,
  //     0b00001000,
  //     0b01000111, // PRN R0
  //     0b00000000,
  //     0b00000001 // HLT
  // };

  // int address = 0;

  // for (int i = 0; i < DATA_LEN; i++)
  // {
  //   cpu->ram[address++] = data[i];
  // }

  // TODO: Replace this with something less hard-coded
  FILE *fp;
  char line[1024];
  int address = 0;
  // if the number of arguments in command line isn't 2, print usage and stop
  if (argc != 2)
  {
    printf("usage: fileio filename\n");
    exit(1);
  }
  // opening file
  fp = fopen(file, "r");
  // if file opening results in null, print error and stop
  if (fp == NULL)
  {
    printf("Error opening file %s\n", file);
    exit(2);
  }
  // loop until file ends
  while (fgets(line, 1024, fp) != NULL)
  {
    char *endptr;
    // storing the binary value of line in val and address of first invalid char in &endptr
    unsigned char val = strtoul(line, &endptr, 2);
    // skip line if line equals endptr
    if (line == endptr)
      continue;
    // assing ram at address to binary value val
    cpu->ram[address++] = val;
  }
}

/**
 * ALU
 */
void alu(struct cpu *cpu, enum alu_op op, unsigned char regA, unsigned char regB)
{
  switch (op)
  {
  case ALU_MUL:
    // TODO
    cpu->registers[regA] *= cpu->registers[regB];
    break;

    // TODO: implement more ALU ops
  case ALU_ADD:
    cpu->registers[regA] += cpu->registers[regB];
    break;
  case ALU_CMP:
    if (cpu->registers[regA] == cpu->registers[regB])
    {
      cpu->fl = cpu->fl | 0b00000001;
    }
    else
    {
      cpu->fl = cpu->fl & 0b00000110;
    }
    if (cpu->registers[regA] < cpu->registers[regB])
    {
      cpu->fl = cpu->fl | 0b00000100;
    }
    else
    {
      cpu->fl = cpu->fl & 0b00000011;
    }
    if (cpu->registers[regA] > cpu->registers[regB])
    {
      cpu->fl = cpu->fl | 0b00000010;
    }
    else
    {
      cpu->fl = cpu->fl & 0b00000101;
    }
    break;
  case ALU_AND:
    cpu->registers[regA] = cpu->registers[regA] & cpu->registers[regB];
    break;
  case ALU_OR:
    cpu->registers[regA] = cpu->registers[regA] | cpu->registers[regB];
    break;
  }
}

/**
 * Run the CPU
 */
void cpu_run(struct cpu *cpu)
{
  int running = 1;         // True until we get a HLT instruction
  cpu->registers[7] = 243; // setting stack pointer to index 255 in ram

  while (running)
  {
    // TODO
    // 1. Get the value of the current instruction (in address PC).
    unsigned char ir = cpu_ram_read(cpu, cpu->pc);
    unsigned char operandA = cpu_ram_read(cpu, cpu->pc + 1);
    unsigned char operandB = cpu_ram_read(cpu, cpu->pc + 2);
    // 2. Figure out how many operands this next instruction requires
    // 3. Get the appropriate value(s) of the operands following this instruction
    // 4. switch() over it to decide on a course of action.
    // 5. Do whatever the instruction should do according to the spec.
    // 6. Move the PC to the next instruction.
    switch (ir)
    {
    case HLT:
      running = 0;
      break;
    case LDI:
      cpu->registers[operandA] = operandB;
      cpu->pc += 3;
      break;
    case PRN:
      printf("%d\n", cpu->registers[operandA]);
      cpu->pc += 2;
      break;
    case MUL:
      alu(cpu, ALU_MUL, operandA, operandB);
      cpu->pc += 3;
      break;
    case PUSH:
      cpu->registers[7]--;
      cpu_ram_write(cpu, cpu->registers[7], cpu->registers[operandA]);
      cpu->pc += 2;
      break;
    case POP:
      cpu->registers[operandA] = cpu_ram_read(cpu, cpu->registers[7]);
      cpu->registers[7]++;
      cpu->pc += 2;
      break;
    case CALL:
      cpu->registers[7]--;
      cpu_ram_write(cpu, cpu->registers[7], cpu->pc + 2);
      cpu->pc = cpu->registers[operandA];
      break;
    case RET:
      cpu->pc = cpu_ram_read(cpu, cpu->registers[7]);
      cpu->registers[7]++;
      break;
    case ADD:
      alu(cpu, ALU_ADD, operandA, operandB);
      cpu->pc += 3;
      break;
    case CMP:
      alu(cpu, ALU_CMP, operandA, operandB);
      cpu->pc += 3;
      break;
    case JMP:
      cpu->pc = cpu->registers[operandA];
      break;
    case JEQ:
      if ((cpu->fl & 0b00000001) == 1)
      {
        cpu->pc = cpu->registers[operandA];
      }
      else
      {
        cpu->pc += 2;
      }
      break;
    case JNE:
      if ((cpu->fl & 0b00000001) == 0)
      {
        cpu->pc = cpu->registers[operandA];
      }
      else
      {
        cpu->pc += 2;
      }
      break;
    case AND:
      alu(cpu, ALU_AND, operandA, operandB);
      cpu->pc += 3;
      break;
    case OR:
      alu(cpu, ALU_OR, operandA, operandB);
      cpu->pc += 3;
      break;
    default:
      printf("Unrecognized instruction\n");
      cpu->pc++;
      break;
    }
  }
}

/**
 * Initialize a CPU struct
 */
void cpu_init(struct cpu *cpu)
{
  // TODO: Initialize the PC and other special registers
  cpu->pc = 0;
  cpu->fl = 0;
  memset(cpu->registers, 0, sizeof(unsigned char) * 8);
  memset(cpu->ram, 0, sizeof(unsigned char) * 256);
}