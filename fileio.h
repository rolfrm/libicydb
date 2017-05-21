void * get_format_out();
void format(const char * fmt, ...);
void with_format_out(void * file, void (* fcn)());
void push_format_out(void * file);
void pop_format_out();
// writes a string to a file, replacing the content
size_t write_string_to_file(const char * buffer, const char * filepath);
// Appends a string to the end of a file. includes a 0 on the end.
size_t append_string_to_file(const char * buffer, const char * filepath);
// Write a buffer to a file, erasing the content.
size_t write_buffer_to_file(const void * buffer,size_t s, const char * filepath);
// Appens a buffer to the file, erasing the content.
size_t append_buffer_to_file(const void * buffer, size_t s, const char * filepath);
// file is a FILE *
char * read_stream_to_string(void * file);
char * read_file_to_string(const char * filepath);
void * read_file_to_buffer(const char * filepath, size_t * out_size);

int enter_dir_of(const char * path);
int get_filename(char * buffer, const char * filepath);

void ensure_directory(const char * path);

extern int iron_default_permissions;

// touch a file. (like linux touch command).
void iron_touch(const char * path);
