#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// メモリは 1MB
#define MEMORY_SIZE (1024 * 1024)

enum Register {
  EAX,
  ECX,
  EDX,
  EBX,
  ESP,
  EBP,
  ESI,
  EDI,
  REGISTERS_COUNT
};
char *registers_name[] = {
  "EAX",
  "ECX",
  "EDX",
  "EBX",
  "ESP",
  "EBP",
  "ESI",
  "EDI",
};

struct Emulator {
  // 汎用レレジスタ
  uint32_t registers[REGISTERS_COUNT];

  // EFLAGS レジスタ
  uint32_t eflags;

  // メモリ(バイト列)
  uint8_t *memory;

  // プログラムカウント
  uint32_t eip;
};

/**
 * エミュレータを作成する
 */
struct Emulator *create_emu(size_t size, uint32_t eip, uint32_t esp) {
  struct Emulator *emu = malloc(sizeof(struct Emulator));
  emu->memory = malloc(size);

  // 汎用レジスタの初期値を全て0にする
  memset(emu->registers, 0, sizeof(emu->registers));

  // レジスタの初期値を指定されたものにする
  emu->eip = eip;
  emu->registers[ESP] = esp;

  return emu;
}

/**
 * エミュレータを破棄する
 */
static void destroy_emu(struct Emulator *emu) {
  free(emu->memory);
  free(emu);
}

/**
 * 汎用レジスタとプログラムカウンタの値を標準出力に出力する
 */
static void dump_registers(struct Emulator *emu) {
  int i;

  for (i = 0; i < REGISTERS_COUNT; i++) {
    printf("%s = %08x\n", registers_name[i], emu->registers[i]);
  }

  printf("EIP = %08x\n", emu->eip);
}

uint32_t get_code8(struct Emulator *emu, int index) {
  return emu->memory[emu->eip + index];
}

uint32_t get_sign_code8(struct Emulator *emu, int index) {
  return (int8_t)emu->memory[emu->eip + index];
}

uint32_t get_code32(struct Emulator *emu, int index) {
  int i;
  uint32_t ret = 0;

  // リトルエンディアンでメモリの値を取得する
  for (i = 0; i < 4; i++) {
    ret |= get_code8(emu, index + i) << (i * 8);
  }

  return ret;
}

/**
 * 汎用レジスタに32ビットの即値をコピーする mov 命令に対応
 */
void mov_r32_imm32(struct Emulator *emu) {
  uint8_t reg = get_code8(emu, 0) - 0xb8;
  uint32_t value = get_code32(emu, 1);
  emu->registers[reg] = value;
  emu->eip += 5;
}

/**
 * 1バイトのメモリ番地を取る jmp 命令, ショートジャンプ命令に対応
 */
void short_jump(struct Emulator *emu) {
  int8_t diff = get_sign_code8(emu, 1);
  emu->eip += (diff + 2);
}

typedef void instruction_func_t(struct Emulator *);
instruction_func_t *instructions[256];
void init_instructions(void) {
  int i;
  memset(instructions, 0, sizeof(instructions));
  for (i = 0; i < 8; i++) {
    instructions[0xb8 + i] = mov_r32_imm32;
  }
  instructions[0xeb] = short_jump;
}

int main(int argc, char *argv[]) {
  FILE *binary;
  struct Emulator *emu;

  if (argc != 2) {
    printf("usages: px86 [filename]\n");
    return 1;
  }

  // EIP が0、ESP が0x7c00の状態のエミュレータを作る
  emu = create_emu(MEMORY_SIZE, 0x0000, 0x7c00);

  binary = fopen(argv[1], "rb");
  if (binary == NULL) {
    printf("%sファイルを開けません\n", argv[1]);
    return 1;
  }

  // 機械語ファイルを読み込む(最大512バイト)
  fread(emu->memory, 1, 0x200, binary);
  fclose(binary);

  init_instructions();

  while (emu->eip < MEMORY_SIZE) {
    uint8_t code = get_code8(emu, 0);
    // 現在のプログラムカウンタと実行されるバイナリを出力する
    printf("EIP = %X, Code = %02X\n", emu->eip, code);

    if (instructions[code] == NULL) {
      // 実装されていない命令が来たら VM を終了する
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
