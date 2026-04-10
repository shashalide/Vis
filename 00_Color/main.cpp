#include <GLApp.h>
#include <FontRenderer.h>

class ColorPicker : public GLApp {
public:
  Image image{640,480};
  FontRenderer fr{"helvetica_neue.bmp", "helvetica_neue.pos"};
  std::shared_ptr<FontEngine> fe{nullptr};
  std::string text;

  ColorPicker(const std::vector<std::string>& args) :
  GLApp{800,800,1,"Color Picker", true, false, false, args} {}

  float convertFunc(int n, float H, float S, float V) {
    const float k =  fmod(float(n + H/60), 6.0f);
    return V - V * S * std::max<float>(0, std::min<float>(std::min<float>(k, 4.0f-k), 1.0f));
  }

  Vec3 convertPosFromHSVToRGB(float x, float y) {
    const float H = float(360*x), S = y, V = 1.0f;
    float r = convertFunc(5, H, S, V);
    float g = convertFunc(3, H, S, V);
    float b = convertFunc(1, H, S, V);
    return Vec3{r,g,b};
  }
  
  virtual void init() override {
    fe = fr.generateFontEngine();
    for (uint32_t y = 0;y<image.height;++y) {
      for (uint32_t x = 0;x<image.width;++x) {
        const Vec3 rgb = convertPosFromHSVToRGB(float(x)/image.width, float(y)/image.height);
        image.setNormalizedValue(x,y,0,rgb.r); image.setNormalizedValue(x,y,1,rgb.g);
        image.setNormalizedValue(x,y,2,rgb.b); image.setValue(x,y,3,255);
      }
    }
    GL(glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA));
    GL(glBlendEquation(GL_FUNC_ADD));
    GL(glEnable(GL_BLEND));
  }
  
  virtual void mouseMove(double xPosition, double yPosition) override {
    Dimensions s = glEnv.getWindowSize();
    if (xPosition < 0 || xPosition > s.width || yPosition < 0 || yPosition > s.height) return;
    const Vec3 hsv{float(360*xPosition/s.width),float(1.0-yPosition/s.height),1.0f};
    const Vec3 rgb = convertPosFromHSVToRGB(float(xPosition/s.width), float(1.0-yPosition/s.height));
    std::stringstream ss; ss << "HSV: " << hsv << "  RGB: " << rgb; text = ss.str();
  }
    
  virtual void draw() override {
    drawImage(image);

    const Dimensions dim{ glEnv.getFramebufferSize() };
    fe->render(text, dim.aspect(), 0.03f, {0,-0.9f}, Alignment::Center, {0,0,0,1});
  }
};

#ifdef _WIN32
#include <Windows.h>

INT WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PSTR lpCmdLine, INT nCmdShow) {
  std::vector<std::string> args = getArgsWindows();
#else
int main(int argc, char** argv) {
  std::vector<std::string> args{argv + 1, argv + argc};
#endif
  try {
    ColorPicker app{args};
    app.run();
  }
  catch (const GLException& e) {
    std::stringstream ss;
    ss << "Insufficient OpenGL Support " << e.what();
#ifndef _WIN32
    std::cerr << ss.str().c_str() << std::endl;
#else
    MessageBoxA(
                NULL,
                ss.str().c_str(),
                "OpenGL Error",
                MB_ICONERROR | MB_OK
                );
#endif
    return EXIT_FAILURE;
  }
  return EXIT_SUCCESS;
}
