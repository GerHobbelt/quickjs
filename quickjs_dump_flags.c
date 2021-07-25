
#include "quickjs.h"

#include <string.h>
#include <stdarg.h>


static enum qjs_dump_flags current_dump_flags = 0;
static QJS_DUMP_OUTPUT current_dump_output_channel = NULL;

const struct qjs_dump_flags_keyword qjs_dump_flags_keyword_list[] =
{
	{        QJS_DUMP_FLAG_ALL,        "all" },

	{        QJS_DUMP_FLAG_BYTECODE_MASK,        "bytecode" },
	{        QJS_DUMP_FLAG_BYTECODE_PASS3,        "bytecode_pass3" },
	{        QJS_DUMP_FLAG_BYTECODE_PASS2,        "bytecode_pass2" },
	{        QJS_DUMP_FLAG_BYTECODE_PASS1,        "bytecode_pass1" },
	{        QJS_DUMP_FLAG_BYTECODE_STDLIB,        "bytecode_stdlib" },
	{        QJS_DUMP_FLAG_BYTECODE_HEX,        "bytecode_hex" },
	{        QJS_DUMP_FLAG_BYTECODE_LINETABLE,        "bytecode_linetable" },
	{        QJS_DUMP_FLAG_LEAKS,        "leaks" },
	{        QJS_DUMP_FLAG_ATOMS,        "atoms" },
	{        QJS_DUMP_FLAG_SHAPES,        "shapes" },
	{        QJS_DUMP_FLAG_OBJECTS,        "objects" },
	{        QJS_DUMP_FLAG_MEM,        "mem" },
	{        QJS_DUMP_FLAG_CLOSURE,        "closure" },
	{        QJS_DUMP_FLAG_FREE,        "free" },
	{        QJS_DUMP_FLAG_GC,        "gc" },
	{        QJS_DUMP_FLAG_GC_FREE,        "gc_free" },
	{        QJS_DUMP_FLAG_MODULE_RESOLVE,        "module_resolve" },
	{        QJS_DUMP_FLAG_PROMISE,        "promise" },
	{        QJS_DUMP_FLAG_READ_OBJECT,        "read_object" },

	// sentinel
	{        0,        NULL }
};

void qjs_clear_dump_flags(void)
{
	current_dump_flags = 0;
}

void qjs_set_dump_flags(enum qjs_dump_flags enable_flags_combo)
{
	current_dump_flags |= enable_flags_combo;
}

void qjs_unset_dump_flags(enum qjs_dump_flags disable_flags_combo)
{
	current_dump_flags &= disable_flags_combo;
}

enum qjs_dump_flags qjs_get_current_dump_flags(void)
{
	return current_dump_flags;
}

// simple helper:
int qjs_test_dump_flag(enum qjs_dump_flags flag_to_test_for)
{
	return (current_dump_flags & flag_to_test_for);
}

void qjs_set_dump_output_channel(QJS_DUMP_OUTPUT output_channel)
{
	current_dump_output_channel = output_channel;
}

QJS_DUMP_OUTPUT qjs_get_current_dump_output_channel(void)
{
#if defined(HAS_DEFAULT_QJS_DUMP_OUTPUT)
	return current_dump_output_channel ? current_dump_output_channel : stderr;
#else
	// return any user-overridden type value as-is: we won't know anything about 'sensible default' here
	// so it's the user's responsibility to handle that scenario in userland code.
	return current_dump_output_channel;
#endif
}

/*
  To translate dump flags specified as a (comma/semicolon/colon/pipe-separated)
  string, use the PARSE call.

  Note that any unrecognized keyword can be parsed by a user-specified CALLBACK,
  which can take any action, including tweaking the dump flags passed into that
  CALLBACK as an additional parameter.
  The CALLBACK must return the (possibly adjusted) flag set as return value.

  The callback MAY also set the output channel by setting up the QJS_DUMP_OUTPUT reference
  that's passed as an extra parameter: QJS and QJSC use this to set up dumping
  to stderr or stdout, using the provided `qjs_parse_dump_flags_default_cli_callback()`,
  for example.

  The PARSE call does not modify any currently active flags; use the SET calls
  to change the active settings (flags and output channel) to the values returned
  by the PARSE call.
*/

static int matches_keyword(const char* blurb, size_t blurb_len, const char* keyword)
{
#undef MAX
#define MAX(a, b) ((a) > (b) ? (a) : (b))

	size_t l = strlen(keyword);
	return !strnicmp(blurb, keyword, MAX(l, blurb_len));
}

