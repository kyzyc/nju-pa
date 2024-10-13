#include <common.h>
#include <elf.h>
#include <stdlib.h>
#include <unistd.h>

#ifndef CONFIG_TARGET_AM
static FILE *elf_fp = NULL;
static char *str_tbl = NULL;
static Elf32_Sym *sym_tbl = NULL;
static uint32_t symbol_count = 0;

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

static void read_section_header(int32_t fd, Elf32_Ehdr *eh,
                                Elf32_Shdr *sh_table) {
  uint32_t i;

  assert(lseek(fd, (off_t)eh->e_shoff, SEEK_SET) == (off_t)eh->e_shoff);

  for (i = 0; i < eh->e_shnum; i++) {
    assert(read(fd, (void *)&sh_table[i], eh->e_shentsize) == eh->e_shentsize);
  }
}

static void read_elf_header(int32_t fd, Elf32_Ehdr *eh) {
  Assert(lseek(fd, (off_t)0, SEEK_SET) == (off_t)0, "lseek");
  Assert(read(fd, (void *)eh, sizeof(Elf32_Ehdr)) == sizeof(Elf32_Ehdr),
         "read elf header");
}

static char *read_section32(int32_t fd, Elf32_Shdr *sh) {
  char *buff = malloc(sh->sh_size);

  assert(buff != NULL);
  assert(lseek(fd, (off_t)sh->sh_offset, SEEK_SET) == (off_t)sh->sh_offset);
  assert(read(fd, (void *)buff, sh->sh_size) == sh->sh_size);

  return buff;
}

static void read_symbol(int32_t fd, Elf32_Ehdr *eh, Elf32_Shdr *sh,
                         uint32_t symbol_table) {
  //   uint32_t i, symbol_count;

  sym_tbl = (Elf32_Sym *)read_section32(fd, &sh[symbol_table]);

  /* Read linked string-table
   * Section containing the string table having names of
   * symbols of this section
   */
  uint32_t str_tbl_ndx = sh[symbol_table].sh_link;
  str_tbl = read_section32(fd, &sh[str_tbl_ndx]);

  symbol_count = (sh[symbol_table].sh_size / sizeof(Elf32_Sym));

  //   for (i = 0; i < symbol_count; i++) {
  //     printf("0x%08x ", sym_tbl[i].st_value);
  //     printf("0x%02x ", ELF32_ST_BIND(sym_tbl[i].st_info));
  //     printf("0x%02x ", ELF32_ST_TYPE(sym_tbl[i].st_info));
  //     printf("0x%02x ", sym_tbl[i].st_size);
  //     printf("%s\n", (str_tbl + sym_tbl[i].st_name));
  //   }

  return;
}

void init_elf(const char *elf_file) {
  elf_fp = stdout;
  if (elf_file != NULL) {
    FILE *fp = fopen(elf_file, "r");
    Assert(fp, "Can not open '%s'", elf_file);
    elf_fp = fp;
  }

  int32_t fd = fileno(elf_fp);

  Elf32_Ehdr *elf_header = (Elf32_Ehdr *)malloc(sizeof(Elf32_Ehdr));

  read_elf_header(fd, elf_header);

  if (!iself(elf_header)) {
    free(elf_header);
    Assert(0, "not an elf file!");
  }

  Elf32_Shdr *sec_header =
      (Elf32_Shdr *)malloc(elf_header->e_shentsize * elf_header->e_shnum);

  read_section_header(fd, elf_header, sec_header);

  for (int i = 0; i < elf_header->e_shnum; i++) {
    if (sec_header[i].sh_type == SHT_SYMTAB) {
      read_symbol(fd, elf_header, sec_header, i);
    }
  }

  Log("elf is scuessfully parsed!");
}

static int indent_cnt = 0;

#define INDENT printf("  ");
#define PRINT_INDENT(cnt)           \
  do {                              \
    for (int i = 0; i < cnt; i++) { \
      INDENT;                       \
    }                               \
  } while (0)

#define FTRACE_Log_newline(format, ...) \
    _Log(ANSI_FMT(format, ANSI_FG_BLUE) "\n", ## __VA_ARGS__)

#define FTRACE_Log(format, ...) \
    _Log(ANSI_FMT(format, ANSI_FG_BLUE) , ## __VA_ARGS__)

int find_stat(vaddr_t pc, vaddr_t snpc) {
  int pcfid = -1, snpcfid = -1;
  bool snpc_begin = false;

  for (int i = 0; i < symbol_count; i++) {
    if (ELF32_ST_TYPE(sym_tbl[i].st_info) != STT_FUNC) {
      continue;
    }
    uint32_t fstart = sym_tbl[i].st_value;
    if (pc >= fstart && pc < (fstart + sym_tbl[i].st_size)) {
      pcfid = i;
    } else if (snpc >= fstart && snpc < (fstart + sym_tbl[i].st_size)) {
      snpcfid = i;
      if (snpc == fstart) {
        snpc_begin = true;
      } else {
        snpc_begin = false;
      }
    }
  }

  if (pcfid != -1 && snpcfid != -1 && pcfid != snpcfid) {
    if (snpc_begin == true) {
      FTRACE_Log("0x%08x ", pc);
      PRINT_INDENT(indent_cnt);
      FTRACE_Log_newline("call[%s@0x%08x]", (str_tbl + sym_tbl[snpcfid].st_name), snpc);
      indent_cnt++;
    } else {
      FTRACE_Log("0x%08x ", pc);
      PRINT_INDENT(indent_cnt - 1);
      FTRACE_Log_newline("ret[%s]", (str_tbl + sym_tbl[pcfid].st_name));
      indent_cnt--;
    }
  }

  return 0;
}
#endif
