#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include "instruction.h"
#include "emulator.h"
#include "emulator_function.h"
#include "modrm.h"

instruction_func_t *instructions[256];

/**
 * mov r32, imm32
 */
static void mov_r32_imm32(Emulator *emu) {
  uint8_t reg = get_code8(emu, 0) - 0xb8;
  uint32_t value = get_code32(emu, 1);
  emu->registers[reg] = value;
  emu->eip += 5;
}

/**
 * mov r32, rm32
 */
static void mov_r32_rm32(Emulator *emu) {
  emu->eip += 1;
  ModRM modrm;
  parse_modrm(emu, &modrm);
  uint32_t rm32 = get_rm32(emu, &modrm);
  set_r32(emu, &modrm, rm32);
}

/**
 * add rm32, r32
 */
static void add_rm32_r32(Emulator *emu) {
  emu->eip += 1;
  ModRM modrm;
  parse_modrm(emu, &modrm);
  uint32_t r32 = get_r32(emu, &modrm);
  uint32_t rm32 = get_rm32(emu, &modrm);
  set_rm32(emu, &modrm, rm32 + r32);
}

/**
 * mov rm32, r32
 */
static void mov_rm32_r32(Emulator *emu) {
  emu->eip += 1;
  ModRM modrm;
  parse_modrm(emu, &modrm);
  uint32_t r32 = get_r32(emu, &modrm);
  set_rm32(emu, &modrm, r32);
}

/**
 * sub rm32, imm8
 */
static void sub_rm32_imm8(Emulator *emu, ModRM *modrm) {
  uint32_t rm32 = get_rm32(emu, modrm);
  uint32_t imm8 = (int32_t)get_sign_code8(emu, 0);
  emu->eip += 1;
  set_rm32(emu, modrm, rm32 - imm8);
}

/**
 * 減算命令(sub)は Mod/RM の REG ビットをオペコードの拡張として使うタイプの命令,
 * 最初の1バイトだけでは実際の命令が決まらない.
 */
static void code_83(Emulator *emu) {
  emu->eip += 1;
  ModRM modrm;
  parse_modrm(emu, &modrm);

  switch (modrm.opecode) {
  case 5:
    sub_rm32_imm8(emu, &modrm);
    break;
  default:
    printf("not implemented: 83 /%d\n", modrm.opecode);
    exit(1);
  }
}

/**
 * mov rm32, imm32
 */
static void mov_rm32_imm32(Emulator *emu) {
  emu->eip += 1;
  ModRM modrm;
  parse_modrm(emu, &modrm);
  uint32_t value = get_code32(emu, 0);
  emu->eip += 4;
  set_rm32(emu, &modrm, value);
}

/**
 * inc rm32
 */
static void inc_rm32(Emulator *emu, ModRM *modrm) {
  uint32_t value = get_rm32(emu, modrm);
  set_rm32(emu, modrm, value + 1);
}

/**
 * inc も sub と同じく1バイトのオペコードだけでは決まらないタイプの命令.
 * ModR/M の REG ビットが0のときだけ inc だと決まる
 */
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
  instructions[0x83] = code_83;
  instructions[0x89] = mov_rm32_r32;
  instructions[0x8B] = mov_r32_rm32;
  for (i = 0; i < 8; i++) {
    instructions[0xB8 + i] = mov_r32_imm32;
  }
  instructions[0xC7] = mov_rm32_imm32;
  instructions[0xE9] = near_jump;
  instructions[0xEB] = short_jump;
  instructions[0xFF] = code_ff;
}
