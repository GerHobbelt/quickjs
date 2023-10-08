#ifndef LOG_H
#define LOG_H 1
//#include "Config.hpp"
//#include <mutex>
//#include <memory>
#include "string_view.hpp"
#include <stdio.h>
// namespace NodeCPP
// {
	class Console final
	{
	public:
		enum priority:char
		{
			// print developer information (trace stack)
			developer=0,
			// debug information (not product usage)
			debug,
			// detailed program information
			info,
			// an under the control problem
			notice,
			// considerable problem (eg: network problem,bad configuration), may be terminated
			warn,
			// handleable unexpected error in codes, will be terminated gradually
			error,
			// unhandleable unexpected error, will be terminated
			crit,
			// error occured from low-level (eg:OS, Hardware), terminate imediately
			emerg,
			// print the error any way
			any=99,
		};
	private:
		static const priority current_priority=priority::developer;

		// std
		static inline void log (){putchar('\n');}
		static inline void log(int v){printf("%d",v);}
		static inline void log(unsigned long i){printf("%lu",i);}
		static inline void log(long i){printf("%lu",i);}

		static inline void log(const string_view& str)
			{puts(str.data());}

		static inline void log(char const*const str)
			{printf("%s",str);}

		static inline void log(void *ptr)
			{printf("%p",ptr);}

		static inline void log(char c)
			{putchar(c);}

		static inline void log(bool b)
			{b?printf("true"):printf("false");}

		// std
		template<typename T,typename... Arg>
		static inline void log (T v, const Arg... args)
			{log(v);log(args...);}


	public:
		static void init()
		// :lock()
		{
			setvbuf(stdout, nullptr, _IONBF, 0);
			setvbuf(stderr, nullptr, _IONBF, 0);
			if(current_priority >= warn)
				printf("Dont close this windows!\n");
		}
		static Console & endl(Console &thiz);



		//stdarg
		template<typename ... Arg>
		static void log (const priority p,Arg... args)
		{
			if(current_priority <= p)
				log(args...);
			//return *this;
		}
		//stdarg
		template<typename Arg>
		static void log (const priority p,Arg arg)
		{
			if(current_priority <= p)
				log(arg);
			//return *this;
		}

		static void write(const priority p,char const*const str,size_t s)
		{
            if(current_priority <= p)
                fwrite(str, 1, s, stdout);
			//return *this;
		}

	};
// }
#endif
