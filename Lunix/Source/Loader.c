#include <elf.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "Lunix.h"

#define PAGE_SIZE 0x1000u
#define PAGE_MASK (PAGE_SIZE - 1u)

typedef struct {
  uint8_t *Data;
  size_t Size;
} FileType;

static FileType ReadFile(const char *Path) {
  FILE *File = fopen(Path, "rb");
  if (!File)
    return (FileType){0};

  if (fseek(File, 0, SEEK_END) != 0) {
    fclose(File);
    return (FileType){0};
  }

  long FileSize = ftell(File);
  if (FileSize <= 0) {
    fclose(File);
    return (FileType){0};
  }

  if (fseek(File, 0, SEEK_SET) != 0) {
    fclose(File);
    return (FileType){0};
  }

  uint8_t *Data = malloc((size_t)FileSize);
  if (!Data) {
    fclose(File);
    return (FileType){0};
  }

  if (fread(Data, 1, (size_t)FileSize, File) != (size_t)FileSize) {
    free(Data);
    fclose(File);
    return (FileType){0};
  }

  fclose(File);

  return (FileType){.Data = Data, .Size = (size_t)FileSize};
}

static uint64_t AlignDown(uint64_t Value) {
  return Value & ~(uint64_t)PAGE_MASK;
}

static uint64_t AlignUp(uint64_t Value) {
  return (Value + PAGE_MASK) & ~(uint64_t)PAGE_MASK;
}

int LunixLoadProgram(const char *ProgramPath, uint32_t *Entry) {
  if (!ProgramPath || !Entry)
    return LUNIX_NOT_OK;

  FileType File = ReadFile(ProgramPath);
  if (!File.Data)
    return LUNIX_NOT_OK;

  int Result = LUNIX_NOT_OK;
  void *Allocation = NULL;

  if (File.Size < sizeof(Elf32_Ehdr))
    goto Cleanup;

  Elf32_Ehdr *Header = (Elf32_Ehdr *)File.Data;

  if (memcmp(Header->e_ident, ELFMAG, SELFMAG) != 0)
    goto Cleanup;

  if (Header->e_ident[EI_CLASS] != ELFCLASS32 ||
      Header->e_ident[EI_DATA] != ELFDATA2LSB ||
      Header->e_ident[EI_VERSION] != EV_CURRENT ||
      Header->e_machine != EM_ARM || Header->e_version != EV_CURRENT)
    goto Cleanup;

  if (Header->e_phentsize != sizeof(Elf32_Phdr))
    goto Cleanup;

  uint64_t PHDRStart = Header->e_phoff;
  uint64_t PHDRSize = (uint64_t)Header->e_phnum * sizeof(Elf32_Phdr);

  if (PHDRStart > File.Size || PHDRSize > File.Size - PHDRStart)
    goto Cleanup;

  Elf32_Phdr *PHDRs = (Elf32_Phdr *)(File.Data + Header->e_phoff);

  uint64_t MinAddress = UINT64_MAX;
  uint64_t MaxAddress = 0;

  for (uint32_t i = 0; i < Header->e_phnum; i++) {
    Elf32_Phdr *PH = &PHDRs[i];

    if (PH->p_type != PT_LOAD || PH->p_memsz == 0)
      continue;

    uint64_t SegmentEnd = (uint64_t)PH->p_vaddr + PH->p_memsz;

    if (PH->p_filesz > PH->p_memsz || SegmentEnd > UINT32_MAX ||
        (uint64_t)PH->p_offset + PH->p_filesz > File.Size)
      goto Cleanup;

    uint64_t Start = AlignDown(PH->p_vaddr);
    uint64_t End = AlignUp(SegmentEnd);

    if (Start < MinAddress)
      MinAddress = Start;

    if (End > MaxAddress)
      MaxAddress = End;
  }

  if (MinAddress == UINT64_MAX || MaxAddress <= MinAddress)
    goto Cleanup;

  uint64_t MemorySize = MaxAddress - MinAddress;

  if (MemorySize > UINT32_MAX || MemorySize > SIZE_MAX - PAGE_MASK)
    goto Cleanup;

  size_t AllocationSize = (size_t)MemorySize + PAGE_MASK;

  Allocation = malloc(AllocationSize);
  if (!Allocation)
    goto Cleanup;

  uintptr_t RawAddress = (uintptr_t)Allocation;

  uintptr_t PhysicalAddress = (RawAddress + PAGE_MASK) & ~(uintptr_t)PAGE_MASK;

  if (PhysicalAddress > UINT32_MAX ||
      (uint64_t)PhysicalAddress + MemorySize > (uint64_t)UINT32_MAX + 1)
    goto Cleanup;

  uint8_t *PhysicalMemory = (uint8_t *)PhysicalAddress;

  // Clear the entire address range, including gaps and BSS.
  memset(PhysicalMemory, 0, (size_t)MemorySize);

  // Copy ELF segments into physical memory.
  for (uint32_t i = 0; i < Header->e_phnum; i++) {
    Elf32_Phdr *PH = &PHDRs[i];

    if (PH->p_type != PT_LOAD || PH->p_memsz == 0)
      continue;

    size_t Offset = PH->p_vaddr - (uint32_t)MinAddress;

    memcpy(PhysicalMemory + Offset, File.Data + PH->p_offset, PH->p_filesz);
  }

  if (!mmu_map((uint32_t)MinAddress, (uint32_t)PhysicalAddress,
               (uint32_t)MemorySize, MAP_USER | MAP_PRIV)) {
    fprintf(stderr, "[Lunix] Failed to map ELF memory\n");
    goto Cleanup;
  }

  Allocation = NULL;

  *Entry = Header->e_entry;

  Result = LUNIX_OK;

Cleanup:
  free(Allocation);
  free(File.Data);

  return Result;
}
