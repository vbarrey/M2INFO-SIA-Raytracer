#include "parser.h"
#include "scene.h"
#include "camera.h"
#include "block.h"
#include "timer.h"
#include "bitmap.h"
#include "integrator.h"
#include "sampler.h"
#include "viewer.h"

#include <tbb/parallel_for.h>
#include <tbb/blocked_range.h>
#include <tbb/task_scheduler_init.h>
#include <filesystem/resolver.h>
#include <thread>

static int threadCount = -1;
static bool gui = true;

static void render(Scene *scene, const std::string &filename) {
    const Camera *camera = scene->camera();
    Vector2i outputSize = camera->getOutputSize();
    /* Allocate memory for the entire output image and clear it */
    ImageBlock result(outputSize, camera->getReconstructionFilter());

    bool done = false;
    Viewer::render(scene, &result, &done, threadCount);

    /* Now turn the rendered image block into
       a properly normalized bitmap */
    std::unique_ptr<Bitmap> bitmap(result.toBitmap());

    /* Determine the filename of the output bitmap */
    std::string outputName = filename;
    size_t lastdot = outputName.find_last_of(".");
    if (lastdot != std::string::npos)
        outputName.erase(lastdot, std::string::npos);

    /* Save using the OpenEXR format */
    bitmap->saveEXR(outputName + ".exr");

    /* Save tonemapped (sRGB) output using the PNG format */
    bitmap->savePNG(outputName + ".png", true);
}

int main(int argc, char **argv) {
    if (argc <= 1) {
        cerr << "Syntax: " << argv[0] << " <scene.scn | image.exr> [--no-gui] [--threads N]" <<  endl;
        return -1;
    }

    std::string sceneName = "";
    std::string exrName = "";

    for (int i = 1; i < argc; ++i) {
        std::string token(argv[i]);
        if (token == "-t" || token == "--threads") {
            if (i+1 >= argc) {
                cerr << "\"--threads\" argument expects a positive integer following it." << endl;
                return -1;
            }
            threadCount = atoi(argv[i+1]);
            i++;
            if (threadCount <= 0) {
                cerr << "\"--threads\" argument expects a positive integer following it." << endl;
                return -1;
            }

            continue;
        }
        else if (token == "--no-gui") {
            gui = false;
            continue;
        }

        filesystem::path path(argv[i]);

        try {
            if (path.extension() == "scn") {
                sceneName = argv[i];

                /* Add the parent directory of the scene file to the
                   file resolver. That way, the XML file can reference
                   resources (OBJ files, textures) using relative paths */
                getFileResolver()->prepend(path.parent_path());
            } else if (path.extension() == "exr") {
                /* Alternatively, provide a basic OpenEXR image viewer */
                exrName = argv[i];
            } else {
                cerr << "Fatal error: unknown file \"" << argv[i]
                     << "\", expected an extension of type .scn or .exr" << endl;
            }
        } catch (const std::exception &e) {
            cerr << "Fatal error: " << e.what() << endl;
            return -1;
        }
    }

    if (exrName !="" && sceneName !="") {
        cerr << "Both .scn and .exr files were provided. Please only provide one of them." << endl;
        return -1;
    }
    else if (exrName == "" && sceneName == "") {
        cerr << "Please provide the path to a .scn (or .exr) file." << endl;
        return -1;
    }
    else if (exrName != "") {
        if (!gui) {
            cerr << "Flag --no-gui was set. Please remove it to display the EXR file." << endl;
            return -1;
        }
        try {
            nanogui::init();
            Viewer *viewer = new Viewer(threadCount);
            viewer->loadImage(exrName);
            nanogui::mainloop(50.f);
            delete viewer;
            nanogui::shutdown();
        } catch (const std::exception &e) {
            cerr << e.what() << endl;
            return -1;
        }
    }
    else { // sceneName != ""
        try {
            if (gui) {
                nanogui::init();
                Viewer *viewer = new Viewer(threadCount);
                viewer->loadScene(sceneName);
            
                nanogui::mainloop(50.f);

                delete viewer;
                nanogui::shutdown();
            } else {
                std::unique_ptr<Object> root(loadFromXML(sceneName));
                /* When the XML root object is a scene, start rendering it .. */
                if (root->getClassType() == Object::EScene)
                    render(static_cast<Scene *>(root.get()), sceneName);
            }
        } catch (const std::exception &e) {
            cerr << e.what() << endl;
            return -1;
        }
    }

    return 0;
}
