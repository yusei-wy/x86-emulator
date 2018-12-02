#ifndef EMULATOR_FUNCTION_H_
#define EMULATOR_FUNCTION_H_

#include <stdint.h>

#include "emulator.h"

/* プログラムカウントから相対位置にある符号なし 8bit 値を取得 */
uint32_t get_code8(Emulator *emu, int index);

/* プログラムカウントから相対位置にある符号付き 8bit 値を取得 */
int32_t get_sign_code8(Emulator *emu, int index);

/* プログラムカウントから相対位置にある符号なし 32bit 値を取得 */
uint32_t get_code32(Emulator *emu, int index);

/* プログラムカウンタから相対値位置にある符号付き 32bit 値を取得 */
int32_t get_sign_code32(Emulator *emu, int index);

/* index 番目の 32bit 汎用レジスタの値を取得する */
uint32_t get_register32(Emulator *emu, int index);

/* index 番目の 32bit 汎用レジスタに値を設定する */
void set_register32(Emulator *emu, int index, uint32_t value);

/* メモリの index 番地の 8bit 値を取得する */
uint32_t get_memory8(Emulator *emu, uint32_t address);

/* メモリの index 番地の 32bit 値を取得する */
uint32_t get_memory32(Emulator *emu, uint32_t address);

/* メモリの index 番地に 8bit 値を設定する */
void set_memory8(Emulator *emu, uint32_t address, uint32_t value);

/* メモリの index 番地に 32bit 値を設定する */
void set_memory32(Emulator *emu, uint32_t address, uint32_t value);

/* スタックに 32bit 値を設定する */
void push32(Emulator *emu, uint32_t value);

/* スタックから 32bit 値を取り出す */
uint32_t pop32(Emulator *emu);

#endif
