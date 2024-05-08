// Copyright (c) 2024 Meerkat
#include "../inc/Engine.h"
#include "../inc/Instance.h"
#include "../inc/Logging.h"
#include "../inc/Device.h"
#include "../inc/Swapchain.h"
#include "../inc/Pipeline.h"
#include "../inc/Framebuffer.h"
#include "../inc/Commands.h"
#include "../inc/Sync.h"
#include "../inc/Scene.h"
#include "../inc/Descriptors.h"
#include "../inc/ObjMesh.h"

void Engine::init(
  uint32_t width, uint32_t height, GLFWwindow* window, bool debugMode) {
  mWidth    = width;
  mHeight   = height;
  mWindow   = window;
  mHasDebug = debugMode;

  if (mHasDebug) {
    printf("Initializing app...\n");
  }

  make_instance();
  make_device();
  make_descriptor_set_layouts();
  make_pipeline();
  finalize_setup();
  make_assets();

  if (mHasDebug) {
    printf("Initializing app done.\n");
  }
}

void Engine::destroy() {
  mDevice.waitIdle();

  if (mHasDebug) {
    printf("Destroying app...\n");
  }

  mDevice.destroyCommandPool(mCommandPool);

  mDevice.destroyPipeline(mGraphicsPipeline);
  mDevice.destroyPipelineLayout(mPipelineLayout);
  mDevice.destroyRenderPass(mRenderPass);

  cleanup_swapchain();

  mDevice.destroyDescriptorSetLayout(mFrameSetLayout);

  delete mMeshes;

  for (const auto& [_, texture] : mMaterials) {
    delete texture;
  }

  mDevice.destroyDescriptorSetLayout(mMeshSetLayout);
  mDevice.destroyDescriptorPool(mMeshDescriptorPool);

  mDevice.destroy();
  mInstance.destroySurfaceKHR(mSurface);

  if (mHasDebug) {
    mInstance.destroyDebugUtilsMessengerEXT(mDebugMessenger, nullptr,
                                            mDldi);
  }

  mInstance.destroy();

  if (mHasDebug) {
    printf("Destroying done.\n");
  }
}

void Engine::make_instance() {
  mInstance = vkInit::make_instance(mHasDebug, "Engine");
  mDldi     = vk::DispatchLoaderDynamic(mInstance, vkGetInstanceProcAddr);
  if (mHasDebug) {
    mDebugMessenger = vkInit::make_debug_messenger(mInstance, mDldi);
  }

  VkSurfaceKHR surface;
  VkResult createSurfaceResult =
    glfwCreateWindowSurface(mInstance, mWindow, nullptr, &surface);
  if (createSurfaceResult != VK_SUCCESS) {
    if (mHasDebug) {
      printf("Failed to create the glfw surface for Vulkan.\n");
    }
  } else {
    if (mHasDebug) {
      printf("Successfully creasted glfw surface for Vulkan.\n");
    }
  }
  mSurface = surface;
}

void Engine::make_device() {
  mPhysicalDevice = vkInit::choose_physical_device(mInstance, mHasDebug);
  mDevice         =
    vkInit::create_logical_device(mPhysicalDevice, mSurface, mHasDebug);
  std::array<vk::Queue, 2> queues =
    vkInit::get_queue(mPhysicalDevice, mDevice, mSurface, mHasDebug);
  mGraphicsQueue = queues[0];
  mPresentQueue  = queues[1];

  make_swapchain();

  mFrameNumber = 0;
}

void Engine::make_swapchain() {
  vkInit::SwapChainBundle bundle = vkInit::create_swapchain(
    mPhysicalDevice, mDevice, mSurface, mWidth, mHeight, mHasDebug);
  mSwapchain       = bundle.swapchain;
  mSwapchainFrames = bundle.frames;
  mSwapchainFormat = bundle.format;
  mSwapchainExtent = bundle.extent;

  mMaxFramesInFlight = static_cast<uint32_t>(mSwapchainFrames.size());

  for (vkUtil::SwapChainFrame& frame : mSwapchainFrames) {
    frame.mDevice = mDevice;
    frame.mPhysicalDevice = mPhysicalDevice;
    frame.mWidth = mSwapchainExtent.width;
    frame.mHeight = mSwapchainExtent.height;

    frame.make_depth_resources();
  }
}

