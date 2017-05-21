#define LOG_DEBUG 1
/*#define ANSI_COLOR_RED     "\x1b[31m"
#define ANSI_COLOR_GREEN   "\x1b[32m"
#define ANSI_COLOR_YELLOW  "\x1b[33m"
#define ANSI_COLOR_BLUE    "\x1b[34m"
#define ANSI_COLOR_MAGENTA "\x1b[35m"
#define ANSI_COLOR_CYAN    "\x1b[36m"
#define ANSI_COLOR_RESET   "\x1b[0m"
#define ANSI_COLOR_GRAY "\x1b[37m"
*/
#define ANSI_COLOR_RED     ""
#define ANSI_COLOR_GREEN   ""
#define ANSI_COLOR_YELLOW  ""
#define ANSI_COLOR_BLUE    ""
#define ANSI_COLOR_MAGENTA ""
#define ANSI_COLOR_CYAN    ""
#define ANSI_COLOR_RESET   ""
#define ANSI_COLOR_GRAY ""


void log_print(const char * fmt, ...);
extern __thread int logd_enable;
#define log(...) ({log_print(ANSI_COLOR_YELLOW __VA_ARGS__); log_print(ANSI_COLOR_RESET);})
#define logd(...) ({if(logd_enable){ if(LOG_DEBUG){log_print(ANSI_COLOR_GRAY); log_print(__VA_ARGS__); log_print(ANSI_COLOR_RESET);}}})
#define loge(...) {log_print(ANSI_COLOR_RED);log_print(__VA_ARGS__); log_print(ANSI_COLOR_GRAY);log_print(" ");}
#define ERROR_TRACE logd( "error: at '" __FILE__  "' line %i: \n",  __LINE__);

// used for error handling
// needs to be implemented in user code
void _error(const char * file, int line, const char * message, ...);
#define ERROR(msg,...) _error(__FILE__,__LINE__,msg, ##__VA_ARGS__)

#ifdef DEBUG
#define ASSERT(expr) if(!(expr)){ERROR("Assertion '" #expr "' Failed");}
#define UNREACHABLE() {ERROR("Should not be reachable");}
#else
#define ASSERT(expr) if(1) if(!(expr)){ERROR("Assertion '" #expr "' Failed");}

#define UNREACHABLE();
#endif

void iron_log_stacktrace();
//extern void (* iron_log_printer)(const char * fnt, va_list lst);
