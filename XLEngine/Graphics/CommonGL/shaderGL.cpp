#include <stdlib.h>
#include "shaderGL.h"
#include "vertexBufferGL.h"
#include "../../log.h"
#include "../../filestream.h"
#include "../../Math/crc32.h"

#include <GL/glew.h>
#include <GL/gl.h>
#include <GL/glu.h>

#include <assert.h>

#define MAX_PARAMETER_COUNT 64

static char*  s_workBuffer;
static size_t s_workBufferSize;

const char* readFileIntoString(const char* path);
void freeWorkBuffer();

//////////////////////////
// Static Functions
//////////////////////////
void ShaderGL::init()
{
	s_workBufferSize = 0;
	s_workBuffer     = NULL;
}

void ShaderGL::destroy()
{
	s_workBufferSize = 0;
	free(s_workBuffer);
}

void ShaderGL::clear()
{
	glUseProgram(0);
}

//////////////////////////
// Implementation
//////////////////////////
ShaderGL::ShaderGL()
{
	m_glShaderID        = 0;
	m_shaderParamCount  = 0;
	m_requiredVtxAttrib = 0;

	m_param = NULL;
	m_stateDirty = 0xffffffffffffffffULL;	//everything is dirty initially.
}

ShaderGL::~ShaderGL()
{
	glDeleteShader( m_glShaderID );
	m_glShaderID = 0;

	for (u32 p=0; p<m_shaderParamCount; p++)
	{
		free( m_param[p].data );
	}
	delete [] m_param;
}

bool ShaderGL::load(const char* vsShader, const char* psShader)
{
	bool success = compileShader(vsShader, psShader);

	if (success)
	{
		//get parameters.
		s32 paramCount = 0, bufferSize = 0;
		GLsizei nameLen;
		GLenum  paramType;
		char paramName[256];

		static ShaderParam param[MAX_PARAMETER_COUNT];
		s32 usedCount = 0;

		glGetProgramiv( m_glShaderID, GL_ACTIVE_UNIFORMS, &paramCount );
		for (s32 p=0; p<paramCount; p++)
		{
			glGetActiveUniform( m_glShaderID, p, 256, &nameLen, &bufferSize, &paramType, paramName );

			//skip built in uniforms
			if (paramName[0] == 'g' && paramName[1] == 'l')
			{
				continue;
			}

			param[usedCount].nameHash = CRC32::get( (u8*)paramName, nameLen );
			param[usedCount].glID = p;
			param[usedCount].type = paramType;
			param[usedCount].size = max( bufferSize, 4 );

			usedCount++;
		}

		m_shaderParamCount = usedCount;
		m_param = new ShaderParam[usedCount];
		m_paramMap.clear();
		for (u32 p=0; p<m_shaderParamCount; p++)
		{
			m_param[p].nameHash = param[p].nameHash;
			m_param[p].glID = param[p].glID;
			m_param[p].type = param[p].type;
			m_param[p].size = param[p].size;

			m_param[p].data = malloc( bufferSize );
			memset(m_param[p].data, 0, m_param[p].size);

			m_paramMap[ m_param[p].nameHash ] = p;
		}
	}

	return success;
}

void ShaderGL::bind()
{
	glUseProgram( m_glShaderID );
}