void Engine::recreate_swapchain() {
  int32_t w = 0;
  int32_t h = 0;
  while (w == 0 || h == 0) {
    glfwGetFramebufferSize(mWindow, &w, &h);
    glfwWaitEvents();
  }
  mWidth = w;
  mHeight = h;

  mDevice.waitIdle();

  cleanup_swapchain();
  make_swapchain();
  make_framebuffers();
  make_frame_resources();

  vkInit::CommandBufferInputChunk commandBufferInput{};
  commandBufferInput.device      = mDevice;
  commandBufferInput.frames      = &mSwapchainFrames;
  commandBufferInput.commandPool = mCommandPool;

  vkInit::make_frame_command_buffers(&commandBufferInput, mHasDebug);
}

void Engine::make_descriptor_set_layouts() {
  {
    vkInit::DescriptorSetLayoutData frameBindings;
    frameBindings.count = 2;
    frameBindings.indices.push_back(0);
    frameBindings.types.push_back(vk::DescriptorType::eUniformBuffer);
    frameBindings.counts.push_back(1);
    frameBindings.stages.push_back(vk::ShaderStageFlagBits::eVertex);

    frameBindings.indices.push_back(1);
    frameBindings.types.push_back(vk::DescriptorType::eStorageBuffer);
    frameBindings.counts.push_back(1);
    frameBindings.stages.push_back(vk::ShaderStageFlagBits::eVertex);

    mFrameSetLayout = vkInit::make_descriptor_set_layout(
      mDevice, frameBindings, mHasDebug);
  }

  {
    vkInit::DescriptorSetLayoutData meshBindings;
    meshBindings.count = 1;
    meshBindings.indices.push_back(0);
    meshBindings.types.push_back(vk::DescriptorType::eCombinedImageSampler);
    meshBindings.counts.push_back(1);
    meshBindings.stages.push_back(vk::ShaderStageFlagBits::eFragment);

    mMeshSetLayout = vkInit::make_descriptor_set_layout(
      mDevice, meshBindings, mHasDebug);
  }
}

void Engine::make_pipeline() {
  vkInit::GraphicsPipelineInBundle specification {};
  specification.device               = mDevice;
  specification.vertexFilepath       = "./bin/shaders/default.vert.spv";
  specification.fragmenFilepath      = "./bin/shaders/default.frag.spv";
  specification.extent               = mSwapchainExtent;
  specification.swapchainFormat      = mSwapchainFormat;
  specification.depthFormat          = mSwapchainFrames[0].mDepthFormat;
  specification.descriptorSetLayouts = { mFrameSetLayout, mMeshSetLayout };

  vkInit::GraphicsPipelineOutBundle output =
    vkInit::make_graphics_pipeline(specification, mHasDebug);

  mPipelineLayout   = output.pipelineLayout;
  mRenderPass       = output.renderPass;
  mGraphicsPipeline = output.graphicsPipeline;
}

void Engine::finalize_setup() {
  make_framebuffers();

  mCommandPool =
    vkInit::make_command_pool(mDevice, mPhysicalDevice, mSurface, mHasDebug);

  vkInit::CommandBufferInputChunk commandBufferInput{};
  commandBufferInput.device      = mDevice;
  commandBufferInput.frames      = &mSwapchainFrames;
  commandBufferInput.commandPool = mCommandPool;

  mMainCommandBuffer =
    vkInit::make_command_buffer(&commandBufferInput, mHasDebug);
  vkInit::make_frame_command_buffers(&commandBufferInput, mHasDebug);

  make_frame_resources();
}

