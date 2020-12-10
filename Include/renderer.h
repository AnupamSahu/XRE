#ifndef RENDERER_H
#define RENDERER_H

#include <glad/glad.h>
#include <glm/glm.hpp>


#include <mesh.h>
#include <shader.h>
#include <lights.h>
#include <xre_configuration.h>

#include <string>
#include <vector>
#include <random>

#define XRE_MAX_POINT_SHADOW_MAPS 3

namespace xre
{
#pragma region Data Structures

	enum RENDER_PIPELINE
	{
		DEFERRED,
		FORWARD
	};
	enum LIGHTING_MODE
	{
		PBR,
		BLINNPHONG
	};

	struct model_information
	{
		bool* setup_success = NULL;
		std::string model_name = "";
		bool dynamic;
		unsigned int object_VAO = 0;
		unsigned int indices_size = 0;
		const xre::Shader* object_shader = NULL;
		const glm::mat4* object_model_matrix = NULL;
		std::vector<Texture>* object_textures = NULL;
		std::vector<std::string>* texture_types = NULL;
		BoundingVolume mesh_aabb;
		bool frustum_cull = false;
	};

	struct point_light_space_matrix_cube
	{
		glm::mat4 point_light_space_matrix_0;
		glm::mat4 point_light_space_matrix_1;
		glm::mat4 point_light_space_matrix_2;
		glm::mat4 point_light_space_matrix_3;
		glm::mat4 point_light_space_matrix_4;
		glm::mat4 point_light_space_matrix_5;
	};


#pragma endregion

	class RenderSystem
	{
	private:

#pragma region Functions

		void createForwardFramebuffers();
		void createQuad();
		void createShadowMapFramebuffers();
		void createDeferredBuffers();
		void clearDeferredBuffers();
		void createDirectionalLightMatrix(glm::vec3 light_position, glm::vec3 light_front);
		void createPointLightMatrices(glm::vec3 light_position, unsigned int light_index);
		void createBlurringFramebuffers();
		void clearForwardFramebuffer();
		void clearDefaultFramebuffer();
		void clearDirectionalShadowMapFramebuffer();
		void clearDirectionalShadowBlurringFramebuffers();
		void clearPointShadowFramebuffer();
		void clearPrimaryBlurringFramebuffers();
		void directionalShadowPass();
		void pointShadowPass();
		void ForwardColorPass();
		void deferredFillPass();
		void deferredColorPass();
		void blurPass(unsigned int main_color_texture, unsigned int ssao_texture, unsigned int amount);
		void directionalSoftShadowPass();
		void SSAOPass();
		void createSSAOData();
		void createSSAOKernel(unsigned int num_samples);
		void createSSAONoise();
		float lerp(float a, float b, float t);

		RenderSystem(unsigned int screen_width, unsigned int screen_height, const glm::vec4& background_color, float lights_near_plane_p, float lights_far_plane_p, int shadow_map_width_p, int shadow_map_height_p, RENDER_PIPELINE render_pipeline, LIGHTING_MODE light_mode);

#pragma endregion

#pragma region Rendering Pipeline Data

		static RenderSystem* instance;

		RENDER_PIPELINE rendering_pipeline;
		LIGHTING_MODE lighting_model;

		// just making shit more complicated than it has to be.
		unsigned int framebuffer_width, framebuffer_height;

		// Forward Shading
		unsigned int
			ForwardFramebuffer,
			ForwardFramebuffer_primary_texture,
			ForwardFramebuffer_secondary_texture,
			ForwardFramebuffer_Color_Attachments[2],
			ForwardFramebuffer_depth_texture,
			ForwardFramebufferRenderbuffer;

		// Deferred Shading
		unsigned int DeferredDataFrameBuffer,
			DeferredFinalBuffer;

		unsigned int DeferredRenderBuffer;

		unsigned int DeferredFinal_primary_texture,
			DeferredFinal_secondary_texture;

