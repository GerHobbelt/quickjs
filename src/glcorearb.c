#include "glcorearb.h"
#include "CrossPlatform.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#if IS_WINDOWS_OS
	#include <windows.h>
	#define GLWLibraryName					"opengl32.dll"
	#define GLWHandle						HMODULE
	#define GLWOpen(N)						LoadLibrary(N)
	// wglGetProcAddress cant obtain legacy opengl functions
	#define GLWObtainLegacy(HANDLE,SYMBOL)	GetProcAddress(HANDLE,SYMBOL)
	#define GLWObtain(HANDLE,SYMBOL)		wglGetProcAddress((LPCSTR)SYMBOL)
	#define GLWClose(HANDLE)				FreeLibrary(HANDLE)
	#define GLWGetProcAddress(name)			wglGetProcAddress((LPCSTR)name)
	#define GLWErrorReason()				"win32-UNKNOWN"
	// why decltype?
	#define PROC_LEGACY(V)					retval|=(V=(__typeof__(V))GLWObtainLegacy(libgl,#V))==NULL
	#define PROC(V)							retval|=(V=(__typeof__(V))GLWObtain(libgl,#V))==NULL
	#define PROC_EXT(V)						if(V==NULL) V=(__typeof__(V))GLWObtain(libgl,#V "EXT");
#else
	#include <dlfcn.h>
	#define GLWLibraryName					"libGL.so"
	#define GLWHandle						void*
	#define GLWOpen(N)						dlopen(N, RTLD_NOW | RTLD_LOCAL)
	#define GLWObtain(HANDLE,SYMBOL)		dlsym(HANDLE,SYMBOL)
	#define GLWObtainLegacy(HANDLE,SYMBOL)	dlsym(HANDLE,SYMBOL)
	#define GLWClose(HANDLE)				dlclose(HANDLE)
	#define GLWGetProcAddress(name)			NULL
	#define GLWErrorReason()				dlerror()
	#define PROC_LEGACY(V)					retval|=(V=(__typeof__(V))GLWObtain(libgl,#V))==NULL
	#define PROC(V)							retval|=(V=(__typeof__(V))GLWObtain(libgl,#V))==NULL
	#define PROC_EXT(V)						if(V==NULL) V=(__typeof__(V))GLWObtain(libgl,#V "EXT");
//#else#error "undefined compiler"
#endif

#define DEBUG(...) printf(__VA_ARGS__)
static GLWHandle libgl=NULL;
static int32_t gl_current_version = 0x0;
// strings are terminated with '\0' or ' '
// and array size is known is
static char const** extensions=0;
static uint32_t extensions_count =0;

static int str2int(const char*s,int len)
{
	int result=0;
	while (len > 0)
	{
		result*=10;
		result+= *s - '0';
		s++;
		len--;
	}
	return result;
}
static int32_t ParseVersion(const char *s)
{
	if(s == 0)
		return 0;

	// Examle:
	//   f   s
	// 0 1 2 3 4
	// 4 . 3   A
	// also must expect and handle invalide string
	// (eg: null string, short string, no number available)

	// must: s[first_index] == '.'
	int first_index=0;
	// must: s[second_index]  == ' ' OR '.'
	int second_index;

	while('0' <= s[first_index] && s[first_index] <= '9'){
		first_index++;
	}
	if(first_index < 1 || 3 < first_index || s[first_index] != '.')
		return 0;
	second_index = first_index+1;

	while('0' <= s[second_index] && s[second_index] <= '9')
		second_index++;
	if(second_index <= (first_index+1) || (first_index+2) < second_index )
		return 0;

	int32_t v = str2int(s,first_index) << 8;
	if(v == 0)
		return 0;
	v += str2int(s+(first_index + 1),second_index - (first_index+1));
	return v;
}
static int32_t ParseExtensions(const char *s)
{
	if(s == 0)
	{
		if(extensions)
		{
			free(extensions);
			extensions = 0;
		}
		return 0;
	}
	int x=0;
	int count=0;
	//0='\0', 1=!' ', 2=' '
	char prev_type=0;
	while (s[x] != 0)
	{
		if(s[x] == ' ')
		{
			prev_type = 2;
		}
		else
		{
			if(prev_type != 1)
				count++;
			prev_type = 1;
		}
		x++;
	}
	if(count < 1)
		return 0;
	extensions = (char const **) malloc(sizeof(char*)*count);
	x=0;
	prev_type=0;
	count = 0;
	while (s[x] != 0)
	{
		if(s[x] == ' ')
		{
			prev_type = 2;
		}
		else
		{
			//if previuos character was space but this one is not
			if(prev_type != 1)
			{
				extensions[count] = s+x;
				count++;
			}
			prev_type = 1;
		}
		x++;
	}
	return count;
}