void Engine::make_assets() {
  mMeshes = new VertexMenagerie();

  std::unordered_map<vkMesh::MeshTypes, std::vector<const char*>>
  model_filenames = {
    { vkMesh::MeshTypes::GROUND,
      { "./res/obj/ground.obj", "./res/obj/ground.mtl" } },
    { vkMesh::MeshTypes::GIRL,
      { "./res/obj/girl.obj", "./res/obj/girl.mtl" } },
    { vkMesh::MeshTypes::SKULL,
      { "./res/obj/skull.obj", "./res/obj/skull.mtl" } },
  };

  for (const auto& [key, value] : model_filenames) {
    vkMesh::ObjMesh obj(value[0], value[1], glm::mat4(1.0f));
    mMeshes->consume(key, obj.vertices, obj.indices);
  }

  VertexMenagerie::FinalizationChunk finalizationChunk {};
  finalizationChunk.physicalDevice = mPhysicalDevice;
  finalizationChunk.device         = mDevice;
  finalizationChunk.queue          = mGraphicsQueue;
  finalizationChunk.commandBuffer  = mMainCommandBuffer;

  mMeshes->finalize(finalizationChunk);

  // Materials
  std::unordered_map<vkMesh::MeshTypes, const char*> filenames = {
    std::make_pair(vkMesh::MeshTypes::GROUND, "./res/tex/ground.jpg"),
    std::make_pair(vkMesh::MeshTypes::GIRL, "./res/tex/none.png"),
    std::make_pair(vkMesh::MeshTypes::SKULL, "./res/tex/skull.png") };

  vkInit::DescriptorSetLayoutData bindings;
  bindings.count = 1;
  bindings.types.push_back(vk::DescriptorType::eCombinedImageSampler);
  mMeshDescriptorPool = vkInit::make_descriptor_pool(
    mDevice, static_cast<uint32_t>(filenames.size()), bindings, mHasDebug);

  vkImage::TextureInputChunk textureInfo {};
  textureInfo.commandBuffer  = mMainCommandBuffer;
  textureInfo.queue          = mGraphicsQueue;
  textureInfo.device         = mDevice;
  textureInfo.physicalDevice = mPhysicalDevice;
  textureInfo.layout         = mMeshSetLayout;
  textureInfo.descriptorPool = mMeshDescriptorPool;

  for (const auto& [type, filename] : filenames) {
    textureInfo.filename = filename;
    mMaterials[type] = new vkImage::Texture{};
    mMaterials[type]->init(textureInfo);
  }
}

void Engine::prepare_scene(vk::CommandBuffer commandBuffer) {
  vk::Buffer vertexBuffers[] = {
    mMeshes->getVertexBuffer().buffer };

  vk::DeviceSize offsets[] = { 0 };
  commandBuffer.bindVertexBuffers(0, 1, vertexBuffers, offsets);
  commandBuffer.bindIndexBuffer(mMeshes->getIndexBuffer().buffer,
                                0, vk::IndexType::eUint32);
}

void Engine::prepare_frame(uint32_t frameIndex, Scene* scene) {
  glm::vec3 eye    = { -1.0f,  0.0f,  1.0f };
  glm::vec3 center = {  1.0f,  0.0f,  1.0f };
  glm::vec3 up     = {  0.0f,  0.0f,  1.0f };

  glm::mat4 view = glm::lookAt(eye, center, up);

  float povAngle = glm::radians(45.0f);
  float aspectRatio =
    static_cast<float>(mSwapchainExtent.width) /
    static_cast<float>(mSwapchainExtent.height);
  float near = 0.1f;
  float far = 100.0f;
  glm::mat4 proj = glm::perspective(povAngle, aspectRatio, near, far);
  proj[1][1] *= -1;

  vkUtil::SwapChainFrame& frame = mSwapchainFrames[frameIndex];
  frame.mCameraData.view = view;
  frame.mCameraData.projection = proj;
  frame.mCameraData.viewProjection = proj * view;

  memcpy(frame.mCameraDataWriteLocation,
         &frame.mCameraData,
         sizeof(vkUtil::UniformBufferObject));

  size_t i = 0;
  for (const auto& [key, value] : scene->positions) {
    for (const glm::vec3& position : value) {
      frame.mModelTransforms[i] = glm::translate(glm::mat4(1.0f), position);
      ++i;
    }
  }
  memcpy(frame.mModelBufferWriteLocation,
         frame.mModelTransforms.data(),
         i * sizeof(glm::mat4));

  frame.write_descriptor_set();
}

