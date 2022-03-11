#include "bool.h"
#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>

#define CLASS_TESTS 1

typedef unsigned int uintptr_t;


typedef enum _vartype
{
    V_NULL = 0,
    V_INT,
    V_REAL,
    V_TEXT,
    V_OBJECT,
} VarType;

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

char const *vartype_to_char(VarType type)
{
    switch (type)
    {
    case V_NULL:
        return "NULL";
    case V_INT:
        return "INT";
    case V_REAL:
        return "REAL";
    case V_TEXT:
        return "TEXT";
    case V_OBJECT:
        return "OBJECT";
    }
}

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

typedef struct _namedarg
{
    char const *name;
    VarType type;
    VarValue value;
    struct _namedarg *next;
} NamedArg;

typedef struct _class_member
{
    char const *name;
    VarType type;
    VarValue value;
    AttributeFlag flags;
    struct _class *owner_class;
    struct _class_member *next;
} ClassMember;


// for generic methods we have to control the signature of the function pointer
// for any vtable
// this is why Python has *args and *kwargs as this is the way arguments can be passed
// in a generic way
// We need to be able to pass any type of value back and forth
typedef NamedArg *(*generic_fn)(struct _class *this, NamedArg *input_values);

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

typedef struct _virtual_table
{
    VirtualTableEntry *head;
    int size;
} VirtualTable;

void virtual_table_add_entry(VirtualTable *vtable, VirtualTableEntry *entry)
{
    entry->next = vtable->head;
    vtable->head = entry;
    vtable->size;
}

int formal_argument_size(FormalArgument* arg) {
    int count = 0;
    FormalArgument* runner = arg;
    while ( runner ) {
        count++;
        runner = runner->next;
    }
    return count;
}

char *make_internal_name(VirtualTableEntry *entry)
{
    char *buffer;
    int len = strlen(entry->name);
    int underscore_count = formal_argument_size(entry->formals);
    FormalArgument *runner = entry->formals;
    while (runner)
    {
        len += strlen(vartype_to_char(runner->type));
        runner = runner->next;
    }
    buffer = malloc(sizeof(char) * len + 1);
    memset(buffer, 0, len);
    sprintf(buffer, "%s", entry->name);
    int p = strlen(entry->name);
    runner = entry->formals;
    while (runner)
    {
        buffer[p] = '_';
        p++;
        sprintf(&buffer[p], "%s", vartype_to_char(runner->type));
        p += strlen(vartype_to_char(runner->type));
        runner = runner->next;
    }
    printf("%s\n", buffer);
    return buffer;
}

int named_arg_list_size(NamedArg *head);

utf8* make_internal_name_from_function_call(char const * name, NamedArg* args ) {
    char *buffer;
    int len = strlen(name);
    int underscore_count = named_arg_list_size(args);
    NamedArg *runner = args;
    while (runner)
    {
        len += strlen(vartype_to_char(runner->type));
        runner = runner->next;
    }
    buffer = malloc(sizeof(char) * len + 1);
    memset(buffer, 0, len);
    sprintf(buffer, "%s", name);
    int p = strlen(name);
    runner = args;
    while (runner)
    {
        buffer[p] = '_';
        p++;
        sprintf(&buffer[p], "%s", vartype_to_char(runner->type));
        p += strlen(vartype_to_char(runner->type));
        runner = runner->next;
    }
    printf("%s\n", buffer);
    utf8* text = make_utf8(buffer);
    free(buffer);
    return text;
}

VirtualTableEntry *virtual_table_entry_create(char const *name, generic_fn fn, FormalArgument *formals)
{
    VirtualTableEntry *entry = (VirtualTableEntry *)malloc(sizeof(VirtualTableEntry));
    entry->name = name;
    entry->fn = fn;
    entry->formals = formals;
    entry->next = NULL;
    char* c_name = make_internal_name(entry);
    entry->internal_name = make_utf8(c_name);
    free(c_name);
    return entry;
}

void virtual_table_add_method(VirtualTable *vtable, char const *name, generic_fn fn, FormalArgument *formals)
{
    VirtualTableEntry *entry = virtual_table_entry_create(name, fn, formals);
    virtual_table_add_entry(vtable, entry);
}

VirtualTableEntry *virtual_table_get(VirtualTable *vtable, utf8 *name)
{
    VirtualTableEntry *runner = vtable->head;
    while (runner && utf8_is_equal(runner->internal_name, name))
    {
        runner = runner->next;
    }
    return runner;
}