int8_t glwHasExtension(const char *e)
{
	if(extensions == 0)
		return 0;
	for(uint32_t x=0;x<extensions_count;x++)
	{
		if(0==strcmp(e,extensions[x]))
			return 1;
	}
	return 0;
}
int32_t glwGetVesion()
{
	return gl_current_version;
}
int32_t glwDestroy()
{
	if(extensions)
	{
		free(extensions);
		extensions = 0;
	}
	if(libgl)
		return GLWClose(libgl);
	else
		return 0;
}
int32_t glwInitialize(int requested_version)
{
	int32_t retval=0;
	// makes an executable object file specified by file available to the calling program.
	if(!libgl)
		libgl = GLWOpen(GLWLibraryName);
	if(!libgl)
	{
		DEBUG("Unable to open %s: %s\n",GLWLibraryName,GLWErrorReason());
		return 1;
	}
	// obtain the address of a symbol defined within an object.
	PROC_LEGACY(glGetString);
	if(!glGetString)
	{
		DEBUG("Invalide library : %s, %s\n",GLWLibraryName,GLWErrorReason());
		return 1;
	}

	gl_current_version = ParseVersion((const char *)glGetString(GL_VERSION));
	if(gl_current_version == 0)
	{
		DEBUG("Unable to get OpenGL version\n");
		return 1;
	}
	if(gl_current_version < requested_version)
	{
		DEBUG("Unable to get required OpenGL version (current version: %s)\n",glGetString(GL_VERSION));
		return 1;
	}
	ParseExtensions((const char *)glGetString(GL_EXTENSIONS));



	PROC_LEGACY(glCullFace);
	PROC_LEGACY(glFrontFace);
	PROC_LEGACY(glHint);
	PROC_LEGACY(glLineWidth);
	PROC_LEGACY(glPointSize);
	PROC_LEGACY(glScissor);
	PROC_LEGACY(glTexParameterf);
	PROC_LEGACY(glTexParameterfv);
	PROC_LEGACY(glTexParameteri);
	PROC_LEGACY(glTexParameteriv);
	PROC_LEGACY(glTexImage1D);
	PROC_LEGACY(glTexImage2D);
	PROC_LEGACY(glDrawBuffer);
	PROC_LEGACY(glClear);
	PROC_LEGACY(glClearColor);
	PROC_LEGACY(glClearStencil);
	PROC_LEGACY(glClearDepth);
	PROC_LEGACY(glStencilMask);
	PROC_LEGACY(glColorMask);
	PROC_LEGACY(glDepthMask);
	PROC_LEGACY(glDisable);
	PROC_LEGACY(glEnable);
	PROC_LEGACY(glFinish);
	PROC_LEGACY(glFlush);
	PROC_LEGACY(glBlendFunc);
	PROC_LEGACY(glLogicOp);
	PROC_LEGACY(glStencilFunc);
	PROC_LEGACY(glStencilOp);
	PROC_LEGACY(glDepthFunc);
	PROC_LEGACY(glPixelStorei);
	PROC_LEGACY(glReadBuffer);
	PROC_LEGACY(glReadPixels);
	PROC_LEGACY(glGetBooleanv);
	PROC_LEGACY(glGetDoublev);
	PROC_LEGACY(glGetError);
	PROC_LEGACY(glGetFloatv);
	PROC_LEGACY(glGetIntegerv);
	PROC_LEGACY(glGetTexImage);
	PROC_LEGACY(glGetTexParameterfv);
	PROC_LEGACY(glGetTexParameteriv);
	PROC_LEGACY(glGetTexLevelParameterfv);
	PROC_LEGACY(glGetTexLevelParameteriv);
	PROC_LEGACY(glIsEnabled);
	PROC_LEGACY(glDepthRange);
	PROC_LEGACY(glViewport);
	if(retval)
	{
		DEBUG("was unable to load some GL_VERSION_1_0 functions\n");
		return 1;
	}

	if(gl_current_version < 0x101)
	{
		goto EXT;
	}
	PROC_LEGACY(glBindTexture);
	PROC_LEGACY(glDrawArrays);
	PROC_LEGACY(glDrawElements);
	PROC_LEGACY(glCopyTexImage2D);
	PROC_LEGACY(glCopyTexSubImage2D);
	PROC_LEGACY(glDeleteTextures);
	PROC_LEGACY(glGenTextures);
	PROC_LEGACY(glGetPointerv);
	PROC_LEGACY(glIsTexture);
	PROC_LEGACY(glTexSubImage2D);
	if(retval)
	{
		DEBUG("was unable to load some GL_VERSION_1_1 functions\n");
		return 1;
	}

	if(gl_current_version < 0x102)
	{
		goto EXT;
	}
	PROC(glCopyTexSubImage3D);
	PROC(glDrawRangeElements);
	PROC(glTexImage3D);
	PROC(glTexSubImage3D);
	if(retval)
	{
		DEBUG("was unable to load some GL_VERSION_1_2 functions\n");
		return 1;
	}

	if(gl_current_version < 0x103)
	{
		goto EXT;
	}
	PROC(glActiveTexture);
	PROC(glCompressedTexImage2D);
	PROC(glCompressedTexImage3D);
	PROC(glCompressedTexSubImage2D);
	PROC(glCompressedTexSubImage3D);
	PROC(glGetCompressedTexImage);
	PROC(glSampleCoverage);
	if(retval)
	{
		DEBUG("was unable to load some GL_VERSION_1_3 functions\n");
		return 1;
	}

	if(gl_current_version < 0x104)
	{
		goto EXT;
	}
	PROC(glBlendColor);
	PROC(glBlendEquation);
	PROC(glBlendFuncSeparate);
	PROC(glMultiDrawArrays);
	PROC(glMultiDrawElements);
	PROC(glPointParameterf);
	PROC(glPointParameterfv);
	PROC(glPointParameteri);
	PROC(glPointParameteriv);
	if(retval)
	{
		DEBUG("was unable to load some GL_VERSION_1_4 functions\n");
		return 1;
	}

	if(gl_current_version < 0x105)
	{
		goto EXT;
	}
	PROC(glBeginQuery);
	PROC(glBindBuffer);
	PROC(glBufferData);
	PROC(glBufferSubData);
	PROC(glDeleteBuffers);
	PROC(glDeleteQueries);
	PROC(glEndQuery);
	PROC(glGenBuffers);
	PROC(glGenQueries);
	PROC(glGetBufferParameteriv);
	PROC(glGetBufferPointerv);
	PROC(glGetBufferSubData);
	PROC(glGetQueryObjectiv);
	PROC(glGetQueryObjectuiv);
	PROC(glGetQueryiv);
	PROC(glIsBuffer);
	PROC(glIsQuery);
	PROC(glMapBuffer);
	PROC(glUnmapBuffer);
	if(retval)
	{
		DEBUG("was unable to load some GL_VERSION_1_5 functions\n");
		return 1;
	}

	if(gl_current_version < 0x200)
	{
		goto EXT;
	}
	PROC(glAttachShader);
	PROC(glBindAttribLocation);
	PROC(glBlendEquationSeparate);
	PROC(glCompileShader);
	PROC(glCreateProgram);
	PROC(glCreateShader);
	PROC(glDeleteProgram);
	PROC(glDeleteShader);
	PROC(glDetachShader);
	PROC(glDisableVertexAttribArray);
	PROC(glDrawBuffers);
	PROC(glEnableVertexAttribArray);
	PROC(glGetActiveAttrib);
	PROC(glGetActiveUniform);
	PROC(glGetAttachedShaders);
	PROC(glGetAttribLocation);
	PROC(glGetProgramInfoLog);
	PROC(glGetProgramiv);
	PROC(glGetShaderInfoLog);
	PROC(glGetShaderSource);
	PROC(glGetShaderiv);
	PROC(glGetUniformLocation);
	PROC(glGetUniformfv);
	PROC(glGetUniformiv);
	PROC(glGetVertexAttribPointerv);
	PROC(glGetVertexAttribdv);
	PROC(glGetVertexAttribfv);
	PROC(glGetVertexAttribiv);
	PROC(glIsProgram);
	PROC(glIsShader);
	PROC(glLinkProgram);
	PROC(glShaderSource);
	PROC(glStencilFuncSeparate);
	PROC(glStencilMaskSeparate);
	PROC(glStencilOpSeparate);
	PROC(glUniform1f);
	PROC(glUniform1fv);
	PROC(glUniform1i);
	PROC(glUniform1iv);
	PROC(glUniform2f);
	PROC(glUniform2fv);
	PROC(glUniform2i);
	PROC(glUniform2iv);
	PROC(glUniform3f);
	PROC(glUniform3fv);
	PROC(glUniform3i);
	PROC(glUniform3iv);
	PROC(glUniform4f);
	PROC(glUniform4fv);
	PROC(glUniform4i);
	PROC(glUniform4iv);
	PROC(glUniformMatrix2fv);
	PROC(glUniformMatrix3fv);
	PROC(glUniformMatrix4fv);
	PROC(glUseProgram);
	PROC(glValidateProgram);
	PROC(glVertexAttrib1d);
	PROC(glVertexAttrib1dv);
	PROC(glVertexAttrib1f);
	PROC(glVertexAttrib1fv);
	PROC(glVertexAttrib1s);
	PROC(glVertexAttrib1sv);
	PROC(glVertexAttrib2d);
	PROC(glVertexAttrib2dv);
	PROC(glVertexAttrib2f);
	PROC(glVertexAttrib2fv);
	PROC(glVertexAttrib2s);
	PROC(glVertexAttrib2sv);
	PROC(glVertexAttrib3d);
	PROC(glVertexAttrib3dv);
	PROC(glVertexAttrib3f);
	PROC(glVertexAttrib3fv);
	PROC(glVertexAttrib3s);
	PROC(glVertexAttrib3sv);
	PROC(glVertexAttrib4Nbv);
	PROC(glVertexAttrib4Niv);
	PROC(glVertexAttrib4Nsv);
	PROC(glVertexAttrib4Nub);
	PROC(glVertexAttrib4Nubv);
	PROC(glVertexAttrib4Nuiv);
	PROC(glVertexAttrib4Nusv);
	PROC(glVertexAttrib4bv);
	PROC(glVertexAttrib4d);
	PROC(glVertexAttrib4dv);
	PROC(glVertexAttrib4f);
	PROC(glVertexAttrib4fv);
	PROC(glVertexAttrib4iv);
	PROC(glVertexAttrib4s);
	PROC(glVertexAttrib4sv);
	PROC(glVertexAttrib4ubv);
	PROC(glVertexAttrib4uiv);
	PROC(glVertexAttrib4usv);
	PROC(glVertexAttribPointer);
	if(retval)
	{
		DEBUG("was unable to load some GL_VERSION_2_0 functions\n");
		return 1;
	}

	if(gl_current_version < 0x201)
	{
		goto EXT;
	}
	PROC(glUniformMatrix2x3fv);
	PROC(glUniformMatrix3x2fv);
	PROC(glUniformMatrix2x4fv);
	PROC(glUniformMatrix4x2fv);
	PROC(glUniformMatrix3x4fv);
	PROC(glUniformMatrix4x3fv);
	if(retval)
	{
		DEBUG("was unable to load some GL_VERSION_2_1 functions\n");
		return 1;
	}

	if(gl_current_version < 0x300)
	{
		goto EXT;
	}
	PROC(glBeginConditionalRender);
	PROC(glBeginTransformFeedback);
	PROC(glBindFragDataLocation);
	PROC(glBindBufferRange);
	PROC(glBindBufferBase);
	PROC(glClampColor);
	PROC(glClearBufferfi);
	PROC(glClearBufferfv);
	PROC(glClearBufferiv);
	PROC(glClearBufferuiv);
	PROC(glColorMaski);
	PROC(glDisablei);
	PROC(glEnablei);
	PROC(glEndConditionalRender);
	PROC(glEndTransformFeedback);
	PROC(glGetBooleani_v);
	PROC(glGetIntegeri_v);
	PROC(glGetFragDataLocation);
	PROC(glGetStringi);
	PROC(glGetTexParameterIiv);
	PROC(glGetTexParameterIuiv);
	PROC(glGetTransformFeedbackVarying);
	PROC(glGetUniformuiv);
	PROC(glGetVertexAttribIiv);
	PROC(glGetVertexAttribIuiv);
	PROC(glIsEnabledi);
	PROC(glTexParameterIiv);
	PROC(glTexParameterIuiv);
	PROC(glTransformFeedbackVaryings);
	PROC(glUniform1ui);
	PROC(glUniform1uiv);
	PROC(glUniform2ui);
	PROC(glUniform2uiv);
	PROC(glUniform3ui);
	PROC(glUniform3uiv);
	PROC(glUniform4ui);
	PROC(glUniform4uiv);
	PROC(glVertexAttribI1i);
	PROC(glVertexAttribI1iv);
	PROC(glVertexAttribI1ui);
	PROC(glVertexAttribI1uiv);
	PROC(glVertexAttribI2i);
	PROC(glVertexAttribI2iv);
	PROC(glVertexAttribI2ui);
	PROC(glVertexAttribI2uiv);
	PROC(glVertexAttribI3i);
	PROC(glVertexAttribI3iv);
	PROC(glVertexAttribI3ui);
	PROC(glVertexAttribI3uiv);
	PROC(glVertexAttribI4bv);
	PROC(glVertexAttribI4i);
	PROC(glVertexAttribI4iv);
	PROC(glVertexAttribI4sv);
	PROC(glVertexAttribI4ubv);
	PROC(glVertexAttribI4ui);
	PROC(glVertexAttribI4uiv);
	PROC(glVertexAttribI4usv);
	PROC(glVertexAttribIPointer);
	PROC(glBindFramebuffer);
	PROC(glBindRenderbuffer);
	PROC(glBlitFramebuffer);
	PROC(glCheckFramebufferStatus);
	PROC(glDeleteFramebuffers);
	PROC(glDeleteRenderbuffers);
	PROC(glFramebufferRenderbuffer);
	PROC(glFramebufferTexture1D);
	PROC(glFramebufferTexture2D);
	PROC(glFramebufferTexture3D);
	PROC(glFramebufferTextureLayer);
	PROC(glGenFramebuffers);
	PROC(glGenRenderbuffers);
	PROC(glGenerateMipmap);
	PROC(glGetFramebufferAttachmentParameteriv);
	PROC(glGetRenderbufferParameteriv);
	PROC(glIsFramebuffer);
	PROC(glIsRenderbuffer);
	PROC(glRenderbufferStorage);
	PROC(glRenderbufferStorageMultisample);
	PROC(glFlushMappedBufferRange);
	PROC(glMapBufferRange);
	PROC(glBindVertexArray);
	PROC(glDeleteVertexArrays);
	PROC(glGenVertexArrays);
	PROC(glIsVertexArray);
	if(retval)
	{
		DEBUG("was unable to load some GL_VERSION_3_0 functions\n");
		return 1;
	}

	if(gl_current_version < 0x301)
	{
		goto EXT;
	}
	PROC(glDrawArraysInstanced);
	PROC(glDrawElementsInstanced);
	PROC(glPrimitiveRestartIndex);
	PROC(glTexBuffer);
	PROC(glGetActiveUniformBlockName);
	PROC(glGetActiveUniformBlockiv);
	PROC(glGetActiveUniformName);
	PROC(glGetActiveUniformsiv);
	PROC(glGetUniformBlockIndex);
	PROC(glGetUniformIndices);
	PROC(glUniformBlockBinding);
	PROC(glCopyBufferSubData);
	if(retval)
	{
		DEBUG("was unable to load some GL_VERSION_3_1 functions\n");
		return 1;
	}

	if(gl_current_version < 0x302)
	{
		goto EXT;
	}
	PROC(glFramebufferTexture);
	PROC(glGetBufferParameteri64v);
	PROC(glGetInteger64i_v);
	PROC(glDrawElementsBaseVertex);
	PROC(glDrawElementsInstancedBaseVertex);
	PROC(glDrawRangeElementsBaseVertex);
	PROC(glMultiDrawElementsBaseVertex);
	PROC(glProvokingVertex);
	PROC(glClientWaitSync);
	PROC(glDeleteSync);
	PROC(glFenceSync);
	PROC(glGetInteger64v);
	PROC(glGetSynciv);
	PROC(glIsSync);
	PROC(glWaitSync);
	PROC(glGetMultisamplefv);
	PROC(glSampleMaski);
	PROC(glTexImage2DMultisample);
	PROC(glTexImage3DMultisample);
	if(retval)
	{
		DEBUG("was unable to load some GL_VERSION_3_2 functions\n");
		return 1;
	}

	if(gl_current_version < 0x303)
	{
		goto EXT;
	}
	PROC(glVertexAttribDivisor);
	PROC(glBindFragDataLocationIndexed);
	PROC(glGetFragDataIndex);
	PROC(glBindSampler);
	PROC(glDeleteSamplers);
	PROC(glGenSamplers);
	PROC(glGetSamplerParameterIiv);
	PROC(glGetSamplerParameterIuiv);
	PROC(glGetSamplerParameterfv);
	PROC(glGetSamplerParameteriv);
	PROC(glIsSampler);
	PROC(glSamplerParameterIiv);
	PROC(glSamplerParameterIuiv);
	PROC(glSamplerParameterf);
	PROC(glSamplerParameterfv);
	PROC(glSamplerParameteri);
	PROC(glSamplerParameteriv);
	PROC(glGetQueryObjecti64v);
	PROC(glGetQueryObjectui64v);
	PROC(glQueryCounter);
	PROC(glVertexAttribP1ui);
	PROC(glVertexAttribP1uiv);
	PROC(glVertexAttribP2ui);
	PROC(glVertexAttribP2uiv);
	PROC(glVertexAttribP3ui);
	PROC(glVertexAttribP3uiv);
	PROC(glVertexAttribP4ui);
	PROC(glVertexAttribP4uiv);
	if(retval)
	{
		DEBUG("was unable to load some GL_VERSION_3_3 functions\n");
		return 1;
	}

	if(gl_current_version < 0x400)
	{
		goto EXT;
	}
	PROC(glBlendEquationSeparatei);
	PROC(glBlendEquationi);
	PROC(glBlendFuncSeparatei);
	PROC(glBlendFunci);
	PROC(glMinSampleShading);
	PROC(glDrawArraysIndirect);
	PROC(glDrawElementsIndirect);
	PROC(glGetUniformdv);
	PROC(glUniform1d);
	PROC(glUniform1dv);
	PROC(glUniform2d);
	PROC(glUniform2dv);
	PROC(glUniform3d);
	PROC(glUniform3dv);
	PROC(glUniform4d);
	PROC(glUniform4dv);
	PROC(glUniformMatrix2dv);
	PROC(glUniformMatrix2x3dv);
	PROC(glUniformMatrix2x4dv);
	PROC(glUniformMatrix3dv);
	PROC(glUniformMatrix3x2dv);
	PROC(glUniformMatrix3x4dv);
	PROC(glUniformMatrix4dv);
	PROC(glUniformMatrix4x2dv);
	PROC(glUniformMatrix4x3dv);
	PROC(glGetActiveSubroutineName);
	PROC(glGetActiveSubroutineUniformName);
	PROC(glGetActiveSubroutineUniformiv);
	PROC(glGetProgramStageiv);
	PROC(glGetSubroutineIndex);
	PROC(glGetSubroutineUniformLocation);
	PROC(glGetUniformSubroutineuiv);
	PROC(glUniformSubroutinesuiv);
	PROC(glPatchParameterfv);
	PROC(glPatchParameteri);
	PROC(glBindTransformFeedback);
	PROC(glDeleteTransformFeedbacks);
	PROC(glDrawTransformFeedback);
	PROC(glGenTransformFeedbacks);
	PROC(glIsTransformFeedback);
	PROC(glPauseTransformFeedback);
	PROC(glResumeTransformFeedback);
	PROC(glBeginQueryIndexed);
	PROC(glDrawTransformFeedbackStream);
	PROC(glEndQueryIndexed);
	PROC(glGetQueryIndexediv);
	if(retval)
	{
		DEBUG("was unable to load some GL_VERSION_4_0 functions\n");
		return 1;
	}

	if(gl_current_version < 0x401)
	{
		goto EXT;
	}
	PROC(glClearDepthf);
	PROC(glDepthRangef);
	PROC(glReleaseShaderCompiler);
	PROC(glGetShaderPrecisionFormat);
	PROC(glShaderBinary);
	PROC(glGetProgramBinary);
	PROC(glProgramBinary);
	PROC(glProgramParameteri);
	PROC(glActiveShaderProgram);
	PROC(glBindProgramPipeline);
	PROC(glCreateShaderProgramv);
	PROC(glDeleteProgramPipelines);
	PROC(glGenProgramPipelines);
	PROC(glGetProgramPipelineInfoLog);
	PROC(glGetProgramPipelineiv);
	PROC(glIsProgramPipeline);
	PROC(glProgramUniform1d);
	PROC(glProgramUniform1dv);
	PROC(glProgramUniform1f);
	PROC(glProgramUniform1fv);
	PROC(glProgramUniform1i);
	PROC(glProgramUniform1iv);
	PROC(glProgramUniform1ui);
	PROC(glProgramUniform1uiv);
	PROC(glProgramUniform2d);
	PROC(glProgramUniform2dv);
	PROC(glProgramUniform2f);
	PROC(glProgramUniform2fv);
	PROC(glProgramUniform2i);
	PROC(glProgramUniform2iv);
	PROC(glProgramUniform2ui);
	PROC(glProgramUniform2uiv);
	PROC(glProgramUniform3d);
	PROC(glProgramUniform3dv);
	PROC(glProgramUniform3f);
	PROC(glProgramUniform3fv);
	PROC(glProgramUniform3i);
	PROC(glProgramUniform3iv);
	PROC(glProgramUniform3ui);
	PROC(glProgramUniform3uiv);
	PROC(glProgramUniform4d);
	PROC(glProgramUniform4dv);
	PROC(glProgramUniform4f);
	PROC(glProgramUniform4fv);
	PROC(glProgramUniform4i);
	PROC(glProgramUniform4iv);
	PROC(glProgramUniform4ui);
	PROC(glProgramUniform4uiv);
	PROC(glProgramUniformMatrix2dv);
	PROC(glProgramUniformMatrix2fv);
	PROC(glProgramUniformMatrix2x3dv);
	PROC(glProgramUniformMatrix2x3fv);
	PROC(glProgramUniformMatrix2x4dv);
	PROC(glProgramUniformMatrix2x4fv);
	PROC(glProgramUniformMatrix3dv);
	PROC(glProgramUniformMatrix3fv);
	PROC(glProgramUniformMatrix3x2dv);
	PROC(glProgramUniformMatrix3x2fv);
	PROC(glProgramUniformMatrix3x4dv);
	PROC(glProgramUniformMatrix3x4fv);
	PROC(glProgramUniformMatrix4dv);
	PROC(glProgramUniformMatrix4fv);
	PROC(glProgramUniformMatrix4x2dv);
	PROC(glProgramUniformMatrix4x2fv);
	PROC(glProgramUniformMatrix4x3dv);
	PROC(glProgramUniformMatrix4x3fv);
	PROC(glUseProgramStages);
	PROC(glValidateProgramPipeline);
	PROC(glGetVertexAttribLdv);
	PROC(glVertexAttribL1d);
	PROC(glVertexAttribL1dv);
	PROC(glVertexAttribL2d);
	PROC(glVertexAttribL2dv);
	PROC(glVertexAttribL3d);
	PROC(glVertexAttribL3dv);
	PROC(glVertexAttribL4d);
	PROC(glVertexAttribL4dv);
	PROC(glVertexAttribLPointer);
	PROC(glDepthRangeArrayv);
	PROC(glDepthRangeIndexed);
	PROC(glGetDoublei_v);
	PROC(glGetFloati_v);
	PROC(glScissorArrayv);
	PROC(glScissorIndexed);
	PROC(glScissorIndexedv);
	PROC(glViewportArrayv);
	PROC(glViewportIndexedf);
	PROC(glViewportIndexedfv);
	if(retval)
	{
		DEBUG("was unable to load some GL_VERSION_4_1 functions\n");
		return 1;
	}

	if(gl_current_version < 0x402)
	{
		goto EXT;
	}
	PROC(glDrawArraysInstancedBaseInstance);
	PROC(glDrawElementsInstancedBaseInstance);
	PROC(glDrawElementsInstancedBaseVertexBaseInstance);
	PROC(glGetInternalformativ);
	PROC(glGetActiveAtomicCounterBufferiv);
	PROC(glBindImageTexture);
	PROC(glMemoryBarrier);
	PROC(glTexStorage1D);
	PROC(glTexStorage2D);
	PROC(glTexStorage3D);
	PROC(glDrawTransformFeedbackInstanced);
	PROC(glDrawTransformFeedbackStreamInstanced);
	if(retval)
	{
		DEBUG("was unable to load some GL_VERSION_4_2 functions\n");
		return 1;
	}

	if(gl_current_version < 0x403)
	{
		goto EXT;
	}
	PROC(glClearBufferData);
	PROC(glClearBufferSubData);
	PROC(glClearNamedBufferDataEXT);
	PROC(glClearNamedBufferSubDataEXT);
	PROC(glDispatchCompute);
	PROC(glDispatchComputeIndirect);
	PROC(glCopyImageSubData);
	PROC(glFramebufferParameteri);
	PROC(glGetFramebufferParameteriv);
	PROC(glGetNamedFramebufferParameterivEXT);
	PROC(glNamedFramebufferParameteriEXT);
	PROC(glGetInternalformati64v);
	PROC(glInvalidateBufferData);
	PROC(glInvalidateBufferSubData);
	PROC(glInvalidateFramebuffer);
	PROC(glInvalidateSubFramebuffer);
	PROC(glInvalidateTexImage);
	PROC(glInvalidateTexSubImage);
	PROC(glMultiDrawArraysIndirect);
	PROC(glMultiDrawElementsIndirect);
	PROC(glGetProgramInterfaceiv);
	PROC(glGetProgramResourceIndex);
	PROC(glGetProgramResourceLocation);
	PROC(glGetProgramResourceLocationIndex);
	PROC(glGetProgramResourceName);
	PROC(glGetProgramResourceiv);
	PROC(glShaderStorageBlockBinding);
	PROC(glTexBufferRange);
	PROC(glTextureBufferRangeEXT);
	PROC(glTexStorage2DMultisample);
	PROC(glTexStorage3DMultisample);
	PROC(glTextureStorage2DMultisampleEXT);
	PROC(glTextureStorage3DMultisampleEXT);
	PROC(glTextureView);
	PROC(glBindVertexBuffer);
	PROC(glVertexArrayBindVertexBufferEXT);
	PROC(glVertexArrayVertexAttribBindingEXT);
	PROC(glVertexArrayVertexAttribFormatEXT);
	PROC(glVertexArrayVertexAttribIFormatEXT);
	PROC(glVertexArrayVertexAttribLFormatEXT);
	PROC(glVertexArrayVertexBindingDivisorEXT);
	PROC(glVertexAttribBinding);
	PROC(glVertexAttribFormat);
	PROC(glVertexAttribIFormat);
	PROC(glVertexAttribLFormat);
	PROC(glVertexBindingDivisor);
	PROC(glDebugMessageCallback);
	PROC(glDebugMessageControl);
	PROC(glDebugMessageInsert);
	PROC(glGetDebugMessageLog);
	PROC(glGetObjectLabel);
	PROC(glGetObjectPtrLabel);
	PROC(glObjectLabel);
	PROC(glObjectPtrLabel);
	PROC(glPopDebugGroup);
	PROC(glPushDebugGroup);
	if(retval)
	{
		DEBUG("was unable to load some GL_VERSION_4_3 functions\n");
		return 1;
	}

	if(gl_current_version < 0x404)
	{
		goto EXT;
	}
	PROC(glBufferStorage);
	PROC(glClearTexImage);
	PROC(glClearTexSubImage);
	PROC(glBindBuffersBase);
	PROC(glBindBuffersRange);
	PROC(glBindImageTextures);
	PROC(glBindSamplers);
	PROC(glBindTextures);
	PROC(glBindVertexBuffers);
	if(retval)
	{
		DEBUG("was unable to load some GL_VERSION_4_4 functions\n");
		return 1;
	}

	if(gl_current_version < 0x405)
	{
		goto EXT;
	}
	PROC(glGetGraphicsResetStatus);
	PROC(glGetnCompressedTexImage);
	PROC(glGetnTexImage);
	PROC(glGetnUniformdv);
	PROC(glClipControl);
	PROC(glBindTextureUnit);
	PROC(glBlitNamedFramebuffer);
	PROC(glCheckNamedFramebufferStatus);
	PROC(glClearNamedBufferData);
	PROC(glClearNamedBufferSubData);
	PROC(glClearNamedFramebufferfi);
	PROC(glClearNamedFramebufferfv);
	PROC(glClearNamedFramebufferiv);
	PROC(glClearNamedFramebufferuiv);
	PROC(glCompressedTextureSubImage1D);
	PROC(glCompressedTextureSubImage2D);
	PROC(glCompressedTextureSubImage3D);
	PROC(glCopyNamedBufferSubData);
	PROC(glCopyTextureSubImage1D);
	PROC(glCopyTextureSubImage2D);
	PROC(glCopyTextureSubImage3D);
	PROC(glCreateBuffers);
	PROC(glCreateFramebuffers);
	PROC(glCreateProgramPipelines);
	PROC(glCreateQueries);
	PROC(glCreateRenderbuffers);
	PROC(glCreateSamplers);
	PROC(glCreateTextures);
	PROC(glCreateTransformFeedbacks);
	PROC(glCreateVertexArrays);
	PROC(glDisableVertexArrayAttrib);
	PROC(glEnableVertexArrayAttrib);
	PROC(glFlushMappedNamedBufferRange);
	PROC(glGenerateTextureMipmap);
	PROC(glGetCompressedTextureImage);
	PROC(glGetNamedBufferParameteri64v);
	PROC(glGetNamedBufferParameteriv);
	PROC(glGetNamedBufferPointerv);
	PROC(glGetNamedBufferSubData);
	PROC(glGetNamedFramebufferAttachmentParameteriv);
	PROC(glGetNamedFramebufferParameteriv);
	PROC(glGetNamedRenderbufferParameteriv);
	PROC(glGetQueryBufferObjecti64v);
	PROC(glGetQueryBufferObjectiv);
	PROC(glGetQueryBufferObjectui64v);
	PROC(glGetQueryBufferObjectuiv);
	PROC(glGetTextureImage);
	PROC(glGetTextureLevelParameterfv);
	PROC(glGetTextureLevelParameteriv);
	PROC(glGetTextureParameterIiv);
	PROC(glGetTextureParameterIuiv);
	PROC(glGetTextureParameterfv);
	PROC(glGetTextureParameteriv);
	PROC(glGetTransformFeedbacki64_v);
	PROC(glGetTransformFeedbacki_v);
	PROC(glGetTransformFeedbackiv);
	PROC(glGetVertexArrayIndexed64iv);
	PROC(glGetVertexArrayIndexediv);
	PROC(glGetVertexArrayiv);
	PROC(glInvalidateNamedFramebufferData);
	PROC(glInvalidateNamedFramebufferSubData);
	PROC(glMapNamedBuffer);
	PROC(glMapNamedBufferRange);
	PROC(glNamedBufferData);
	PROC(glNamedBufferStorage);
	PROC(glNamedBufferSubData);
	PROC(glNamedFramebufferDrawBuffer);
	PROC(glNamedFramebufferDrawBuffers);
	PROC(glNamedFramebufferParameteri);
	PROC(glNamedFramebufferReadBuffer);
	PROC(glNamedFramebufferRenderbuffer);
	PROC(glNamedFramebufferTexture);
	PROC(glNamedFramebufferTextureLayer);
	PROC(glNamedRenderbufferStorage);
	PROC(glNamedRenderbufferStorageMultisample);
	PROC(glTextureBuffer);
	PROC(glTextureBufferRange);
	PROC(glTextureParameterIiv);
	PROC(glTextureParameterIuiv);
	PROC(glTextureParameterf);
	PROC(glTextureParameterfv);
	PROC(glTextureParameteri);
	PROC(glTextureParameteriv);
	PROC(glTextureStorage1D);
	PROC(glTextureStorage2D);
	PROC(glTextureStorage2DMultisample);
	PROC(glTextureStorage3D);
	PROC(glTextureStorage3DMultisample);
	PROC(glTextureSubImage1D);
	PROC(glTextureSubImage2D);
	PROC(glTextureSubImage3D);
	PROC(glTransformFeedbackBufferBase);
	PROC(glTransformFeedbackBufferRange);
	PROC(glUnmapNamedBuffer);
	PROC(glVertexArrayAttribBinding);
	PROC(glVertexArrayAttribFormat);
	PROC(glVertexArrayAttribIFormat);
	PROC(glVertexArrayAttribLFormat);
	PROC(glVertexArrayBindingDivisor);
	PROC(glVertexArrayElementBuffer);
	PROC(glVertexArrayVertexBuffer);
	PROC(glVertexArrayVertexBuffers);
	PROC(glGetnUniformfv);
	PROC(glGetnUniformiv);
	PROC(glGetnUniformuiv);
	PROC(glReadnPixels);
	PROC(glTextureBarrier);
	PROC(glMemoryBarrierByRegion);
	PROC(glGetCompressedTextureSubImage);
	PROC(glGetTextureSubImage);
	if(retval)
	{
		DEBUG("was unable to load some GL_VERSION_4_5 functions\n");
		return 1;
	}

	if(gl_current_version < 0x406)
	{
		goto EXT;
	}
	PROC(glMultiDrawArraysIndirectCount);
	PROC(glMultiDrawElementsIndirectCount);
	PROC(glSpecializeShader);
	PROC(glPolygonOffsetClamp);
	PROC(glPrimitiveBoundingBoxARB);
	if(retval)
	{
		DEBUG("was unable to load some GL_VERSION_4_6 functions\n");
		return 1;
	}

	EXT:
	retval=0;
	PROC_EXT(glBindFramebuffer)
	PROC_EXT(glCheckFramebufferStatus)
	PROC_EXT(glDeleteFramebuffers)
	PROC_EXT(glFramebufferTexture1D)
	PROC_EXT(glFramebufferTexture2D)
	PROC_EXT(glFramebufferTexture3D)
	PROC_EXT(glFramebufferTextureLayer)
	PROC_EXT(glGenFramebuffers)
	PROC_EXT(glGenerateMipmap)
	PROC_EXT(glGetFramebufferAttachmentParameteriv)
	PROC_EXT(glIsFramebuffer)

	PROC_EXT(glBindRenderbuffer)
	PROC_EXT(glDeleteRenderbuffers)
	PROC_EXT(glFramebufferRenderbuffer)
	PROC_EXT(glGenRenderbuffers)
	PROC_EXT(glRenderbufferStorage)
	PROC_EXT(glGetRenderbufferParameteriv)
	PROC_EXT(glIsRenderbuffer)

	PROC_EXT(glRenderbufferStorageMultisample)
	if(!glRenderbufferStorageMultisample)
		glRenderbufferStorageMultisample = (PFNGLRENDERBUFFERSTORAGEMULTISAMPLEPROC)GLWObtain(libgl,"glRenderbufferStorageMultisampleANGLE");
	if(!glRenderbufferStorageMultisample)
		glRenderbufferStorageMultisample = (PFNGLRENDERBUFFERSTORAGEMULTISAMPLEPROC)GLWObtain(libgl,"glRenderbufferStorageMultisampleAPPLE");
	return 0;
}
/* ----------------------------- GL_VERSION_1_0 ---------------------------- \*/
PFNGLCULLFACEPROC glCullFace;
PFNGLFRONTFACEPROC glFrontFace;
PFNGLHINTPROC glHint;
PFNGLLINEWIDTHPROC glLineWidth;
PFNGLPOINTSIZEPROC glPointSize;
PFNGLSCISSORPROC glScissor;
PFNGLTEXPARAMETERFPROC glTexParameterf;
PFNGLTEXPARAMETERFVPROC glTexParameterfv;
PFNGLTEXPARAMETERIPROC glTexParameteri;
PFNGLTEXPARAMETERIVPROC glTexParameteriv;
PFNGLTEXIMAGE1DPROC glTexImage1D;
PFNGLTEXIMAGE2DPROC glTexImage2D;
PFNGLDRAWBUFFERPROC glDrawBuffer;
PFNGLCLEARPROC glClear;
PFNGLCLEARCOLORPROC glClearColor;
PFNGLCLEARSTENCILPROC glClearStencil;
PFNGLCLEARDEPTHPROC glClearDepth;
PFNGLSTENCILMASKPROC glStencilMask;
PFNGLCOLORMASKPROC glColorMask;
PFNGLDEPTHMASKPROC glDepthMask;
PFNGLDISABLEPROC glDisable;
PFNGLENABLEPROC glEnable;
PFNGLFINISHPROC glFinish;
PFNGLFLUSHPROC glFlush;
PFNGLBLENDFUNCPROC glBlendFunc;
PFNGLLOGICOPPROC glLogicOp;
PFNGLSTENCILFUNCPROC glStencilFunc;
PFNGLSTENCILOPPROC glStencilOp;
PFNGLDEPTHFUNCPROC glDepthFunc;
PFNGLPIXELSTOREIPROC glPixelStorei;
PFNGLREADBUFFERPROC glReadBuffer;
PFNGLREADPIXELSPROC glReadPixels;
PFNGLGETBOOLEANVPROC glGetBooleanv;
PFNGLGETDOUBLEVPROC glGetDoublev;
PFNGLGETERRORPROC glGetError;
PFNGLGETFLOATVPROC glGetFloatv;
PFNGLGETINTEGERVPROC glGetIntegerv;
PFNGLGETSTRINGPROC glGetString;
PFNGLGETTEXIMAGEPROC glGetTexImage;
PFNGLGETTEXPARAMETERFVPROC glGetTexParameterfv;
PFNGLGETTEXPARAMETERIVPROC glGetTexParameteriv;
PFNGLGETTEXLEVELPARAMETERFVPROC glGetTexLevelParameterfv;
PFNGLGETTEXLEVELPARAMETERIVPROC glGetTexLevelParameteriv;
PFNGLISENABLEDPROC glIsEnabled;
PFNGLDEPTHRANGEPROC glDepthRange;
PFNGLVIEWPORTPROC glViewport;
/* ----------------------------- GL_VERSION_1_1 ---------------------------- \*/
PFNGLBINDTEXTUREPROC glBindTexture;
PFNGLDRAWARRAYSPROC glDrawArrays;
PFNGLDRAWELEMENTSPROC glDrawElements;
PFNGLCOPYTEXIMAGE2DPROC glCopyTexImage2D;
PFNGLCOPYTEXSUBIMAGE2DPROC glCopyTexSubImage2D;
PFNGLDELETETEXTURESPROC glDeleteTextures;
PFNGLGENTEXTURESPROC glGenTextures;
PFNGLGETPOINTERVPROC glGetPointerv;
PFNGLISTEXTUREPROC glIsTexture;
PFNGLTEXSUBIMAGE2DPROC glTexSubImage2D;
/* ----------------------------- GL_VERSION_1_2 ---------------------------- \*/
PFNGLCOPYTEXSUBIMAGE3DPROC glCopyTexSubImage3D;
PFNGLDRAWRANGEELEMENTSPROC glDrawRangeElements;
PFNGLTEXIMAGE3DPROC glTexImage3D;
PFNGLTEXSUBIMAGE3DPROC glTexSubImage3D;
/* ----------------------------- GL_VERSION_1_3 ---------------------------- \*/
PFNGLACTIVETEXTUREPROC glActiveTexture;
PFNGLCOMPRESSEDTEXIMAGE2DPROC glCompressedTexImage2D;
PFNGLCOMPRESSEDTEXIMAGE3DPROC glCompressedTexImage3D;
PFNGLCOMPRESSEDTEXSUBIMAGE2DPROC glCompressedTexSubImage2D;
PFNGLCOMPRESSEDTEXSUBIMAGE3DPROC glCompressedTexSubImage3D;
PFNGLGETCOMPRESSEDTEXIMAGEPROC glGetCompressedTexImage;
PFNGLSAMPLECOVERAGEPROC glSampleCoverage;
/* ----------------------------- GL_VERSION_1_4 ---------------------------- \*/
PFNGLBLENDCOLORPROC glBlendColor;
PFNGLBLENDEQUATIONPROC glBlendEquation;
PFNGLBLENDFUNCSEPARATEPROC glBlendFuncSeparate;
PFNGLMULTIDRAWARRAYSPROC glMultiDrawArrays;
PFNGLMULTIDRAWELEMENTSPROC glMultiDrawElements;
PFNGLPOINTPARAMETERFPROC glPointParameterf;
PFNGLPOINTPARAMETERFVPROC glPointParameterfv;
PFNGLPOINTPARAMETERIPROC glPointParameteri;
PFNGLPOINTPARAMETERIVPROC glPointParameteriv;
/* ----------------------------- GL_VERSION_1_5 ---------------------------- \*/
PFNGLBEGINQUERYPROC glBeginQuery;
PFNGLBINDBUFFERPROC glBindBuffer;
PFNGLBUFFERDATAPROC glBufferData;
PFNGLBUFFERSUBDATAPROC glBufferSubData;
PFNGLDELETEBUFFERSPROC glDeleteBuffers;
PFNGLDELETEQUERIESPROC glDeleteQueries;
PFNGLENDQUERYPROC glEndQuery;
PFNGLGENBUFFERSPROC glGenBuffers;
PFNGLGENQUERIESPROC glGenQueries;
PFNGLGETBUFFERPARAMETERIVPROC glGetBufferParameteriv;
PFNGLGETBUFFERPOINTERVPROC glGetBufferPointerv;
PFNGLGETBUFFERSUBDATAPROC glGetBufferSubData;
PFNGLGETQUERYOBJECTIVPROC glGetQueryObjectiv;
PFNGLGETQUERYOBJECTUIVPROC glGetQueryObjectuiv;
PFNGLGETQUERYIVPROC glGetQueryiv;
PFNGLISBUFFERPROC glIsBuffer;
PFNGLISQUERYPROC glIsQuery;
PFNGLMAPBUFFERPROC glMapBuffer;
PFNGLUNMAPBUFFERPROC glUnmapBuffer;
/* ----------------------------- GL_VERSION_2_0 ---------------------------- \*/
PFNGLATTACHSHADERPROC glAttachShader;
PFNGLBINDATTRIBLOCATIONPROC glBindAttribLocation;
PFNGLBLENDEQUATIONSEPARATEPROC glBlendEquationSeparate;
PFNGLCOMPILESHADERPROC glCompileShader;
PFNGLCREATEPROGRAMPROC glCreateProgram;
PFNGLCREATESHADERPROC glCreateShader;
PFNGLDELETEPROGRAMPROC glDeleteProgram;
PFNGLDELETESHADERPROC glDeleteShader;
PFNGLDETACHSHADERPROC glDetachShader;
PFNGLDISABLEVERTEXATTRIBARRAYPROC glDisableVertexAttribArray;
PFNGLDRAWBUFFERSPROC glDrawBuffers;
PFNGLENABLEVERTEXATTRIBARRAYPROC glEnableVertexAttribArray;
PFNGLGETACTIVEATTRIBPROC glGetActiveAttrib;
PFNGLGETACTIVEUNIFORMPROC glGetActiveUniform;
PFNGLGETATTACHEDSHADERSPROC glGetAttachedShaders;
PFNGLGETATTRIBLOCATIONPROC glGetAttribLocation;
PFNGLGETPROGRAMINFOLOGPROC glGetProgramInfoLog;
PFNGLGETPROGRAMIVPROC glGetProgramiv;
PFNGLGETSHADERINFOLOGPROC glGetShaderInfoLog;
PFNGLGETSHADERSOURCEPROC glGetShaderSource;
PFNGLGETSHADERIVPROC glGetShaderiv;
PFNGLGETUNIFORMLOCATIONPROC glGetUniformLocation;
PFNGLGETUNIFORMFVPROC glGetUniformfv;
PFNGLGETUNIFORMIVPROC glGetUniformiv;
PFNGLGETVERTEXATTRIBPOINTERVPROC glGetVertexAttribPointerv;
PFNGLGETVERTEXATTRIBDVPROC glGetVertexAttribdv;
PFNGLGETVERTEXATTRIBFVPROC glGetVertexAttribfv;
PFNGLGETVERTEXATTRIBIVPROC glGetVertexAttribiv;
PFNGLISPROGRAMPROC glIsProgram;
PFNGLISSHADERPROC glIsShader;
PFNGLLINKPROGRAMPROC glLinkProgram;
PFNGLSHADERSOURCEPROC glShaderSource;
PFNGLSTENCILFUNCSEPARATEPROC glStencilFuncSeparate;
PFNGLSTENCILMASKSEPARATEPROC glStencilMaskSeparate;
PFNGLSTENCILOPSEPARATEPROC glStencilOpSeparate;
PFNGLUNIFORM1FPROC glUniform1f;
PFNGLUNIFORM1FVPROC glUniform1fv;
PFNGLUNIFORM1IPROC glUniform1i;
PFNGLUNIFORM1IVPROC glUniform1iv;
PFNGLUNIFORM2FPROC glUniform2f;
PFNGLUNIFORM2FVPROC glUniform2fv;
PFNGLUNIFORM2IPROC glUniform2i;
PFNGLUNIFORM2IVPROC glUniform2iv;
PFNGLUNIFORM3FPROC glUniform3f;
PFNGLUNIFORM3FVPROC glUniform3fv;
PFNGLUNIFORM3IPROC glUniform3i;
PFNGLUNIFORM3IVPROC glUniform3iv;
PFNGLUNIFORM4FPROC glUniform4f;
PFNGLUNIFORM4FVPROC glUniform4fv;
PFNGLUNIFORM4IPROC glUniform4i;
PFNGLUNIFORM4IVPROC glUniform4iv;
PFNGLUNIFORMMATRIX2FVPROC glUniformMatrix2fv;
PFNGLUNIFORMMATRIX3FVPROC glUniformMatrix3fv;
PFNGLUNIFORMMATRIX4FVPROC glUniformMatrix4fv;
PFNGLUSEPROGRAMPROC glUseProgram;
PFNGLVALIDATEPROGRAMPROC glValidateProgram;
PFNGLVERTEXATTRIB1DPROC glVertexAttrib1d;
PFNGLVERTEXATTRIB1DVPROC glVertexAttrib1dv;
PFNGLVERTEXATTRIB1FPROC glVertexAttrib1f;
PFNGLVERTEXATTRIB1FVPROC glVertexAttrib1fv;
PFNGLVERTEXATTRIB1SPROC glVertexAttrib1s;
PFNGLVERTEXATTRIB1SVPROC glVertexAttrib1sv;
PFNGLVERTEXATTRIB2DPROC glVertexAttrib2d;
PFNGLVERTEXATTRIB2DVPROC glVertexAttrib2dv;
PFNGLVERTEXATTRIB2FPROC glVertexAttrib2f;
PFNGLVERTEXATTRIB2FVPROC glVertexAttrib2fv;
PFNGLVERTEXATTRIB2SPROC glVertexAttrib2s;
PFNGLVERTEXATTRIB2SVPROC glVertexAttrib2sv;
PFNGLVERTEXATTRIB3DPROC glVertexAttrib3d;
PFNGLVERTEXATTRIB3DVPROC glVertexAttrib3dv;
PFNGLVERTEXATTRIB3FPROC glVertexAttrib3f;
PFNGLVERTEXATTRIB3FVPROC glVertexAttrib3fv;
PFNGLVERTEXATTRIB3SPROC glVertexAttrib3s;
PFNGLVERTEXATTRIB3SVPROC glVertexAttrib3sv;
PFNGLVERTEXATTRIB4NBVPROC glVertexAttrib4Nbv;
PFNGLVERTEXATTRIB4NIVPROC glVertexAttrib4Niv;
PFNGLVERTEXATTRIB4NSVPROC glVertexAttrib4Nsv;
PFNGLVERTEXATTRIB4NUBPROC glVertexAttrib4Nub;
PFNGLVERTEXATTRIB4NUBVPROC glVertexAttrib4Nubv;
PFNGLVERTEXATTRIB4NUIVPROC glVertexAttrib4Nuiv;
PFNGLVERTEXATTRIB4NUSVPROC glVertexAttrib4Nusv;
PFNGLVERTEXATTRIB4BVPROC glVertexAttrib4bv;
PFNGLVERTEXATTRIB4DPROC glVertexAttrib4d;
PFNGLVERTEXATTRIB4DVPROC glVertexAttrib4dv;
PFNGLVERTEXATTRIB4FPROC glVertexAttrib4f;
PFNGLVERTEXATTRIB4FVPROC glVertexAttrib4fv;
PFNGLVERTEXATTRIB4IVPROC glVertexAttrib4iv;
PFNGLVERTEXATTRIB4SPROC glVertexAttrib4s;
PFNGLVERTEXATTRIB4SVPROC glVertexAttrib4sv;
PFNGLVERTEXATTRIB4UBVPROC glVertexAttrib4ubv;
PFNGLVERTEXATTRIB4UIVPROC glVertexAttrib4uiv;
PFNGLVERTEXATTRIB4USVPROC glVertexAttrib4usv;
PFNGLVERTEXATTRIBPOINTERPROC glVertexAttribPointer;
/* ----------------------------- GL_VERSION_2_1 ---------------------------- \*/
PFNGLUNIFORMMATRIX2X3FVPROC glUniformMatrix2x3fv;
PFNGLUNIFORMMATRIX2X3FVPROC glUniformMatrix3x2fv;
PFNGLUNIFORMMATRIX2X3FVPROC glUniformMatrix2x4fv;
PFNGLUNIFORMMATRIX2X3FVPROC glUniformMatrix4x2fv;
PFNGLUNIFORMMATRIX2X3FVPROC glUniformMatrix3x4fv;
PFNGLUNIFORMMATRIX2X3FVPROC glUniformMatrix4x3fv;
/* ----------------------------- GL_VERSION_3_0 ---------------------------- \*/
PFNGLBEGINCONDITIONALRENDERPROC glBeginConditionalRender;
PFNGLBEGINTRANSFORMFEEDBACKPROC glBeginTransformFeedback;
PFNGLBINDFRAGDATALOCATIONPROC glBindFragDataLocation;
PFNGLBINDBUFFERRANGEPROC glBindBufferRange;
PFNGLBINDBUFFERBASEPROC glBindBufferBase;
PFNGLCLAMPCOLORPROC glClampColor;
PFNGLCLEARBUFFERFIPROC glClearBufferfi;
PFNGLCLEARBUFFERFVPROC glClearBufferfv;
PFNGLCLEARBUFFERIVPROC glClearBufferiv;
PFNGLCLEARBUFFERUIVPROC glClearBufferuiv;
PFNGLCOLORMASKIPROC glColorMaski;
PFNGLDISABLEIPROC glDisablei;
PFNGLENABLEIPROC glEnablei;
PFNGLENDCONDITIONALRENDERPROC glEndConditionalRender;
PFNGLENDTRANSFORMFEEDBACKPROC glEndTransformFeedback;
PFNGLGETBOOLEANI_VPROC glGetBooleani_v;
PFNGLGETINTEGERI_VPROC glGetIntegeri_v;
PFNGLGETFRAGDATALOCATIONPROC glGetFragDataLocation;
PFNGLGETSTRINGIPROC glGetStringi;
PFNGLGETTEXPARAMETERIIVPROC glGetTexParameterIiv;
PFNGLGETTEXPARAMETERIUIVPROC glGetTexParameterIuiv;
PFNGLGETTRANSFORMFEEDBACKVARYINGPROC glGetTransformFeedbackVarying;
PFNGLGETUNIFORMUIVPROC glGetUniformuiv;
PFNGLGETVERTEXATTRIBIIVPROC glGetVertexAttribIiv;
PFNGLGETVERTEXATTRIBIUIVPROC glGetVertexAttribIuiv;
PFNGLISENABLEDIPROC glIsEnabledi;
PFNGLTEXPARAMETERIIVPROC glTexParameterIiv;
PFNGLTEXPARAMETERIUIVPROC glTexParameterIuiv;
PFNGLTRANSFORMFEEDBACKVARYINGSPROC glTransformFeedbackVaryings;
PFNGLUNIFORM1UIPROC glUniform1ui;
PFNGLUNIFORM1UIVPROC glUniform1uiv;
PFNGLUNIFORM2UIPROC glUniform2ui;
PFNGLUNIFORM2UIVPROC glUniform2uiv;
PFNGLUNIFORM3UIPROC glUniform3ui;
PFNGLUNIFORM3UIVPROC glUniform3uiv;
PFNGLUNIFORM4UIPROC glUniform4ui;
PFNGLUNIFORM4UIVPROC glUniform4uiv;
PFNGLVERTEXATTRIBI1IPROC glVertexAttribI1i;
PFNGLVERTEXATTRIBI1IVPROC glVertexAttribI1iv;
PFNGLVERTEXATTRIBI1UIPROC glVertexAttribI1ui;
PFNGLVERTEXATTRIBI1UIVPROC glVertexAttribI1uiv;
PFNGLVERTEXATTRIBI2IPROC glVertexAttribI2i;
PFNGLVERTEXATTRIBI2IVPROC glVertexAttribI2iv;
PFNGLVERTEXATTRIBI2UIPROC glVertexAttribI2ui;
PFNGLVERTEXATTRIBI2UIVPROC glVertexAttribI2uiv;
PFNGLVERTEXATTRIBI3IPROC glVertexAttribI3i;
PFNGLVERTEXATTRIBI3IVPROC glVertexAttribI3iv;
PFNGLVERTEXATTRIBI3UIPROC glVertexAttribI3ui;
PFNGLVERTEXATTRIBI3UIVPROC glVertexAttribI3uiv;
PFNGLVERTEXATTRIBI4BVPROC glVertexAttribI4bv;
PFNGLVERTEXATTRIBI4IPROC glVertexAttribI4i;
PFNGLVERTEXATTRIBI4IVPROC glVertexAttribI4iv;
PFNGLVERTEXATTRIBI4SVPROC glVertexAttribI4sv;
PFNGLVERTEXATTRIBI4UBVPROC glVertexAttribI4ubv;
PFNGLVERTEXATTRIBI4UIPROC glVertexAttribI4ui;
PFNGLVERTEXATTRIBI4UIVPROC glVertexAttribI4uiv;
PFNGLVERTEXATTRIBI4USVPROC glVertexAttribI4usv;
PFNGLVERTEXATTRIBIPOINTERPROC glVertexAttribIPointer;
PFNGLBINDFRAMEBUFFERPROC glBindFramebuffer;
PFNGLBINDRENDERBUFFERPROC glBindRenderbuffer;
PFNGLBLITFRAMEBUFFERPROC glBlitFramebuffer;
PFNGLCHECKFRAMEBUFFERSTATUSPROC glCheckFramebufferStatus;
PFNGLDELETEFRAMEBUFFERSPROC glDeleteFramebuffers;
PFNGLDELETERENDERBUFFERSPROC glDeleteRenderbuffers;
PFNGLFRAMEBUFFERRENDERBUFFERPROC glFramebufferRenderbuffer;
PFNGLFRAMEBUFFERTEXTURE1DPROC glFramebufferTexture1D;
PFNGLFRAMEBUFFERTEXTURE2DPROC glFramebufferTexture2D;
PFNGLFRAMEBUFFERTEXTURE3DPROC glFramebufferTexture3D;
PFNGLFRAMEBUFFERTEXTURELAYERPROC glFramebufferTextureLayer;
PFNGLGENFRAMEBUFFERSPROC glGenFramebuffers;
PFNGLGENRENDERBUFFERSPROC glGenRenderbuffers;
PFNGLGENERATEMIPMAPPROC glGenerateMipmap;
PFNGLGETFRAMEBUFFERATTACHMENTPARAMETERIVPROC glGetFramebufferAttachmentParameteriv;
PFNGLGETRENDERBUFFERPARAMETERIVPROC glGetRenderbufferParameteriv;
PFNGLISFRAMEBUFFERPROC glIsFramebuffer;
PFNGLISRENDERBUFFERPROC glIsRenderbuffer;
PFNGLRENDERBUFFERSTORAGEPROC glRenderbufferStorage;
PFNGLRENDERBUFFERSTORAGEMULTISAMPLEPROC glRenderbufferStorageMultisample;
PFNGLFLUSHMAPPEDBUFFERRANGEPROC glFlushMappedBufferRange;
PFNGLMAPBUFFERRANGEPROC glMapBufferRange;
PFNGLBINDVERTEXARRAYPROC glBindVertexArray;
PFNGLDELETEVERTEXARRAYSPROC glDeleteVertexArrays;
PFNGLGENVERTEXARRAYSPROC glGenVertexArrays;
PFNGLISVERTEXARRAYPROC glIsVertexArray;
/* ----------------------------- GL_VERSION_3_1 ---------------------------- \*/
PFNGLDRAWARRAYSINSTANCEDPROC glDrawArraysInstanced;
PFNGLDRAWELEMENTSINSTANCEDPROC glDrawElementsInstanced;
PFNGLPRIMITIVERESTARTINDEXPROC glPrimitiveRestartIndex;
PFNGLTEXBUFFERPROC glTexBuffer;
PFNGLGETACTIVEUNIFORMBLOCKNAMEPROC glGetActiveUniformBlockName;
PFNGLGETACTIVEUNIFORMBLOCKIVPROC glGetActiveUniformBlockiv;
PFNGLGETACTIVEUNIFORMNAMEPROC glGetActiveUniformName;
PFNGLGETACTIVEUNIFORMSIVPROC glGetActiveUniformsiv;
PFNGLGETUNIFORMBLOCKINDEXPROC glGetUniformBlockIndex;
PFNGLGETUNIFORMINDICESPROC glGetUniformIndices;
PFNGLUNIFORMBLOCKBINDINGPROC glUniformBlockBinding;
PFNGLCOPYBUFFERSUBDATAPROC glCopyBufferSubData;
/* ----------------------------- GL_VERSION_3_2 ---------------------------- \*/
PFNGLFRAMEBUFFERTEXTUREPROC glFramebufferTexture;
PFNGLGETBUFFERPARAMETERI64VPROC glGetBufferParameteri64v;
PFNGLGETINTEGER64I_VPROC glGetInteger64i_v;
PFNGLDRAWELEMENTSBASEVERTEXPROC glDrawElementsBaseVertex;
PFNGLDRAWELEMENTSINSTANCEDBASEVERTEXPROC glDrawElementsInstancedBaseVertex;
PFNGLDRAWRANGEELEMENTSBASEVERTEXPROC glDrawRangeElementsBaseVertex;
PFNGLMULTIDRAWELEMENTSBASEVERTEXPROC glMultiDrawElementsBaseVertex;
PFNGLPROVOKINGVERTEXPROC glProvokingVertex;
PFNGLCLIENTWAITSYNCPROC glClientWaitSync;
PFNGLDELETESYNCPROC glDeleteSync;
PFNGLFENCESYNCPROC glFenceSync;
PFNGLGETINTEGER64VPROC glGetInteger64v;
PFNGLGETSYNCIVPROC glGetSynciv;
PFNGLISSYNCPROC glIsSync;
PFNGLWAITSYNCPROC glWaitSync;
PFNGLGETMULTISAMPLEFVPROC glGetMultisamplefv;
PFNGLSAMPLEMASKIPROC glSampleMaski;
PFNGLTEXIMAGE2DMULTISAMPLEPROC glTexImage2DMultisample;
PFNGLTEXIMAGE3DMULTISAMPLEPROC glTexImage3DMultisample;
/* ----------------------------- GL_VERSION_3_3 ---------------------------- \*/
PFNGLVERTEXATTRIBDIVISORPROC glVertexAttribDivisor;
PFNGLBINDFRAGDATALOCATIONINDEXEDPROC glBindFragDataLocationIndexed;
PFNGLGETFRAGDATAINDEXPROC glGetFragDataIndex;
PFNGLBINDSAMPLERPROC glBindSampler;
PFNGLDELETESAMPLERSPROC glDeleteSamplers;
PFNGLGENSAMPLERSPROC glGenSamplers;
PFNGLGETSAMPLERPARAMETERIIVPROC glGetSamplerParameterIiv;
PFNGLGETSAMPLERPARAMETERIUIVPROC glGetSamplerParameterIuiv;
PFNGLGETSAMPLERPARAMETERFVPROC glGetSamplerParameterfv;
PFNGLGETSAMPLERPARAMETERIVPROC glGetSamplerParameteriv;
PFNGLISSAMPLERPROC glIsSampler;
PFNGLSAMPLERPARAMETERIIVPROC glSamplerParameterIiv;
PFNGLSAMPLERPARAMETERIUIVPROC glSamplerParameterIuiv;
PFNGLSAMPLERPARAMETERFPROC glSamplerParameterf;
PFNGLSAMPLERPARAMETERFVPROC glSamplerParameterfv;
PFNGLSAMPLERPARAMETERIPROC glSamplerParameteri;
PFNGLSAMPLERPARAMETERIVPROC glSamplerParameteriv;
PFNGLGETQUERYOBJECTI64VPROC glGetQueryObjecti64v;
PFNGLGETQUERYOBJECTUI64VPROC glGetQueryObjectui64v;
PFNGLQUERYCOUNTERPROC glQueryCounter;
PFNGLVERTEXATTRIBP1UIPROC glVertexAttribP1ui;
PFNGLVERTEXATTRIBP1UIVPROC glVertexAttribP1uiv;
PFNGLVERTEXATTRIBP2UIPROC glVertexAttribP2ui;
PFNGLVERTEXATTRIBP2UIVPROC glVertexAttribP2uiv;
PFNGLVERTEXATTRIBP3UIPROC glVertexAttribP3ui;
PFNGLVERTEXATTRIBP3UIVPROC glVertexAttribP3uiv;
PFNGLVERTEXATTRIBP4UIPROC glVertexAttribP4ui;
PFNGLVERTEXATTRIBP4UIVPROC glVertexAttribP4uiv;
/* ----------------------------- GL_VERSION_4_0 ---------------------------- \*/
PFNGLBLENDEQUATIONSEPARATEIPROC glBlendEquationSeparatei;
PFNGLBLENDEQUATIONIPROC glBlendEquationi;
PFNGLBLENDFUNCSEPARATEIPROC glBlendFuncSeparatei;
PFNGLBLENDFUNCIPROC glBlendFunci;
PFNGLMINSAMPLESHADINGPROC glMinSampleShading;
PFNGLDRAWARRAYSINDIRECTPROC glDrawArraysIndirect;
PFNGLDRAWELEMENTSINDIRECTPROC glDrawElementsIndirect;
PFNGLGETUNIFORMDVPROC glGetUniformdv;
PFNGLUNIFORM1DPROC glUniform1d;
PFNGLUNIFORM1DVPROC glUniform1dv;
PFNGLUNIFORM2DPROC glUniform2d;
PFNGLUNIFORM2DVPROC glUniform2dv;
PFNGLUNIFORM3DPROC glUniform3d;
PFNGLUNIFORM3DVPROC glUniform3dv;
PFNGLUNIFORM4DPROC glUniform4d;
PFNGLUNIFORM4DVPROC glUniform4dv;
PFNGLUNIFORMMATRIX2DVPROC glUniformMatrix2dv;
PFNGLUNIFORMMATRIX2X3DVPROC glUniformMatrix2x3dv;
PFNGLUNIFORMMATRIX2X4DVPROC glUniformMatrix2x4dv;
PFNGLUNIFORMMATRIX3DVPROC glUniformMatrix3dv;
PFNGLUNIFORMMATRIX3X2DVPROC glUniformMatrix3x2dv;
PFNGLUNIFORMMATRIX3X4DVPROC glUniformMatrix3x4dv;
PFNGLUNIFORMMATRIX4DVPROC glUniformMatrix4dv;
PFNGLUNIFORMMATRIX4X2DVPROC glUniformMatrix4x2dv;
PFNGLUNIFORMMATRIX4X3DVPROC glUniformMatrix4x3dv;
PFNGLGETACTIVESUBROUTINENAMEPROC glGetActiveSubroutineName;
PFNGLGETACTIVESUBROUTINEUNIFORMNAMEPROC glGetActiveSubroutineUniformName;
PFNGLGETACTIVESUBROUTINEUNIFORMIVPROC glGetActiveSubroutineUniformiv;
PFNGLGETPROGRAMSTAGEIVPROC glGetProgramStageiv;
PFNGLGETSUBROUTINEINDEXPROC glGetSubroutineIndex;
PFNGLGETSUBROUTINEUNIFORMLOCATIONPROC glGetSubroutineUniformLocation;
PFNGLGETUNIFORMSUBROUTINEUIVPROC glGetUniformSubroutineuiv;
PFNGLUNIFORMSUBROUTINESUIVPROC glUniformSubroutinesuiv;
PFNGLPATCHPARAMETERFVPROC glPatchParameterfv;
PFNGLPATCHPARAMETERIPROC glPatchParameteri;
PFNGLBINDTRANSFORMFEEDBACKPROC glBindTransformFeedback;
PFNGLDELETETRANSFORMFEEDBACKSPROC glDeleteTransformFeedbacks;
PFNGLDRAWTRANSFORMFEEDBACKPROC glDrawTransformFeedback;
PFNGLGENTRANSFORMFEEDBACKSPROC glGenTransformFeedbacks;
PFNGLISTRANSFORMFEEDBACKPROC glIsTransformFeedback;
PFNGLPAUSETRANSFORMFEEDBACKPROC glPauseTransformFeedback;
PFNGLRESUMETRANSFORMFEEDBACKPROC glResumeTransformFeedback;
PFNGLBEGINQUERYINDEXEDPROC glBeginQueryIndexed;
PFNGLDRAWTRANSFORMFEEDBACKSTREAMPROC glDrawTransformFeedbackStream;
PFNGLENDQUERYINDEXEDPROC glEndQueryIndexed;
PFNGLGETQUERYINDEXEDIVPROC glGetQueryIndexediv;
/* ----------------------------- GL_VERSION_4_1 ---------------------------- */
PFNGLCLEARDEPTHFPROC glClearDepthf;
PFNGLDEPTHRANGEFPROC glDepthRangef;
PFNGLRELEASESHADERCOMPILERPROC glReleaseShaderCompiler;
PFNGLGETSHADERPRECISIONFORMATPROC glGetShaderPrecisionFormat;
PFNGLSHADERBINARYPROC glShaderBinary;
PFNGLGETPROGRAMBINARYPROC glGetProgramBinary;
PFNGLPROGRAMBINARYPROC glProgramBinary;
PFNGLPROGRAMPARAMETERIPROC glProgramParameteri;
PFNGLACTIVESHADERPROGRAMPROC glActiveShaderProgram;
PFNGLBINDPROGRAMPIPELINEPROC glBindProgramPipeline;
PFNGLCREATESHADERPROGRAMVPROC glCreateShaderProgramv;
PFNGLDELETEPROGRAMPIPELINESPROC glDeleteProgramPipelines;
PFNGLGENPROGRAMPIPELINESPROC glGenProgramPipelines;
PFNGLGETPROGRAMPIPELINEINFOLOGPROC glGetProgramPipelineInfoLog;
PFNGLGETPROGRAMPIPELINEIVPROC glGetProgramPipelineiv;
PFNGLISPROGRAMPIPELINEPROC glIsProgramPipeline;
PFNGLPROGRAMUNIFORM1DPROC glProgramUniform1d;
PFNGLPROGRAMUNIFORM1DVPROC glProgramUniform1dv;
PFNGLPROGRAMUNIFORM1FPROC glProgramUniform1f;
PFNGLPROGRAMUNIFORM1FVPROC glProgramUniform1fv;
PFNGLPROGRAMUNIFORM1IPROC glProgramUniform1i;
PFNGLPROGRAMUNIFORM1IVPROC glProgramUniform1iv;
PFNGLPROGRAMUNIFORM1UIPROC glProgramUniform1ui;
PFNGLPROGRAMUNIFORM1UIVPROC glProgramUniform1uiv;
PFNGLPROGRAMUNIFORM2DPROC glProgramUniform2d;
PFNGLPROGRAMUNIFORM2DVPROC glProgramUniform2dv;
PFNGLPROGRAMUNIFORM2FPROC glProgramUniform2f;
PFNGLPROGRAMUNIFORM2FVPROC glProgramUniform2fv;
PFNGLPROGRAMUNIFORM2IPROC glProgramUniform2i;
PFNGLPROGRAMUNIFORM2IVPROC glProgramUniform2iv;
PFNGLPROGRAMUNIFORM2UIPROC glProgramUniform2ui;
PFNGLPROGRAMUNIFORM2UIVPROC glProgramUniform2uiv;
PFNGLPROGRAMUNIFORM3DPROC glProgramUniform3d;
PFNGLPROGRAMUNIFORM3DVPROC glProgramUniform3dv;
PFNGLPROGRAMUNIFORM3FPROC glProgramUniform3f;
PFNGLPROGRAMUNIFORM3FVPROC glProgramUniform3fv;
PFNGLPROGRAMUNIFORM3IPROC glProgramUniform3i;
PFNGLPROGRAMUNIFORM3IVPROC glProgramUniform3iv;
PFNGLPROGRAMUNIFORM3UIPROC glProgramUniform3ui;
PFNGLPROGRAMUNIFORM3UIVPROC glProgramUniform3uiv;
PFNGLPROGRAMUNIFORM4DPROC glProgramUniform4d;
PFNGLPROGRAMUNIFORM4DVPROC glProgramUniform4dv;
PFNGLPROGRAMUNIFORM4FPROC glProgramUniform4f;
PFNGLPROGRAMUNIFORM4FVPROC glProgramUniform4fv;
PFNGLPROGRAMUNIFORM4IPROC glProgramUniform4i;
PFNGLPROGRAMUNIFORM4IVPROC glProgramUniform4iv;
PFNGLPROGRAMUNIFORM4UIPROC glProgramUniform4ui;
PFNGLPROGRAMUNIFORM4UIVPROC glProgramUniform4uiv;
PFNGLPROGRAMUNIFORMMATRIX2DVPROC glProgramUniformMatrix2dv;
PFNGLPROGRAMUNIFORMMATRIX2FVPROC glProgramUniformMatrix2fv;
PFNGLPROGRAMUNIFORMMATRIX2X3DVPROC glProgramUniformMatrix2x3dv;
PFNGLPROGRAMUNIFORMMATRIX2X3FVPROC glProgramUniformMatrix2x3fv;
PFNGLPROGRAMUNIFORMMATRIX2X4DVPROC glProgramUniformMatrix2x4dv;
PFNGLPROGRAMUNIFORMMATRIX2X4FVPROC glProgramUniformMatrix2x4fv;
PFNGLPROGRAMUNIFORMMATRIX3DVPROC glProgramUniformMatrix3dv;
PFNGLPROGRAMUNIFORMMATRIX3FVPROC glProgramUniformMatrix3fv;
PFNGLPROGRAMUNIFORMMATRIX3X2DVPROC glProgramUniformMatrix3x2dv;
PFNGLPROGRAMUNIFORMMATRIX3X2FVPROC glProgramUniformMatrix3x2fv;
PFNGLPROGRAMUNIFORMMATRIX3X4DVPROC glProgramUniformMatrix3x4dv;
PFNGLPROGRAMUNIFORMMATRIX3X4FVPROC glProgramUniformMatrix3x4fv;
PFNGLPROGRAMUNIFORMMATRIX4DVPROC glProgramUniformMatrix4dv;
PFNGLPROGRAMUNIFORMMATRIX4FVPROC glProgramUniformMatrix4fv;
PFNGLPROGRAMUNIFORMMATRIX4X2DVPROC glProgramUniformMatrix4x2dv;
PFNGLPROGRAMUNIFORMMATRIX4X2FVPROC glProgramUniformMatrix4x2fv;
PFNGLPROGRAMUNIFORMMATRIX4X3DVPROC glProgramUniformMatrix4x3dv;
PFNGLPROGRAMUNIFORMMATRIX4X3FVPROC glProgramUniformMatrix4x3fv;
PFNGLUSEPROGRAMSTAGESPROC glUseProgramStages;
PFNGLVALIDATEPROGRAMPIPELINEPROC glValidateProgramPipeline;
PFNGLGETVERTEXATTRIBLDVPROC glGetVertexAttribLdv;
PFNGLVERTEXATTRIBL1DPROC glVertexAttribL1d;
PFNGLVERTEXATTRIBL1DVPROC glVertexAttribL1dv;
PFNGLVERTEXATTRIBL2DPROC glVertexAttribL2d;
PFNGLVERTEXATTRIBL2DVPROC glVertexAttribL2dv;
PFNGLVERTEXATTRIBL3DPROC glVertexAttribL3d;
PFNGLVERTEXATTRIBL3DVPROC glVertexAttribL3dv;
PFNGLVERTEXATTRIBL4DPROC glVertexAttribL4d;
PFNGLVERTEXATTRIBL4DVPROC glVertexAttribL4dv;
PFNGLVERTEXATTRIBLPOINTERPROC glVertexAttribLPointer;
PFNGLDEPTHRANGEARRAYVPROC glDepthRangeArrayv;
PFNGLDEPTHRANGEINDEXEDPROC glDepthRangeIndexed;
PFNGLGETDOUBLEI_VPROC glGetDoublei_v;
PFNGLGETFLOATI_VPROC glGetFloati_v;
PFNGLSCISSORARRAYVPROC glScissorArrayv;
PFNGLSCISSORINDEXEDPROC glScissorIndexed;
PFNGLSCISSORINDEXEDVPROC glScissorIndexedv;
PFNGLVIEWPORTARRAYVPROC glViewportArrayv;
PFNGLVIEWPORTINDEXEDFPROC glViewportIndexedf;
PFNGLVIEWPORTINDEXEDFVPROC glViewportIndexedfv;
/* ----------------------------- GL_VERSION_4_2 ---------------------------- */
PFNGLDRAWARRAYSINSTANCEDBASEINSTANCEPROC glDrawArraysInstancedBaseInstance;
PFNGLDRAWELEMENTSINSTANCEDBASEINSTANCEPROC glDrawElementsInstancedBaseInstance;
PFNGLDRAWELEMENTSINSTANCEDBASEVERTEXBASEINSTANCEPROC glDrawElementsInstancedBaseVertexBaseInstance;
PFNGLGETINTERNALFORMATIVPROC glGetInternalformativ;
PFNGLGETACTIVEATOMICCOUNTERBUFFERIVPROC glGetActiveAtomicCounterBufferiv;
PFNGLBINDIMAGETEXTUREPROC glBindImageTexture;
PFNGLMEMORYBARRIERPROC glMemoryBarrier;
PFNGLTEXSTORAGE1DPROC glTexStorage1D;
PFNGLTEXSTORAGE2DPROC glTexStorage2D;
PFNGLTEXSTORAGE3DPROC glTexStorage3D;
PFNGLDRAWTRANSFORMFEEDBACKINSTANCEDPROC glDrawTransformFeedbackInstanced;
PFNGLDRAWTRANSFORMFEEDBACKSTREAMINSTANCEDPROC glDrawTransformFeedbackStreamInstanced;
/* ----------------------------- GL_VERSION_4_3 ---------------------------- */
PFNGLCLEARBUFFERDATAPROC glClearBufferData;
PFNGLCLEARBUFFERSUBDATAPROC glClearBufferSubData;
PFNGLCLEARNAMEDBUFFERDATAEXTPROC glClearNamedBufferDataEXT;
PFNGLCLEARNAMEDBUFFERSUBDATAEXTPROC glClearNamedBufferSubDataEXT;
PFNGLDISPATCHCOMPUTEPROC glDispatchCompute;
PFNGLDISPATCHCOMPUTEINDIRECTPROC glDispatchComputeIndirect;
PFNGLCOPYIMAGESUBDATAPROC glCopyImageSubData;
PFNGLFRAMEBUFFERPARAMETERIPROC glFramebufferParameteri;
PFNGLGETFRAMEBUFFERPARAMETERIVPROC glGetFramebufferParameteriv;
PFNGLGETNAMEDFRAMEBUFFERPARAMETERIVEXTPROC glGetNamedFramebufferParameterivEXT;
PFNGLNAMEDFRAMEBUFFERPARAMETERIEXTPROC glNamedFramebufferParameteriEXT;
PFNGLGETINTERNALFORMATI64VPROC glGetInternalformati64v;
PFNGLINVALIDATEBUFFERDATAPROC glInvalidateBufferData;
PFNGLINVALIDATEBUFFERSUBDATAPROC glInvalidateBufferSubData;
PFNGLINVALIDATEFRAMEBUFFERPROC glInvalidateFramebuffer;
PFNGLINVALIDATESUBFRAMEBUFFERPROC glInvalidateSubFramebuffer;
PFNGLINVALIDATETEXIMAGEPROC glInvalidateTexImage;
PFNGLINVALIDATETEXSUBIMAGEPROC glInvalidateTexSubImage;
PFNGLMULTIDRAWARRAYSINDIRECTPROC glMultiDrawArraysIndirect;
PFNGLMULTIDRAWELEMENTSINDIRECTPROC glMultiDrawElementsIndirect;
PFNGLGETPROGRAMINTERFACEIVPROC glGetProgramInterfaceiv;
PFNGLGETPROGRAMRESOURCEINDEXPROC glGetProgramResourceIndex;
PFNGLGETPROGRAMRESOURCELOCATIONPROC glGetProgramResourceLocation;
PFNGLGETPROGRAMRESOURCELOCATIONINDEXPROC glGetProgramResourceLocationIndex;
PFNGLGETPROGRAMRESOURCENAMEPROC glGetProgramResourceName;
PFNGLGETPROGRAMRESOURCEIVPROC glGetProgramResourceiv;
PFNGLSHADERSTORAGEBLOCKBINDINGPROC glShaderStorageBlockBinding;
PFNGLTEXBUFFERRANGEPROC glTexBufferRange;
PFNGLTEXTUREBUFFERRANGEEXTPROC glTextureBufferRangeEXT;
PFNGLTEXSTORAGE2DMULTISAMPLEPROC glTexStorage2DMultisample;
PFNGLTEXSTORAGE3DMULTISAMPLEPROC glTexStorage3DMultisample;
PFNGLTEXTURESTORAGE2DMULTISAMPLEEXTPROC glTextureStorage2DMultisampleEXT;
PFNGLTEXTURESTORAGE3DMULTISAMPLEEXTPROC glTextureStorage3DMultisampleEXT;
PFNGLTEXTUREVIEWPROC glTextureView;
PFNGLBINDVERTEXBUFFERPROC glBindVertexBuffer;
PFNGLVERTEXARRAYBINDVERTEXBUFFEREXTPROC glVertexArrayBindVertexBufferEXT;
PFNGLVERTEXARRAYVERTEXATTRIBBINDINGEXTPROC glVertexArrayVertexAttribBindingEXT;
PFNGLVERTEXARRAYVERTEXATTRIBFORMATEXTPROC glVertexArrayVertexAttribFormatEXT;
PFNGLVERTEXARRAYVERTEXATTRIBIFORMATEXTPROC glVertexArrayVertexAttribIFormatEXT;
PFNGLVERTEXARRAYVERTEXATTRIBLFORMATEXTPROC glVertexArrayVertexAttribLFormatEXT;
PFNGLVERTEXARRAYVERTEXBINDINGDIVISOREXTPROC glVertexArrayVertexBindingDivisorEXT;
PFNGLVERTEXATTRIBBINDINGPROC glVertexAttribBinding;
PFNGLVERTEXATTRIBFORMATPROC glVertexAttribFormat;
PFNGLVERTEXATTRIBIFORMATPROC glVertexAttribIFormat;
PFNGLVERTEXATTRIBLFORMATPROC glVertexAttribLFormat;
PFNGLVERTEXBINDINGDIVISORPROC glVertexBindingDivisor;
PFNGLDEBUGMESSAGECALLBACKPROC glDebugMessageCallback;
PFNGLDEBUGMESSAGECONTROLPROC glDebugMessageControl;
PFNGLDEBUGMESSAGEINSERTPROC glDebugMessageInsert;
PFNGLGETDEBUGMESSAGELOGPROC glGetDebugMessageLog;
PFNGLGETOBJECTLABELPROC glGetObjectLabel;
PFNGLGETOBJECTPTRLABELPROC glGetObjectPtrLabel;
PFNGLOBJECTLABELPROC glObjectLabel;
PFNGLOBJECTPTRLABELPROC glObjectPtrLabel;
PFNGLPOPDEBUGGROUPPROC glPopDebugGroup;
PFNGLPUSHDEBUGGROUPPROC glPushDebugGroup;
/* ----------------------------- GL_VERSION_4_4 ---------------------------- */
PFNGLBUFFERSTORAGEPROC glBufferStorage;
PFNGLCLEARTEXIMAGEPROC glClearTexImage;
PFNGLCLEARTEXSUBIMAGEPROC glClearTexSubImage;
PFNGLBINDBUFFERSBASEPROC glBindBuffersBase;
PFNGLBINDBUFFERSRANGEPROC glBindBuffersRange;
PFNGLBINDIMAGETEXTURESPROC glBindImageTextures;
PFNGLBINDSAMPLERSPROC glBindSamplers;
PFNGLBINDTEXTURESPROC glBindTextures;
PFNGLBINDVERTEXBUFFERSPROC glBindVertexBuffers;
/* ----------------------------- GL_VERSION_4_5 ---------------------------- */
PFNGLGETGRAPHICSRESETSTATUSPROC glGetGraphicsResetStatus;
PFNGLGETNCOMPRESSEDTEXIMAGEPROC glGetnCompressedTexImage;
PFNGLGETNTEXIMAGEPROC glGetnTexImage;
PFNGLGETNUNIFORMDVPROC glGetnUniformdv;
PFNGLCLIPCONTROLPROC glClipControl;
PFNGLBINDTEXTUREUNITPROC glBindTextureUnit;
PFNGLBLITNAMEDFRAMEBUFFERPROC glBlitNamedFramebuffer;
PFNGLCHECKNAMEDFRAMEBUFFERSTATUSPROC glCheckNamedFramebufferStatus;
PFNGLCLEARNAMEDBUFFERDATAPROC glClearNamedBufferData;
PFNGLCLEARNAMEDBUFFERSUBDATAPROC glClearNamedBufferSubData;
PFNGLCLEARNAMEDFRAMEBUFFERFIPROC glClearNamedFramebufferfi;
PFNGLCLEARNAMEDFRAMEBUFFERFVPROC glClearNamedFramebufferfv;
PFNGLCLEARNAMEDFRAMEBUFFERIVPROC glClearNamedFramebufferiv;
PFNGLCLEARNAMEDFRAMEBUFFERUIVPROC glClearNamedFramebufferuiv;
PFNGLCOMPRESSEDTEXTURESUBIMAGE1DPROC glCompressedTextureSubImage1D;
PFNGLCOMPRESSEDTEXTURESUBIMAGE2DPROC glCompressedTextureSubImage2D;
PFNGLCOMPRESSEDTEXTURESUBIMAGE3DPROC glCompressedTextureSubImage3D;
PFNGLCOPYNAMEDBUFFERSUBDATAPROC glCopyNamedBufferSubData;
PFNGLCOPYTEXTURESUBIMAGE1DPROC glCopyTextureSubImage1D;
PFNGLCOPYTEXTURESUBIMAGE2DPROC glCopyTextureSubImage2D;
PFNGLCOPYTEXTURESUBIMAGE3DPROC glCopyTextureSubImage3D;
PFNGLCREATEBUFFERSPROC glCreateBuffers;
PFNGLCREATEFRAMEBUFFERSPROC glCreateFramebuffers;
PFNGLCREATEPROGRAMPIPELINESPROC glCreateProgramPipelines;
PFNGLCREATEQUERIESPROC glCreateQueries;
PFNGLCREATERENDERBUFFERSPROC glCreateRenderbuffers;
PFNGLCREATESAMPLERSPROC glCreateSamplers;
PFNGLCREATETEXTURESPROC glCreateTextures;
PFNGLCREATETRANSFORMFEEDBACKSPROC glCreateTransformFeedbacks;
PFNGLCREATEVERTEXARRAYSPROC glCreateVertexArrays;
PFNGLDISABLEVERTEXARRAYATTRIBPROC glDisableVertexArrayAttrib;
PFNGLENABLEVERTEXARRAYATTRIBPROC glEnableVertexArrayAttrib;
PFNGLFLUSHMAPPEDNAMEDBUFFERRANGEPROC glFlushMappedNamedBufferRange;
PFNGLGENERATETEXTUREMIPMAPPROC glGenerateTextureMipmap;
PFNGLGETCOMPRESSEDTEXTUREIMAGEPROC glGetCompressedTextureImage;
PFNGLGETNAMEDBUFFERPARAMETERI64VPROC glGetNamedBufferParameteri64v;
PFNGLGETNAMEDBUFFERPARAMETERIVPROC glGetNamedBufferParameteriv;
PFNGLGETNAMEDBUFFERPOINTERVPROC glGetNamedBufferPointerv;
PFNGLGETNAMEDBUFFERSUBDATAPROC glGetNamedBufferSubData;
PFNGLGETNAMEDFRAMEBUFFERATTACHMENTPARAMETERIVPROC glGetNamedFramebufferAttachmentParameteriv;
PFNGLGETNAMEDFRAMEBUFFERPARAMETERIVPROC glGetNamedFramebufferParameteriv;
PFNGLGETNAMEDRENDERBUFFERPARAMETERIVPROC glGetNamedRenderbufferParameteriv;
PFNGLGETQUERYBUFFEROBJECTI64VPROC glGetQueryBufferObjecti64v;
PFNGLGETQUERYBUFFEROBJECTIVPROC glGetQueryBufferObjectiv;
PFNGLGETQUERYBUFFEROBJECTUI64VPROC glGetQueryBufferObjectui64v;
PFNGLGETQUERYBUFFEROBJECTUIVPROC glGetQueryBufferObjectuiv;
PFNGLGETTEXTUREIMAGEPROC glGetTextureImage;
PFNGLGETTEXTURELEVELPARAMETERFVPROC glGetTextureLevelParameterfv;
PFNGLGETTEXTURELEVELPARAMETERIVPROC glGetTextureLevelParameteriv;
PFNGLGETTEXTUREPARAMETERIIVPROC glGetTextureParameterIiv;
PFNGLGETTEXTUREPARAMETERIUIVPROC glGetTextureParameterIuiv;
PFNGLGETTEXTUREPARAMETERFVPROC glGetTextureParameterfv;
PFNGLGETTEXTUREPARAMETERIVPROC glGetTextureParameteriv;
PFNGLGETTRANSFORMFEEDBACKI64_VPROC glGetTransformFeedbacki64_v;
PFNGLGETTRANSFORMFEEDBACKI_VPROC glGetTransformFeedbacki_v;
PFNGLGETTRANSFORMFEEDBACKIVPROC glGetTransformFeedbackiv;
PFNGLGETVERTEXARRAYINDEXED64IVPROC glGetVertexArrayIndexed64iv;
PFNGLGETVERTEXARRAYINDEXEDIVPROC glGetVertexArrayIndexediv;
PFNGLGETVERTEXARRAYIVPROC glGetVertexArrayiv;
PFNGLINVALIDATENAMEDFRAMEBUFFERDATAPROC glInvalidateNamedFramebufferData;
PFNGLINVALIDATENAMEDFRAMEBUFFERSUBDATAPROC glInvalidateNamedFramebufferSubData;
PFNGLMAPNAMEDBUFFERPROC glMapNamedBuffer;
PFNGLMAPNAMEDBUFFERRANGEPROC glMapNamedBufferRange;
PFNGLNAMEDBUFFERDATAPROC glNamedBufferData;
PFNGLNAMEDBUFFERSTORAGEPROC glNamedBufferStorage;
PFNGLNAMEDBUFFERSUBDATAPROC glNamedBufferSubData;
PFNGLNAMEDFRAMEBUFFERDRAWBUFFERPROC glNamedFramebufferDrawBuffer;
PFNGLNAMEDFRAMEBUFFERDRAWBUFFERSPROC glNamedFramebufferDrawBuffers;
PFNGLNAMEDFRAMEBUFFERPARAMETERIPROC glNamedFramebufferParameteri;
PFNGLNAMEDFRAMEBUFFERREADBUFFERPROC glNamedFramebufferReadBuffer;
PFNGLNAMEDFRAMEBUFFERRENDERBUFFERPROC glNamedFramebufferRenderbuffer;
PFNGLNAMEDFRAMEBUFFERTEXTUREPROC glNamedFramebufferTexture;
PFNGLNAMEDFRAMEBUFFERTEXTURELAYERPROC glNamedFramebufferTextureLayer;
PFNGLNAMEDRENDERBUFFERSTORAGEPROC glNamedRenderbufferStorage;
PFNGLNAMEDRENDERBUFFERSTORAGEMULTISAMPLEPROC glNamedRenderbufferStorageMultisample;
PFNGLTEXTUREBUFFERPROC glTextureBuffer;
PFNGLTEXTUREBUFFERRANGEPROC glTextureBufferRange;
PFNGLTEXTUREPARAMETERIIVPROC glTextureParameterIiv;
PFNGLTEXTUREPARAMETERIUIVPROC glTextureParameterIuiv;
PFNGLTEXTUREPARAMETERFPROC glTextureParameterf;
PFNGLTEXTUREPARAMETERFVPROC glTextureParameterfv;
PFNGLTEXTUREPARAMETERIPROC glTextureParameteri;
PFNGLTEXTUREPARAMETERIVPROC glTextureParameteriv;
PFNGLTEXTURESTORAGE1DPROC glTextureStorage1D;
PFNGLTEXTURESTORAGE2DPROC glTextureStorage2D;
PFNGLTEXTURESTORAGE2DMULTISAMPLEPROC glTextureStorage2DMultisample;
PFNGLTEXTURESTORAGE3DPROC glTextureStorage3D;
PFNGLTEXTURESTORAGE3DMULTISAMPLEPROC glTextureStorage3DMultisample;
PFNGLTEXTURESUBIMAGE1DPROC glTextureSubImage1D;
PFNGLTEXTURESUBIMAGE2DPROC glTextureSubImage2D;
PFNGLTEXTURESUBIMAGE3DPROC glTextureSubImage3D;
PFNGLTRANSFORMFEEDBACKBUFFERBASEPROC glTransformFeedbackBufferBase;
PFNGLTRANSFORMFEEDBACKBUFFERRANGEPROC glTransformFeedbackBufferRange;
PFNGLUNMAPNAMEDBUFFERPROC glUnmapNamedBuffer;
PFNGLVERTEXARRAYATTRIBBINDINGPROC glVertexArrayAttribBinding;
PFNGLVERTEXARRAYATTRIBFORMATPROC glVertexArrayAttribFormat;
PFNGLVERTEXARRAYATTRIBIFORMATPROC glVertexArrayAttribIFormat;
PFNGLVERTEXARRAYATTRIBLFORMATPROC glVertexArrayAttribLFormat;
PFNGLVERTEXARRAYBINDINGDIVISORPROC glVertexArrayBindingDivisor;
PFNGLVERTEXARRAYELEMENTBUFFERPROC glVertexArrayElementBuffer;
PFNGLVERTEXARRAYVERTEXBUFFERPROC glVertexArrayVertexBuffer;
PFNGLVERTEXARRAYVERTEXBUFFERSPROC glVertexArrayVertexBuffers;
PFNGLGETNUNIFORMFVPROC glGetnUniformfv;
PFNGLGETNUNIFORMIVPROC glGetnUniformiv;
PFNGLGETNUNIFORMUIVPROC glGetnUniformuiv;
PFNGLREADNPIXELSPROC glReadnPixels;
PFNGLTEXTUREBARRIERPROC glTextureBarrier;
PFNGLMEMORYBARRIERBYREGIONPROC glMemoryBarrierByRegion;
PFNGLGETCOMPRESSEDTEXTURESUBIMAGEPROC glGetCompressedTextureSubImage;
PFNGLGETTEXTURESUBIMAGEPROC glGetTextureSubImage;
/* ----------------------------- GL_VERSION_4_6 ---------------------------- */
PFNGLMULTIDRAWARRAYSINDIRECTCOUNTPROC glMultiDrawArraysIndirectCount;
PFNGLMULTIDRAWELEMENTSINDIRECTCOUNTPROC glMultiDrawElementsIndirectCount;
PFNGLSPECIALIZESHADERPROC glSpecializeShader;
PFNGLPOLYGONOFFSETCLAMPPROC glPolygonOffsetClamp;
PFNGLPRIMITIVEBOUNDINGBOXARBPROC glPrimitiveBoundingBoxARB;