		unsigned int DeferredGbuffer_position,
			DeferredGbuffer_color,
			DeferredGbuffer_model_normal,
			DeferredGbuffer_tangent,
			DeferredGbuffer_texture_normal,
			DeferredGbuffer_texture_normal_view,
			DeferredGbuffer_texture_mor;

		unsigned int DeferredFrameBuffer_primary_color_attachments[6];
		unsigned int DeferredFinal_attachments[2];

		bool horizontal = true, first_iteration = true;
		bool first_draw = true;

		std::vector<model_information> draw_queue;
		unsigned int quadVAO, quadVBO;
		unsigned int screen_texture;
		bool deferred;

		bool frustum_test_started;

		glm::vec4 bg_color;

#pragma endregion

#pragma region Rendering Effects Data

		// SSAO
		unsigned int SSAOFrameBuffer;
		unsigned int SSAOFramebuffer_color;

		std::vector<glm::vec3> ssao_kernel;
		std::vector<glm::vec3> ssao_noise;

#pragma endregion

#pragma region Additional Effects Data

		// PrimaryBlurring Buffers
		unsigned int PrimaryBlurringFramebuffers[2],
			PrimaryBlurringFramebuffer_bloom_textures[2],
			PrimaryBlurringFramebuffer_ssao_textures[2],
			PrimaryBlurringFramebuffers_attachments[2];

		// DirectionalShadowBlur Buffers
		unsigned int DirectionalShadowBlurringFramebuffers[2],
			DirectionalShadowBlurring_soft_shadow_textures[2];

		unsigned int random_rotation_texture;

#pragma endregion

#pragma region Shaders

		Shader* deferredFillShader;
		Shader* deferredColorShader;
		Shader* SSAOShader;
		Shader* quadShader;
		Shader* depthShader_point;
		Shader* depthShader_directional;
		Shader* bloomSSAO_blur_Shader;
		Shader* directional_shadow_blur_Shader;

#pragma endregion

#pragma region Shadow Mapping

		unsigned int directional_shadow_framebuffer, directional_shadow_depth_storage, directional_shadow_renderbuffer;
		unsigned int point_shadow_framebuffer[XRE_MAX_POINT_SHADOW_MAPS],
			point_shadow_depth_storage[XRE_MAX_POINT_SHADOW_MAPS],
			point_shadow_depth_attachment[XRE_MAX_POINT_SHADOW_MAPS];

		unsigned int shadow_map_width, shadow_map_height;
		float light_near_plane, light_far_plane;

#pragma endregion

#pragma region Lights

		DirectionalLight* directional_light;
		std::vector<PointLight*> point_lights;

		glm::mat4 directional_light_projection;
		glm::mat4 directional_light_space_matrix;
		glm::mat4 point_light_projection;
		point_light_space_matrix_cube point_light_space_matrix_cube_array[5];

		std::vector<Light*> lights;

#pragma endregion

#pragma region Camera

		const glm::mat4* camera_view_matrix;
		const glm::mat4* camera_projection_matrix;
		const glm::vec3* camera_position;

#pragma endregion
	
	public:

		static RenderSystem* renderer();
		static RenderSystem* renderer(unsigned int screen_width, unsigned int screen_height, const glm::vec4& background_color, float lights_near_plane_p, float lights_far_plane_p, int shadow_map_width_p, int shadow_map_height_p, RENDER_PIPELINE render_pipeline, LIGHTING_MODE light_mode);

		RenderSystem(RenderSystem& other) = delete;

		void draw(unsigned int vertex_array_object, unsigned int indices_size, const xre::Shader& object_shader, const glm::mat4& model_matrix, std::vector<Texture>* object_textures, std::vector<std::string>* texture_types, std::string model_name, bool isdynamic, bool* setup_success, BoundingVolume aabb);
		void drawToScreen();
		void SwitchPfx(bool option);

		void setCameraMatrices(const glm::mat4* view, const glm::mat4* projection, const glm::vec3* position);
		void addToRenderSystem(Light* light);

		glm::vec3 world_view_pos;
	};
}
#endif