void ShaderGL::uploadData(GraphicsDevice* device)
{
	if (m_stateDirty == 0)
	{
		return;
	}

	for (u32 p=0; p<m_shaderParamCount; p++)
	{
		if ( m_stateDirty & (1ULL << u64(p)) )
		{
			ShaderParam& param = m_param[p];
			switch (param.type)
			{
				case GL_FLOAT:
					glProgramUniform1fv( m_glShaderID, param.glID, 1, (f32*)param.data );
					break;
				case GL_FLOAT_VEC2:
					glProgramUniform2fv( m_glShaderID, param.glID, 1, (f32*)param.data );
					break;
				case GL_FLOAT_VEC3:
					glProgramUniform3fv( m_glShaderID, param.glID, 1, (f32*)param.data );
					break;
				case GL_FLOAT_VEC4: 
					glProgramUniform4fv( m_glShaderID, param.glID, 1, (f32*)param.data );
					break;
				case GL_INT:
					glProgramUniform1iv( m_glShaderID, param.glID, 1, (s32*)param.data );
					break;
				case GL_INT_VEC2:
					glProgramUniform2iv( m_glShaderID, param.glID, 1, (s32*)param.data );
					break;
				case GL_INT_VEC3: 
					glProgramUniform3iv( m_glShaderID, param.glID, 1, (s32*)param.data );
					break;
				case GL_INT_VEC4: 
					glProgramUniform4iv( m_glShaderID, param.glID, 1, (s32*)param.data );
					break;
				case GL_BOOL:
					glProgramUniform1iv( m_glShaderID, param.glID, 1, (s32*)param.data );
					break;
				case GL_BOOL_VEC2:
					glProgramUniform2iv( m_glShaderID, param.glID, 1, (s32*)param.data );
					break;
				case GL_BOOL_VEC3:
					glProgramUniform3iv( m_glShaderID, param.glID, 1, (s32*)param.data );
					break;
				case GL_BOOL_VEC4:
					glProgramUniform4iv( m_glShaderID, param.glID, 1, (s32*)param.data );
					break;
				case GL_FLOAT_MAT2:
					glProgramUniformMatrix2fv( m_glShaderID, param.glID, 1, false, (f32*)param.data );
					break;
				case GL_FLOAT_MAT3:
					glProgramUniformMatrix3fv( m_glShaderID, param.glID, 1, false, (f32*)param.data );
					break;
				case GL_FLOAT_MAT4:
					glProgramUniformMatrix4fv( m_glShaderID, param.glID, 1, false, (f32*)param.data );
					break;
				case GL_SAMPLER_2D:
					glProgramUniform1iv( m_glShaderID, param.glID, 1, (s32*)param.data );
					//A cast is required to the base OpenGL graphics device since that is as highest class in the hierarchy that can "friend" ShaderGL
					((GraphicsDeviceGL*)device)->setTexture( param.texHandle, *((s32*)param.data) );
					break;
				case GL_SAMPLER_CUBE:
					glProgramUniform1iv( m_glShaderID, param.glID, 1, (s32*)param.data );
					//A cast is required to the base OpenGL graphics device since that is as highest class in the hierarchy that can "friend" ShaderGL
					((GraphicsDeviceGL*)device)->setTexture( param.texHandle, *((s32*)param.data) );
					break;
			};
		}
	}
	m_stateDirty = 0;
}

s32  ShaderGL::getParameter(const char* name) const
{
	const u32 nameHash = CRC32::get( (u8*)name, strlen(name) );
	return getParameter(nameHash);
}

s32  ShaderGL::getParameter(u32 nameHash) const
{
	const ParameterMap::const_iterator iparam = m_paramMap.find(nameHash);
	if (iparam != m_paramMap.end())
	{
		return iparam->second;
	}
	return -1;
}

void ShaderGL::updateParameter(s32 id, void* data, u32 size)
{
	if (id < 0) { return; }

	ShaderParam& param = m_param[id];
	u32 copySize = min( size, param.size );

	memcpy(param.data, data, copySize);
	param.texHandle = 0;

	m_stateDirty |= ( 1ULL << u64(id) );
}

void ShaderGL::updateParameter(s32 id, TextureHandle texture, u32 slot, bool force/*=false*/)
{
	if (id < 0) { return; }
	ShaderParam& param = m_param[id];

	//nothing to change.
	if (param.texHandle == texture && !force)
	{
		return;
	}

	updateParameter(id, &slot, sizeof(slot));
	param.texHandle = texture;
}

