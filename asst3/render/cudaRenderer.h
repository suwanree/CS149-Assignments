#ifndef __CUDA_RENDERER_H__
#define __CUDA_RENDERER_H__

#include "circleRenderer.h"
#include <vector_types.h>

class CudaRenderer : public CircleRenderer {
 private:
  Image* image;
  SceneName sceneName;

  int numCircles;
  float* position;
  float* velocity;
  float* color;
  float* radius;

  float* cudaDevicePosition;
  float* cudaDeviceVelocity;
  float* cudaDeviceColor;
  float* cudaDeviceRadius;
  float* cudaDeviceImageData;
  
  int* cudaDeviceTileCountForCircle;
  int* cudaDeviceTileCountForCircleExScan;
  uint64_t* cudaDeviceTileIndicesForCircle;
  int* cudaDeviceTileStart;
  int* cudaDeviceTileEnd;

  int* tileCounts;
  int mallocedPair;

 public:
  CudaRenderer();
  virtual ~CudaRenderer();

  const Image* getImage();

  void setup();

  void loadScene(SceneName name, int seed = 0);

  void allocOutputImage(int width, int height);

  void clearImage();

  void advanceAnimation();

  void render();

  void shadePixel(int circleIndex, float pixelCenterX, float pixelCenterY,
                  float px, float py, float pz, float* pixelData);
};

#endif