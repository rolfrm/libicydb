#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <stdint.h>
#include <unistd.h>
#include "types.h"
#include "mem.h"
#include "log.h"
#include "array.h"
#include "fileio.h"
#include "utils.h"
#include "libgen.h"
const char * get_exec_path(){
  static char * buffer = NULL;
  static int size = 0;
  
  if(buffer == NULL){
    buffer = alloc(size = 16);
    while(true){
      int r = readlink("/proc/self/exe", buffer, size);
      if(r <= 0 || r == size)
	buffer = ralloc(buffer, size = size * 2);
      else
	break;
    }
  }
  return buffer;
}

void build_template(int argc, char ** argv, const char * template_file, const char * output_file){
  
  char * name = argv[1];
  logd("Template file: %s\n", template_file);
  char * template = read_file_to_string(template_file);

  template = realloc(template, 1000000);
  replace_inplace(template, "TABLE_NAME", name);
  char *columns1[argc - 2];
  char *columns2[argc - 2];
  //char * column_names[argc - 2];
  char * column_types[argc - 2];
  char * icy_mems[argc - 2];
  char * ptr_attrs1[argc - 2];
  char * ptr_attrs2[argc - 2];
  char * column_sizes[argc - 2];
  char * column_name_strs[argc - 2];
  char * column_pointers[argc - 2];
  for(int i = 2; i < argc; i++){
    char * column = argv[i];
    int splitcnt;    
    char ** parts = string_split(column, ":", &splitcnt);
    ASSERT(splitcnt == 2);
    char * column_name = parts[0];
    char * column_type = parts[1];

    columns1[i - 2] = fmtstr("%s %s", column_type, column_name);
    columns2[i - 2] = fmtstr("%s * %s", column_type, column_name);
    //column_names[i - 2] = column_name;
    column_types[i - 2] = column_type;
    icy_mems[i - 2] = fmtstr("icy_mem * %s_area", column_name);
    ptr_attrs1[i - 2] = fmtstr("(void* )&%s", column_name);
    ptr_attrs2[i - 2] = fmtstr("(void* )%s", column_name);
    column_sizes[i - 2] = fmtstr("sizeof(%s)", column_type);
    column_name_strs[i - 2] = fmtstr("(char *)\"%s\"", column_name);
    column_pointers[i - 2] = fmtstr("(void *)table->%s", column_name);
  }
  char * value_columns = string_join(array_count(columns1), ";\n  ", columns2);
  char * value_columns2 = string_join(array_count(columns1), ", ", columns1);
  char * value_columns3 = string_join(array_count(columns2), ", ", columns2);
  char * areas = string_join(array_count(icy_mems), ";\n  ", icy_mems);
  char * ptrs1 = string_join(array_count(columns2), ", ", ptr_attrs1);
  char * ptrs2 = string_join(array_count(columns2), ", ", ptr_attrs2);
  char * colcount = fmtstr("%i", argc - 2);
  char * colsize = fmtstr("{%s}", string_join(array_count(columns2),", ", column_sizes));
  char * namearray = fmtstr("{%s}", string_join(array_count(columns2),", ", column_name_strs));
  char * columnptrs = fmtstr("{%s}", string_join(array_count(columns2),", ", column_pointers));
  char * column_type_names = fmtstr("{\"%s\"}", string_join(array_count(columns2),"\", \"", column_types));
  replace_inplace(template, "VALUE_COLUMNS1", value_columns);
  replace_inplace(template, "VALUE_COLUMNS2", value_columns2);
  replace_inplace(template, "VALUE_COLUMNS3", value_columns3);
  replace_inplace(template, "MEM_AREAS", areas);
  replace_inplace(template, "INDEX_TYPE", column_types[0]);
  replace_inplace(template, "VALUE_PTRS1", ptrs1);
  replace_inplace(template, "VALUE_PTRS2", ptrs2);
  replace_inplace(template, "COLUMN_COUNT", colcount);
  replace_inplace(template, "COLUMN_SIZES", colsize);
  replace_inplace(template, "COLUMN_NAMES", namearray);
  replace_inplace(template, "COLUMN_TYPE_NAMES", column_type_names);

  replace_inplace(template, "COLUMN_POINTERS", columnptrs);
  write_buffer_to_file(template, strlen(template), output_file);
}

//usage
// compile_table [name] 
int main(int argc, char ** argv){
  logd("program: %s\n", get_exec_path());
  logd("Args:");
  for(int i = 0; i < argc; i++){
    logd("%i %s\n", i, argv[i]);
  }

  const char * dir = get_exec_path();
  dir = dirname((char *)dir);
  char buffer[strlen(dir) + sizeof("icy_vector_template.h") + 5];
  memset(buffer, 0, sizeof(buffer));
  sprintf(buffer, "%s/%s", dir, "icy_vector_template.h");
  build_template(argc, argv, buffer, fmtstr("%s.h", argv[1]));

  sprintf(buffer, "%s/%s", dir, "icy_vector_template.c");
  build_template(argc, argv, buffer, fmtstr("%s.c", argv[1]));
  
}