bool ShaderGL::compileShader(const char* vsShader, const char* psShader)
{
	bool success = true;
	u32 vsShaderHandle=0, psShaderHandle=0;

	success &= compileShaderStage(vsShaderHandle, SHADER_VERTEX, vsShader);
	success &= compileShaderStage(psShaderHandle, SHADER_PIXEL,  psShader);
	if (!success)
	{
		return false;
	}

	//link the program
	m_glShaderID = glCreateProgram();
	glAttachShader( m_glShaderID, vsShaderHandle );
	glAttachShader( m_glShaderID, psShaderHandle );
	glLinkProgram(  m_glShaderID );

	//get errors
	GLint status=GL_FALSE;
	glGetProgramiv(m_glShaderID, GL_LINK_STATUS, &status);

	GLint infoLogLength=0;
	glGetProgramiv(m_glShaderID, GL_INFO_LOG_LENGTH, &infoLogLength);

	if ( infoLogLength > 1 )
	{
		GLchar* strInfoLog = (GLchar*)malloc(infoLogLength + 1);
		glGetProgramInfoLog(m_glShaderID, infoLogLength, NULL, strInfoLog);

		LOG( LOG_ERROR, "Linking warning(s) and/or error(s) in shader (%s, %s):\n %s", vsShader, psShader, strInfoLog );
		free(strInfoLog);
	}
	success &= (status == GL_TRUE);
	if (!success)
	{
		return false;
	}

	//bind vertex attributes.
	m_requiredVtxAttrib = 0;
	for (u32 attr=0; attr<VATTR_COUNT; attr++)
	{
		//does this shader even have this attribute?
		const s32 origLoc = glGetAttribLocation(m_glShaderID, VertexBufferGL::c_vertexAttrib[attr]);
		if (origLoc >= 0)
		{
			glBindAttribLocation(m_glShaderID, attr, VertexBufferGL::c_vertexAttrib[attr]); 
			m_requiredVtxAttrib |= (1<<attr);
		}
	}
	glLinkProgram(  m_glShaderID );

	//get errors
	glGetProgramiv(m_glShaderID, GL_LINK_STATUS, &status);
	assert(status);

	if (!status)
	{
		return false;
	}
		
	return success;
}

bool ShaderGL::compileShaderStage(u32& handle, ShaderStage stage, const char *path)
{
	GLint status;
	const char* shaderCode = readFileIntoString(path);
	const GLenum glStage[] = { GL_VERTEX_SHADER, GL_FRAGMENT_SHADER };

	handle = glCreateShader(glStage[stage]);
	glShaderSource(handle, 1, (const GLchar **)&shaderCode, NULL);
	glCompileShader(handle);
	glGetShaderiv(handle, GL_COMPILE_STATUS, &status);

	GLint infoLogLength;
	glGetShaderiv(handle, GL_INFO_LOG_LENGTH, &infoLogLength);

	if ( infoLogLength > 1 )
	{
		GLchar* strInfoLog = (GLchar*)malloc(infoLogLength + 1);
		glGetShaderInfoLog(handle, infoLogLength, NULL, strInfoLog);

		LOG( LOG_ERROR, "Compilation warning(s) and error(s) in shader %s:\n%s", path, strInfoLog );
		free(strInfoLog);
	}

	return (status == GL_TRUE);
}

const char* readFileIntoString(const char* path)
{
	FileStream file;
	if (!file.open(path, FileStream::MODE_READ))
	{
		return NULL;
	}

	size_t size = file.getSize();
	if ( size+1 > s_workBufferSize )
	{
		s_workBufferSize = 2*size+1;

		free(s_workBuffer);
		s_workBuffer = (char*)malloc(s_workBufferSize);
	}

	if (!s_workBuffer)
	{
		file.close();
		return NULL;
	}

	file.readBuffer(s_workBuffer, size);
	file.close();

	s_workBuffer[ size ] = 0;

	return s_workBuffer;
}
