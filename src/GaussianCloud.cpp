//
// Created by Briac on 27/08/2025.
//

#include "GaussianCloud.h"
#include "RenderingBase/VAO.h"

#include "imgui/imgui.h"
#include "glm/ext/matrix_transform.hpp"
#include "glm/gtc/matrix_inverse.hpp"

#include "../resources/shaders/common/CommonTypes.h"

using namespace glm;

const GLenum FBO_FORMAT = GL_RGBA16F;

void GaussianCloud::prepareRender(Camera &camera) {

    const int width = camera.getFramebufferSize().x;
    const int height = camera.getFramebufferSize().y;

    const GLenum formats[] = {GL_RGBA8, GL_RGBA16F, GL_RGBA32F};

    if(
            width != fbo.getWidth() ||
            height != fbo.getHeight()) {
        fbo.init(width, height);
        fbo.createAttachment(GL_COLOR_ATTACHMENT0, FBO_FORMAT, GL_RGBA, GL_FLOAT);
        fbo.drawBuffersAllAttachments();
        if(!fbo.checkComplete()){
            exit(0);
        }

        emptyfbo.init(width, height);
        emptyfbo.makeEmpty();
        if(!emptyfbo.checkComplete()){
            exit(0);
        }

    }

    const int zero = 0;
    visible_gaussians_counter.storeData(&zero, 1, sizeof(int), 0, false, false, true);

    const mat4 rot = glm::rotate(mat4(1.0f), radians(180.0f), vec3(1, 0, 0));

    Uniforms uniforms_cpu = {};
    uniforms_cpu.viewMat = camera.getViewMatrix() * rot;
    uniforms_cpu.projMat = camera.getProjectionMatrix();

    uniforms_cpu.camera_pos = vec4(camera.getPosition(), 1.0f);

    uniforms_cpu.num_gaussians = num_gaussians;
    uniforms_cpu.near_plane = camera.getNearPlane();
    uniforms_cpu.far_plane = camera.getFarPlane();
    uniforms_cpu.scale_modifier = scale_modifier;

    uniforms_cpu.selected_gaussian = selected_gaussian;
    uniforms_cpu.min_opacity = min_opacity;
    uniforms_cpu.width = width;
    uniforms_cpu.height = height;

    auto fov2focal = [](float fov, float pixels){
        return pixels / (2.0f * tan(fov / 2.0f));
    };

    uniforms_cpu.focal_x = fov2focal(camera.getFovX(), width);
    uniforms_cpu.focal_y = fov2focal(camera.getFovY(), height);
    uniforms_cpu.antialiasing = int(antialiasing);
    uniforms_cpu.front_to_back = int(front_to_back);

    uniforms_cpu.positions = reinterpret_cast<vec4 *>(positions.getGLptr());
    uniforms_cpu.rotations = reinterpret_cast<vec4 *>(rotations.getGLptr());
    uniforms_cpu.scales = reinterpret_cast<vec4 *>(scales.getGLptr());
    uniforms_cpu.opacities = reinterpret_cast<float *>(opacities.getGLptr());
    uniforms_cpu.sh_coeffs_red = reinterpret_cast<float *>(sh_coeffs[0].getGLptr());
    uniforms_cpu.sh_coeffs_green = reinterpret_cast<float *>(sh_coeffs[1].getGLptr());
    uniforms_cpu.sh_coeffs_blue = reinterpret_cast<float *>(sh_coeffs[2].getGLptr());

    uniforms_cpu.visible_gaussians_counter = reinterpret_cast<int *>(visible_gaussians_counter.getGLptr());
    uniforms_cpu.gaussians_depth = reinterpret_cast<float *>(gaussians_depths.getGLptr());
    uniforms_cpu.gaussians_indices = reinterpret_cast<int *>(gaussians_indices.getGLptr());
    uniforms_cpu.sorted_depths = reinterpret_cast<float *>(sorted_depths.getGLptr());
    uniforms_cpu.sorted_gaussian_indices = reinterpret_cast<int *>(sorted_gaussian_indices.getGLptr());

    uniforms_cpu.bounding_boxes = reinterpret_cast<vec4 *>(bounding_boxes.getGLptr());
    uniforms_cpu.conic_opacity = reinterpret_cast<vec4 *>(conic_opacity.getGLptr());
    uniforms_cpu.eigen_vecs = reinterpret_cast<vec2 *>(eigen_vecs.getGLptr());
    uniforms_cpu.predicted_colors = reinterpret_cast<vec4 *>(predicted_colors.getGLptr());

    uniforms_cpu.ground_truth_image = 0;
    uniforms_cpu.accumulated_image_fwd = fbo.getAttachment(GL_COLOR_ATTACHMENT0)->getImageHandle();

    uniforms.storeData(&uniforms_cpu, 1, sizeof(Uniforms));
    glBindBufferBase(GL_UNIFORM_BUFFER, 0, uniforms.getID());

}

