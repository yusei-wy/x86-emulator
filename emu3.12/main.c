#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "emulator.h"
#include "emulator_function.h"
#include "instruction.h"

// メモリは 1MB
#define MEMORY_SIZE (1024 * 1024)

char *registers_name[] = {
    "EAX", "ECX", "EDX", "EBX", "ESP", "EBP", "ESI", "EDI",
};

/**
 * Emulator のメモリにバイナリファルの内容を512バイトコピーする
 */
static void read_binary(Emulator *emu, const char *filename) {
  FILE *binary;

  binary = fopen(filename, "rb");

  if (binary == NULL) {
    printf("%s not opend file\n", filename);
    exit(1);
  }

  fread(emu->memory + 0x7c00, 1, 0x200, binary);
  fclose(binary);
}

/**
 * 汎用レジスタとプログラムカウンタの値を標準出力に出力する
 */
static void dump_registers(Emulator *emu) {
  int i;

  for (i = 0; i < REGISTERS_COUNT; i++)
    printf("%s = %08x\n", registers_name[i], get_register32(emu, i));

  printf("EIP = %08x\n", emu->eip);
}

/**
 * エミュレータを作成する
 */
Emulator *create_emu(size_t size, uint32_t eip, uint32_t esp) {
  Emulator *emu = malloc(sizeof(Emulator));

  emu->memory = malloc(size);

  memset(emu->registers, 0, sizeof(emu->registers));

  // レジスタの初期値を指定されたものにする
  emu->eip = eip;
  emu->registers[ESP] = esp;

  return emu;
}

/**
 * エミュレータを破棄する
 */
static void destroy_emu(Emulator *emu) {
  free(emu->memory);
  free(emu);
}

int opt_remove_at(int argc, char *argv[], int index) {
  if (index < 0 || argc <= index) {
    return argc;
  } else {
    int i = index;
    for (; i < argc - 1; i++)
      argv[i] = argv[i + 1];
    argv[i] = NULL;
    return argc - 1;
  }
}

int main(int argc, char *argv[]) {
  Emulator *emu;
  int i;
  int quiet = 0;

  i = 1;
  while (i < argc) {
    if (strcmp(argv[i], "-q") == 0) {
      quiet = 1;
      argc = opt_remove_at(argc, argv, i);
    } else {
      i++;
    }
  }

  // 引数が1つ以上でなければエラー
  if (argc != 2) {
    printf("usages: px86 [filename]\n");
    return 1;
  }

  // 命令セットの初期化
  init_instructions();

  // メモリ 1MB で EIP, ESP が0x7C00の状態の Emulator を作る
  emu = create_emu(MEMORY_SIZE, 0x7C00, 0x7C00);

  // 引数で与えられたバイナリを読む
  read_binary(emu, argv[1]);

  while (emu->eip < MEMORY_SIZE) {
    uint8_t code = get_code8(emu, 0);
    // 現在のプログラムカウンタと実行されるバイナリを出力する
    if (!quiet)
      printf("EIP = %X, Code = %02X\n", emu->eip, code);

    if (instructions[code] == NULL) {
      // 実装されていない命令が来たら Emulator を終了する
      printf("\n\nNot Implemented: %x\n", code);
      break;
    }

    // 命令の実行
    instructions[code](emu);

    // EIP が0になったらプログラム終了
    if (emu->eip == 0x00) {
      printf("\n\nend of program\n\n");
      break;
    }
  }

  dump_registers(emu);
  destroy_emu(emu);
  return 0;
}
