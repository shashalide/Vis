#include <iomanip>
#include <fstream>

#include <GLApp.h>
#include <bmp.h>
#include <Grid2D.h>

class ImageProcessing : public GLApp {
public:
  Image image;
    
  ImageProcessing(const std::vector<std::string>& args) :
  GLApp(512, 512, 4, "Image Processing", true, false, false, args)
  {
  }
    
  uint8_t getScaleValue(uint32_t x, uint32_t y, Vec3 scale) const {
    return uint8_t(image.getValue(x,y,0)*scale.x + image.getValue(x,y,1)*scale.y + image.getValue(x,y,2)*scale.z);
  }
  
  void toGrayscale(bool uniform=false) {
    Image grayScaleImage = image;
    const Vec3 scale = uniform ? Vec3{0.333f,0.333f,0.333f} : Vec3{0.299f,0.587f,0.114f};
    for (uint32_t y = 0;y<image.height;++y) {
      for (uint32_t x = 0;x<image.width;++x) {
        grayScaleImage.setValue(x,y,getScaleValue(x,y,scale));
      }
    }
    image = grayScaleImage;
  }

  void loadImage() {
    try {
      image = BMP::load("lenna.bmp");
    } catch (...) {
      image = Image(512,512);
      for (uint32_t y = 0;y<image.height;++y) {
        for (uint32_t x = 0;x<image.width;++x) {
          image.setNormalizedValue(x,y,0,float(x)/image.width);
          image.setNormalizedValue(x,y,1,float(y)/image.height);
          image.setNormalizedValue(x,y,2,0.5f);
          image.setValue(x,y,3,255);
        }
      }
    }
  }
  
  virtual void init() override {
    loadImage();
  }
      
  virtual void draw() override {
    drawImage(image);
  }
  
  
  std::string toString(bool bSmallTable=true) {
    const std::string lut1{"$@B%8&WM#*oahkbdpqwmZO0QLCJUYXzcvunxrjft/\\|()1{}[]?-_+~<>i!lI;:,\"^`'. "};
    const std::string lut2{"@%#*+=-:. "};
    const std::string& lut = bSmallTable ? lut2 : lut1;
    
    std::stringstream ss;

    for (uint32_t y = 0;y<image.height;y+=2) {
      for (uint32_t x = 0;x<image.width;x+=2) {
        const uint8_t v = image.getLumiValue(x,image.height-1-y);
        ss << lut[(v*(lut.length()-1))/255] << lut[(v*(lut.length()-1))/255];
      }
      ss << "\n";
    }

    return ss.str();
  }
  
  void filter(const Grid2D& filter) {
    Image filteredImage = image;
    const uint32_t hw = uint32_t(filter.getWidth()/2);
    const uint32_t hh = uint32_t(filter.getHeight()/2);
      
    for (uint32_t y = hh;y<image.height-hh;y+=1) {
      for (uint32_t x = hw;x<image.width-hw;x+=1) {
        for (uint8_t c = 0;c<image.componentCount;c+=1) {
            float conv = 0.0f;
            for (uint32_t u = 0;u<filter.getHeight();u+=1) {
              for (uint32_t v = 0;v<filter.getWidth();v+=1) {
                conv += float(image.getValue((x+u-hw),(y+v-hh),c)) * filter.getValue(u, v);
              }
            }
            filteredImage.setValue(x,y,c,uint8_t(fabs(conv)));
        }
      }
    }
      image = filteredImage;
  }
  
  virtual void keyboard(int key, int scancode, int action, int mods) override {
    if (action == GLENV_PRESS) {
      switch (key) {
        case GLENV_KEY_ESCAPE :
          closeWindow();
          break;
        case GLENV_KEY_M :
          {
            Grid2D mean{3,3};
            mean.fill(1.0f/(mean.getHeight()*mean.getWidth()));
            filter(mean);
          }
          break;
        case GLENV_KEY_A :
          filter({3,3, {-1, 0, 1,
                        -2, 0, 2,
                        -1, 0, 1}});
          break;
        case GLENV_KEY_B:
          filter({3,3, {-1,-2,-1,
                         0, 0, 0,
                         1, 2, 1}});
        break;
        case GLENV_KEY_G :
          toGrayscale(false);
          break;
        case GLENV_KEY_H :
          toGrayscale(true);
          break;
        case GLENV_KEY_R :
          loadImage();
          break;
        case GLENV_KEY_T : {
          std::ofstream file{ "ascii.txt" };
          file << toString() << std::endl;
          break;
        }
      }
    }
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
      ImageProcessing app{args};
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
