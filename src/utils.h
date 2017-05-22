
#define STRINGIZE_DETAIL(x) #x
#define STRINGIZE(x) STRINGIZE_DETAIL(x)

#define array_element_size(array) sizeof(array[0])
#define array_count(array) (sizeof(array)/array_element_size(array))

#define UNUSED(x) (void)(x)

#define auto __auto_type

#define MAX(a,b) \
   ({ auto _a = (a); \
       auto _b = (b); \
     _a > _b ? _a : _b; })

#define MIN(a,b) \
   ({ auto _a = (a); \
       auto _b = (b); \
     _a < _b ? _a : _b; })

#define CLAMP(value, min, max)({					\
      auto _value = value; auto _min = min; auto _max = max;		\
      _value < _min ? _min : _value > _max ? _max : _value;		\
    })

#define ABS(a) ({ auto _a = a; _a < 0 ? -_a : _a;})

// gets the sign of value -1 or 1.
#define SIGN(x) (x < 0 ? -1 : 1)

#define lambda(return_type, body_and_args) \
  ({ \
    return_type __fn__ body_and_args \
    __fn__; \
  })

// swap two variables. Each should be same type
#define SWAP(x,y){ auto tmp = x; x = y; y = tmp;} 


void iron_register_deinitializer(void (* f)(void * data), void * data);
void iron_deinitialize();