enum qjs_dump_flags qjs_parse_dump_flags(const char* delimited_keywords, qjs_dump_flags_parse_callback_f* unrecognized_keyword_handler, QJS_DUMP_OUTPUT* output_channel_ref)
{
	static const char* delims = ",;:|";
	enum qjs_dump_flags flags = 0;

	while (delimited_keywords && *delimited_keywords)
	{
		// is the keyword preceded by '!' or '~' ? --> negate!
		int negate = 0;
		while (strchr("!~", *delimited_keywords))
		{
			negate = !negate;
			delimited_keywords++;
		}

		// got a keyword to decode?
		size_t l = strcspn(delimited_keywords, delims);
		if (l > 0)
		{
			const struct qjs_dump_flags_keyword* kwd = qjs_dump_flags_keyword_list;
			int rv = -1;

			// check if the keyword matches any in the list.
			//
			// Unhandled ones and NIL entries are deferred to the user-provided callback, if any.
			//
			// Do note that the definition list has all the NIL specials at the end: those are there
			// only to aid CLI/on-line help output routines, but are NOT processed by this call!
			while (kwd->keyword)
			{
				if (matches_keyword(delimited_keywords, l, kwd->keyword))
				{
					// MATCH! --> process if this sets any flags:
					enum qjs_dump_flags b = kwd->bits;
					if (b)
					{
						if (negate)
						{
							flags &= ~b;
						}
						else
						{
							flags |= b;
						}
						rv = 0;
					}
					else
					{
						// defer NIL-bit slots to the callback, if any.
					}
					// all done with this bit of the input
					break;
				}

				kwd++;
			}

			// process all NIL-bits and unmatched words in the callback:
			if (rv && unrecognized_keyword_handler)
			{
				rv = (*unrecognized_keyword_handler)(delimited_keywords, l, negate, &flags, output_channel_ref);
			}
			if (rv)
			{
				fprintf(stderr, "Error: could not process dump flag '%.*s'.\n", (int)l, delimited_keywords);
			}
		}

		// now skip the delimiter(s) until the start of the next keyword (or EOF).
		delimited_keywords += l;
		l = strspn(delimited_keywords, delims);
		delimited_keywords += l;
	}

	return flags;
}

// default/sample provided qjs_parse_dump_flags_default_cli_callback CALLBACK:
int qjs_parse_dump_flags_default_cli_callback(const char* keyword, size_t keyword_length, int negate, enum qjs_dump_flags *current_flags, QJS_DUMP_OUTPUT* output_channel_ref)
{
#if defined(HAS_DEFAULT_QJS_DUMP_OUTPUT)
	if (matches_keyword(keyword, keyword_length, "stdout") || matches_keyword(keyword, keyword_length, "to-stdout"))
	{
		*output_channel_ref = !negate ? stdout : stderr;
		return 0;
	}
	else if (matches_keyword(keyword, keyword_length, "stderr") || matches_keyword(keyword, keyword_length, "to-stderr"))
	{
		*output_channel_ref = !negate ? stderr : stdout;
		return 0;
	}
#endif
	if (matches_keyword(keyword, keyword_length, "none"))
	{
		*current_flags = !negate ? 0 : QJS_DUMP_FLAG_ALL;
		return 0;
	}
	return 1;
}

// ------------------------------------------------------------------------

// provide the default implementations for the dump output routines:

#if defined(HAS_DEFAULT_QJS_DUMP_OUTPUT)

size_t qjs_vprintf(QJS_DUMP_OUTPUT channel, const char* fmt, va_list args)
{
	return vfprintf(channel, fmt, args);
}

/**
	The non va_list equivalent of qjs_vprintf.
*/
size_t qjs_printf(QJS_DUMP_OUTPUT channel, const char* fmt, ...)
{
	va_list ap;

	va_start(ap, fmt);
	int rv = vfprintf(channel, fmt, ap);
	va_end(ap);

	return rv;
}

#endif



void qjs_dump_vprintf(const char* fmt, va_list args)
{
	qjs_vprintf(qjs_get_current_dump_output_channel(), fmt, args);
}

size_t qjs_dump_printf(const char* fmt, ...)
{
	va_list ap;

	va_start(ap, fmt);
	int rv = qjs_vprintf(qjs_get_current_dump_output_channel(), fmt, ap);
	va_end(ap);
	return rv;
}

void qjs_dump_line(const char* msg)
{
	qjs_printf(qjs_get_current_dump_output_channel(), "%s", msg);
}

void qjs_dump_putchar(const char c)
{
	qjs_printf(qjs_get_current_dump_output_channel(), "%c", c);
}
