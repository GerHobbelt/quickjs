#pragma once

#ifdef __cplusplus
extern "C" {
#endif

/*
  The default dump channel is a FILE* type: `stderr`, but you can override this by #define-ing
  QJS_DUMP_OUTPUT to any type you prefer.
*/
#ifndef QJS_DUMP_OUTPUT
// Note that we include the '*' in here: users can define output types as simple as `int` and
// SHOULD NOT be bothered by having to taking the address-of (dereferencing), etc. in our SET/GET
// I/F calls.
#define QJS_DUMP_OUTPUT         FILE*
#define HAS_DEFAULT_QJS_DUMP_OUTPUT 1
#endif

/*
   The DUMP_* flag bits: all named QJS_DUMP_FLAG_* while the *configuration* #define's
   are named DUMP_*: those decide at compile time which dump types will be permitted,
   while the QJS_DUMP_FLAG_* configuration bits decide which dump types will actually
   be executed (turned ON) at RUN-TIME.
 */

enum qjs_dump_flags {
	/* DUMP_BYTECODE - Define this if you want to be able to dump bytecode */
		/* dump the bytecode of the compiled functions: combination of bits
		   1: dump pass 3 final byte code
		   2: dump pass 2 code
		   4: dump pass 1 code
		   8: dump stdlib functions
		  16: dump bytecode in hex
		  32: dump line number table
		 */
	QJS_DUMP_FLAG_BYTECODE_MASK = 0x003F,

	QJS_DUMP_FLAG_BYTECODE_PASS3 = 0x0001,
	QJS_DUMP_FLAG_BYTECODE_PASS2 = 0x0002,
	QJS_DUMP_FLAG_BYTECODE_PASS1 = 0x0004,
	QJS_DUMP_FLAG_BYTECODE_STDLIB = 0x0008,
	QJS_DUMP_FLAG_BYTECODE_HEX = 0x0010,
	QJS_DUMP_FLAG_BYTECODE_LINETABLE = 0x0020,

	/* DUMP_LEAKS - Define this if you want to dump memory leaks; set to compile-time DUMP_LEAKS to 2 for more dumping */
	QJS_DUMP_FLAG_LEAKS = 0x0040,

	/* DUMP_ATOMS - Define this if you want to dump atoms when freeing context */
	QJS_DUMP_FLAG_ATOMS = 0x0080,

	/* DUMP_SHAPES - Define this if you want to dump shapes when freeing context */
	QJS_DUMP_FLAG_SHAPES = 0x0100,

	/* DUMP_OBJECTS - Define this if you want to dump objects when freeing context */
	QJS_DUMP_FLAG_OBJECTS = 0x0200,

	/* DUMP_MEM - Define this if you want to dump memory when freeing context */
	QJS_DUMP_FLAG_MEM = 0x0400,

	/* DUMP_CLOSURE  */
	QJS_DUMP_FLAG_CLOSURE = 0x0800,

	/* DUMP_FREE - Define this if you want to dump a message on freeing objects */
	QJS_DUMP_FLAG_FREE = 0x1000,

	/* DUMP_GC - Define this if you want to dump a message on garbage collection */
	QJS_DUMP_FLAG_GC = 0x2000,

	/* DUMP_GC_FREE - Define this if you want to dump a message when the garbage collector frees a resource */
	QJS_DUMP_FLAG_GC_FREE = 0x4000,

	/* DUMP_MODULE_RESOLVE - Define this if you want to debug module resolution */
	QJS_DUMP_FLAG_MODULE_RESOLVE = 0x00008000,

	/* DUMP_PROMISE - Define this if you want to debug promises */
	QJS_DUMP_FLAG_PROMISE = 0x00010000,

	/* DUMP_READ_OBJECT - Define this if you want to debug binary object reader */
	QJS_DUMP_FLAG_READ_OBJECT = 0x00020000,

	QJS_DUMP_FLAG_ALL = 0x0003FFFF,
};

struct qjs_dump_flags_keyword
{
	enum qjs_dump_flags bits;
	const char* keyword;
};

/*
  NIL-entry (.keyword==NULL) delimited list of recognized dump flag keywords.

  Keywords with a NIL `bits` value are special and processed by the provided CLI CALLBACK.
*/
extern const struct qjs_dump_flags_keyword qjs_dump_flags_keyword_list[];

/*
  To set the dump flags, use the CLEAR, then SET call. Use the UNSET and SET calls
  to adjust the current dump flags.
*/
extern void qjs_clear_dump_flags(void);
extern void qjs_set_dump_flags(enum qjs_dump_flags enable_flags_combo);
extern void qjs_unset_dump_flags(enum qjs_dump_flags disable_flags_combo);
extern enum qjs_dump_flags qjs_get_current_dump_flags(void);

// simple helper for quickjs et al:
inline int qjs_test_dump_flag(enum qjs_dump_flags flag_to_test_for);

// set up the dump output channel; STDERR by default
extern void qjs_set_dump_output_channel(QJS_DUMP_OUTPUT output_channel);
extern QJS_DUMP_OUTPUT qjs_get_current_dump_output_channel(void);

/*
  To translate dump flags specified as a (comma/semicolon/colon/pipe-separated)
  string, use the PARSE call.

  Note that any unrecognized keyword can be parsed by a user-specified CALLBACK,
  which can take any action, including tweaking the dump flags passed into that
  CALLBACK as an additional reference parameter.
  The CALLBACK must return non-zero(TRUE/ERROR) when it did not process the keyword,
  or zero(FALSE/SUCCESS) when the keyword was successfully processed.

  The callback MAY also set the output channel by setting up the QJS_DUMP_OUTPUT reference
  that's passed as an extra parameter: QJS and QJSC use this to set up dumping
  to stderr or stdout, using the provided `qjs_parse_dump_flags_default_cli_callback()`,
  for example.

  The PARSE call does not modify any currently active flags; use the SET calls
  to change the active settings (flags and output channel) to the values returned
  by the PARSE call.
*/
typedef int qjs_dump_flags_parse_callback_f(const char* keyword, size_t keyword_length, enum qjs_dump_flags* current_flags, QJS_DUMP_OUTPUT* output_channel_ref);

extern enum qjs_dump_flags qjs_parse_dump_flags(const char* delimited_keywords, qjs_dump_flags_parse_callback_f* unrecognized_keyword_handler, QJS_DUMP_OUTPUT* output_channel_ref);

// default/sample provided qjs_parse_dump_flags_default_cli_callback CALLBACK:
extern int qjs_parse_dump_flags_default_cli_callback(const char* keyword, size_t keyword_length, enum qjs_dump_flags* current_flags, QJS_DUMP_OUTPUT* output_channel_ref);


#ifdef __cplusplus
} /* extern "C" { */
#endif

