#pragma once

#include "scene.h"
#include "block.h"
#include "camera.h"

#include <nanogui/screen.h>

class Sampler;

class Viewer : public nanogui::Screen
{
    // A scene contains a list of objects, a list of light sources and a camera.
    Scene* m_scene = nullptr;
    
    // GLSL shader programs
    nanogui::ref<nanogui::Shader> m_tonemapProgram;

    ImageBlock* m_resultImage = nullptr;
    std::string m_curentFilename;
    bool m_renderingDone;
    int m_threadCount;

    nanogui::ref<nanogui::Texture> m_texture;
    nanogui::ref<nanogui::RenderPass> m_renderPass;

    // GUI
    nanogui::Slider *m_slider = nullptr;
    nanogui::Button* m_button1 = nullptr;
    nanogui::Button* m_button2 = nullptr;
    nanogui::CheckBox *m_checkbox = nullptr;
    nanogui::Widget *m_panel = nullptr;
    float m_scale = 1.f;
    int m_srgb = 1;

  protected:
    void initializeGL();

    /** This method is automatically called everytime the opengl windows is resized. */
    virtual bool resize_event(const nanogui::Vector2i& size) override;

    /** This method is automatically called everytime the opengl windows has to be refreshed. */
    virtual void draw_contents() override;

    /** This method is automatically called everytime a key is pressed */
    virtual bool keyboard_event(int key, int scancode, int action, int modifiers)override;

    /** This method is called when files are dropped on the window */
    virtual bool drop_event(const std::vector<std::string> &filenames) override;

    static void renderBlock(Scene* scene, Sampler *sampler, ImageBlock& block);

  public: 
    EIGEN_MAKE_ALIGNED_OPERATOR_NEW

    static void render(Scene* scene, ImageBlock* result, bool* done, int threadCount);

    /** This method load a 3D scene from a file */
    void loadScene(const std::string &filename);

    /** This method load an OpenEXR image from a file */
    void loadImage(const filesystem::path &filename);

    // default constructor
    Viewer(int threadCount);
};
