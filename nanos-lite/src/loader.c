#include <elf.h>
#include <proc.h>
// #include <stdlib.h>
#include <unistd.h>

#ifdef __LP64__
#define Elf_Ehdr Elf64_Ehdr
#define Elf_Phdr Elf64_Phdr
#else
#define Elf_Ehdr Elf32_Ehdr
#define Elf_Phdr Elf32_Phdr
#endif

extern uint8_t ramdisk_start;

extern size_t ramdisk_read(void *buf, size_t offset, size_t len);
extern size_t ramdisk_write(const void *buf, size_t offset, size_t len);

static Elf32_Ehdr *read_elf_header() {
  Elf32_Ehdr *elf_header = (Elf32_Ehdr *)malloc(sizeof(Elf32_Ehdr));
  assert(ramdisk_read(elf_header, 0, sizeof(Elf32_Ehdr)));
  return elf_header;
}

static Elf32_Phdr *read_program_header(Elf32_Ehdr *eh) {
  Elf32_Phdr *ph_table = (Elf32_Phdr *)malloc(eh->e_phnum * eh->e_phentsize);
  uint32_t i;

  for (i = 0; i < eh->e_phnum; i++) {
    assert(ramdisk_read(&ph_table[i],
                        (off_t)(eh->e_phoff + (i * eh->e_phentsize)),
                        eh->e_phentsize) == eh->e_phentsize);
  }

  return ph_table;
}

static bool iself(Elf32_Ehdr *eh) {
  /* ELF magic bytes are 0x7f,'E','L','F'
   * Using  octal escape sequence to represent 0x7f
   */
  if (!strncmp((char *)eh->e_ident,
               "\x7f"
               "ELF",
               4)) {
    /* IS a ELF file */
    return true;
  } else {
    /* Not ELF file */
    return false;
  }
}

static uintptr_t loader(PCB *pcb, const char *filename) {
  Elf32_Ehdr *elf_header = read_elf_header();
  if (!iself(elf_header)) {
    free(elf_header);
    assert("not an elf file!");
  }

  uintptr_t start_addr = elf_header->e_entry;

  Elf32_Phdr *ph_tabl = read_program_header(elf_header);

  for (int i = 0; i < elf_header->e_phnum; i++) {
    Elf32_Phdr ph = ph_tabl[i];
    if (ph.p_type == PT_LOAD) {
      memcpy((void *)ph.p_vaddr, (void *)(&ramdisk_start + ph.p_offset),
             ph.p_filesz);
      memset((void *)(ph.p_vaddr + ph.p_filesz), 0, ph.p_memsz - ph.p_filesz);
    }
  }

  free(elf_header);
  free(ph_tabl);

  return start_addr;
}

void naive_uload(PCB *pcb, const char *filename) {
  uintptr_t entry = loader(pcb, filename);
  Log("Jump to entry = %p", entry);
  ((void (*)())entry)();
}
