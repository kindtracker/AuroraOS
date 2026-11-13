#include <stdint.h>

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;
typedef int8_t s8;
typedef int16_t s16;
typedef int32_t s32;
typedef int64_t s64;

#define VA_CODE 0x00100000u
#define VA_HEAP 0x08000000u
#define VA_STACK_TOP 0x10000000u
#define VA_STACK 0x0FF00000u
#define VA_MAP_BEGIN 0x10000000u
#define VA_MAP_END 0x14000000u
#define LINEAR_DELTA 0x10000000u
#define VA_VRAM 0x1F000000u
#define PA_VRAM 0x18000000u
#define VRAM_SIZE 0x00600000u
#define VA_IO 0x1EC00000u
#define PA_IO 0x10100000u
#define IO_SIZE 0x00400000u
#define VA_CONFIG 0x1FF80000u
#define VA_SHARED 0x1FF81000u
#define VA_TLS 0x1FF82000u
#define TLS_SIZE 0x200u
#define VA_VECTORS 0xFFFF0000u
#define VA_FONT_SOURCE 0x1D000000u
#define PA_MPCORE 0x17E00000u
#define JOB_PARAMS 0x25100000u
#define JOB_CODE 0x25101000u
#define SUP_END 0x25300000u

enum {
  MAP_USER = 1,
  MAP_PRIV = 2,
  MAP_DEVICE = 4,
  MAP_UNCACHED = 8,
  MAP_RO = 16
};

enum { LUNIX_OK, LUNIX_NOT_OK };

void mmu_reset(void);
bool mmu_map(u32 va, u32 pa, u32 size, u32 flags);
void mmu_unmap(u32 va, u32 size);
u32 mmu_pa(u32 va);
bool mmu_user_write_ok(u32 va, u32 size);
bool mmu_user_ok(u32 va, u32 size);
void mmu_enter(void);
void mmu_leave(void);

int LunixLoadProgram(const char *ProgramPath, uint32_t *Entry);
