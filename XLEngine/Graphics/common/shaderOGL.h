#pragma once

#include "../common/graphicsDeviceOGL.h"
#include <map>

enum ShaderStage
{
	SHADER_VERTEX = 0,
	SHADER_PIXEL,
	SHADER_STAGE_COUNT
};

struct ShaderParam
{
	u32 nameHash;
	s32 glID;

	u32 type;
	u32 size;
	void* data;

	TextureHandle texHandle;
};

typedef std::map<u32, u32> ParameterMap;

class ShaderOGL
{
	public:
        ShaderOGL();
        virtual ~ShaderOGL();

		bool load(const char* vsShader, const char* psShader);

		void bind();
		void uploadData(GraphicsDevice* device);

		s32  getParameter(const char* name) const;
		s32  getParameter(u32 nameHash) const;
		void updateParameter(s32 id, void* data, u32 size);
		void updateParameter(s32 id, TextureHandle texture, u32 slot, bool force=false);

		u32  getRequiredVertexAttrib() const { return m_requiredVtxAttrib; }

		static void init();
		static void destroy();
		static void clear();
				
    protected:
		u32 m_glShaderID;
		u32 m_shaderParamCount;
		u32 m_requiredVtxAttrib;

		ShaderParam* m_param;
		u64			 m_stateDirty;		//bitfield: which parameters are dirty (param 3 = 1<<3)

		ParameterMap m_paramMap;
    private:
		bool compileShader(const char* vsShader, const char* psShader);
		bool compileShaderStage(u32& handle, ShaderStage stage, const char *path);
};
