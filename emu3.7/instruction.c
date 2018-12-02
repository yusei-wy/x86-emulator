#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include "instruction.h"
#include "emulator.h"
#include "emulator_function.h"
#include "modrm.h"

instruction_func_t *instructions[256];

static void mov_r32_imm32(Emulator *emu) {
  uint8_t reg = get_code8(emu, 0) - 0xb8;
  uint32_t value = get_code32(emu, 1);
  set_register32(emu, reg, value);
  emu->eip += 5;
}

static void mov_r32_rm32(Emulator *emu) {
  emu->eip += 1;
  ModRM modrm;
  parse_modrm(emu, &modrm);
  uint32_t rm32 = get_rm32(emu, &modrm);
  set_r32(emu, &modrm, rm32);
}

static void add_rm32_r32(Emulator *emu) {
  emu->eip += 1;
  ModRM modrm;
  parse_modrm(emu, &modrm);
  uint32_t r32 = get_r32(emu, &modrm);
  uint32_t rm32 = get_rm32(emu, &modrm);
  set_rm32(emu, &modrm, rm32 + r32);
}

static void mov_rm32_r32(Emulator *emu) {
  emu->eip += 1;
  ModRM modrm;
  parse_modrm(emu, &modrm);
  uint32_t r32 = get_r32(emu, &modrm);
  set_rm32(emu, &modrm, r32);
}


/* push r32
   intel のマニュアルでは push r32 は 50+rd */
static void push_r32(Emulator *emu) {
  uint8_t reg = get_code8(emu, 0) - 0x50;
  push32(emu, get_register32(emu, reg));
  emu->eip += 1;
}

/* pop r32
   intel のマニュアルでは pop r32 は 58+rd */
static void pop_r32(Emulator *emu) {
  uint8_t reg = get_code8(emu, 0) - 0x58;
  set_register32(emu, reg, pop32(emu));
  emu->eip += 1;
}

static void push_imm32(Emulator *emu) {
  uint32_t value = get_code32(emu, 1);
  push32(emu, value);
  emu->eip += 5;
}

static void push_imm8(Emulator *emu) {
  uint8_t value = get_code8(emu, 1);
  push32(emu, value);
  emu->eip += 2;
}

static void add_rm32_imm8(Emulator *emu, ModRM *modrm) {
  uint32_t rm32 = get_rm32(emu, modrm);
  uint32_t imm8 = (int32_t)get_sign_code8(emu, 0);
  emu->eip += 1;
  set_rm32(emu, modrm, rm32 + imm8);
}

static void sub_rm32_imm8(Emulator *emu, ModRM *modrm) {
  uint32_t rm32 = get_rm32(emu, modrm);
  uint32_t imm8 = (int32_t)get_sign_code8(emu, 0);
  emu->eip += 1;
  set_rm32(emu, modrm, rm32 - imm8);
}

/* 減算命令(sub)は Mod/RM の REG ビットをオペコードの拡張として使うタイプの命令
   最初の1バイトだけでは実際の命令が決まらない. */
static void code_83(Emulator *emu) {
  emu->eip += 1;
  ModRM modrm;
  parse_modrm(emu, &modrm);

  switch (modrm.opecode) {
  case 0:
    add_rm32_imm8(emu, &modrm);
    break;
  case 5:
    sub_rm32_imm8(emu, &modrm);
    break;
  default:
    printf("not implemented: 83 /%d\n", modrm.opecode);
    exit(1);
  }
}

static void mov_rm32_imm32(Emulator *emu) {
  emu->eip += 1;
  ModRM modrm;
  parse_modrm(emu, &modrm);
  uint32_t value = get_code32(emu, 0);
  emu->eip += 4;
  set_rm32(emu, &modrm, value);
}

static void inc_rm32(Emulator *emu, ModRM *modrm) {
  uint32_t value = get_rm32(emu, modrm);
  set_rm32(emu, modrm, value + 1);
}

/* inc も sub と同じく1バイトのオペコードだけでは決まらないタイプの命令.
   ModR/M の REG ビットが0のときだけ inc だと決まる */
static void code_ff(Emulator *emu) {
  emu->eip += 1;
  ModRM modrm;
  parse_modrm(emu, &modrm);

  switch (modrm.opecode) {
  case 0:
    inc_rm32(emu, &modrm);
    break;
  default:
    printf("not implemented: FF /%d\n", modrm.opecode);
    exit(1);
  }
}

/**
 * rel32 = 「32bitの相対的な番地」
 * call の次の命令の番地を基準にして前後 32bit の範囲でジャンプできる
 */
static void call_rel32(Emulator *emu) {
  int32_t diff = get_sign_code32(emu, 1);
  push32(emu, emu->eip + 5);
  emu->eip += (diff + 5);
}

/**
 * スタックのトップにある 32bit 値が前に実行された call によって
 * push された番地だと思ってそこにジャンプする
 */
static void ret(Emulator *emu) {
  emu->eip = pop32(emu);
}

/**
 * mov esp, ebp と pop ebp をまとめて実行
 */
static void leave(Emulator *emu) {
  uint32_t ebp = get_register32(emu, EBP);
  set_register32(emu, ESP, ebp);
  set_register32(emu, EBP, pop32(emu));
  emu->eip += 1;
}

/**
 * 1バイトのメモリ番地を取る jmp 命令, ショートジャンプ命令に対応
 */
static void short_jump(Emulator *emu) {
  int8_t diff = get_sign_code8(emu, 1);
  emu->eip += (diff + 2);
}

/**
 * ニアジャンプはオペランドのビット数が違うだけでショートジャンプと同じ動作
 */
static void near_jump(Emulator *emu) {
  int32_t diff = get_sign_code32(emu, 1);
  emu->eip += (diff + 5);
}

/**
 * 命令エミュレート用の関数を instructions 配列に登録
 */
void init_instructions(void) {
  int i;
  memset(instructions, 0, sizeof(instructions));
  instructions[0x01] = add_rm32_r32;

  for (i = 0; i < 8; i++) {
    instructions[0x50 + i] = push_r32;
  }

  for (i = 0; i < 8; i++) {
    instructions[0x58 + i] = pop_r32;
  }

  instructions[0x68] = push_imm32;
  instructions[0x6a] = push_imm8;

  instructions[0x83] = code_83;
  instructions[0x89] = mov_rm32_r32;
  instructions[0x8b] = mov_r32_rm32;

  for (i = 0; i < 8; i++) {
    instructions[0xb8 + i] = mov_r32_imm32;
  }

  instructions[0xc3] = ret;
  instructions[0xc7] = mov_rm32_imm32;
  instructions[0xc9] = leave;

  instructions[0xe8] = call_rel32;
  instructions[0xe9] = near_jump;
  instructions[0xeb] = short_jump;
  instructions[0xff] = code_ff;
}