typedef struct _class
{
    struct _class *parent;
    char const *type;

    // we have generic functions
    VirtualTable *vtable;

    // these methods are for every class
    bool (*is_same_type)(struct _class const *this, struct _class const *other);
    bool (*is_derived)(struct _class const *this, struct _class const *other);
    void (*constructor)(struct _class *this);
    void (*destructor)(struct _class *this);
    bool (*is_equal)(struct _class const *this, struct _class const *other);

    ClassMember* members;
} Class;

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

NamedArg *create_named_argument(char const *name, VarType type, VarValue value)
{
    NamedArg *new_named_arg = malloc(sizeof(NamedArg));
    new_named_arg->name = name;
    new_named_arg->type = type;
    new_named_arg->value = value;
    return new_named_arg;
}

NamedArg *create_integer(char const *name, int x)
{
    VarValue v;
    v.num = x;
    return create_named_argument(name, V_INT, v);
}

NamedArg *create_real(char const *name, double x)
{
    VarValue v;
    v.real = x;
    return create_named_argument(name, V_REAL, v);
}

NamedArg *create_text(char const *name, char *x)
{
    VarValue v;
    v.text = make_utf8(x);
    return create_named_argument(name, V_TEXT, v);
}

// NamedArg* create_string(char const * name, char const * x ) {
//     VarValue v;
//     v.text = x;
//     return create_named_argument(name, V_TEXT, v);
// }

void named_arg_list_add(NamedArg *head, NamedArg *to_add)
{
    NamedArg *runner = head;
    while (runner->next)
    {
        runner = runner->next;
    }
    runner->next = to_add;
}

NamedArg *named_arg_list_get(NamedArg *head, int index)
{
    NamedArg *runner = head;
    while (index)
    {
        runner = runner->next;
        index--;
    }
    return runner;
}

int named_arg_list_size(NamedArg *head)
{
    if (!head)
    {
        return 0;
    }
    NamedArg *runner = head;
    int counter = 1;
    while (runner->next)
    {
        runner = runner->next;
        counter++;
    }
    return counter;
}

int named_arg_print(NamedArg *arg)
{
    if (arg->type == V_NULL)
    {
        printf("NULL");
    }
    else if (arg->type == V_INT)
    {
        printf("%d", arg->value.num);
    }
    else if (arg->type == V_REAL)
    {
        printf("%f", arg->value.real);
    }
    else if (arg->type == V_TEXT)
    {
        printf("%s", arg->value.text->bytes);
    }
}

FormalArgument *formal_argument_create(char const *name, VarType type)
{
    FormalArgument *arg = (FormalArgument *)malloc(sizeof(FormalArgument));
    arg->name = name;
    arg->type = type;
    return arg;
}

void format_argument_list_add(FormalArgument *head, FormalArgument *new_arg)
{
    FormalArgument *runner = head;
    while (runner->next)
    {
        runner = runner->next;
    }
    runner->next = new_arg;
}


void class_default_constructor(struct _class *this)
{
    if (this->parent)
    {
        this->parent->constructor(this->parent);
    }
    printf("Creating new class %p of type %s\n", this, this->type);
}

void class_default_destructor(struct _class *this)
{
    if (this->parent)
    {
        this->parent->destructor(this->parent);
    }
    printf("Destroying class %p of type %s\n", this, this->type);
    free(this);
    this = NULL;
}

bool class_default_is_equal(struct _class const *this, struct _class const *other)
{
    return this == other;
}

bool class_default_is_same_type(struct _class const *this, struct _class const *other)
{
    return (this->type == other->type || strcmp(this->type, other->type) == 0);
}

bool class_default_is_derived(struct _class const *this, struct _class const *other)
{
    bool is_derived = (this->type == other->type || strcmp(this->type, other->type) == 0);
    if (is_derived == False)
    {
        if (this->parent)
        {
            is_derived = class_default_is_derived(this->parent, other);
        }
    }
    return is_derived;
}

void class_add_default_methods(Class *cls)
{
    cls->constructor = class_default_constructor;
    cls->destructor = class_default_destructor;
    cls->is_same_type = class_default_is_same_type;
    cls->is_equal = class_default_is_equal;
    cls->is_derived = class_default_is_derived;
}