void GaussianCloud::render(Camera &camera) {

    prepareRender(camera);

    if(renderAsQuads){
        if(softwareBlending){
            emptyfbo.bind();
            glViewport(0, 0, fbo.getWidth(), fbo.getHeight());
            const GLuint ID = fbo.getAttachment(GL_COLOR_ATTACHMENT0)->getID();
            vec4 value = vec4(0.0f, 0.0f, 0.0f, 1.0f);
            glClearTexImage(ID, 0, GL_RGBA, GL_FLOAT, &value);
        }else{
            fbo.bind();
            glViewport(0, 0, fbo.getWidth(), fbo.getHeight());
            // need to clear with alpha = 1 for front to back blending
            glClearColor(0.0f,0.0f,0.0f,1.0f);
            glClear(GL_COLOR_BUFFER_BIT);

        }

        glEnable(GL_BLEND);
        if(front_to_back){
            glBlendEquation(GL_FUNC_ADD);
            glBlendFuncSeparate(GL_DST_ALPHA, GL_ONE,GL_ZERO,GL_ONE_MINUS_SRC_ALPHA);
        }else{
            glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
        }
        glDisable(GL_CULL_FACE);


        {
            auto& q = timers[OPERATIONS::TEST_VISIBILITY].push_back();
            q.begin();
            // cull non-visible gaussians
            testVisibilityShader.start();
            glDispatchCompute((num_gaussians+127)/128, 1, 1);
            glMemoryBarrier(GL_ALL_BARRIER_BITS);
            testVisibilityShader.stop();
            q.end();
            glCopyNamedBufferSubData(visible_gaussians_counter.getID(), counter.getID(), 0, 0, sizeof(int));
        }

        // read back the number of visible gaussians. That's a cpu / gpu synchronization, but it's ok.
        num_visible_gaussians = *(int*)glMapNamedBuffer(counter.getID(), GL_READ_ONLY);
        glUnmapNamedBuffer(counter.getID());

        // sort the gaussians by depth
        {
            auto& q = timers[OPERATIONS::SORT].push_back();
            q.begin();
            sort.sort(gaussians_depths, sorted_depths, gaussians_indices, sorted_gaussian_indices, num_visible_gaussians);
            q.end();
        }

        {
            auto& q = timers[OPERATIONS::COMPUTE_BOUNDING_BOXES].push_back();
            q.begin();
            computeBoundingBoxesShader.start();
            glDispatchCompute((num_visible_gaussians+127)/128, 1, 1);
            glMemoryBarrier(GL_ALL_BARRIER_BITS);
            computeBoundingBoxesShader.stop();
            q.end();
        }

        {
            auto& q = timers[OPERATIONS::PREDICT_COLORS_VISIBLE].push_back();
            q.begin();
            // Evaluate the sh basis only for the visible gaussians
            // Groups of 128 threads, with 16 threads working together on the same gaussian: 8*16 = 128
            predictColorsShader.start();
            glDispatchCompute((num_visible_gaussians+7)/8, 1, 1);
            glMemoryBarrier(GL_ALL_BARRIER_BITS);
            predictColorsShader.stop();
            q.end();
        }

        if(selected_gaussian != -1){
            glFinish();
            auto box = bounding_boxes.getAsFloats(4);
            auto conic = conic_opacity.getAsFloats(4);
            auto eigen_vec = eigen_vecs.getAsFloats(2);

            ImGui::Text("Bounding box: %.1f %.1f %.1f %.1f", box[0], box[1], box[2], box[3]);
            ImGui::Text("conic_opacity: %.4f %.4f %.4f %.2f", conic[0], conic[1], conic[2], conic[3]);
            ImGui::Text("eigen_vec: %.2f %.2f", eigen_vec[0], eigen_vec[1]);
        }

        {
            auto& q = timers[OPERATIONS::DRAW_AS_QUADS].push_back();
            q.begin();
            // draw a 2D quad for every visible gaussian
            auto& s = softwareBlending ? quad_interlock_Shader : quadShader;
            s.start();

            if(softwareBlending){
                const GLuint ID = fbo.getAttachment(GL_COLOR_ATTACHMENT0)->getID();
                glBindImageTexture(0, ID, 0, false, 0, GL_READ_WRITE, FBO_FORMAT);
            }

            VAO vao; // empty vertex array
            vao.bind();
            glMemoryBarrier(GL_ALL_BARRIER_BITS);
            glDrawArrays(GL_TRIANGLES, 0, num_visible_gaussians * 6);
            glMemoryBarrier(GL_ALL_BARRIER_BITS);
            vao.unbind();

            s.stop();
            q.end();
        }

        glEnable(GL_CULL_FACE);
        glDisable(GL_BLEND);

        {
            auto& q = timers[OPERATIONS::BLIT_FBO].push_back();
            q.begin();
            fbo.blit(0, GL_COLOR_BUFFER_BIT);
            q.end();
        }

        if(softwareBlending){
            emptyfbo.unbind();
        }else{
            fbo.unbind();
        }
    }

    if(renderAsPoints) {
        glEnable(GL_DEPTH_TEST);

        {
            // Predict colors for all the gaussians
            auto& q = timers[OPERATIONS::PREDICT_COLORS_ALL].push_back();
            q.begin();
            predictColorsForAllShader.start();
            // Groups of 128 threads, with 16 threads working together on the same gaussian: 8*16 = 128
            glDispatchCompute((num_gaussians+7)/8, 1, 1);
            glMemoryBarrier(GL_ALL_BARRIER_BITS);
            predictColorsForAllShader.stop();
            q.end();
        }

        {
            auto& q = timers[OPERATIONS::DRAW_AS_POINTS].push_back();
            q.begin();
            // Draw as a point cloud
            glEnable(GL_PROGRAM_POINT_SIZE);
            pointShader.start();
            VAO vao; // empty vertex array
            vao.bind();
            glDrawArrays(GL_POINTS, 0, num_gaussians);
            vao.unbind();
            pointShader.stop();
            q.end();
            glDisable(GL_PROGRAM_POINT_SIZE);
        }

    }

}

