#include "utf8.h"

typedef unsigned int uintptr_t;



typedef enum _vartype
{
    V_NULL = 0,
    V_INT,
    V_REAL,
    V_TEXT,
    V_OBJECT,
} VarType;

char const *vartype_to_char(VarType type);





typedef enum _attribute_flag
{
    V_PUBLIC    = 0,
    V_PRIVATE   = 1,
    V_PROTECTED = 2,
    V_STATIC    = 4,
    V_MUTABLE   = 8
} AttributeFlag;

#define IS_PUBLIC_VAR(x)        x & V_PUBLIC    == V_PUBLIC   
#define IS_PRIVATE_VAR(x)       x & V_PRIVATE   == V_PRIVATE
#define IS_PROTECTED_VAR(x)     x & V_PROTECTED == V_PROTECTED
#define IS_STATIC_VAR(x)        x & V_STATIC    == V_STATIC
#define IS_MUTABLE_VAR(x)       x & V_MUTABLE   == V_MUTABLE
#define IS_CONSTANT_VAR(x)      x & V_MUTABLE   != V_MUTABLE

typedef enum _method_flag
{
    M_PUBLIC    = 0,
    M_PRIVATE   = 1,
    M_PROTECTED = 2,
    M_STATIC    = 4
} MethodFlag;

#define IS_PUBLIC_METHOD(x)        x & M_PUBLIC    == M_PUBLIC   
#define IS_PRIVATE_METHOD(x)       x & M_PRIVATE   == M_PRIVATE
#define IS_PROTECTED_METHOD(x)     x & M_PROTECTED == M_PROTECTED
#define IS_STATIC_METHOD(x)        x & M_STATIC    == M_STATIC


typedef union _variable_value
{
    int num;
    double real;
    utf8 *text;
    struct _class *object;
} VarValue;




typedef struct _formal_argument
{
    char const *name;
    VarType type;
    struct _formal_argument *next;
} FormalArgument;

FormalArgument *formal_argument_create(char const *name, VarType type);
void format_argument_list_add(FormalArgument *head, FormalArgument *new_arg);
int formal_argument_size(FormalArgument* arg);






typedef struct _namedarg
{
    char const *name;
    VarType type;
    VarValue value;
    struct _namedarg *next;
} NamedArg;

NamedArg *create_named_argument(char const *name, VarType type, VarValue value);
NamedArg *create_integer(char const *name, int x);
NamedArg *create_real(char const *name, double x);
NamedArg *create_text(char const *name, char *x);
void named_arg_list_add(NamedArg *head, NamedArg *to_add);
NamedArg *named_arg_list_get(NamedArg *head, int index);
int named_arg_list_size(NamedArg *head);
int named_arg_print(NamedArg *arg);



typedef struct _class_member
{
    char const *name;
    VarType type;
    VarValue value;
    AttributeFlag flags;
    struct _class *owner_class;
    struct _class_member *next;
} ClassMember;

/************************************************************
 * Virtual Table allowing for dispatch
 * Allows overloading through creating an internal name
 * that includes the function name plus types of arguments.
 ************************************************************/


// for generic methods we have to control the signature of the function pointer
// for any vtable
// self is why Python has *args and *kwargs as self is the way arguments can be passed
// in a generic way
// We need to be able to pass any type of value back and forth
typedef NamedArg *(*generic_fn)(struct _class *self, NamedArg *input_values);

typedef struct _virtual_table_entry
{
    MethodFlag flags;
    utf8* internal_name;
    char const *name;
    generic_fn fn;
    FormalArgument *formals;
    struct _class *owner_class;
    struct _virtual_table_entry *next;
} VirtualTableEntry;

char *make_internal_name(VirtualTableEntry *entry);
utf8* make_internal_name_from_function_call(char const * name, NamedArg* args );
VirtualTableEntry *virtual_table_entry_create(char const *name, generic_fn fn, FormalArgument *formals);

typedef struct _virtual_table
{
    VirtualTableEntry *head;
    int size;
} VirtualTable;

void virtual_table_add_entry(VirtualTable *vtable, VirtualTableEntry *entry);
void virtual_table_add_method(VirtualTable *vtable, char const *name, generic_fn fn, FormalArgument *formals);
VirtualTableEntry *virtual_table_get(VirtualTable *vtable, utf8 *name);




/************************************************************
 * Class Object
 ************************************************************/

typedef struct _class
{
    struct _class *parent;
    char const *type;

    // we have generic functions
    VirtualTable *vtable;

    // these methods are for every class
    bool (*is_same_type)(struct _class const *self, struct _class const *other);
    bool (*is_derived)(struct _class const *self, struct _class const *other);
    void (*constructor)(struct _class *self);
    void (*destructor)(struct _class *self);
    bool (*is_equal)(struct _class const *self, struct _class const *other);

    ClassMember* members;
} Class;

void class_default_constructor(struct _class *self);
void class_default_destructor(struct _class *self);
bool class_default_is_equal(struct _class const *self, struct _class const *other);
bool class_default_is_same_type(struct _class const *self, struct _class const *other);
bool class_default_is_derived(struct _class const *self, struct _class const *other);
void class_add_default_methods(Class *cls);
Class *class_create(char const *type);
Class *subclass_create(char const *parent_type, char const *child_type);
NamedArg *class_call_method(Class *cls, char const *name, NamedArg *args);
NamedArg *class_method_print(struct _class *self, NamedArg *input_values);

// we can define some macros to make it a bit less wordy
#define CONSTRUCT(x) x->constructor(x);
#define DESTROY(x) x->destructor(x)
#define IS_SAME_TYPE(x, y) x->is_same_type(x, y)
#define IS_EQUAL(x, y) x->is_equal(x, y)
#define IS_DERIVED(x, y) x->is_derived(x, y)





typedef struct _environment 
{
    // we can use the same mechanism to store 
    // local variables, static members, static methods, functions
    // all in the same place by manipulating the flags and 
    // setting owner_class to either NULL or non-NULL for class-owned items.
    ClassMember*    members;
    VirtualTable*   methods;
    struct _environment * next;
} Environment;