Class *class_create(char const *type)
{
    Class *cls = (Class *)malloc(sizeof(Class));
    cls->parent = NULL;
    cls->type = type;
    VirtualTable *vt = (VirtualTable *)malloc(sizeof(VirtualTable *));
    cls->vtable = vt;
    class_add_default_methods(cls);
    return cls;
}

NamedArg *class_call_method(Class *cls, char const *name, NamedArg *args)
{
    utf8* internal_name = make_internal_name_from_function_call(name, args);
    VirtualTableEntry *method = virtual_table_get(cls->vtable, internal_name);
    if (!method)
    {
        printf("No method called: %s\n", internal_name->bytes);
        return NULL;
    }
    return method->fn(cls, args);
}


NamedArg *class_method_print(struct _class *this, NamedArg *input_values)
{
    int n = named_arg_list_size(input_values);
    for (int ii = 0; ii < n; ii++)
    {
        NamedArg *arg = named_arg_list_get(input_values, ii);
        named_arg_print(arg);
        printf("\n");
    }
    return NULL;
}

// we can define some macros to make it a bit less wordy
#define CONSTRUCT(x) x->constructor(x);
#define DESTROY(x) x->destructor(x)
#define IS_SAME_TYPE(x, y) x->is_same_type(x, y)
#define IS_EQUAL(x, y) x->is_equal(x, y)
#define IS_DERIVED(x, y) x->is_derived(x, y)

Class *subclass_create(char const *parent_type, char const *child_type)
{
    Class *parent = class_create(parent_type);
    Class *child = class_create(child_type);
    child->parent = parent;
    return child;
};

#ifdef CLASS_TESTS

void argument_example()
{
    FormalArgument *farg0 = formal_argument_create("a", V_INT);
    FormalArgument *farg1 = formal_argument_create("b", V_INT);
    format_argument_list_add(farg0, farg1);

    Class *new_dog_1 = class_create("Dog");
    // in this version we did not need to give any arguments though so we cannot do overloading
    virtual_table_add_method(new_dog_1->vtable, "print", class_method_print, farg0);

    FormalArgument *farg2 = formal_argument_create("a", V_TEXT);
    FormalArgument *farg3 = formal_argument_create("b", V_TEXT);
    format_argument_list_add(farg2, farg3);

    // in this version we did not need to give any arguments though so we cannot do overloading
    virtual_table_add_method(new_dog_1->vtable, "print", class_method_print, farg2);

    NamedArg *arg0 = create_integer("a", 1);
    NamedArg *arg1 = create_integer("b", 2);
    named_arg_list_add(arg0, arg1);

    class_call_method(new_dog_1, "print", arg0);

    NamedArg *arg2 = create_text("a", "hello");
    NamedArg *arg3 = create_text("b", "goodbye");
    named_arg_list_add(arg2, arg3);

    class_call_method(new_dog_1, "print", arg2);
}

void utf8_example() {
    utf8* text = make_utf8("hello");
    printf("%d %d %d %s\n", text->hash, text->byte_count, text->character_count, text->bytes);
    utf8_destroy(&text);
    utf8* text2 = make_utf8("\xe6\x97\xa5\xd1\x88");
    printf("%d %d %d %s\n", text2->hash, text2->byte_count, text2->character_count, text2->bytes);
    utf8_destroy(&text2);

    utf8* a = make_utf8("here");
    utf8* b = make_utf8("here");
    utf8* c = make_utf8("herE");
    assert(utf8_is_equal(a,b) == True);
    assert(utf8_is_equal(a,c) == False);
}

int main()
{
    Class *new_dog_1 = class_create("Dog");
    Class *new_dog_2 = class_create("Dog");
    CONSTRUCT(new_dog_1);
    CONSTRUCT(new_dog_2);
    assert(IS_SAME_TYPE(new_dog_1, new_dog_2) == True);
    assert(IS_EQUAL(new_dog_1, new_dog_2) == False);
    DESTROY(new_dog_1);
    DESTROY(new_dog_2);

    Class *new_dog_3 = class_create("Dog");
    Class *golden_retriever = subclass_create("Dog", "GoldenRetriever");
    CONSTRUCT(new_dog_3);
    CONSTRUCT(golden_retriever);
    assert(IS_DERIVED(golden_retriever, new_dog_3));
    assert(IS_DERIVED(new_dog_3, golden_retriever) == False);

    argument_example();
    utf8_example();
}
#endif