void GaussianCloud::initShaders() {
    pointShader.init_uniforms({});
    testVisibilityShader.init_uniforms({});
    computeBoundingBoxesShader.init_uniforms({});
    quadShader.init_uniforms({});
    quad_interlock_Shader.init_uniforms({});
    predictColorsShader.init_uniforms({});
    predictColorsForAllShader.init_uniforms({});

    counter.storeData(nullptr, 1, sizeof(int), GL_MAP_READ_BIT | GL_CLIENT_STORAGE_BIT, false, true, true);
}

// Helper to display a little (?) mark which shows a tooltip when hovered.
// In your own code you may want to display an actual icon if you are using a merged icon fonts (see misc/fonts/README.txt)
static void HelpMarker(const char* desc)
{
    ImGui::SameLine();
    ImGui::TextDisabled("(?)");
    if (ImGui::IsItemHovered())
    {
        ImGui::BeginTooltip();
        ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35.0f);
        ImGui::TextUnformatted(desc);
        ImGui::PopTextWrapPos();
        ImGui::EndTooltip();
    }
}


void GaussianCloud::GUI(Camera& camera) {

    const float frac = num_visible_gaussians / float(num_gaussians) * 100.0f;
    ImGui::Text("There are %d currently visible gaussians (%.1f%%).", num_visible_gaussians, frac);

    ImGui::Checkbox("Render as points", &renderAsPoints);
    ImGui::Checkbox("Render as quads", &renderAsQuads);
    ImGui::Checkbox("Antialiasing", &antialiasing);
    ImGui::Checkbox("Front to back blending", &front_to_back);
    ImGui::Checkbox("Software alpha-blending", &softwareBlending);
    HelpMarker("Perform alpha-blending manually with ARB_fragment_shader_interlock "
               "to define a critical section in the fragment shader.");
    ImGui::SliderFloat("scale_modifier", &scale_modifier, 0.001f, 1.0f, "%.3f", ImGuiSliderFlags_Logarithmic);
    ImGui::SliderFloat("min_opacity", &min_opacity, 0.01f, 1.0f, "%.3f", ImGuiSliderFlags_Logarithmic);

    ImGui::SliderInt("Selected gaussian", &selected_gaussian, -1, num_gaussians-1);

    if(selected_gaussian >= 0 && selected_gaussian < num_gaussians){
        ImGui::Text("Position: %.3f %.3f %.3f %.3f", positions_cpu[selected_gaussian].x, positions_cpu[selected_gaussian].y, positions_cpu[selected_gaussian].z, positions_cpu[selected_gaussian].w);
        ImGui::Text("Scale: %.3f %.3f %.3f %.3f", scales_cpu[selected_gaussian].x, scales_cpu[selected_gaussian].y, scales_cpu[selected_gaussian].z, scales_cpu[selected_gaussian].w);
        ImGui::Text("Rotation: %.3f %.3f %.3f %.3f", rotations_cpu[selected_gaussian].x, rotations_cpu[selected_gaussian].y, rotations_cpu[selected_gaussian].z, rotations_cpu[selected_gaussian].w);
        ImGui::Text("Opacity: %.3f", opacities_cpu[selected_gaussian]);
    }

    if(ImGui::TreeNode("Timers")){
        if(renderAsPoints){
            ImGui::Text("Point rendering:");
            ImGui::Text("Predict colors: %.3fms", timers[OPERATIONS::PREDICT_COLORS_ALL].getLastResult() * 1.0E-6);
            ImGui::Text("Draw points: %.3fms", timers[OPERATIONS::DRAW_AS_POINTS].getLastResult() * 1.0E-6);

            float total = 0.0f;
            for(int i=0; i<= OPERATIONS::DRAW_AS_POINTS; i++){
                total += timers[i].getLastResult() * 1.0E-6;
            }
            ImGui::Text("Total: %.3fms", total);

            ImGui::Separator();
        }
        if(renderAsQuads){
            ImGui::Text("Quad rendering:");
            ImGui::Text("Test visibility: %.3fms", timers[OPERATIONS::TEST_VISIBILITY].getLastResult() * 1.0E-6);
            ImGui::Text("Sort: %.3fms", timers[OPERATIONS::SORT].getLastResult() * 1.0E-6);
            ImGui::Text("Compute bounding boxes: %.3fms", timers[OPERATIONS::COMPUTE_BOUNDING_BOXES].getLastResult() * 1.0E-6);
            ImGui::Text("Predict colors: %.3fms", timers[OPERATIONS::PREDICT_COLORS_VISIBLE].getLastResult() * 1.0E-6);
            ImGui::Text("Draw quads: %.3fms", timers[OPERATIONS::DRAW_AS_QUADS].getLastResult() * 1.0E-6);
            ImGui::Text("Blit framebuffer: %.3fms", timers[OPERATIONS::BLIT_FBO].getLastResult() * 1.0E-6);

            float total = 0.0f;
            for(int i=OPERATIONS::TEST_VISIBILITY; i<= OPERATIONS::BLIT_FBO; i++){
                total += timers[i].getLastResult() * 1.0E-6;
            }
            ImGui::Text("Total: %.3fms", total);

            ImGui::Separator();
        }
        ImGui::TreePop();
    }
}