void Engine::make_framebuffers() {
  vkInit::FramebufferInput framebufferInput{};
  framebufferInput.device          = mDevice;
  framebufferInput.renderPass      = mRenderPass;
  framebufferInput.swapchainExtent = mSwapchainExtent;

  vkInit::make_framebuffers(framebufferInput, &mSwapchainFrames, mHasDebug);
}

void Engine::make_frame_resources() {
  vkInit::DescriptorSetLayoutData bindings;
  bindings.count = 1;
  bindings.types.push_back(vk::DescriptorType::eUniformBuffer);
  mFrameDescriptorPool = vkInit::make_descriptor_pool(
    mDevice,
    static_cast<uint32_t>(mSwapchainFrames.size()),
    bindings,
    mHasDebug);

  for (vkUtil::SwapChainFrame& f : mSwapchainFrames) {
    vkUtil::SwapChainFrame::SyncObjects so {};
    f.mInFlight = vkInit::make_fence(mDevice, mHasDebug);
    f.mImageAvailable = vkInit::make_semaphore(mDevice, mHasDebug);
    f.mRenderFinished = vkInit::make_semaphore(mDevice, mHasDebug);

    f.make_descriptor_resources();

    f.mDescriptorSet = vkInit::allocate_descriptor_set(
       mDevice, mFrameDescriptorPool, mFrameSetLayout, mHasDebug);
  }
}

void Engine::render_objects(vk::CommandBuffer commandBuffer,
                           vkMesh::MeshTypes objType,
                           uint32_t* startInstance, uint32_t instanceCount) {
  uint32_t firstIndex = mMeshes->getOffset(objType);
  uint32_t indexCount = mMeshes->getSize(objType);
  mMaterials[objType]->use(commandBuffer, mPipelineLayout);
  commandBuffer.drawIndexed(
    indexCount, instanceCount, firstIndex, 0, *startInstance);
  *startInstance += instanceCount;
}

void Engine::record_draw_commands(vk::CommandBuffer commandBuffer,
                                  uint32_t imageIndex, Scene* scene) {
  vk::CommandBufferBeginInfo beginInfo {};
  try {
    commandBuffer.begin(beginInfo);
  } catch (vk::SystemError err) {
    if (mHasDebug) {
      printf("Failed to begin recording command buffer. Error: %s\n",
             err.what());
    }
  }

  vk::ClearValue clearColor  =
    { { 1.0f, 0.5f, 0.25f, 1.0f } };
  vk::ClearValue clearDepth {};
  clearDepth.depthStencil = vk::ClearDepthStencilValue(1.0f, 0);
  std::vector<vk::ClearValue> clearValues = { { clearColor, clearDepth } };

  vk::RenderPassBeginInfo renderPassInfo {};
  renderPassInfo.renderPass          = mRenderPass;
  renderPassInfo.framebuffer         =
    mSwapchainFrames[imageIndex].mFramebuffer;
  renderPassInfo.renderArea.offset.x = 0;
  renderPassInfo.renderArea.offset.y = 0;
  renderPassInfo.renderArea.extent = mSwapchainExtent;
  renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
  renderPassInfo.pClearValues = clearValues.data();

  commandBuffer.beginRenderPass(&renderPassInfo, vk::SubpassContents::eInline);

  commandBuffer.bindDescriptorSets(
    vk::PipelineBindPoint::eGraphics,
    mPipelineLayout, 0,
    mSwapchainFrames[imageIndex].mDescriptorSet,
    nullptr);

  commandBuffer.bindPipeline(
    vk::PipelineBindPoint::eGraphics, mGraphicsPipeline);

  prepare_scene(commandBuffer);

  {
    uint32_t startInstance = 0;
    for (const auto& [key, val] : scene->positions) {
      render_objects(
        commandBuffer,
        key,
        &startInstance,
        static_cast<uint32_t>(val.size())
      );
    }
  }

  commandBuffer.endRenderPass();

  try {
    commandBuffer.end();
  } catch (vk::SystemError err) {
    if (mHasDebug) {
      printf("Failed to finish recording command buffer. Error: %s\n",
             err.what());
    }
  }
}

