#ifndef MODRM_H_
#define MODRM_H_

#include <stdint.h>
#include "emulator.h"

// ModR/M を表す構造体
typedef struct {
  uint8_t mod;

  // opecode と reg_index は別名で同じもの
  union {
    uint8_t opecode;
    uint8_t reg_index;
  };

  uint8_t rm;

  // SIB が必要な mod/rm の組み合わせのときに使う
  uint8_t sib;

  union {
    int8_t disp8; // disp8 は符号付き整数
    uint32_t disp32;
  };
} ModRM;

/**
 * ModR/M, SIB, ディスプレースメントを解析する
 *
 * emu から ModR/M, SIB, ディスプレースメントを読み取って modrm にセットする。
 * 呼び出したとき emu->eip は ModR/M バイトを指している必要がある。
 * この関数は emu->eip を即値（即値がない場合は次の命令）の先頭を指すように変更する。
 *
 * args
 *    emu: eip が ModR/M バイトの先頭を指しているエミュレータ構造体
 *    modrm: 解析結果を格納する構造体
 */
void parse_modrm(Emulator *emu, ModRM *modrm);

/**
 * ModR/M の内容に基づきメモリの実効アドレスを計算する
 * 
 * modrm->mod は 0,1,2 のいずれかでなければならない
 */
uint32_t calc_memory_address(Emulator *emu, ModRM *modrm);

/**
 * rm32 のレジスタまたはメモリの 32bit 値を取得する
 */
uint32_t get_rm32(Emulator *emu, ModRM *modrm);

/**
 * rm32 のレジスタまたはメモリの 32bit 値を設定する
 *
 * modrm の内容に従って value を目的のメモリまたはレジスタに書き込む
 *
 * args
 *    emu: エミュレータ構造体（eip はどこを指していてもいい）
 *    modrm: ModR/M (SIB, disp を含む)
 *    value: 即値
 */
void set_rm32(Emulator *emu, ModRM *modrm, uint32_t value);

/**
 * r32 のレジスタの 32bit 値を取得する
 */
uint32_t get_r32(Emulator *emu, ModRM *modrm);

/**
 * r32 のレジスタの 32bit 値を設定する
 */
void set_r32(Emulator *emu, ModRM *modrm, uint32_t value);

#endif