void Engine::render(Scene* scene) {
  vk::Fence inFlight = mSwapchainFrames[mFrameNumber].mInFlight;
  mDevice.waitForFences(
    1, &inFlight, VK_TRUE, UINT64_MAX);

  uint32_t imageIndex;
  try {
    vk::ResultValue acquire =
      mDevice.acquireNextImageKHR(
        mSwapchain, UINT64_MAX,
        mSwapchainFrames[mFrameNumber].mImageAvailable, nullptr);
    imageIndex = acquire.value;
  } catch (vk::OutOfDateKHRError) {
    recreate_swapchain();
    return;
  }

  vk::CommandBuffer commandBuffer =
    mSwapchainFrames[mFrameNumber].mCommandBuffer;
  commandBuffer.reset();

  prepare_frame(imageIndex, scene);

  record_draw_commands(commandBuffer, imageIndex, scene);

  vk::Semaphore waitSemaphores[] =
    { mSwapchainFrames[mFrameNumber].mImageAvailable };
  vk::Semaphore signalSemaphores[] =
    { mSwapchainFrames[mFrameNumber].mRenderFinished };
  vk::PipelineStageFlags waitStages[] =
    { vk::PipelineStageFlagBits::eColorAttachmentOutput };
  vk::SubmitInfo submitInfo {};
  submitInfo.waitSemaphoreCount   = 1;
  submitInfo.pWaitSemaphores      = waitSemaphores;
  submitInfo.pWaitDstStageMask    = waitStages;
  submitInfo.commandBufferCount   = 1;
  submitInfo.pCommandBuffers      = &commandBuffer;
  submitInfo.signalSemaphoreCount = 1;
  submitInfo.pSignalSemaphores    = signalSemaphores;

  mDevice.resetFences(1, &inFlight);

  try {
    mGraphicsQueue.submit(
      submitInfo, inFlight);
  } catch (vk::SystemError err) {
    if (mHasDebug) {
      printf("Failed to submit draw command buffer. Error: %s\n",
             err.what());
    }
  }

  vk::SwapchainKHR swapchains[] = { mSwapchain };
  vk::PresentInfoKHR presentInfo {};
  presentInfo.waitSemaphoreCount = 1;
  presentInfo.pWaitSemaphores    = signalSemaphores;
  presentInfo.swapchainCount     = 1;
  presentInfo.pSwapchains        = swapchains;
  presentInfo.pImageIndices      = &imageIndex;

  vk::Result present;
  try {
    present = mPresentQueue.presentKHR(presentInfo);
  } catch (vk::OutOfDateKHRError err) {
    present = vk::Result::eErrorOutOfDateKHR;
  }

  if (present == vk::Result::eErrorOutOfDateKHR
      || present == vk::Result::eSuboptimalKHR) {
    printf("Recreating...\n");
    recreate_swapchain();
    return;
  }

  mFrameNumber = (mFrameNumber + 1) % mMaxFramesInFlight;
}

void Engine::cleanup_swapchain() {
  for (vkUtil::SwapChainFrame& f : mSwapchainFrames) {
    f.destroy();
  }
  mDevice.destroySwapchainKHR(mSwapchain);

  mDevice.destroyDescriptorPool(mFrameDescriptorPool);
}
